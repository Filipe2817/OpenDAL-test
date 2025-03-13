# Direct Wildcard
dwildcard = $(wildcard $(1)/$(2))

# Recursive Wildcard
rwildcard = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

# Makefile Settings
MAKEFLAGS += --no-print-directory
.DEFAULT_GOAL := help

# Pretty stuff (using %b in printf to interpret escape sequences as special characters)
NO_COLOR = \033[m
YELLOW_COLOR = \033[0;33m
RED_COLOR = \033[0;31m
GREEN_COLOR = \033[0;32m
DELETING_STRING = "[DELETING]"
COMPILING_STRING = "[COMPILING]"
LINKING_STRING = "[LINKING]"
ERROR_STRING = "[ERROR]"
INFO_STRING = "[INFO]"
OK_STRING = "[OK]"

# Load .env file
ifneq (,$(wildcard ../.env))
    include ../.env
    export
endif

# Compiler settings
# Default build as release (override with 'make BUILD=debug' or 'make BUILD=sanitizer')
BUILD = release
FLAGS.release = -Wall -Wextra -pedantic-errors -O3
FLAGS.sanitizer = -Wall -Wextra -pedantic-errors -g -fsanitize=address
FLAGS.debug = -Wall -Wextra -pedantic-errors -O0 -g
CC = gcc
CFLAGS = $(FLAGS.$(BUILD))
OPENDAL_INCLUDE = -I$(OPENDAL_PATH)/bindings/c/include
OPENDAL_LIB_PATH = -L$(OPENDAL_PATH)/bindings/c/target/debug
OPENDAL_LIBS = -lopendal_c
LDFLAGS = -Wl,-rpath=$(OPENDAL_PATH)/bindings/c/target/debug # Embed library path

# Build settings
BUILD_DIR := build
SIMPLE_DIRS := random_tests demos function_examples
PROJECT_DIRS :=
ALLOWED_DIRS := $(SIMPLE_DIRS) $(PROJECT_DIRS)

.PHONY: $(SIMPLE_DIRS) $(PROJECT_DIRS) help build-opendal clean-opendal clean

# Rules
################################

define simple_dir
$(1)_SOURCES := $$(call dwildcard,$(1),*.c)
$(1)_TARGETS := $$($(1)_SOURCES:%.c=$$(BUILD_DIR)/%)

$$($(1)_TARGETS): $$(BUILD_DIR)/%: %.c
	@mkdir -p $$(dir $$@)
	@printf "%b" "$$(YELLOW_COLOR)$$(COMPILING_STRING)$$(NO_COLOR) $$<\n"
	@$$(CC) $$(CFLAGS) $$(OPENDAL_INCLUDE) $$(OPENDAL_LIB_PATH) $$(LDFLAGS) $$< $$(OPENDAL_LIBS) -o $$@

$(1): $$(BUILD_DIR) $$($(1)_TARGETS)
	@printf "%b" "$$(GREEN_COLOR)$$(OK_STRING)$$(NO_COLOR) Built $(1) with $$(BUILD) configuration\n"
	@printf "%b" "$$(YELLOW_COLOR)$$(INFO_STRING)$$(NO_COLOR) Use './build/$(1)/<exec_name>' to run\n"
endef

$(foreach dir,$(SIMPLE_DIRS),$(eval $(call simple_dir,$(dir))))

################################

define project_dir
$(1)_SOURCES := $$(call rwildcard,$(1)/src,*.c)
$(1)_HEADERS := $$(call rwildcard,$(1)/include,*.h)
$(1)_OBJECTS := $$($(1)_SOURCES:$(1)/src/%.c=$$(BUILD_DIR)/$(1)/objs/%.o)
$(1)_TARGET := $$(BUILD_DIR)/$(1)/$(1)

$$(BUILD_DIR)/$(1)/objs/%.o: $(1)/src/%.c $$($(1)_HEADERS)
	@mkdir -p $$(dir $$@)
	@printf "%b" "$$(YELLOW_COLOR)$$(COMPILING_STRING)$$(NO_COLOR) $$<\n"
	@$$(CC) $$(CFLAGS) $$(OPENDAL_INCLUDE) -I$(1)/include -c $$< -o $$@

$$($(1)_TARGET): $$($(1)_OBJECTS)
	@mkdir -p $$(dir $$@)
	@printf "%b" "$$(YELLOW_COLOR)$$(LINKING_STRING)$$(NO_COLOR) $$@\n"
	@$$(CC) $$(CFLAGS) $$^ $$(OPENDAL_LIB_PATH) $$(LDFLAGS) $$(OPENDAL_LIBS) -o $$@
	@printf "%b" "$$(GREEN_COLOR)$$(OK_STRING)$$(NO_COLOR) Built $(1) with $$(BUILD) configuration\n"
	@printf "%b" "$$(YELLOW_COLOR)$$(INFO_STRING)$$(NO_COLOR) Use './$$($(1)_TARGET)' to run\n"

$(1): $$($(1)_TARGET)
endef

$(foreach dir,$(PROJECT_DIRS),$(eval $(call project_dir,$(dir))))

################################

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

help:
	@printf "%b" "$(YELLOW_COLOR)$(INFO_STRING)$(NO_COLOR) Available commands:\n"
	@printf "    $(YELLOW_COLOR)make help$(NO_COLOR)               - Show this help message\n"
	@printf "    $(YELLOW_COLOR)make build-opendal$(NO_COLOR)      - Build OpenDAL library\n"
	@printf "    $(YELLOW_COLOR)make clean-opendal$(NO_COLOR)      - Remove OpenDAL library\n"
	@printf "    $(YELLOW_COLOR)make clean$(NO_COLOR)              - Remove build directory\n"
	@printf "    $(YELLOW_COLOR)make <dir>$(NO_COLOR)              - Build specific directory (allowed: $(ALLOWED_DIRS))\n"

build-opendal:
	@printf "%b" "$(YELLOW_COLOR)$(INFO_STRING)$(NO_COLOR) Building OpenDAL...\n"
	@./scripts/build_opendal.sh build

clean-opendal:
	@printf "%b" "$(RED_COLOR)$(DELETING_STRING)$(NO_COLOR) OpenDAL build...\n"
	@./scripts/build_opendal.sh clean

clean:
	@printf "%b" "$(RED_COLOR)$(DELETING_STRING) $(NO_COLOR)$(BUILD_DIR) directory\n"
	@rm -rf $(BUILD_DIR)
