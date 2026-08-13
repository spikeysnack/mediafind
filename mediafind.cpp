/* mediafind.cpp */

#include "mediafind.hpp"
#include "config.hpp"

extern const int DEBUG;

// -rw-r----- (octal)
#define DB_PERMS 0640


// if files aren't already there
// (creating could fail)
bool create_databases_if_needed( pathmap& pm ) noexcept(false)  {

  bool OK = true;
  // octal
  const fs::perms perms = static_cast<fs::perms>(DB_PERMS);

  // structured binding and iteration
  for(auto& [ dirpath , dbname ] :  pm) {

    fs::path filepath = dbname;

    if ( ! fs::exists(filepath) ) {
      try {
	// create empty file
	std::ofstream f(filepath);
	f.close();
	// set permissions
	fs::permissions(filepath, perms, fs::perm_options::replace);
      }
      catch (const fs::filesystem_error& e) {
	println(stderr, "Filesystem permissions error:\t{}\n", e.what() );
	OK = false;
	break;
      }

    }
  }
  return OK;
}


// does a full update of each database file
// one at a time
bool update_database ( const string& dirpath, const string& dbfile) {

  const string updatedb_bin = which("updatedb");

  std::ostringstream  sb;

  // build a command
  sb << updatedb_bin << " "
     << "-l no "
     << "-o " << dbfile << " "
     << "--prunepaths \"\" "
     << "-U " << dirpath <<  " 2>&1";

  string cmd = sb.str();

  auto results = exec(cmd);

  if (! results.empty() ) {
    std::istringstream stream(results);
    std::string line;
    // Read line by line
    while (std::getline(stream, line)) {
      std::cout << line << '\n';
    }
  }
  return true;
}


// change database files back to user/group
bool chown_databases( vector<string> files ) {

  const string sudo_bin     = which("sudo");
  const string chown_bin    = which("chown");

  auto user = get_env_var("USER");

  if (!user) {
    fatal<string_view>(string_view("Cannot Get USER env"), 1);
  }

  struct passwd*  pw = nullptr;

  pw = getpwnam(string(*user).c_str());

  if (pw == nullptr) {
    fatal<string_view>(string_view("Cannot Get PW struct"), 1);
  }

  auto my_uid =  pw->pw_uid;
  auto my_gid =  pw->pw_gid;
  string uid_gid = to_string(my_uid) + ":" + to_string(my_gid);

  size_t len = 0;
  for (const auto& s : files) { len += s.size() + 1; }

  std::string file_list;
  file_list.reserve(len+1);
  std::ostringstream  sb;

  for( auto& f : files ) {
    sb << f << " ";
  }

  file_list = sb.str();

  sb.str("");
  sb.clear();

  // effective uid is root no pwd needed
  if (geteuid() == 0) {
    println(stderr, "running chown as root ...");

    sb  << chown_bin  << " "
	<< uid_gid  << " "
	<< file_list << " 2>&1";
  }
  else {  // sudo user needs user passwd
    println(stderr, "running chown via sudo ...");
    sb << sudo_bin << " -S "
       << chown_bin  << " "
       << uid_gid  << " "
       << file_list << " 2>&1";
  }

  string cmd = sb.str();

  // std::system launches the external process
  auto results = exec(cmd);

  if (! results.empty() ) {
    std::istringstream stream(results);
    std::string line;
    // Read line by line
    while (std::getline(stream, line)) {
      std::cout << line << '\n';
    }
  }

  return true;
}


// help msg
void help() {

  const string helpstr = R"(
	mediafind  <-option> <search term>>
	-h help
	-d output dirs only with matching files
	-m show metadata
	-u update databases
	-a audio databases only
	-v video databases only
    )";


  println(stdout, "{}", helpstr);
}



// jthread runs this
void launch_instance(int id, const string& exe, const string& term, string& dbfile, bool dirs_only) {

  debug("THREAD ID: " , id);

  std::ostringstream  sb;

  sb << exe
     << " --database " << dbfile
     << " --ignore-case " << "\"" << term << "\""
     << " 2>&1";

  string cmd = sb.str();

  // std::system launches the external process
  auto results = exec(cmd);

  if (! results.empty() ) {
    std::istringstream stream(results);
    std::string line;

    // Read line by line
    while (std::getline(stream, line)) {

      if ( dirs_only ) {
	if (fs::is_directory(line) )  { std::cout << line << '\n'; }
      }
      else {
	println( stdout, "{}", line);
      }
    }

  } // if
}


// settable options
enum class Option {
  AUDIO_ONLY, VIDEO_ONLY, DIRS_ONLY,
  UPDATE_DB, HELP, METADATA, UNKNOWN
};

// map allows "switch on string" via hashtable hack
Option  get_option(const string& opt) {

  Option ret = Option::UNKNOWN;

  unordered_map<string,Option>  options =
    {
      { "-h" , Option::HELP       },
      { "-d" , Option::DIRS_ONLY  },
      { "-m" , Option::METADATA   },
      { "-u" , Option::UPDATE_DB  },
      { "-a" , Option::AUDIO_ONLY },
      { "-v" , Option::VIDEO_ONLY }
    };

  auto it = options.find(opt);

  if (it != options.end()) { ret = it->second; }

  return ret;
}


// do it
int main(int argc, char* argv[], [[maybe_unused]] char* env[]) {

  const vector<string> args{argv+1, argv+argc};
  vector<string> dirs{};
  vector<string> files{};

  string term;
  if (args.size() == 0) {
    println(stderr, "No term given to search for");
    exit(1);
  }

  // user's home dir
  string home = *get_env_var("HOME");

  // path of executables
  const string sudo_bin     = which("sudo");
  const string updatedb_bin = which("updatedb");
  const string realpath_bin = which("realpath");
  const string locate_bin   = which("locate");

  // option state vars
  bool help_only {false};
  bool dirs_only {false};
  bool update_db {false};
  bool show_metadata {false};
  bool audio_only{false};
  bool video_only{false};

  //  set option state vars
  for ( auto& arg : args ) {

    if ( arg[0] == '-' ) {

      switch ( get_option(arg) ) {

      case Option::HELP: { help_only = true; break;}

      case Option::DIRS_ONLY: { dirs_only = true; break; }

      case Option::METADATA: { show_metadata = true; break; }

      case Option::UPDATE_DB: { update_db = true; break; }

      case Option::AUDIO_ONLY: { audio_only = true; break; }

      case Option::VIDEO_ONLY: { video_only = true; break; }

      case Option::UNKNOWN: {} [[fallthrough]];
      default: {
	println(stderr, " Unknown option: {}", arg);
	help();
	exit(2);
      }

      } // switch
    } // if
    else {
      // search term
      term = arg;
    }
  } // for

  
  if (! create_databases_if_needed(video_map) ) {
    println(stderr, "ERROR some video databases could not be created");
    exit(1);
  }
  if (! create_databases_if_needed(audio_map) ) {
    println(stderr, "ERROR some audio databases could not be created");;
    exit(1);
  }

  // 3rd map is both
  pathmap& both = audio_map;
  
  // have to copy
  both.insert(video_map.begin(), video_map.end());

  //  merger destroys video (not wanted)
  //  both.merge(video);
  pathmap&  db =  (audio_only)? audio_map : (video_only)? video_map : both;

  // outputs metatdata in json
  if(show_metadata) { print_metadata() ; exit(0);}

  // show help
  if (help_only)    { help(); exit(0);}

  // chose both as "only" stupid
  if (audio_only && video_only) {
    println(stderr, "can't have video only and audio only. (default: both)");
    exit(2);
  }


  // execute db updates
  if (update_db) {
    vector<string> files;

    for ( auto& [dir, file] : db ) {
      update_database( dir, file);
      files.push_back(file);
    }

    // change to user:group
    chown_databases(files);
    exit(0);
  }


  // thread pool
  std::vector<std::jthread> pool;


  // Launch multiple instances concurrently
  size_t num_dbs = db.size();

  auto it = db.begin();

  for (size_t i = 0; i < num_dbs; ++i) {

    // dir, database file
    auto& [key, dbfile]  = *it++;

    // add to vector, and start (join thread)

    pool.emplace_back(launch_instance, i,
		      locate_bin, term,
		      std::ref(dbfile), dirs_only );
  }

  // std::jthread destructor automatically joins the threads
  return 0;
}

/* END */
