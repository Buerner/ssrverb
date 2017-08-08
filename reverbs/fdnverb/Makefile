
CC = g++
CFLAGS = -Wall -std=c++11 -O3

OS := $(shell uname)

ifeq ($(OS),Darwin)
    CFLAGS +=  -mmacosx-version-min=10.7
    CFLAGS += -stdlib=libc++
endif

SEARCH_PATHS = -I../.. \
	       	   -I./include 


OBJ_DIR = build/.obj/
BUILD_DIR = build/

SRC := $(wildcard src/*.cpp)
SRC := $(filter-out src/main.cpp, $(SRC))
#SRC := $(filter-out src/DynamicFDN.cpp, $(SRC))
OBJ = $(addprefix $(OBJ_DIR), $(notdir $(SRC:.cpp=.o) ) )


all: mk_build_dir $(OBJ)

# Generic rule to create .o files from .cpp files
$(OBJ_DIR)%.o: src/%.cpp
	$(CC) $(CFLAGS) $(SEARCH_PATHS) -c $< -o $@

#.PHONY: dynfdn
#dynfdn:
#	$(CC) $(CFLAGS)  $(SEARCH_PATHS) -c src/DynamicFDN.cpp -o $(OBJ_DIR)DynamicFDN.o

.PHONY: mk_build_dir
mk_build_dir:
	mkdir -p $(OBJ_DIR)

.PHONY: doc
doc:
	doxygen

.PHONY: clean
clean:
	rm -f $(OBJ_DIR)*.o


