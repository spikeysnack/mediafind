/* config.hpp */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "mediafind.hpp"

/*  edit these dirs and filenames for your setup */

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


// const auto pic_dirs = std::to_array<const string>(
//   {
//   } );

// const auto pic_dbfiles = std::to_array<const string>(
//   {
//   } );


// lamdba makes correlation
auto make_map = [] (auto& a, auto& b) -> pathmap
 {
  pathmap pm;
  for (size_t i = 0; i < a.size(); ++i) {
    pm[a[i]] = b[i];
  }
  // println( "map: {}", std::format("{}", pm) );
  return pm;
};


auto && video_map = make_map(video_dirs, video_dbfiles);
auto && audio_map = make_map(audio_dirs, audio_dbfiles);
// auto && pic_map = make_map(pic_dirs,pic_dbfiles);

#endif

