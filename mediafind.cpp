/* mediafind.cpp */

#include "mediafind.hpp"
#include "user_config.hpp"

extern const int DEBUGGING_ENABLED;

/** file permissions default **/
// -rw-r----- (octal)
#define DB_PERMS 0640
const fs::perms perms = static_cast<fs::perms>(DB_PERMS);


/***  
included from mediafind.hpp  

const char APP_METADATA[];
static constexpr std::tuple version{1, 0, 3};
static constexpr const char* compile_date = __DATE__;

bool read_config_file( const string& config_file, pathmap& v, pathmap& a);
void print_db(const pathmap& pm, std::ostream& os);
void print_version();
void print_metadata ();

vector<string> get_PATH();
const fs::path which(const string& name);

const string lshift( vector<string>& vec);
string exec(const string& cmd);
***/



// if files aren't already there
// (creating could fail)
bool create_databases_if_needed(pathmap& pm) noexcept(false)  {

  bool OK = true;

  // structured binding and iteration
  for(auto& [ dirpath , dbname ] :  pm) {

    if (dbname.empty() ) continue;
    
    fs::path filepath = dbname;
    
    if ( ! fs::exists(filepath) ) {
      try {
	// create if not exists
	fs::path dirpath = filepath.parent_path();
    
	if (!dirpath.empty()) {
	  fs::create_directories(dirpath);
	}

	// create empty file
	ofstream f(filepath);
	f.close();
	// set permissions
	fs::permissions(filepath, perms, fs::perm_options::replace);
      }
      catch (const fs::filesystem_error& e) {
	WARN("Filesystem permissions error:\t{}\n", e.what() );
	OK = false;
	break;
      }
    } // if
  } // for
  
  return OK;
}


// does a full update of each database file
// one at a time
bool update_database ( const string& dirpath, const string& dbfile) {

  const string updatedb_bin = which("updatedb");

  ostringstream  sb;

  // build a command
  sb << updatedb_bin << " "
     << "-l no "
     << "-o " << dbfile << " "
     << "--prunepaths \"\" "
     << "-U " << dirpath <<  " 2>&1";

  string cmd = sb.str();

  auto results = exec(cmd);

  if (! results.empty() ) {
    istringstream stream(results);
    string line;
    // Read line by line
    while (getline(stream, line)) {
      WARN("{}",  line);
    }
  }
  return true;
}


// change database files back to user/group if not current user
bool chown_databases_if_needed( vector<string> files ) {

  const string chown_bin = which("chown");

  auto user = get_env_var("USER");

  if (!user) {
    FATAL(1, "Cannot Get USER env");
  }

  struct passwd*  pw = nullptr;
  pw = getpwnam(string(*user).c_str());
  if (pw == nullptr) {
    FATAL(1, "Cannot Get PW struct");
  }

  auto my_uid =  pw->pw_uid;
  auto my_gid =  pw->pw_gid;
  string uid_gid = to_string(my_uid) + ":" + to_string(my_gid);
  struct stat file_stat;

  size_t len = 0;
  for (const auto& s : files) { len += s.size() + 1; }

  string file_list{};
  file_list.reserve(len+1);
  ostringstream  sb;
  

  for( auto& f : files ) {

    if ( stat(f.c_str(), & file_stat) != 0) {
      FATAL(1, "Error getting file stats for {}" , f);
    }
    uid_t f_uid = file_stat.st_uid;
    gid_t f_gid = file_stat.st_gid;
    
    bool skip = ((f_uid == my_uid) && (f_gid == my_gid));

    // add to string builder
    if ( !skip) { sb << f << " "; }
  }

  file_list = sb.str();

  // only if some files had wrong user:group
  if ( ! file_list.empty() ) {

    sb.str("");
    sb.clear();

    WARN("running chown via sudo on");
    WARN("{}", file_list);
    
    sb << chown_bin  << " "
       << uid_gid    << " "
       << file_list  << " 2>&1";
    
    string cmd = sb.str();

    // std::system launches the external process
    auto results = exec(cmd);

    // only if error occured
    if (! results.empty() ) {
      istringstream stream(results);
      string line;
      // Read line by line
      while (getline(stream, line)) {
	WARN("{}", line);
      }
    }
  }
  return true;
}



// add a database file to the user db dir
// and update it
// uses full dir as name
void add_dir(const string& dir, pathmap& pm) {

  fs::path p(dir);

  string d = p.string();
  
  //convert slash to period 
  std::replace(d.begin(), d.end(), '/', '.');
  
  const string db = base_dir + "/" + d;

  // add to pm
  pm.emplace( dir, db);

  if ( create_databases_if_needed(pm) ) {
    println(stdout, "{} created successfuly ... ", db );
    
  } else {
    FATAL(1, "creation of db file {} failed.", db );
  }

  println( " updating  {} " , db);
  
  if ( update_database(dir , db) ) {
    println(stdout, "{} updated successfuly", db );
  }  else {
    FATAL(1, "update of db file {} failed.", db );
  }

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
        --version  show program version
        --add-dir  <dir>  search additional dir
    )";

  println(stdout, "{}", helpstr);
}

// version
void print_version() {
  auto [ major, minor, rev ] = version;
  println(stdout, "mediafind v{}.{}.{}\t{}", major, minor, rev, compile_date);
}


// jthread runs this
void launch_instance(int id, const string& executable, const string& term, string& dbfile, const bool& dirs_only) {

  DEBUG("THREAD ID: {}" , id);

  ostringstream  sb;

  sb << executable
     << " --database " << dbfile
     << " --ignore-case " << "\"" << term << "\""
     << " 2>&1";

  string cmd = sb.str();

  // std::system launches the external process
  auto results = exec(cmd);

  if (! results.empty() ) {
    
    istringstream stream{results};
    string line;

    // Read line by line
    while (getline(stream, line)) {

      if (dirs_only)
	fs::is_directory(line) && (println(stdout, "{}", line), true );
      else
	println(stdout, "{}", line); 
            
    } // while getline
  } // if results  
} // launch_instance

// settable options
enum class Option {
  AUDIO_ONLY, VIDEO_ONLY, DIRS_ONLY, PICS_ONLY,
  UPDATE_DB, ADD_DIR, HELP, METADATA, VERSION, UNKNOWN
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
      { "-v" , Option::VIDEO_ONLY },
      
      { "--version" , Option::VERSION },
      { "--add-dir" , Option::ADD_DIR }
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
  vector<string> additional_dirs{};
  
  
  string term;

  if (args.size() == 0) {
    FATAL(1, "No term given to search for");
  }

  // user's home dir
  string home = *get_env_var("HOME");

  if (! read_config_file(config_file, video_map, audio_map) ) {
    FATAL(2, "could not find  or could not read {}", config_file);
  }

  // path of executables
  const string sudo_bin     = which("sudo");
  const string updatedb_bin = which("updatedb");
  const string locate_bin   = which("locate");
  // const string realpath_bin = which("realpath");

  // option state vars (1-byte)
  struct option_vars
  {
    bool help_only     : 1;
    bool dirs_only     : 1;
    bool audio_only    : 1;
    bool video_only    : 1;

    bool update_db     : 1; 
    bool add_dirs      : 1;
    bool show_metadata : 1; 
    bool version       : 1;
  };
  
 option_vars ov{};

  //  set option state vars
  for ( auto it = args.begin() ; it != args.end() ; ++it) {
    
    if ( (*it)[0] == '-' ) {

      switch ( get_option(*it) ) {

      case Option::HELP:       { ov.help_only = true; break;}

      case Option::DIRS_ONLY:  { ov.dirs_only = true; break; }

      case Option::AUDIO_ONLY: { ov.audio_only = true; break; }

      case Option::VIDEO_ONLY: { ov.video_only = true; break; }

      case Option::UPDATE_DB:  { ov.update_db = true; break; }


      case Option::ADD_DIR:    {
	it++;
	auto& a_dir = *it;
	additional_dirs.push_back(a_dir);
	ov.add_dirs = true;
	break;
      }

      case Option::METADATA: { ov.show_metadata = true; break; }
	
      case Option::VERSION:  { ov.version = true; break;}
      	
      case Option::UNKNOWN:  {} [[fallthrough]];
	
      default:               {
	println(stderr, " Unknown option: {}", *it);
	help();
	FATAL(2, "use only above options.");
      }

      } // switch
      
    } // if
    else {
      // search term
      term = *it;
    }
  } // for

  // print program ver
  if(ov.version) { print_version(); exit(0);}
    
  // outputs metatdata in json
  if(ov.show_metadata) { print_metadata() ; exit(0);}

  // show help
  if (ov.help_only)    { help(); exit(0);}

  // chose both as "only" stupid
  if (ov.audio_only && ov.video_only) {
    FATAL( 2, "can't have video only and audio only. (default: both)");
  }
  
  if (! create_databases_if_needed(video_map) ) {
    FATAL(1,"ERROR some video databases could not be created");
  }
  if (! create_databases_if_needed(audio_map) ) {
    FATAL( 1, "ERROR some audio databases could not be created");;
  }

  // --add-dir 
  if (ov.add_dirs) {
    for (const auto& d : additional_dirs) { add_dir(d, extra_map); }
  }
  
  pathmap both_maps{}; 

  bool use_both = not(ov.audio_only || ov.video_only);

  //copy video and/or audio
  if (use_both) {
    both_maps = video_map; 
    both_maps.insert(audio_map.begin() , audio_map.end() );
  }
  
  pathmap&  db =  (ov.audio_only)? audio_map : (ov.video_only)? video_map : both_maps;
  
  // add additional dirs from cmdline
  if ( extra_map.size() > 0 ) { db.insert(extra_map.begin(), extra_map.end() ); }

  // print_db(db, std::cerr);

  // db files just created
  {
    for (auto & [dir, dbfile] : db )  {

      vector<string> zero_files;
    
      if ( ! fs::exists(dbfile) ) {
	WARN( "dbfile ",  dbfile, "does not exist\n");

	// create empty file
	ofstream f(dbfile);
	f.close();
      
	
	// set permissions
	fs::permissions(dbfile, perms, fs::perm_options::replace);      
      }
        
      if ( fs::is_empty(dbfile) ) {      
	WARN("{} is a zero length file", dbfile);
	update_database( dir, dbfile);
	zero_files.push_back(dbfile);
	chown_databases_if_needed(zero_files);
      }
    }
  }
  
  if (ov.update_db) {
    vector<string> files;

    for ( auto& [dir, file] : db ) {
      println(stdout, "updating dir {} ==> {}", dir, file);      
      update_database( dir, file);
      files.push_back(file);
    }
    // change to user:group    
    chown_databases_if_needed(files);
    // stop here
    exit(0);
  }

  if ( ! term.empty() ) {

    // thread pool
    vector<jthread> pool;

    // Launch multiple instances concurrently
    size_t num_dbs = db.size();

    auto it = db.begin();

    for (size_t i = 0; i < num_dbs; ++i) {

      // dir, database file
      auto& [key, dbfile]  = *it++;

      // add to vector, and start (join thread)

      const auto dirsonly = ov.dirs_only;
      pool.emplace_back(launch_instance, i,
			locate_bin, term,
			std::ref(dbfile), dirsonly );
    }
  }
  
  // std::jthread destructor automatically joins the threads
  return 0;
}

/* END */
