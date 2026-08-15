# mediafind #

[![Build with GCC 16](https://github.com/spikeysnack/mediafind/actions/workflows/build.yml/badge.svg)](https://github.com/spikeysnack/mediafind/actions/workflows/build.yml)

## Introduction ##

a fast local database search tool for media files.


## Description ##


Many people keep a store of audio and video files
on their mounted storage, and it can become hard to find
things after a lot of accumulation.

This program  keeps local locate databases in the user's
local home directory for fast searching. It is non-destructive
to the files it scans, not altering permissions, modification time.
It may update access time *(atime)*, 
depending on how the file system is mounted. 

locate works fine by itself, and I use it a lot to find
things, but I did not want it to scan my media partitions
as its database  would get far too large.



## make/install: ##

Simply edit the header file config.hpp to reflect the 
base dir for the database files,
and the top audio and video directories to search in,
and each corresponding database file for that dir.

I named them the database files as 
".mediadb.<dirname>" to be clear about it.

As I added storage, I added mounted partitions from a
a server called xeon that had a motherboard crash,
so the dir names became weird reflecting that.
Anyway use your own dirs and file names.

A simple make command should build the executable,
and you can copy it anywhere your PATH.

**Required external binaries: locate , updatedb, chown, sudo.**
You should already have them or what distro are you using.
sudo is only required to chown the database files to your
ownership after an update.

## how fast? ##

*real fast* -- it searches all the databases in separate threads.

With the  pthreads library installed, it will be faster still.

-------------------------------------------------------------------------------

### trouble compiling ###

requires  *c++23* or better  (c++26 preferred) for `println`, use std::cerr instead if
you don't have that going.

for instance: 

`println( stdout, "{}", line);`

would become

`std::cout << line << "\n";`

-------------------------------------------------------------------------------

### updating ###

the updating may take a few seconds to reflect changes to the 
file systems being scanned. locate is efficient by marking only
changes since the last updates, but updating happens one database
file at a time to prevent any data races.

updatedb  skips  unreadable files and dirs silently for the most part.

Running `mediafind -u` will have the same effect on the dirs scanned.
It may spit out "permission denied" or it may not, depending on
your /etc/updatedb.conf settings.
