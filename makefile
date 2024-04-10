CC = gcc
CFLAGS = -Wall -pedantic -ansi
FILENAME = ps

Assembler: dataImage.o  firstPass.o inputAnalyze.o assembler.o printFunctions.o secondPass.o labelTableLinkedList.o
	$(CC) -g dataImage.o firstPass.o inputAnalyze.o assembler.o printFunctions.o secondPass.o labelTableLinkedList.o -o Assembler -lm

dataImage.o: dataImage.c dataImage.h defaults.h
	$(CC) -c dataImage.c $(CFLAGS) -o dataImage.o

firstPass.o: firstPass.c firstPass.h defaults.h inputAnalyze.h labelTableLinkedList.h dataImage.h printFunctions.h
	$(CC) -c firstPass.c $(CFLAGS) -o firstPass.o

inputAnalyze.o: inputAnalyze.c inputAnalyze.h defaults.h
	$(CC) -c inputAnalyze.c $(CFLAGS) -o inputAnalyze.o

assembler.o: assembler.c labelTableLinkedList.h firstPass.h secondPass.h
	$(CC) -c assembler.c $(CFLAGS) -o assembler.o

printFunctions.o: printFunctions.c printFunctions.h defaults.h labelTableLinkedList.h dataImage.h
	$(CC) -c printFunctions.c $(CFLAGS) -o printFunctions.o

secondPass.o: secondPass.c secondPass.h defaults.h firstPass.h inputAnalyze.h labelTableLinkedList.h printFunctions.h dataImage.h
	$(CC) -c secondPass.c $(CFLAGS) -o secondPass.o

labelTableLinkedList.o: labelTableLinkedList.c labelTableLinkedList.h defaults.h
	$(CC) -c labelTableLinkedList.c $(CFLAGS) -o labelTableLinkedList.o

clean:
	rm -f *.o 

run: Assembler clean
	./Assembler $(FILENAME)

