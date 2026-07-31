CC      = gcc
CFLAGS  = -std=c23 -Wall -Wextra -O3 -march=native
TARGET  = bcc

SRCS    = c4.c load.c
OBJS    = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(OBJS) $(TARGET)

.PHONY: all clean run