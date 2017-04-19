mythread.a: mythread.o Queue.o
	ar rcs mythread.a mythread.o Queue.o

mythread.o: mythread.c mythread.h Queue.c Queue.h
	gcc -c -O2 mythread.c -o mythread.o

Queue.o: Queue.c Queue.h
	gcc -c -O2 Queue.c -o Queue.o