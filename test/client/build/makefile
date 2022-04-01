#############################################################
# Generic Makefile for C/C++ Program
#
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

BOOST_INC_PATH=/usr/local/include
BOOST_LIB_PATH=/usr/local/lib
BOOST_LIB_PATH=/usr/local/boost_1.78.0/lib
BOOST_INC_PATH=/usr/local/boost_1.78.0/include

SRCINC=../../../src
SRCBUILD=../../../src/build
SABER=../../../src/Web/saber
BASE85=../../../src/Web/base85
AES=../../../src/Web/aes
JSON=../../../src/Web/json/rapidjson
HASH=../../../src/Web/hash


PTHREADLIB=/lib64/libpthread.a
RTLIB=/lib64/librt.a
DLLIB=/lib64/libdl.a
CLIB=/lib64/libc.a

# The linker options.
MY_LIBS   = \
			$(SRCBUILD)/libomicro.a \
			$(SABER)/SABER/Reference_Implementation_KEM/libsaber.a \
			$(BASE85)/ascii85/ascii85.o \
			$(AES)/tiny-AES-c/aes.o \
			$(HASH)/xxHash/libxxhash.a \
            $(BOOST_LIB_PATH)/libboost_system.a $(BOOST_LIB_PATH)/libboost_filesystem.a \
            $(BOOST_LIB_PATH)/libboost_thread.a $(BOOST_LIB_PATH)/libboost_date_time.a \
			-l crypto -lz -lbz2


# The pre-processor options used by the cpp (man cpp for more).
#CPPFLAGS  = -Wall -I essential -I ddlib -O2

#CPPFLAGS  = -Wall -I $(BOOST_INC_PATH) -g3 -D BOOST_ASIO_ENABLE_HANDLER_TRACKING -D BOOST_ASIO_ENABLE_BUFFER_DEBUGGING
CPPFLAGS  = -Wall -I $(BOOST_INC_PATH) \
            -I $(SRCINC) \
			-I $(SABER) \
			-I $(BASE85) \
			-I $(AES) \
			-I $(JSON)/include  \
			-g

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
PROGRAM   = omclient

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

all: $(PROGRAM)


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
$(PROGRAM):$(OBJS)
ifeq ($(SRC_CXX),)              # C program
	$(LINK.c)   $(OBJS) $(MY_LIBS) -o $@
	@echo Type ./$@ to execute the program.
else                            # C++ program
	$(LINK.cxx) $(OBJS) $(MY_LIBS) -o $@
	@echo Type ./$@ to execute the program.
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
