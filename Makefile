TARGET          := imk
CXX             ?= g++
BUILD_HOST      := build_host.hpp
SRC             != (ls *.cpp compat/poll_bsd.cpp || true)
OBJ             := $(SRC:.cpp=.o)
INSTALL         := install
INSTALL_ARGS    := -o root -g wheel -m 755
INSTALL_DIR     := /usr/local/bin/

INCLUDES        :=
LIBS            :=
CXXFLAGS        := -Wall $(INCLUDES)
LFLAGS          := $(LIBS)

.if $(CXX) == g++ || $(CXX) == clang++ || $(CXX) == c++ || $(CXX) == eg++
    CXXFLAGS := -std=c++11 -pedantic
.endif

.if make(release) || make(install)
    CXXFLAGS += -O3
.elif make(static)
    CXXFLAGS += -static
    LFLAGS += -static
.else # debug
    CXXFLAGS += -g -ggdb -DDEBUG
    LFLAGS += -g
.endif

all: debug
debug: build
release: clean build
	strip $(TARGET)

static: release

build: $(BUILD_HOST) $(TARGET)

$(BUILD_HOST):
	@echo "#define BUILD_HOST \"`hostname`\""      > $(BUILD_HOST)
	@echo "#define BUILD_OS \"`uname`\""          >> $(BUILD_HOST)
	@echo "#define BUILD_PLATFORM \"`uname -m`\"" >> $(BUILD_HOST)
	@echo "#define BUILD_KERNEL \"`uname -r`\""   >> $(BUILD_HOST)

$(TARGET): $(BUILD_HOST) $(OBJ)
	$(CXX) $(LFLAGS) -o $@ $(OBJ)

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
