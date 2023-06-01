# Makefile for writer.c
#
CC := gcc
CROSS_COMPILE := 

ifeq ($(CROSS_COMPILE),aarch64-none-linux-gnu-)
	CC := $(CROSS_COMPILE)gcc
endif

TARGET := writer
SRCS := writer.c
OBJS := $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJS)

