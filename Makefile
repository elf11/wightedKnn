CFLAGS=-Wall -g

all: weightedKNN

weightedKNN: weightedKNN.o
		g++ $(CFLAGS) weightedKNN.o -o weightedKNN

weightedKNN.o: WeightedKNN.cpp
		g++ $(CFLAGS) -c WeightedKNN.cpp -o weightedKNN.o

clean:
		rm -rf *o weightedKNN

