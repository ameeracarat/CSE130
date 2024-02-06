CC = clang
CFLAGS = -Wall -Wextra -Wstrict-prototypes -Werror -pedantic

LIB = asgn2_helper_funcs.a

.PHONY: all clean format
all: httpserver

httpserver: httpserver.o $(LIB)
	$(CC) $(CFLAGS) $^ -o $@

httpserver.o: httpserver.c asgn2_helper_funcs.h
	$(CC) $(CFLAGS) -c $<

format: 
	clang-format -i -style=file *.[ch]

clean:
	rm -f httpserver *.o
