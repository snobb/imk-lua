TARGET          := imk
CXX             ?= g++
BUILD_HOST      := build_host.hpp
SRC             := $(wildcard *.cpp)
OBJ             := $(SRC:.cpp=.o)
OS              := $(shell uname -s)
LUA_VERSION     := 5.2
GRP             := root

INSTALL         := install
INSTALL_ARGS    := -o root -g $(GRP) -m 755
INSTALL_DIR     := /usr/local/bin/

CXXFLAGS        := -Wall $(shell pkg-config --cflags lua$(LUA_VERSION))
LDFLAGS         := $(shell pkg-config --libs lua$(LUA_VERSION))

ifeq ($(CXX), $(filter $(CXX), clang++ g++ c++ eg++))
    CXXFLAGS += -std=c++11 -pedantic
endif

all: debug

debug: CXXFLAGS += -g -ggdb -DDEBUG
debug: LDFLAGS += -g
debug: build

release: CXXFLAGS += -O3
release: clean build
	strip $(TARGET)

static: CXXFLAGS += -static
static: LDFLAGS += -static
static: release

build: $(BUILD_HOST) $(TARGET)

$(BUILD_HOST):
	@echo "#define BUILD_HOST \"`hostname`\""      > $(BUILD_HOST)
	@echo "#define BUILD_OS \"`uname`\""          >> $(BUILD_HOST)
	@echo "#define BUILD_PLATFORM \"`uname -m`\"" >> $(BUILD_HOST)
	@echo "#define BUILD_KERNEL \"`uname -r`\""   >> $(BUILD_HOST)

$(TARGET): $(BUILD_HOST) $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $(OBJ)

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

install: release
	$(INSTALL) $(INSTALL_ARGS) $(TARGET) $(INSTALL_DIR)
	@echo "DONE"

clean:
	-rm -f *.core
	-rm -f $(BUILD_HOST)
	-rm -f $(TARGET)
	-rm -f *.o compat/*.o

.PHONY : all debug release static build install clean
