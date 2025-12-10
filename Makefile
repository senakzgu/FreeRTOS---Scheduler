hepsi: derle calistir

derle: Scheduler Tasks Main
	gcc ./lib/scheduler.o ./lib/task.o ./lib/main.o -o ./bin/program -pthread

Scheduler:
	gcc -I "./include" -c ./src/scheduler.c -o ./lib/scheduler.o

Tasks:
	gcc -I "./include" -c ./src/task.c -o ./lib/task.o

Main:
	gcc -I "./include" -c ./src/main.c -o ./lib/main.o

calistir:
	./bin/program giris.txt

clean:
	rm -f ./lib/*.o ./bin/program