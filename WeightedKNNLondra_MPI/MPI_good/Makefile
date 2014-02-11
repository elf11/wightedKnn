CC=mpic++
CFLAGS=-c -Wall -Wextra
LDFLAGS=-lm

all: WeightedKNN

WeightedKNN: WeightedKNN.o
	$(CC) $(LDFLAGS) WeightedKNN.o -o WeightedKNN

WeightedKNN.o: WeightedKNN.cpp
	$(CC) $(CFLAGS) WeightedKNN.cpp

clean:
	rm -rf *.o WeightedKNN	
