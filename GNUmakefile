ROOT_DIR := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
ALL_TARGETS := all base clean
MAKE_FILE := Makefile

DEFAULT_BUILD_DIR := build.cmake
BUILD_DIR := $(shell if [ -f $(MAKE_FILE) ]; then echo "."; else echo $(DEFAULT_BUILD_DIR); fi)

.PHONY: $(ALL_TARGETS)

all: base
	make -C $(BUILD_DIR) -f Makefile

base:
	mkdir -p $(BUILD_DIR)

ifeq ($(DEBUG),y)
	cd $(BUILD_DIR) && cmake -D CMAKE_BUILD_TYPE=Debug $(ROOT_DIR)
else ifneq ("${Workflow_DIR}workflow", "workflow")
	cd $(BUILD_DIR) && cmake -DWorkflow_DIR:STRING=${Workflow_DIR} $(ROOT_DIR)
else
	cd $(BUILD_DIR) && cmake $(ROOT_DIR)
endif

clean:
	rm -rf $(DEFAULT_BUILD_DIR)
	rm -rf _include
	rm -rf _lib

