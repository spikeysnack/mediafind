#!/usr/bin/env make

SHELL    := $(shell command -v bash)
TCLSH    := $(shell command -v tclsh)
JQ       ?= $(shell command -v jq)
PY       ?= $(shell command -v python3)

JS_CMD   ?= $(JQ)
JS_CMD   ?= $(PY) -m json.tool


HOME_DIR    := $(HOME)
CONF_FILE   :=  "media_dirs.conf"
USER_CONFIG := "user_config.hpp"

TARGET   := mediafind

PREFIX  ?= $(HOME_DIR)
BINDIR   = $(PREFIX)/bin
MANDIR   = $(PREFIX)/.local/man/man1
CONFDIR  = $(HOME)/.mediafind
MAN_CATG = 1
MANPAGE  = $(TARGET).$(MAN_CATG)

# Standard utility commands
INSTALL  = install
MKDIR_P  = mkdir -p
RM       = rm -f

CXX = g++-16
CXXFLAGS =  -Wall -Wextra -std=gnu++26 -march=native -O3 -pthread
CXXDEBUGFLAGS =  -O0 -g -Wall -Wextra -std=gnu++26 -D_GLIBCXX_DEBUG -pthread  -fsanitize=thread

define HELP_TEXT
----------------------------------------------------------------------- 
to build:                                      
         1. Edit file media_dirs.txt  -- add directories to search.     
         2. Type 'make config'        -- create 'user_config.hpp'       
         3. Type 'make config_check'  -- check config                     
         4. Type 'make'               --  build executable                
         5. Type 'make test'          -- check executable runs

to install:                                                             
         1. Type 'make install'      -- install executable to local bin/ 
         2. Type 'make install_man'  -- install manpage    to local man/    

to uninstall:                                                           
         1. Type 'make uninstall'    -- remove executable and manpage   
------------------------------------------------------------------------ 
endef
export HELP_TEXT


.PHONY: all clean config_check config test json install uninstall help help2

all:	
	@$(MAKE) -s config_check && $(MAKE) SILENT=0  mediafind || $(MAKE) -s help


mediafind:	 mediafind.hpp mediafind.cpp
	$(CXX) $(CXXFLAGS) -o mediafind mediafind.cpp

config_check:	config.tcl
	@if [ ! -f $(USER_CONFIG) ] ; then \
		echo -e "\nError: $(USER_CONFIG) is missing!\n"; \
		echo -e "1. Edit $(CONF_FILE)\n";   \
		echo -e "2. run make config\n";    \
		exit 1; \
	fi

config:	config.tcl
	@set -eo pipefail; tclsh config.tcl

debug:	mediafind.hpp mediafind.cpp
	@echo "DEBUG BUILD: "
	$(CXX) $(CXXDEBUGFLAGS) -o mediafind mediafind.cpp

install:	$(TARGET)
	@echo "INSTALLING $(TARGET)"
	$(MKDIR_P) $(DESTDIR)$(BINDIR)
	$(MKDIR_P) $(CONFDIR)
	$(INSTALL) -m 0640 $(CONF_FILE) $(CONFDIR)
	$(INSTALL) -m 0755 $(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)

install_man:	$(MANPAGE)
	@echo "INSTALLING $(TARGET) MAN PAGE" 
#	Optional: Install manual pages or assets
	$(MKDIR_P) $(DESTDIR)$(MANDIR)
	$(MKDIR_P) $(DESTDIR)$(MANDIR)

	$(INSTALL) -m 0644 $(MANPAGE) $(DESTDIR)$(MANDIR)/$(MANPAGE)

uninstall:
	@echo "UNINSTALLING $(TARGET)"
	$(RM) $(DESTDIR)$(BINDIR)/$(TARGET)
	@echo "UNINSTALLING $(TARGET) MANPAGE"
	$(RM) $(DESTDIR)$(MANDIR)/$(TARGET).1

# run basic commands to check
test:	mediafind
	@ ./mediafind --version;       t1=$$? ; \
	  ./mediafind -h;              t2=$$?;  \
	  ./mediafind -m | $(JS_CMD);  t3=$$?;  \
	  sum=$$(( $$t1 | $$t2 | $$t3 ));       \
	  if [ $$((sum)) -eq 0 ]; then          \
	     echo -e "\nchecks passed\n";       \
	  else                                  \
	    echo -e "\nsome checks failed\n" ;  \
	 fi



help:
	@echo -e "help2:\n" ; \
	echo "$$HELP_TEXT"

json:	mediafind
	@ ./mediafind -m > mediafind.json
	@ cat mediafind.json |jq 
clean:
	$(RM) -f *~ *.json $(TARGET)

distclean:	clean
	$(RM) $(USER_CONFIG)
	$(RM) -rf build

