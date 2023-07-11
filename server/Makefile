CC ?= $(CROSS_COMPILE)gcc
TARGET ?= aesdsocket
CFLAGS ?= -O0 -Wall -Werror -g $(INCLUDE_DIRS) $(CDEFS)
LDFLAGS ?= -lpthread

SRC := aesdsocket.c

all: aesdsocket
default: aesdsocket

clean:
	-rm -f *.o *.d
	-rm -f aesdsocket

aesdsocket: aesdsocket.c
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $<

