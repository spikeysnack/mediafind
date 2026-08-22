/* user_config.hpp */

#ifndef USER_CONFIG_HPP
#define USER_CONFIG_HPP

#include <string>
#include <array>
#include <unordered_map>
#include "mediafind.hpp"

using std::string;
using std::array;
using pathmap = std::unordered_map<string,string>;


// base_dir for db files
const string base_dir = "/home/chris/.mediafind";
const string config_file = "/home/chris/.mediafind/media_dirs.conf";

static constexpr std::tuple version {2, 0, 1};
static constexpr const char* compile_date = "22 Aug 2026";


// filled by media_dirs.conf
pathmap audio_map{};
pathmap video_map{};
pathmap extra_map{};

#endif
// end 

