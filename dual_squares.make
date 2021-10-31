DUAL_SQUARES_C_FLAGS=-c -O2 -Wall -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winline -Wlong-long -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wno-import -Wpointer-arith -Wredundant-decls -Wshadow -Wstrict-prototypes -Wwrite-strings

dual_squares: dual_squares.o
	gcc -o dual_squares dual_squares.o

dual_squares.o: dual_squares.c dual_squares.make
	gcc ${DUAL_SQUARES_C_FLAGS} -o dual_squares.o dual_squares.c

clean:
	rm -f dual_squares dual_squares.o
