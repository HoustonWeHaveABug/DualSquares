ERICKSON_MATRIX_C_FLAGS=-O2 -Wall -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winline -Wlong-long -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wno-import -Wpointer-arith -Wredundant-decls -Wshadow -Wstrict-prototypes -Wwrite-strings

erickson_matrix: erickson_matrix.o
	gcc -o erickson_matrix erickson_matrix.o

erickson_matrix.o: erickson_matrix.c erickson_matrix.make
	gcc -c ${ERICKSON_MATRIX_C_FLAGS} -o erickson_matrix.o erickson_matrix.c

clean:
	rm -f erickson_matrix erickson_matrix.o
