#!/usr/bin/env make

SHELL    := /usr/bin/bash
HOME_DIR := $(HOME)

TARGET   := mediafind

PREFIX  ?= $(HOME_DIR)
BINDIR   = $(PREFIX)/bin
MANDIR   = $(PREFIX)/.local/man/man1
MAN_CATG = 1
MANPAGE  = $(TARGET).$(MAN_CATG)

# Standard utility commands
INSTALL  = install
MKDIR_P  = mkdir -p
RM       = rm -f


CXX = g++-16
CXXFLAGS =  -Wall -Wextra -std=gnu++26 -march=native -O3 -pthread
CXXDEBUGFLAGS =  -O0 -g -Wall -Wextra -std=gnu++26 -D_GLIBCXX_DEBUG -pthread  -fsanitize=thread

.PHONY: clean test install uninstall

mediafind:	mediafind.hpp mediafind.cpp
	$(CXX) $(CXXFLAGS) -o mediafind mediafind.cpp

debug:	mediafind.hpp mediafind.cpp
	$(CXX) $(CXXDEBUGFLAGS) -o mediafind mediafind.cpp

install:	$(TARGET)
	$(MKDIR_P) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 $(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)

install_man:	$(MANPAGE)
#	Optional: Install manual pages or assets
	$(MKDIR_P) $(DESTDIR)$(MANDIR)
	$(INSTALL) -m 0644 $(MANPAGE) $(DESTDIR)$(MANDIR)/$(MANPAGE)

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/$(TARGET)
	$(RM) $(DESTDIR)$(MANDIR)/$(TARGET).1


test:	mediafind
	./mediafind -d -a "progressive rock"
	./mediafind -d -v "street"

json:	mediafind
	./mediafind -m > mediafind.json
	cat mediafind.json |jq 

clean:
	$(RM) -f *~ *.json $(TARGET)
