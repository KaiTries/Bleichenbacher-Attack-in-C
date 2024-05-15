# Compiler
CC = cc

# Compiler flags
CFLAGS = -I /opt/homebrew/include

# Linker flags
LDFLAGS = -L /opt/homebrew/lib -lgmp

# Source files
SRCS = main.c interval.c bleichenbacher.c rsa.c

# Object files
OBJS = $(SRCS:.c=.o)

# Executable name
TARGET = main

# Default rule
all: $(TARGET) clean_objs

# Rule to link the executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Rule to compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to remove only object files after building the target
clean_objs:
	rm -f $(OBJS)

# Clean rule to remove object files and the executable
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
