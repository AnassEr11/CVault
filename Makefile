CC ?= gcc
CFLAGS = -Wall \
		 -Wextra \
		 -O2 \
		 -fPIC \
		 -Iinclude \
		 -Iinclude/vendor
LDFLAGS = -lssl -lcrypto -lcurl -lpthread -ldl
SRC_DIR = src
OBJ_DIR = obj
BIN_NAME = execute_CVault
TEST_SRC_DIR = test/src
TEST_EXEC_DIR = test/exec
CORE_SRCS = $(wildcard src/core/*.c) \
			$(wildcard src/utils/*.c) \
			$(wildcard src/vendor/*/*.c) \
			$(wildcard src/vendor/argon2/blake2/*.c)
CLI_SRCS  = src/main.c $(wildcard src/cli/*.c)
TEST_SRCS = $(wildcard $(TEST_SRC_DIR)/*.c)
CORE_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(CORE_SRCS))
CLI_OBJS  = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(CLI_SRCS))
TEST_BINS = $(patsubst $(TEST_SRC_DIR)/%.c, $(TEST_EXEC_DIR)/%, $(TEST_SRCS))

.PHONY: all clean dirs cli tests run_tests

all: dirs cli

dirs:
	@mkdir -p $(OBJ_DIR)/core
	@mkdir -p $(OBJ_DIR)/utils
	@mkdir -p $(OBJ_DIR)/cli
	@mkdir -p $(OBJ_DIR)/vendor/sqlite3
	@mkdir -p $(OBJ_DIR)/vendor/argon2/blake2
	@mkdir -p $(TEST_EXEC_DIR)

cli: $(BIN_NAME)

$(BIN_NAME): $(CORE_OBJS) $(CLI_OBJS)
	@echo "Linking CLI Executable..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_EXEC_DIR)/%: $(TEST_SRC_DIR)/%.c $(CORE_OBJS)
	@echo "Compiling Test: $@"
	$(CC) $(CFLAGS) $< $(CORE_OBJS) -o $@ $(LDFLAGS)

tests: dirs $(TEST_BINS)

run_tests: tests
	@echo "RUNNING ALL TESTS..."
	@for test_bin in $(TEST_BINS); do \
		echo "Running $$test_bin ..."; \
		./$$test_bin || exit 1; \
	done

clean:
	@echo "Cleaning up..."
	rm -rf $(OBJ_DIR) $(TEST_EXEC_DIR) $(BIN_NAME)

