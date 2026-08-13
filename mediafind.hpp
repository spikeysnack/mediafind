#ifndef _MEDIAFIND_HPP
#define _MEDIAFIND_HPP
// STL headers
#include <iostream>
#include <memory>
#include <unordered_map>
#include <array>
#include <vector>
#include <string>
#include <string_view>
#include <sstream>
#include <thread>
#include <cstdlib>
#include <cstring>
#include <optional>
#include <filesystem>
#include <fstream>
#include <format>

// some distros don't have it
#ifdef __cpp_lib_print
    #include <print>
    #define PRINTLN 1
#else
    #define PRINTLN 0
#endif


// std c headers
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>

#if __cplusplus < 202302L
    #error "This project requires C++23 or greater!"
#endif

// use objcopy --dump-section .app_metadata=app_metadata.json mediafind
// to get this in json format
// or run ./mediafind -m > file
//        ./mediadinf -m | jq

__attribute__((section(".app_metadata")))
const char APP_METADATA[] =
  "{\"name\": \"mediafind\","
  "\"modified\": \"13 Aug 2026\", "
  "\"author\": \"Chris Reid\", "
  "\"email\": \"spikeysnack@gmail.com\", "
  "\"country\": \"United States of America\", "
  "\"file\": \"mediafind.cpp\", "
  "\"category\": \"video processing\", "
  "\"purpose\": \"fast locate media by keyword\","
  "\"repo\": \"\", "
  "\"license\": \"Public Domain\", "
  "\"license_url\": \"www.unlicense.org\","
  "\"version\" : \"1.2\", "
  "\"status\" : \"working\"}";


// encapsulate all needed objects in a namespace
namespace mediafind {
  using std::array;
  using std::vector;
  using std::unordered_map;
  using std::string;
  using std::string_view;
  using std::to_string;
  #if PRINTLN
  using std::println;
  #endif
  using std::format;
  using std::jthread;
  using std::optional;
  using std::cout;
  using std::cerr;

  // type aliases
  namespace fs = std::filesystem;
  using pathmap = std::unordered_map<string, string>;
  using spair = std::pair<string,string>;

};


using namespace mediafind;

const int DEBUG = 0;

#ifdef DEBUG_BUILD
   DEBUG = 1;
#endif



/***   UTILS  ***/
/*
template <typename... Args>
void perr(Args... args) {
  ((std::cerr << args << " "), ...); 
  std::cerr << '\n';
}
*/


// in case we don't have println
template <typename... Args>
void perr(std::format_string<Args...> fmt, Args&&... args) {
  #if PRINTLN
      std::println(stderr,fmt, std::forward<Args>(args)...);
  #else
      ((std::cerr << std::format(fmt, args) ), ...); 
      std::cerr << '\n';
  #endif
}

template <typename... Args>
void pout(std::format_string<Args...> fmt, Args&&... args) {
  #if PRINTLN
      std::println(stdout,fmt, std::forward<Args>(args)...);
  #else
      ((std::cout << std::format(fmt, args) ), ...); 
      std::cout << '\n';
  #endif
}



// output json string
void print_metadata () {

  string md {APP_METADATA};
  const string from  = "\\\n";
  const string to  = "\n";
  size_t pos = 0;
  size_t end = std::string::npos;

  while ((pos = md.find(from, pos)) != end) {
    md.replace(pos, from.length(), to);
      pos += to.length();
    }

  std::cout << md << "\n";
}

// variadic templates
template<typename... Args>
void debug(Args&&... args) {
  if (DEBUG) {
  ((cerr <<  "DEBUG "  << std::forward<Args>(args) << "\n"), ...);
    cerr << '\n';
  }
}

template<typename... Args>
void warn(Args&&... args) {
  ((cerr <<  "WARNING "  << std::forward<Args>(args) << "\n"), ...);
    cerr << '\n';
}

template<typename... Args, typename ERR=int>
void fatal(Args&&... args, ERR code) {
  ((cerr <<  "FATAL "  << std::forward<Args>(args) << "\n"), ...);
    cerr << '\n';
    exit(code);
}


// query env
std::optional<std::string> get_env_var(const std::string& key) {
  const char* value = std::getenv(key.c_str());
  if (value == nullptr) {
    return std::nullopt;
  }
  return string(value);
}


// get executable path
vector<string> get_PATH() {
  string_view delim = ":";
  vector<string> dirs;
  auto PATH = get_env_var("PATH");

  if (! PATH)
    return dirs;
  // parse into paths
  string path_str(*PATH);

  size_t start = 0;
  size_t end = path_str.find(delim);

  while (end != string::npos) {
    dirs.push_back(path_str.substr(start, end - start));
    start = end + 1;
    end = path_str.find(delim, start);
  }

  dirs.push_back(path_str.substr(start));

  return dirs;
}


// emulate 'which' command
const fs::path which(const string& name) {

  auto dirs = get_PATH();

  for (const auto& dir : dirs) {
    fs::path p = fs::path(dir) / name;
    // Check for file existence first
    if (fs::exists(p) && !fs::is_directory(p)) { return p;  }
  }

  return {}; // Not found
}


// run a string command using popen
// basically a legacy C function, so wrap it in a smart pointer
// the pipe created is dynamically allocated
// with pclose to free the pipe memory
string exec(const string& cmd) {

  array<char, 128> buffer;
  string result{};
  int popen_status = 0;
  int exit_status = 0;;
  debug( "exec: " , cmd);

// Define a custom deleter lambda capturing exit_status by reference

  auto deleter = [&popen_status](FILE* f) {
    if (f) {
      popen_status = pclose(f);
    }
  };

    errno = 0;
    std::unique_ptr<FILE, decltype(deleter)> pipe(popen(cmd.c_str(), "r"), deleter);

    //  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);

  // could fail
  if (!pipe) {
    warn("Pipe error in exec");
    throw std::runtime_error("popen() failed!");
  }

  // Read the output buffer line by line
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }
  
  int errnum = errno;

  // oops a fatal system-level signal was caught by the kernel 
  if (WIFSIGNALED(popen_status)) {
    fprintf(stderr, "piped  process killed by signal: %d\n", WTERMSIG(popen_status));
    string_view errstr = strerror(errnum);
    perr( "ERROR:\t[{}]\n", errstr);
    exit(1);
  } 
  
  // piped process exited with status
  
  if ( WIFEXITED(popen_status) ) {

    exit_status = WEXITSTATUS(popen_status);
  
    if ( (exit_status != 0)  | (errno != 0) ) {
      string_view errstr = strerror(errnum);
      perr("WARNING exec pipe returned {} instead of 0", exit_status);
      perr("ERROR:\t {}" , errstr);
      exit(1);
    }
  }
  
  return result;
}

void print_db(const pathmap& pm, std::ostream& os=std::cout) {

  for (auto it = pm.begin() ;it != pm.end() ; it++) {
    auto [dir, dbfile]  = *it;
    os  << "dir: " << dir << "  dbfile: "  << dbfile << "\n";
  }
  os.flush();
}



// the rest are in mediafind.cpp
bool create_databases_if_needed( pathmap& pm );
bool update_database ( const string& dirpath, const string& dbfile);
bool chown_databases( vector<string> files );
void launch_instance(int id, const string& exe, const string& term, string& dbfile, bool dirs_only);

enum class Option; // forward declare
Option  get_option(const string& opt);
void help();
int main(int argc, char* argv[], [[maybe_unused]] char* env[]);

#endif
