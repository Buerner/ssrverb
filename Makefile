
CC = g++
CFLAGS = -Wall -std=c++11 -O3

OS := $(shell uname)

ifeq ($(OS),Linux)
     LIBS += -lpthread
endif


SEARCH_PATHS = -I../audioprocessing/src \
	       -I../ssrinterface/src \
	       -I../3D/src

BUILD_DIR = build/
OBJ_DIR = $(BUILD_DIR).obj/
LIB_DIR = $(BUILD_DIR)libs
BIN_DIR = $(BUILD_DIR)bins

all: libs
	cd guis/ISMFDNreverb && qmake && make
	@if [ "$(OS)" = "Linux" ]; then\
    	cp guis/ISMFDNreverb/build/ISMFDNreverb $(BIN_DIR);\
	fi
	@if [ "$(OS)" = "Darwin" ]; then\
    	cp -r guis/ISMFDNreverb/build/ISMFDNreverb.app $(BIN_DIR);\
	fi
	cd reverbs/randomizer && make bin
	cp reverbs/randomizer/build/JackRandomizer $(BIN_DIR)

.PHONY: libs
libs: mk_build_dir
	cd laproque && make
	cp laproque/build/libaproque.a $(LIB_DIR)
	cd ssrface && make
	cp ssrface/build/libssrface.a $(LIB_DIR)
	cd reverbs && make
	cp reverbs/build/libssrverb.a $(LIB_DIR)

.PHONY: mk_build_dir
mk_build_dir:
	mkdir -p $(LIB_DIR)
	mkdir -p $(BIN_DIR)

.PHONY: doc
doc:
	cd laproque && doxygen
	cd ssrface && doxygen
	cd reverbs && doxygen

.PHONY: clean
clean:
	cd laproque && make clean
	cd ssrface && make clean
	cd reverbs && make clean
	cd guis/ISMFDNreverb && qmake && make clean
	cd build && rm -rf ./*

