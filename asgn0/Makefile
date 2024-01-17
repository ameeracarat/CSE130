CC     = clang
CFLAGS = -Wall -Wextra -Werror -Wstrict-prototypes -pedantic

all: hello

hello: hello.o
	$(CC) $(CFLAGS) -o hello hello.o

hello.o: hello.c
	$(CC) $(CFLAGS) -c hello.c


format:
	clang-format -i -style=file *.[ch]


clean:
	rm -f *.o hello
