SRC_DIR := src
BUILD_DIR := build
BIN_DIR := bin
DIST_DIR := dist
TOOLS_DIR := tools
SHARE_DIR := share
INCLUDE_DIR := include

CC := gcc
PYTHON := python3
CFLAGS := -Wall -Wextra -std=c99 -I$(INCLUDE_DIR) -I$(BUILD_DIR) -MMD -MP
TEST_CFLAGS := -Wall -Wextra -std=c99 -I$(INCLUDE_DIR)

COMMON_SRCS := \
	$(SRC_DIR)/show.c \
	$(SRC_DIR)/api_catalog.c \
	$(SRC_DIR)/paixu_search.c \
	$(SRC_DIR)/slist.c \
	$(SRC_DIR)/slist_while.c \
	$(SRC_DIR)/dlist.c \
	$(SRC_DIR)/dlist_while.c \
	$(SRC_DIR)/tree_bst.c

APP_SRCS := \
	$(SRC_DIR)/main.c \
	$(COMMON_SRCS)
APP_OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(APP_SRCS))

SHARE_SRCS := \
	$(SRC_DIR)/share_main.c \
	$(SRC_DIR)/show.c \
	$(SRC_DIR)/api_catalog.c
SHARE_OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SHARE_SRCS))

SHARE_ASSETS_SRC := $(BUILD_DIR)/share_assets.c
SHARE_ASSETS_HDR := $(BUILD_DIR)/share_assets.h
SHARE_ASSETS_OBJ := $(BUILD_DIR)/share_assets.o

DEPS := $(APP_OBJS:.o=.d) $(SHARE_OBJS:.o=.d) $(SHARE_ASSETS_OBJ:.o=.d)

APP := $(BIN_DIR)/listandtree_lab
TEST_LISTS := $(BIN_DIR)/test_lists
TEST_BST := $(BIN_DIR)/test_bst
TEST_ALGORITHMS := $(BIN_DIR)/test_algorithms
SHARE_APP := $(DIST_DIR)/listandtree.out

SHARE_INPUTS := \
	$(TOOLS_DIR)/build_share_assets.py \
	$(SHARE_DIR)/export_makefile \
	$(SHARE_DIR)/user_makefile \
	$(SRC_DIR)/api_catalog.inc \
	$(SRC_DIR)/listandtree_export.h \
	$(SRC_DIR)/paixu_search.c \
	$(SRC_DIR)/slist.c \
	$(SRC_DIR)/slist_while.c \
	$(SRC_DIR)/dlist.c \
	$(SRC_DIR)/dlist_while.c \
	$(SRC_DIR)/tree_bst.c \
	$(INCLUDE_DIR)/paixu_search.h \
	$(INCLUDE_DIR)/slist.h \
	$(INCLUDE_DIR)/slist_while.h \
	$(INCLUDE_DIR)/dlist.h \
	$(INCLUDE_DIR)/dlist_while.h \
	$(INCLUDE_DIR)/tree_bst.h \
	$(INCLUDE_DIR)/listandtree.h

VALGRIND ?= valgrind

.PHONY: all clean rebuild run test valgrind share

all: $(APP)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR) $(DIST_DIR) *.o

rebuild: clean all

run: $(APP)
	./$(APP)

test: $(TEST_LISTS) $(TEST_BST) $(TEST_ALGORITHMS)
	./$(TEST_LISTS)
	./$(TEST_BST)
	./$(TEST_ALGORITHMS)

share: $(SHARE_APP)

$(APP): $(APP_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(TEST_LISTS): tests/test_lists.c $(SRC_DIR)/slist.c $(SRC_DIR)/slist_while.c $(SRC_DIR)/dlist.c $(SRC_DIR)/dlist_while.c | $(BIN_DIR)
	$(CC) $(TEST_CFLAGS) $^ -o $@

$(TEST_BST): tests/test_bst.c $(SRC_DIR)/tree_bst.c | $(BIN_DIR)
	$(CC) $(TEST_CFLAGS) $^ -o $@

$(TEST_ALGORITHMS): tests/test_algorithms.c $(SRC_DIR)/paixu_search.c | $(BIN_DIR)
	$(CC) $(TEST_CFLAGS) $^ -o $@

valgrind: $(TEST_LISTS) $(TEST_BST) $(TEST_ALGORITHMS)
	@if ! command -v $(VALGRIND) >/dev/null 2>&1; then \
		echo "valgrind not found, skipping."; \
	else \
		$(VALGRIND) --leak-check=full --error-exitcode=1 ./$(TEST_LISTS) && \
		$(VALGRIND) --leak-check=full --error-exitcode=1 ./$(TEST_BST) && \
		$(VALGRIND) --leak-check=full --error-exitcode=1 ./$(TEST_ALGORITHMS); \
	fi

$(SHARE_APP): $(SHARE_OBJS) $(SHARE_ASSETS_OBJ) | $(DIST_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(SHARE_ASSETS_SRC) $(SHARE_ASSETS_HDR): $(SHARE_INPUTS)
	mkdir -p $(BUILD_DIR)
	$(PYTHON) $(TOOLS_DIR)/build_share_assets.py $(SHARE_ASSETS_SRC) $(SHARE_ASSETS_HDR)

$(SHARE_ASSETS_OBJ): $(SHARE_ASSETS_SRC) $(SHARE_ASSETS_HDR) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $(SHARE_ASSETS_SRC) -o $@

$(BUILD_DIR)/share_main.o: $(SHARE_ASSETS_HDR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(DIST_DIR):
	mkdir -p $(DIST_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

-include $(DEPS)
