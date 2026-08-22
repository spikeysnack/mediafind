#!/usr/bin/env  tclsh

# 17 Aug 2026

variable program_version  {2 0 1}
variable today [clock format [clock seconds] -format "%d %b %Y" ]         

# defaults -- dirs to scan are in this file
set dirs_file "media_dirs.conf"

# create local user config as user_config.hpp
set user_config_hpp "user_config.hpp"

# dir for database files
set base_dir "$::env(HOME)"
set db_dir "$base_dir/.mediafind"

set config_file "$db_dir/${dirs_file}"

# DO NOT EDIT BELOW THIS LINE #

set TCL_OK 0
set TCL_ERROR 1

namespace eval mediafind {

	variable version     $::program_version
	variable currentDate $::today

    proc read_file {f} {

		try {
			set fd [open "${f}" "r"]
			set contents [read $fd]
		} on error {err inf } {
			puts stderr "could not read $f:  $err"
			exit $::TCL_ERROR
		} finally {
			close $fd
		}

		# split on crlf
		set contents [split $contents "\n"]
		# remove blank lines
		set contents [lsearch -all -inline -not -exact $contents {}]

		return $contents
    }

    # read files from f a add to list 
    proc read_dirs_from_file {f list_name} {
		set files {}

		# matches "[$list_name]"
		set target "\[${list_name}\]"

		# matches "[<alpha>]"
		set head_rgx {\[(.+)\]$}

		set contents [read_file $f]

		set entry 0
		# read lines under target heading only
		foreach line $contents {
			set line [string trim $line]
			# comment  -- skip
			if { [string first "#" $line] eq 0 } {continue}
			
			if { $line eq $target } {
				set entry 1
				continue
			} elseif { [regexp $head_rgx $line] } {
				set entry 0
				continue
			} else {
				if {$entry} {lappend files $line}
			}
		}
		return $files
    }

   # literal quotes
    proc q {s} { return "\"${s}\"" }

    #create database dir
    proc make_db_dir {d} {
		try {
			if { ! [file exists $d] } {
				file mkdir $d
				file attributes $d -permissions 0770
			}
		} on error { why details} {
			set errmsg "make_db_dir failed: $why"
			return -code error "$errmsg"
		}
		return $::TCL_OK
    }

    proc make_version_code {version date} {
		set cpp_code ""

		lassign $version  major minor rev
		
		append cpp_code  "static constexpr std::tuple version {"
		append cpp_code  "$major, $minor, $rev};\n"

		append cpp_code  "static constexpr const char* compile_date = "
		append cpp_code  "[q $date];\n"

		return $cpp_code	
	}

    # writes all lines to a file
    proc write_config_file { cf contents } {

		if { [file exists $cf] } {
			puts stderr "${cf} already exists"
			puts stderr "\n\tremove it or edit it directly"
			return $::TCL_ERROR
		}

		try {
			set fd [open $cf "w"]
			puts $fd $contents
		} on error { err inf } {
			puts stderr "error writing file $cf:\t $err"
			return $::TCL_ERROR
		} finally {
			close $fd
		}
		return $::TCL_OK
    }

# these procs are exported
    set exports {
		make_db_dir
		make_map
		make_version_code
		read_dirs_from_file
		write_config_file
    }

    # make "public" procs
    namespace export {*}$exports

} ; # namespace mediafind


# "main"
if {[info script] eq $::argv0} {

    namespace import mediafind::*

    set status [make_db_dir $db_dir]

    if { $status ne $::TCL_OK } {
		puts stderr "error creating $db_dir"
		return $TCL_ERROR
	}

    puts stdout "reading dirs from file ${dirs_file}."

    # video dirs
	set video  [read_dirs_from_file  $dirs_file video ]
	array set video_arr {}
	foreach v $video {
		set dbname [file tail $v]
		set video_arr($v) ".mediadb.${dbname}"
	}

	set num_video_dirs [array size video_arr]
	if { $num_video_dirs eq 0 } {
		puts stderr "could not get video entries from $dir_file"
		exit $::TCL_ERROR
	}

	# audio dirs
	set audio  [read_dirs_from_file  $dirs_file audio ]
	array set audio_arr {}

	foreach a $audio {
		set dbname [file tail $a]
		set audio_arr($a) ".mediadb.${dbname}"
	}

	set num_audio_dirs [array size audio_arr]

	if { $num_audio_dirs eq 0 } {
		puts stderr "could not get audio entries from $dirs_file"
		exit $::TCL_ERROR
	}

    # here we create the user_config.hpp file
    set header \
"/* user_config.hpp */

#ifndef USER_CONFIG_HPP
#define USER_CONFIG_HPP

#include <string>
#include <array>
#include <unordered_map>
#include \"mediafind.hpp\"

using std::string;
using std::array;
using pathmap = std::unordered_map<string,string>;
"

    set versioning [make_version_code $program_version $today]

# generate user_config code
    set code \
"${header}

// base_dir for db files
const string base_dir = \"${db_dir}\";
const string config_file = \"${config_file}\";

${versioning}

// filled by media_dirs.conf
pathmap audio_map{};
pathmap video_map{};
pathmap extra_map{};

#endif
// end 
"


	puts stdout "writing ${user_config_hpp}"

	set status [write_config_file $user_config_hpp $code]

	if { $status ne 0 } {
		puts stderr "error writing ${user_config_hpp}"
		return $::TCL_ERROR
	} else {
		puts stdout "succesfully generated ${user_config_hpp}."
		puts stdout "now run make."
	}

	return $::TCL_OK
	}

# end

# vim: ai ts=4 sw=2 et sts=2 ft=tcl

# Local Variables:
# mode: tcl-mode
# tab-width: 4
# End:
