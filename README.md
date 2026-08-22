# mediafind #

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

1. edit `media_dirs.txt` to reflect the 
   base dir for the database files,
   and the top audio and video directories to search in,
   and each corresponding database file for that dir.

2. type `make config` to execute the tcl script `config.tcl`. 
   This will create the custom C++ file `user_config.hpp`
   that contains version and mappings for your system.
   
3. type `make`  to build.

4. type `make check` for a basic test. *(jq preferred, python3 if not)*

5. type `make install` to install to your user bin dir

6. type `make install_man` to install manpage in section 1 of your user man dir.

7. type `make uninstall` to remove the binary and man pages. 


I named them the database files as 
".mediadb.<dirname>" to be clear about it.

As I added storage, I added mounted partitions from a
a server called xeon that had a motherboard crash,
so the dir names became weird reflecting that.
Anyway use your own dirs and file names.

A simple make command should build the executable,
and you can copy it anywhere your PATH.

**Required external binaries:** 

   build:  **g++ tclsh make**
   run  :  **locate, chown updatedb**

You should already have them or *what distro are you using*.

## how fast? ##

*real fast* -- it searches all the databases in separate threads.

With the  pthreads library installed, it will be faster still.

./mediafind -a <search term> searches only the audio databases;
./mediafind -v <search term> searches only the video databases;



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

### permisions ###
program runs as a regular user
The database files  (~/.mediafind/<dbfile>) are set to 0640 (-rw-r-----)
so the user can read and write and the group can read them.
Files will be recreated if deleted once the program is run again.
