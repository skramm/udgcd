# makefile for building sample(s) for udgld
# author: S. Kramm, 2015
# Linux only, probably...

COLOR_1=-e "\e[1;33m"
COLOR_2=-e "\e[1;34m"
COLOR_3=-e "\e[1;35m"
COLOR_OFF="\e[0m"

#--------------------------------
# general compiler flags
CFLAGS = -std=c++0x -Wall -O2 -Iinclude

# uncomment this line to print out the different steps
CFLAGS += -DUDGLD_PRINT_STEPS

# don't delete intermediate files
.SECONDARY:

# disable implicit rules
.SUFFIXES:

SHELL=/bin/bash

APP=udgld.hpp
HEADERS=$(wildcard $(SRC_DIR)/*.h*)
BIN_DIR=.
SRC_DIR=.
OBJ_DIR=obj

SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))
EXEC_FILES = $(patsubst $(SRC_DIR)/%.cpp,$(BIN_DIR)/%,$(SRC_FILES))

# default target
all: $(EXEC_FILES)
	@echo "- Done target $@"

run: all
	$(addsuffix ;,$(EXEC_FILES))
#	$(foreach t,$(EXEC_FILES),$t;)

show: $(SRC_FILES)
	@echo SRC_FILES=$(SRC_FILES)
	@echo OBJ_FILES=$(OBJ_FILES)
	@echo EXEC_FILES=$(EXEC_FILES)

doc: run
	doxygen doxyfile 1>$(OBJ_DIR)/doxygen_stdout.txt 2>$(OBJ_DIR)/doxygen_stderr.txt

clean:
	-rm $(OBJ_DIR)/*
	-rm *.layout

cleanall: clean cleandoc
	-rm $(EXEC_FILES)

cleandoc:
	-rm -r html/*
	-rmdir html

# needs 'sudo'
install:
	-cp $(APP) /usr/local/include/$(APP)

# generic compile rule
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	@echo $(COLOR_2) " - Compiling app file $<." $(COLOR_OFF)
	$(L)$(CXX) -o $@ -c $< $(CFLAGS)


# linking
$(BIN_DIR)/%: $(OBJ_DIR)/%.o
	@echo $(COLOR_3) " - Link demo $@." $(COLOR_OFF)
	$(L)$(CXX) -o $@ -s $<  $(LDFLAGS)

