# makefile for building samples for udgcd
# author: S. Kramm, 2015
# see https://github.com/skramm/udgcd
# Linux only, probably.

COLOR_1=-e "\e[1;33m"
COLOR_2=-e "\e[1;34m"
COLOR_OFF="\e[0m"

# don't delete intermediate files
.SECONDARY:

# disable implicit rules
.SUFFIXES:

.PHONY: clearlogfile svg all help runsam doc clean cleanout diff

#--------------------------------
# general compiler flags
# -Wno-unused-result is to avoid the warning on call to std::system() when calling dot (see void CallDot() )
CFLAGS = -std=c++0x -Wall -O2 -Iinclude -Wno-unused-result
CFLAGS = -std=c++11 -Wall -O2 -Iinclude -Wno-unused-result


ifeq "$(PRINT_STEPS)" "Y"
	CFLAGS += -DUDGCD_PRINT_STEPS
endif

ifeq "$(DEVMODE)" "Y"
	CFLAGS += -DUDGCD_DEV_MODE
endif

SHELL=/bin/bash

# files and folders
SRC_DIR=.
APP=udgcd.hpp
HEADERS=$(wildcard $(SRC_DIR)/*.h*)
BIN_DIR=bin
OBJ_DIR=obj

SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))
EXEC_FILES = $(patsubst $(SRC_DIR)/%.cpp,$(BIN_DIR)/%,$(SRC_FILES))
SAMPLE_FILES = $(wildcard samples/*.txt)

GEN_GSAMPLE_FILES = $(wildcard out/gen_graph_*.txt)
GEN_GSAMPLES_OUTPUT = $(patsubst out/%.txt,out/stdout_%.txt,$(GEN_GSAMPLE_FILES))

GEN_SAMPLE_FILES = $(wildcard samples/graph_*.txt)
GEN_SAMPLES_OUTPUT = $(patsubst samples/graph_%.txt,out/stdout_graph_%.txt,$(GEN_SAMPLE_FILES))
#GEN_SAMPLES_PLOT = $(patsubst samples/%.txt,out/sample_%.svg,$(GEN_SAMPLE_FILES))

DOT_FILES=$(wildcard out/*.dot)
SVG_FILES=$(patsubst out/%.dot,out/%.svg,$(DOT_FILES))

# default target
all: $(EXEC_FILES)
	@echo "- Done target $@"

help:
	@echo "Available targets:"
	@echo " -run: runs once all the produced binaries"
	@echo " -runsam: runs cycle detection process on all provided samples"
	@echo " -clean: erase obj files"
	@echo " -cleanout: erase produced output"
	@echo " -cleandoc: erase produced (doxygen-build) documentation"
	@echo " -cleanall: all of the above \"clean\" targets"
	@echo " -doc: builds documentation form source, using doxygen"

	@echo "Available options"
	@echo " -PRINT_STEPS={Y|N} (default: N) : logs some steps to stdout"
	@echo " -DEVMODE={Y|N} (default: N) : lots of additional logging on stdout (useless for end user)"

# runs once all the produced binaries
run: all
	$(addsuffix ;,$(EXEC_FILES))

# runs on all generated samples
rungen: $(GEN_GSAMPLES_OUTPUT) bin/read_graph
	@echo "target $@ done"

# runs cycle detection process on all provided samples
runsam: $(GEN_SAMPLES_OUTPUT) bin/read_graph | clearlogfile
	@echo "target $@ done"

clearlogfile:
	@echo "Running make target 'runsam', results:" > runsam.log

out/stdout_graph_%.txt: samples/graph_%.txt $(EXEC_FILES)
	-bin/read_graph $< > $@;\
	STATUS=$$?; echo "file $<: exit with $$STATUS" >> runsam.log

out/%.svg : out/%.dot
	dot -Tsvg -Nfontsize=24 $< >$@

show: $(SRC_FILES)
	@echo SRC_FILES=$(SRC_FILES)
	@echo OBJ_FILES=$(OBJ_FILES)
	@echo EXEC_FILES=$(EXEC_FILES)
	@echo SAMPLE_FILES=$(SAMPLE_FILES)
	@echo GEN_GSAMPLE_FILES=$(GEN_GSAMPLE_FILES)
	@echo GEN_GSAMPLES_OUTPUT=$(GEN_GSAMPLES_OUTPUT)
	@echo GEN_SAMPLE_FILES=$(GEN_SAMPLE_FILES)
	@echo GEN_SAMPLES_OUTPUT=$(GEN_SAMPLES_OUTPUT)
	@echo GEN_SAMPLES_PLOT =$(GEN_SAMPLES_PLOT)
	@echo DOT_FILES =$(DOT_FILES)
	@echo SVG_FILES =$(SVG_FILES)

doc:
	doxygen doxyfile 1>$(OBJ_DIR)/doxygen_stdout.txt 2>$(OBJ_DIR)/doxygen_stderr.txt
	xdg-open html/index.html

clean:
	@-rm $(OBJ_DIR)/*
	@-rm *.layout

cleanout:
	@-rm out/*

cleanall: clean cleandoc cleanout
	@-rm $(EXEC_FILES)

cleandoc:
	@-rm -r html/*
	@-rmdir html

svg: $(SVG_FILES)
	@echo "target $@ done"

diff:
	git diff --color-words | aha > $(OBJ_DIR)/diff.html
	xdg-open $(OBJ_DIR)/diff.html

# needs 'sudo'
install:
	-cp $(APP) /usr/local/include/$(APP)

# generic compile rule
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	@echo $(COLOR_2) " - Compiling app file $<." $(COLOR_OFF)
	$(L)$(CXX) -o $@ -c $< $(CFLAGS)

# linking
$(BIN_DIR)/%: $(OBJ_DIR)/%.o
	@echo $(COLOR_1) " - Link demo $@." $(COLOR_OFF)
	$(L)$(CXX) -o $@ -s $<  $(LDFLAGS)

bin/test_catch: obj/test_catch.o
	$(CXX) -o bin/test_catch obj/test_catch.o -s
	@echo "done target $@"

# -s option: also shows successful test results
test: bin/test_catch
	bin/test_catch -s

