//Author: Siddharth Deokar
//Weighted K Nearest Neighbor Algorithms
//Date: 05/05/2009

// 1. Number of attributes includes the class attribute
// 2. Class attribute must be the last attribute and 
//    the values should be integers only.
// 3. Update NO_OF_ATT and NO_OF_CLASSES on changing
//    the data set

/*-------------------------------------------------*/
//Uncomment Parameters according the data set using.

//The Heart Data Set
#define NO_OF_ATT 14 
#define NO_OF_CLASSES 2

//The Hill Valley Data Set
//#define NO_OF_ATT 101 
//#define NO_OF_CLASSES 2 

//Wine Data Set
//#define NO_OF_ATT 14 
//#define NO_OF_CLASSES 3 

//Heart-1 Data Set
//#define NO_OF_ATT 14 
//#define NO_OF_CLASSES 5 
/*-------------------------------------------------*/

/* The number of nearest neighbors to use. */
#define K 3
#define LEARNING_RATE 0.005

#define LINE_MAX 10000
#define uint	unsigned int

enum MODE
{
	TRAINING = 0,
	TESTING,
	VALIDATING
};

//Backward Elimination
bool isBEAttIncluded[NO_OF_ATT - 1];

//Attribute Weighting
double attWeights[NO_OF_ATT - 1];

class TrainingExample
{
public:
	//Unique Index
	uint index;
	//Values of all Attributes for a instance
	double Value [NO_OF_ATT];
	//Euclidean Distance
	double Distance;
	//Instance Weight
	double Weight;
	//Is the instance near to anyone
	bool isNearest2AtleastSome;

	TrainingExample()
	{
		for(int i = 0; i < NO_OF_ATT; i++)
			Value[i] = 0.0;
		Distance = 0.0;
		Weight = 0.0;
		index = 0;
		isNearest2AtleastSome = false;
	}

	TrainingExample(double *a)
	{
		for(int i = 0; i < NO_OF_ATT; i++)
			Value[i] = a[i];
		Distance = 0.0;
		Weight = 0.0;
		index = 0;
		isNearest2AtleastSome = false;
	}

	~TrainingExample()
	{
		for(int i = 0; i < NO_OF_ATT; i++)
			Value[i] = 0.0;
		Distance = 0.0;
		Weight = 0.0;
		index = 0;
	}

	void SetVal(double *a)
	{
		for(int i = 0; i < NO_OF_ATT; i++)
			Value[i] = a[i];
	}

	void GetVal(double *a)
	{
		for(int i = 0; i < NO_OF_ATT; i++)
			a[i] = Value[i];
	}

	//Not using this normalization anymore
	//Using Standard Deviation instead
	void NormalizeVals ()
	{
		for (int i = 0; i < NO_OF_ATT - 1; i++)
		{
			Value[i] = Value[i] / (1.0 + Value[i]);
		}
	}

};
