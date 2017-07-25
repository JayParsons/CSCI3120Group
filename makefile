.PHONY: help
help:

sws: sws.o Priority_Heap.o network.o
	gcc -std=c99 -lpthread -o sws sws.o Priority_Heap.o network.o

sws.o: sws.c Priority_Heap.h network.h
	gcc -std=c99 -c sws.c

Priority_Heap.o: Priority_Heap.c Priority_Heap.h
	gcc -std=c99 -c Priority_Heap.c

network.o: network.c network.h
	gcc -std=c99 -c network.c

main: main.o Priority_Heap.o
	gcc -std=c99 -o main main.o Priority_Heap.o

main.o: main.c Priority_Heap.h
	gcc -std=c99 -c main.c

clean:
	rm *.o sws