CC = gcc
CFLAGS = -Wall

all: sim

sim: src/main.o src/scheduler.o
	$(CC) src/main.o src/scheduler.o -o sim

src/main.o: src/main.c src/scheduler.h
	$(CC) $(CFLAGS) -c src/main.c -o src/main.o

src/scheduler.o: src/scheduler.c src/scheduler.h
	$(CC) $(CFLAGS) -c src/scheduler.c -o src/scheduler.o

clean:
	rm -f src/*.o sim
