# Compiler
CC = cc

# Compiler flags
CFLAGS = -I /opt/homebrew/include

# Linker flags
LDFLAGS = -L /opt/homebrew/lib -lgmp

# Source files for the main program
SRCS = main.c interval.c bleichenbacher.c rsa.c

# Source files for the test program (including tests/test.c instead of main.c)
TEST_SRCS = interval.c bleichenbacher.c rsa.c tests/test.c

# Object files for the main program
OBJS = $(SRCS:.c=.o)

# Object files for the test program
TEST_OBJS = $(TEST_SRCS:.c=.o)

# Executable names
TARGET = main
TEST_TARGET = tests/test

# Default rule
all: $(TARGET) clean_objs

# Rule to link the main executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Rule to link the test executable
$(TEST_TARGET): $(TEST_OBJS)
	$(CC) $(TEST_OBJS) -o $(TEST_TARGET) $(LDFLAGS)

# Rule to compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to remove only object files after building the target
clean_objs:
	rm -f $(OBJS) $(TEST_OBJS)

# Clean rule to remove object files and executables
clean:
	rm -f $(OBJS) $(TEST_OBJS) $(TARGET) $(TEST_TARGET)

# Rule to build and run tests
tests: $(TEST_TARGET) clean_objs

.PHONY: all clean clean_objs tests
