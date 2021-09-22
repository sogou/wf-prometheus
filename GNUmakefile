ROOT_DIR := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
ALL_TARGETS := all base clean
MAKE_FILE := Makefile

DEFAULT_BUILD_DIR := build.cmake
BUILD_DIR := $(shell if [ -f $(MAKE_FILE) ]; then echo "."; else echo $(DEFAULT_BUILD_DIR); fi)
CMAKE3 := $(shell if which cmake3>/dev/null ; then echo cmake3; else echo cmake; fi;)

.PHONY: $(ALL_TARGETS)

all: base
	make -C $(BUILD_DIR) -f Makefile

base:
	mkdir -p $(BUILD_DIR)

ifeq ($(DEBUG),y)
	cd $(BUILD_DIR) && $(CMAKE3) -D CMAKE_BUILD_TYPE=Debug $(ROOT_DIR)
else
	cd $(BUILD_DIR) && $(CMAKE3) $(ROOT_DIR)
endif

clean:
	rm -rf $(DEFAULT_BUILD_DIR)
	rm -rf _include
	rm -rf _lib

