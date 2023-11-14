# Adapted from https://makefiletutorial.com/#makefile-cookbook
CC        := clang
CFLAGS    := -Wall -Wextra -g -O0
LDFLAGS   :=

SRC_DIR   := ./src
BUILD_DIR := ./build
EXEC      := tasiadb

SRCS      := $(shell find $(SRC_DIR) -name '*.c')
OBJS      := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

$(BUILD_DIR)/bin/$(EXEC): $(OBJS)
	mkdir -p $(dir $@)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean run
clean:
	rm -r $(BUILD_DIR)

run: $(BUILD_DIR)/bin/$(EXEC)
	./$(BUILD_DIR)/bin/$(EXEC)