.phony all:
all: main

main: main.c
	gcc -pthread queue.c main.c -o ACS

.PHONY clean:
clean:
	-rm -rf *.o *.exe
