

CC = "${CROSS_COMPILE}gcc"
CFLAGS = -g -Wall

# The target is the executable file
build: writer.c
	$(CC) $(CFLAGS) -o writer writer.c

# A common "clean" target to remove generated files
clean:
	rm -f writer
