INCLUDE_DIRS =
LIB_DIRS =
CC ?= $(CROSS_COMPILE)gcc

CDEFS =
CFLAGS ?= -O0 -Wall -Werror -g $(INCLUDE_DIRS) $(CDEFS)
LDFLAGS ?= -pthread -lrt
LIBS =

HFILES =
CFILES = aesdsocket.c

SRCS = ${HFILES} ${CFILES}
OBJS = ${CFILES:.c=.o}

all: aesdsocket
default: aesdsocket
TARGET: ?=aesdsocket

clean:
	-rm -f *.o *.d
	-rm -f aesdsocket

aesdsocket: aesdsocket.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o

.c.o:
	$(CC) $(CFLAGS) -c $<

