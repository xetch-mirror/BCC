.PHONY: all run test clean

CC        = gcc
CFLAGS    = -O2 -std=c2x -march=native
TARGET    = bcc
TEST_TARGET = test_program
BUILD_DIR = build
TARGET_BIN = $(BUILD_DIR)/$(TARGET)
TEST_BIN = $(BUILD_DIR)/$(TEST_TARGET)

SRCS      = c4.c
OBJS      = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))

all: $(TARGET_BIN)

# Create the build directory if it doesn't exist.
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Rule to compile .c files into .o files inside build/.
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link the working C4 interpreter. The loader and preprocessor are not
# integrated into the interpreter and are intentionally not linked here.
$(TARGET_BIN): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

# Run the executable with hello.c.
run: $(TARGET_BIN)
	./$(TARGET_BIN) hello.c

# Compile, link, and run the GCC test program.
test: $(TEST_BIN)
	./$(TEST_BIN)


$(TEST_BIN): test_program.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Clean build artifacts.
clean:
	rm -rf $(BUILD_DIR)
