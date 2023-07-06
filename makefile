# makefile for building samples for udgcd
# author: S. Kramm, 2015-2023
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
CFLAGS = -std=c++11 -Wall -O2 -Iinclude -Wno-unused-result


# test_m4ri.cpp: wrapper_m4ri.hpp

# remove if no need to compare the output ti some ground truth
#CFLAGS += -DUDGCD_NORMALIZE_CYCLES

ifeq "$(PRINT_STEPS)" "Y"
	CFLAGS += -DUDGCD_PRINT_STEPS
endif

ifeq "$(DEVMODE)" "Y"
	CFLAGS += -DUDGCD_DEV_MODE
endif

ifeq "$(DEBUG)" "Y"
	CFLAGS += -g
	LDFLAGS += -g
else
	LDFLAGS += -s
endif

ifeq "$(M4RI)" "Y"
	CFLAGS += -DUDGCD_USE_M4RI
	LDFLAGS +=-lm4ri
endif

SHELL=/bin/bash


# files and folders
SRC_DIR=demo
APP=udgcd.hpp
#HEADERS=$(wildcard $(SRC_DIR)/*.h*)
HEADER=udgcd.hpp
HEADERS = $(wildcard *.h*)
BIN_DIR=build/bin
OBJ_DIR=build/obj

SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))
EXEC_FILES = $(patsubst $(SRC_DIR)/%.cpp,$(BIN_DIR)/%,$(SRC_FILES))
SAMPLE_FILES = $(wildcard samples/*.txt)

GEN_GSAMPLE_FILES = $(wildcard out/gen_graph_*.txt)
GEN_GSAMPLES_OUTPUT = $(patsubst out/%.txt,out/stdout_%.txt,$(GEN_GSAMPLE_FILES))

GEN_SAMPLE_TXT_FILES = $(wildcard samples/graph_*.txt)
GEN_SAMPLE_DOT_FILES = $(wildcard samples/graph_*.dot)

GEN_SAMPLES_OUTPUT = $(patsubst samples/graph_%.txt,out/stdout_graph_%.txt,$(GEN_SAMPLE_TXT_FILES))
GEN_SAMPLES_OUTPUT += $(patsubst samples/graph_%.dot,out/stdout_graph_%.txt,$(GEN_SAMPLE_DOT_FILES))

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
	@echo " -svg: build svg renderings of all the samples produced by target 'runsam', with cycles as colored edges (see folder 'out')"

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
rungen: $(GEN_GSAMPLES_OUTPUT) $(BIN_DIR)/read_graph
	@echo "target $@ done"

# runs cycle detection process on all provided samples
runsam: clearlogfile $(GEN_SAMPLES_OUTPUT) $(BIN_DIR)/read_graph
	@echo "target $@ done"


clearlogfile: makefile
	@echo "Running make target 'runsam', results:" > build/runsam.log

# this one for .txt input files
out/stdout_graph_%.txt: samples/graph_%.txt $(BIN_DIR)/read_graph makefile
	@echo "processing file $<"
	@-$(BIN_DIR)/read_graph $< > $@;\
	STATUS=$$?; echo "file $<: exit with $$STATUS" >> build/runsam.log

# this one for .dot input files
out/stdout_graph_%.txt: samples/graph_%.dot $(BIN_DIR)/read_graph makefile
	@echo "processing file $<"
	@-$(BIN_DIR)/read_graph $< > $@;\
	STATUS=$$?; echo "file $<: exit with $$STATUS" >> build/runsam.log

out/%.svg : out/%.dot
	@echo "generating $@ bn=$(basename $@)"
	dot   -Tsvg -Nfontsize=24 $< >$(basename $@)_dot.svg
	neato -Tsvg -Nfontsize=24 -Elen=1.5 $< >$(basename $@)_neato.svg

show: $(SRC_FILES)
	@echo SRC_FILES=$(SRC_FILES)
	@echo OBJ_FILES=$(OBJ_FILES)
	@echo EXEC_FILES=$(EXEC_FILES)
	@echo SAMPLE_FILES=$(SAMPLE_FILES)
	@echo GEN_GSAMPLE_FILES=$(GEN_GSAMPLE_FILES)
	@echo GEN_GSAMPLES_OUTPUT=$(GEN_GSAMPLES_OUTPUT)
	@echo GEN_SAMPLE_TXT_FILES=$(GEN_SAMPLE_TXT_FILES)
	@echo GEN_SAMPLE_DOT_FILES=$(GEN_SAMPLE_DOT_FILES)
	@echo GEN_SAMPLES_OUTPUT=$(GEN_SAMPLES_OUTPUT)
	@echo GEN_SAMPLES_PLOT =$(GEN_SAMPLES_PLOT)
	@echo DOT_FILES =$(DOT_FILES)
	@echo SVG_FILES =$(SVG_FILES)


doc:
	doxygen misc/doxyfile 1>build/doxygen.stdout 2>build/doxygen.stderr
	xdg-open build/html/index.html

clean:
	@-rm $(OBJ_DIR)/*
	@-rm $(BIN_DIR)/*
	@-rm build/doxygen.*
	@-rm *.layout

cleanout:
	@-rm out/*

cleanall: clean cleandoc cleanout
	@-rm $(EXEC_FILES)

cleandoc:
	@-rm -r build/html/*
#	@-rmdir html

svg: $(SVG_FILES)
	@echo "target $@ done"

diff:
	git diff --color-words | aha > $(OBJ_DIR)/diff.html
	xdg-open $(OBJ_DIR)/diff.html

# needs 'sudo'
install:
	-cp $(APP) /usr/local/include/$(APP)

# generic compile rule
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS) wrapper_m4ri.hpp
	@echo $(COLOR_2) " - Compiling app file $<." $(COLOR_OFF)
	$(CXX) -o $@ -c $< $(CFLAGS)

# linking
# -s option: strip symbol (don't add if debugging)
$(BIN_DIR)/%: $(OBJ_DIR)/%.o
	@echo $(COLOR_1) " - Link demo $@." $(COLOR_OFF)
	$(CXX) -o $@ $<  $(LDFLAGS)
#	$(L)$(CXX) -o $@ $<  $(LDFLAGS)

#$(BIN_DIR)/test_catch: $(OBJ_DIR)/test_catch.o
#	$(CXX) -o bin/test_catch $(OBJ_DIR)/test_catch.o -s
#	@echo "done target $@"

$(BIN_DIR)/test_catch:
	$(CXX) -o $(BIN_DIR)/test_catch test_catch.cpp -s
	@echo "done target $@"

# -s option: also shows successful test results
test: $(BIN_DIR)/test_catch
	$(BIN_DIR)/test_catch

