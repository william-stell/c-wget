COMPILER = gcc
FLAGS = -Wall -pedantic -std=gnu99

COMPILE = $(COMPILER) $(FLAGS)

OBJECTS = main.o networking.o

PROGRAM_NAME = webget

all: $(OBJECTS)
	$(COMPILE) $(OBJECTS) -o $(PROGRAM_NAME)


main.o: main.c
	$(COMPILE) main.c -c

networking.o: networking.c networking.h
	$(COMPILE) networking.c -c


clean: 
	rm *.o
	rm $(PROGRAM_NAME)
