CCFLAGS = -Wall -Wextra -pedantic -std=c99 -O3 -march=native

all: collatz

collatz: collatz.c
	gcc ${CCFLAGS} collatz.c -o collatz

clean:: 
	rm collatz

distclean:: clean
