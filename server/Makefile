

CC ?= "${CROSS_COMPILE}gcc"
CFLAGS ?= -g -Wall

# The target is the executable file
all: aesdsocket.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o aesdsocket aesdsocket.c

# A common "clean" target to remove generated files
clean:
	rm -f aesdsocket
