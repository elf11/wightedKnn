CC=g++
CFLAGS=-c -Wall -Wextra
LDFLAGS=-lm

all: WeightedKNN WeightedKNN_omp

WeightedKNN_omp: WeightedKNN_omp.o
	$(CC) $(LDFLAGS) WeightedKNN_omp.o -o WeightedKNN_omp -fopenmp

WeightedKNN_omp.o: WeightedKNN_omp.cpp
	$(CC) $(CFLAGS) WeightedKNN_omp.cpp -fopenmp

WeightedKNN: WeightedKNN.o
	$(CC) $(LDFLAGS) WeightedKNN.o -o WeightedKNN

WeightedKNN.o: WeightedKNN.cpp
	$(CC) $(CFLAGS) WeightedKNN.cpp

clean:
	rm -rf *.o WeightedKNN WeightedKNN_omp
	

