test: my_malloc.o test.o
	gcc -o test my_malloc.o test.c
test.o: test.c my_malloc.h 
	gcc -c my_malloc.c test.c -pedantic -std=gnu99 -Wall -Werror
my_malloc.o : my_malloc.c my_malloc.c
	gcc -c my_malloc.c -pedantic -std=gnu99 -Wall -Werror
clean: 
	rm -f *~ *.o 
