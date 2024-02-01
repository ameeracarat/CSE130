CC     = clang
CFLAGS = -Wall -Wextra -Werror -Wstrict-prototypes -pedantic

all: memory

memory: memory.o
	$(CC) $(CFLAGS) -o memory memory.o

memory.o: memory.c
	$(CC) $(CFLAGS) -c memory.c


format:
	clang-format -i -style=file *.[ch]


clean:
	rm -f *.o memory
