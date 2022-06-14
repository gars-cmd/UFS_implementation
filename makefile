

all : mymkfs.o main.o main

mymkfs.o: mymkfs.c
	gcc -c mymkfs.c

main.o: main.c
	gcc -c main.c

main: main.o mymkfs.o
	gcc -o test main.o mymkfs.o

.PHONY: clean all

clean:
	rm -f *.o test fs_data 
