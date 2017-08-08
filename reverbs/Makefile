
CC = g++
CFLAGS = -Wall -std=c++11 -stdlib=libc++ -O3

OS := $(shell uname)

ifeq ($(OS),Darwin)
    CFLAGS +=  -mmacosx-version-min=10.7
endif

SEARCH_PATHS = -I.. \
	       -I./include

OBJ_DIR = build/.obj/
BUILD_DIR = build/

LIB_NAME = libssrverb.a

SRC := $(wildcard src/*.cpp)
OBJ = $(addprefix $(OBJ_DIR), $(notdir $(SRC:.cpp=.o) ) ) 

all:  mk_build_dir $(OBJ)
	cd fdnverb && make
	cp fdnverb/build/.obj/*.o build/.obj
	cd ismverb && make
	cp ismverb/build/.obj/*.o build/.obj
	cd randomizer && make
	cp randomizer/build/.obj/*.o build/.obj
	bash -c "ar -rvs $(BUILD_DIR)$(LIB_NAME) $(OBJ_DIR)*.o"

$(OBJ_DIR)%.o: src/%.cpp
	$(CC) $(CFLAGS) $(SEARCH_PATHS) -c $< -o $@

.PHONY: mk_build_dir
mk_build_dir:
	mkdir -p $(OBJ_DIR)

.PHONY: doc
doc:
	doxygen

.PHONY: clean
clean:
	cd fdnverb && make clean
	cd ismverb && make clean
	cd randomizer && make clean
	rm -f $(OBJ_DIR)*.o
	rm -f $(BUILD_DIR)$(LIB_NAME)



