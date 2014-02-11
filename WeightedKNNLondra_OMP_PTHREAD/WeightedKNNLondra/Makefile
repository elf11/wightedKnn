CC=g++
CFLAGS=-g -c -Wall -Wextra
LDFLAGS=-lm
LDFLAGS_pthread=-oterm -lpthread

all: WeightedKNN WeightedKNN_omp WeightedKNN_pthread

WeightedKNN_pthread: WeightedKNN_pthread.o
	$(CC) $(LDFLAGS_pthread) WeightedKNN_pthread.o -o WeightedKNN_pthread

WeightedKNN_pthread.o: WeightedKNN_pthread.cpp
	$(CC) $(CFLAGS) WeightedKNN_pthread.cpp 

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
	

