CC ?= $(CROSS_COMPILE)gcc

CFLAGS ?= -O0 -Wall -Werror -g $(INCLUDE_DIRS) $(CDEFS)
LDFLAGS ?= -lpthread -lrt

SRC := aesdsocket.c

all: aesdsocket
default: aesdsocket
TARGET: ?= aesdsocket

clean:
	-rm -f *.o *.d
	-rm -f aesdsocket

aesdsocket: aesdsocket.c
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $<

