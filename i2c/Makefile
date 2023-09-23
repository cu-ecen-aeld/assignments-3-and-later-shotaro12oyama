CC=gcc

all: i2cioctl
default: i2cioctl

i2cioctl: i2cioctl.o
	$(CROSS_COMPILE)$(CC) i2cioctl.o -o i2cioctl

i2cioctl.o: i2cioctl.c
	$(CROSS_COMPILE)$(CC) -g -c i2cioctl.c -o i2cioctl.o -Wall -Werror

clean:
	rm -f *.o i2cioctl *.elf *.map
