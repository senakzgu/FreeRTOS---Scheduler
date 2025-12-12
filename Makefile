CC = gcc
CFLAGS = -Wall

TARGET = freertos_sim.exe

all: $(TARGET)

$(TARGET): src/main.o src/scheduler.o
	$(CC) src/main.o src/scheduler.o -o $(TARGET)

src/main.o: src/main.c src/scheduler.h
	$(CC) $(CFLAGS) -c src/main.c -o src/main.o

src/scheduler.o: src/scheduler.c src/scheduler.h
	$(CC) $(CFLAGS) -c src/scheduler.c -o src/scheduler.o

clean:
	del /Q src\*.o 2>nul
	del /Q $(TARGET) 2>nul
