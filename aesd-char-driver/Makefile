CC?=${CROSS_COMPILE}gcc
CFLAGS?=-Wall -Werror
TARGET?=aesdsocket
LDFLAGS?= -pthread -lrt

default:
	 ${CC} ${CFLAGS} ${LDFLAGS} -o ${TARGET} ${TARGET}.c
all:
	${CC} ${CFLAGS} ${LDFLAGS} -o ${TARGET} ${TARGET}.c
clean:
	rm -rf aesdsocket
