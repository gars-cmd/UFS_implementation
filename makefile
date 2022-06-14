FLAG = -Wall -g
FPC = -fPIC 

all : mymkfs.o main.o main libmyfs.so

mymkfs.o: mymkfs.c
	gcc $(FLAG)  -c mymkfs.c

main.o: main.c
	gcc $(FLAG) -c main.c

main: main.o mymkfs.o
	gcc $(FLAG) -o test main.o mymkfs.o

libmyfs.so: mymkfs.c
	gcc $(FPC) $(FLAG) mymkfs.c -shared -o libmyfs.so 

.PHONY: clean all

clean:
	rm -f *.o test fs_data libmyfs.so
