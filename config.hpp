/* config.hpp */

/* EDIT THIS FILE FIRST FOR YOUR DIRECTORIES */

#ifndef CONFIG_HPP
#define CONFIG_HPP

// set to true after editing
constexpr bool EDITED = false;
// constexpr bool EDITED = true;

constexpr std::string_view must_edit =
  "\n\nYOU MUST EDIT config.hpp "
  "FIRST FOR YOUR DIRECTORIES.\n"
  "Then set EDITED to true.\n\n";

static_assert(EDITED, must_edit);

#include "mediafind.hpp"

/* this all happens befor main */

// set a dir to store database files (default: user home dir)
const string base_dir = getenv("HOME");

const auto video_dirs = std::to_array<const string>(
 {
  "/media/chris/xeon_movies1",
  "/media/chris/xeon_movies2",
  "/media/chris/xeon_movies3",
  "/media/chris/TV",
  "/media/chris/video_460G"
  } );
  
const auto video_dbfiles = std::to_array<const string>( 
  { base_dir + "/" + ".mediadb.xeon_movies1",
    base_dir + "/" + ".mediadb.xeon_movies2",
    base_dir + "/" + ".mediadb.xeon_movies3",
    base_dir + "/" + ".mediadb.tv"          ,
    base_dir + "/" + ".mediadb.video_460G"
  } );

const auto audio_dirs = std::to_array<const string>(
  {
    "/Audio",
    "/Audio2",
    "/Audio3",
    "/Audio4",
   } );
  
const auto audio_dbfiles = std::to_array<const string>( 
  { base_dir + "/" + ".mediadb.Audio",
    base_dir + "/" + ".mediadb.Audio2",
    base_dir + "/" + ".mediadb.Audio3",
    base_dir + "/" + ".mediadb.Audio4"
  } );


#ifdef USE_PICTURES
 const auto pic_dirs = std::to_array<const string>(
   {
     base_dir + "/" + "Pictures";
   } );

 const auto pic_dbfiles = std::to_array<const string>(
   {
     base_dir + "/" + ".mediadb.Pictures"
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

