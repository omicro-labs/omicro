#############################################################
# Generic Makefile for C/C++ Program
#
# License: GPL (General Public License)
# Author:  whyglinux <whyglinux AT gmail DOT com>
# Date:    2006/03/04 (version 0.1)
#          2007/03/24 (version 0.2)
#          2007/04/09 (version 0.3)
#          2007/06/26 (version 0.4)
#          2008/04/05 (version 0.5)
#
# Description:
# ------------
# This is an easily customizable makefile template. The purpose is to
# provide an instant building environment for C/C++ programs.
#
# It searches all the C/C++ source files in the specified directories,
# makes dependencies, compiles and links to form an executable.
#
# Besides its default ability to build C/C++ programs which use only
# standard C/C++ libraries, you can customize the Makefile to build
# those using other libraries. Once done, without any changes you can
# then build programs using the same or less libraries, even if source
# files are renamed, added or removed. Therefore, it is particularly
# convenient to use it to build codes for experimental or study use.
#
# GNU make is expected to use the Makefile. Other versions of makes
# may or may not work.
#
# Usage:
# ------
# 1. Copy the Makefile to your program directory.
# 2. Customize in the "Customizable Section" only if necessary:
#    * to use non-standard C/C++ libraries, set pre-processor or compiler
#      options to <MY_CFLAGS> and linker ones to <MY_LIBS>
#      (See Makefile.gtk+-2.0 for an example)
#    * to search sources in more directories, set to <SRCDIRS>
#    * to specify your favorite program name, set to <PROGRAM>
# 3. Type make to start building your program.
#
# Make Target:
# ------------
# The Makefile provides the following targets to make:
#   $ make           compile and link
#   $ make NODEP=yes compile and link without generating dependencies
#   $ make objs      compile only (no linking)
#   $ make tags      create tags for Emacs editor
#   $ make ctags     create ctags for VI editor
#   $ make clean     clean objects and the executable file
#   $ make distclean clean objects, the executable and dependencies
#   $ make help      get the usage of the makefile
#
#===========================================================================

## Customizable Section: adapt those variables to suit your program.
##==========================================================================

OS_NAME="`uname -s`"
LC_OS_NAME = $(shell echo $(OS_NAME) | tr '[A-Z]' '[a-z]')
# MAC=darwin
# CENTOS=linux

# The pre-processor and compiler options.
MY_CFLAGS =

##omicro
BOOST_LIB_PATH=/usr/local/boost_1.78.0/lib
BOOST_INC_PATH=/usr/local/boost_1.78.0/include

HASH=../Web/hash
NTRU=../Web/ntru/libntru
TC=../Web/tc/tc
SABER=../Web/saber
BASE85=../Web/base85
AES=../Web/aes
JSON=../Web/json/rapidjson
DILITH=../Web/dilithium
OPENSSL=../Web/openssl/install
ZLIB=../Web/zlib/install
BZIP2=../Web/bzip2/install

DILITH_LIBS=$(DILITH)/dilithium/ref/ref/libdilithium5_ref.a \
            $(DILITH)/dilithium/ref/ref/libdilithium3_ref.a \
            $(DILITH)/dilithium/ref/ref/libdilithium2_ref.a \
            $(DILITH)/dilithium/ref/ref/libfips202_ref.a \
            $(DILITH)/dilithium/ref/ref/libaes256ctr_ref.a 

            #randombytes_dl.o

# The linker options.
MY_LIBS = $(HASH)/xxHash/libxxhash.a \
		  $(SABER)/SABER/Reference_Implementation_KEM/libsaber.a \
		  $(DILITH_LIBS) \
		  $(BASE85)/ascii85/ascii85.o \
		  $(AES)/tiny-AES-c/aes.o \
		  $(TC)/lib/libtokyocabinet.a \
		  $(OPENSSL)/lib64/libcrypto.a \
		  $(ZLIB)/lib/libz.a \
		  $(BZIP2)/lib/libbz2.a \
		  /usr/local/gcc-11.3.0/lib64/libstdc++.a \
		  -ldl

		  #/usr/local/gcc-7.4.0/lib64/libstdc++.a \
          #$(BOOST_LIB_PATH)/libboost_system.a \
		  #$(BOOST_LIB_PATH)/libboost_filesystem.a \

#BOOST_DEFINE = -D BOOST_ASIO_ENABLE_HANDLER_TRACKING -D BOOST_ASIO_ENABLE_BUFFER_DEBUGGING

#BOOST_DEFINE = -DBOOST_BIND_GLOBAL_PLACEHOLDERS 
BOOST_DEFINE = 

#MUDUO_DEFINE = -D MUDUO_STD_STRING -D __GXX_EXPERIMENTAL_CXX0X__

#WORNING_FLAGS = -Wall -Wextra -Wconversion -Wno-unused-parameter -Wno-sign-conversion \
# -Wold-style-cast -Woverloaded-virtual -Wpointer-arith -Wshadow -Wwrite-strings
WORNING_FLAGS = -Wall


# The pre-processor options used by the cpp (man cpp for more).
CPPFLAGS  = $(WORNING_FLAGS) -I $(BOOST_INC_PATH) \
            -I $(HASH) \
            -I $(SABER) \
            -I $(TC)/include \
            -I $(BASE85) \
            -I $(AES) \
            -I $(JSON)/include \
			$(BOOST_DEFINE) \
			-D DILITHIUM_MODE=5 \
			-I $(DILITH)/include \
			-I $(ZLIB)/include \
			-I $(BZIP2)/include \
			-O3

# The options used in linking as well as in any direct use of ld.
ifeq ($(LC_OS_NAME), darwin)
    LDFLAGS   = -L/opt/local/lib -pthread
else
    LDFLAGS   = -L/opt/local/lib -pthread -lrt
endif


# The directories in which source files reside.
# If not specified, only the current directory will be serached.
SRCDIRS   = ../

# The executable file name.
# If not specified, current directory name or `a.out' will be used.
PROGRAM   = omserver

## Implicit Section: change the following only when necessary.
##==========================================================================

# The source file types (headers excluded).
# .c indicates C source files, and others C++ ones.
SRCEXTS = .c .C .cc .cpp .CPP .c++ .cxx .cp

# The header file types.
HDREXTS = .h .H .hh .hpp .HPP .h++ .hxx .hp

# The pre-processor and compiler options.
# Users can override those variables from the command line.
CFLAGS  =
CXXFLAGS= -std=c++14

# The C program compiler.
CC     = gcc

# The C++ program compiler.
CXX    = g++

# Un-comment the following line to compile C programs as C++ ones.
#CC     = $(CXX)

# The command used to delete file.
#RM     = rm -f

ETAGS = etags
ETAGSFLAGS =

CTAGS = ctags
CTAGSFLAGS =

## Stable Section: usually no need to be changed. But you can add more.
##==========================================================================
SHELL   = /bin/sh
EMPTY   =
SPACE   = $(EMPTY) $(EMPTY)
ifeq ($(PROGRAM),)
	q
	q
	q
  CUR_PATH_NAMES = $(subst /,$(SPACE),$(subst $(SPACE),_,$(CURDIR)))
  PROGRAM = $(word $(words $(CUR_PATH_NAMES)),$(CUR_PATH_NAMES))
  ifeq ($(PROGRAM),)
    PROGRAM = a.out
  endif
endif
ifeq ($(SRCDIRS),)
  SRCDIRS = .
endif
SOURCES = $(foreach d,$(SRCDIRS),$(wildcard $(addprefix $(d)/*,$(SRCEXTS))))
HEADERS = $(foreach d,$(SRCDIRS),$(wildcard $(addprefix $(d)/*,$(HDREXTS))))
SRC_CXX = $(filter-out %.c,$(SOURCES))

#OBJS    = $(addsuffix .o, $(basename $(SOURCES)))
UPOBJS    = $(addsuffix .o, $(basename $(SOURCES)))
DOTDOT:= ..
DOT:= .
OBJS= $(subst $(DOTDOT),$(DOT),$(UPOBJS))


## Define some useful variables.
DEP_OPT = $(shell if `$(CC) --version | grep "GCC" >/dev/null`; then \
                  echo "-MM -MP"; else echo "-M"; fi )
DEPEND      = $(CC)  $(DEP_OPT)  $(MY_CFLAGS) $(CFLAGS) $(CPPFLAGS)
COMPILE.c   = $(CC)  $(MY_CFLAGS) $(CFLAGS)   $(CPPFLAGS) -c
COMPILE.cxx = $(CXX) $(MY_CFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c
LINK.c      = $(CC)  $(MY_CFLAGS) $(CFLAGS)   $(CPPFLAGS) $(LDFLAGS)
LINK.cxx    = $(CXX) $(MY_CFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS)

.PHONY: all objs tags ctags clean distclean help show

# Delete the default suffixes
.SUFFIXES:

all: lib $(PROGRAM) 


# Rules for generating object files (.o).
#----------------------------------------
objs:$(OBJS)

%.o:../%.c
	$(COMPILE.c) $< -o $@

%.o:../%.C
	$(COMPILE.cxx) $< -o $@

%.o:../%.cc
	$(COMPILE.cxx) $< -o $@

%.o:../%.cpp
	$(COMPILE.cxx) $< -o $@

%.o:../%.CPP
	$(COMPILE.cxx) $< -o $@

%.o:../%.c++
	$(COMPILE.cxx) $< -o $@

%.o:../%.cp
	$(COMPILE.cxx) $< -o $@

%.o:../%.cxx
	$(COMPILE.cxx) $< -o $@

# Rules for generating the tags.
#-------------------------------------
tags: $(HEADERS) $(SOURCES)
	$(ETAGS) $(ETAGSFLAGS) $(HEADERS) $(SOURCES)

ctags: $(HEADERS) $(SOURCES)
	$(CTAGS) $(CTAGSFLAGS) $(HEADERS) $(SOURCES)

# Rules for generating the executable.
#-------------------------------------
LIB_OBJS := $(filter-out .//main.o, $(OBJS))
lib: $(LIB_OBJS) randombytes_dl.o
	ar cr libomicro.a  $(LIB_OBJS) randombytes_dl.o

$(PROGRAM):$(OBJS)
ifeq ($(SRC_CXX),)              # C program
	$(LINK.c)   $(OBJS) $(MY_LIBS) -o $@
	@echo Type ./$@ to execute the program.
	mkdir -p ../../test/server/bin
	/bin/cp -f $@ ../../test/server/bin/
else                            # C++ program
	$(LINK.cxx) $(OBJS) $(MY_LIBS) -o $@
	@echo Type ./$@ to execute the program.
	mkdir -p ../../test/server/bin
	/bin/cp -f $@ ../../test/server/bin/
endif

ifndef NODEP
ifneq ($(DEPS),)
  sinclude $(DEPS)
endif
endif

clean:
	$(RM) $(OBJS) $(PROGRAM) $(PROGRAM).exe

distclean: clean
	$(RM) $(DEPS) TAGS

# Show help.
help:
	@echo 'Generic Makefile for C/C++ Programs (gcmakefile) version 0.5'
	@echo 'Copyright (C) 2007, 2008 whyglinux <whyglinux@hotmail.com>'
	@echo
	@echo 'Usage: make [TARGET]'
	@echo 'TARGETS:'
	@echo '  all       (=make) compile and link.'
	@echo '  NODEP=yes make without generating dependencies.'
	@echo '  objs      compile only (no linking).'
	@echo '  tags      create tags for Emacs editor.'
	@echo '  ctags     create ctags for VI editor.'
	@echo '  clean     clean objects and the executable file.'
	@echo '  distclean clean objects, the executable and dependencies.'
	@echo '  show      show variables (for debug use only).'
	@echo '  help      print this message.'
	@echo
	@echo 'Report bugs to <whyglinux AT gmail DOT com>.'

# Show variables (for debug use only.)
show:
	@echo 'PROGRAM     :' $(PROGRAM)
	@echo 'SRCDIRS     :' $(SRCDIRS)
	@echo 'HEADERS     :' $(HEADERS)
	@echo 'SOURCES     :' $(SOURCES)
	@echo 'SRC_CXX     :' $(SRC_CXX)
	@echo 'OBJS        :' $(OBJS)
	@echo 'DEPS        :' $(DEPS)
	@echo 'DEPEND      :' $(DEPEND)
	@echo 'COMPILE.c   :' $(COMPILE.c)
	@echo 'COMPILE.cxx :' $(COMPILE.cxx)
	@echo 'link.c      :' $(LINK.c)
	@echo 'link.cxx    :' $(LINK.cxx)

## End of the Makefile ##  Suggestions are welcome  ## All rights reserved ##
##############################################################
