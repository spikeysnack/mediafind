/* config.hpp */

/* EDIT THIS FILE FIRST FOR YOUR DIRECTORIES */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#ifndef BUILD_TEST
// set to true after editing
constexpr bool EDITED = false;
//constexpr bool EDITED = true;

constexpr std::string_view must_edit =
  "\n\nYOU MUST EDIT config.hpp "
  "FIRST FOR YOUR DIRECTORIES.\n"
  "Then set EDITED to true.\n\n";

static_assert(EDITED, must_edit);

#endif

// base dir for db_files
const string base_dir = getenv("HOME");
#define BASE(D)  (base_dir + "/" + D)  

// use your own dirs
// set a dir to store database files (default: user home dir)

#define VIDEO_DIR1 "/media/chris/xeon_movies1"
#define VIDEO_DB1  BASE(".mediadb.xeon_movies1")

#define VIDEO_DIR2  "/media/chris/xeon_movies2"
#define VIDEO_DB2   BASE(".mediadb.xeon_movies2")

#define VIDEO_DIR3  "/media/chris/xeon_movies3"
#define VIDEO_DB3   BASE(".mediadb.xeon_movies3")

#define VIDEO_DIR4  "/media/chris/TV"
#define VIDEO_DB4   BASE(".mediadb.tv")          

#define VIDEO_DIR5  "/media/chris/video_460G"
#define VIDEO_DB5   BASE(".mediadb.video_460G")

#define VIDEO_DIRS { VIDEO_DIR1, VIDEO_DIR2, VIDEO_DIR3, VIDEO_DIR4, VIDEO_DIR5 }
#define VIDEO_DBS  { VIDEO_DB1 , VIDEO_DB2 , VIDEO_DB3 , VIDEO_DB4 , VIDEO_DB5  }


#define AUDIO_DIR1 "/Audio"
#define AUDIO_DB1  BASE(".mediadb.Audio")

#define AUDIO_DIR2 "/Audio2"
#define AUDIO_DB2  BASE(".mediadb.Audio2")

#define AUDIO_DIR3 "/Audio3"
#define AUDIO_DB3  BASE(".mediadb.Audio3")

#define AUDIO_DIR4 "/Audio4"
#define AUDIO_DB4 BASE(".mediadb.Audio4")

#define AUDIO_DIRS { AUDIO_DIR1, AUDIO_DIR2, AUDIO_DIR3, AUDIO_DIR4 }
#define AUDIO_DBS  { AUDIO_DB1 , AUDIO_DB2 , AUDIO_DB3 , AUDIO_DB4  }


#include "mediafind.hpp"

/* this all happens befor main */

const auto video_dirs    = std::to_array<const string>( VIDEO_DIRS );
const auto video_dbfiles = std::to_array<const string>( VIDEO_DBS  );

const auto audio_dirs    = std::to_array<const string>( AUDIO_DIRS );  
const auto audio_dbfiles = std::to_array<const string>( AUDIO_DBS  ); 

// TODO
#ifdef USE_PICTURES
 const auto pic_dirs = std::to_array<const string>(
   {
     BASE("Pictures")
   } );

 const auto pic_dbfiles = std::to_array<const string>(
   {
     BASE(".mediadb.Pictures")
   } );

#endif



// lamdba makes correlation
auto make_map = [] (auto& a, auto& b) -> pathmap
 {
  pathmap pm;
  for (size_t i = 0; i < a.size(); ++i) {
    pm[a[i]] = b[i];
  }
  return pm;
};


auto && video_map = make_map(video_dirs, video_dbfiles);
auto && audio_map = make_map(audio_dirs, audio_dbfiles);
// auto && pic_map = make_map(pic_dirs,pic_dbfiles);

#endif

