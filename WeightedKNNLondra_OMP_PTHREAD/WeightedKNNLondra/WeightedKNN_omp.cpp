//Author: Siddharth Deokar
//Weighted K Nearest Neighbor Algorithms
//Date: 05/05/2009
#include <cstdio>
#include <cstdlib>
#include <list>
#include <cstring>
#include <iostream>
#include <ctime>
#include <iterator>
#include <vector>
#include <algorithm>
#include <cmath>
#include <omp.h>
#include <parallel/algorithm>
#include "TrainingExample_omp.h"

using namespace std;

/* Using a list to store the training and testing examples. */
typedef vector<TrainingExample, 
			allocator<TrainingExample> > TRAINING_EXAMPLES_LIST;

/* Function Definitions */
/////////////////////////////KNN Algorithms//////////////////////////
/* K Nearest Neighbor Algorithm (All attributes treated equally) */
float SimpleKNN (TRAINING_EXAMPLES_LIST *trainList, 
				 int trainExamples, 
				 TRAINING_EXAMPLES_LIST *testList, 
				 int testExamples);

/* Attribute Weighted K Nearest Neighbor Algorithm */
float AttributeWKNN (TRAINING_EXAMPLES_LIST *trainList, 
					 int trainExamples,
					 TRAINING_EXAMPLES_LIST *testList, 
					 int testExamples);

/* Instance Weighted K Nearest Neighbor Algorithm */
float InstanceWKNN (TRAINING_EXAMPLES_LIST *trainList, 
					int trainExamples, 
					TRAINING_EXAMPLES_LIST *testList, 
					int testExamples);

/* K Nearest Neighbor Algorithm with Backward Elimination */
float BackwardElimination (TRAINING_EXAMPLES_LIST *trainList, 
						   int trainExamples,
						   TRAINING_EXAMPLES_LIST *testList, 
						   int testExamples);
///////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
////////Learning Functions for Instance and Attribute Weighted KNN////////
/* Learning weights by running KNN on training data. */
float LearnWeights (TRAINING_EXAMPLES_LIST *tlist, 
					TRAINING_EXAMPLES_LIST data, 
					int iterations, int numExamples, 
					MODE mode, int desiredAccuracy, 
					bool isAttWeightedKNN);

/* Adjust weights by using Gradient Descent */
void AdjustWeightsByGradientDescent (double *qvalue, 
									 TRAINING_EXAMPLES_LIST *tlist, 
									 double error, uint *index, 
									 bool isAttWeightedKNN);

/* Learn weights by cross validation */
float CrossValidate(TRAINING_EXAMPLES_LIST *data, int iterations, 
					int numExamples, bool isAttWKNN);
//////////////////////////////////////////////////////////////////////////

/* Finds K nearest neighbors and predicts class according to algorithm used. */
int PredictByKNN (TRAINING_EXAMPLES_LIST *tlist, double *query, 
				  bool isWeightedKNN, uint *index, MODE mode, 
				  bool isBE, bool isAttWeightedKNN);

/* Test KNN algorithms */
int TestKNN (TRAINING_EXAMPLES_LIST *tlist, TRAINING_EXAMPLES_LIST data, 
			 bool isWeighted, MODE mode, bool isBackwardElimination, 
			 bool isAttWKNN);


/* Reads the training and testing data into the list. */
bool readData4File (const char *filename, 
					TRAINING_EXAMPLES_LIST *rlist, 
					int *rlistExamples);

/* Utility function to read line from a file. */
int GetLine (char *line, int max, FILE *fp);

/* Comparison function used during sorting data. */
bool compare(const TrainingExample t1, const TrainingExample t2);

/* Normalizes data values using standard deviation. */
void NormalizeByStandardDeviation (TRAINING_EXAMPLES_LIST *trainList, 
								   int trainExamples);

void BackwardEliminationInit ();
void InitAttWeights ();

/*-----------------------------------------------------------------*/
/*                                                                 */
/*-----------------------------------------------------------------*/
int main(void)
{
	/* Training Examples */
	TRAINING_EXAMPLES_LIST elist;
	int numTrainingExamples = 0;

	/* Testing Examples */
	TRAINING_EXAMPLES_LIST qlist;
	int numTestingExamples = 0;
	
	//Read Data from File
	if(!readData4File ("train_heart.txt", &elist, &numTrainingExamples))
	//if(!readData4File ("train_hillValley.txt", &elist, &numTrainingExamples))
	//if(!readData4File ("train_wine.txt", &elist, &numTrainingExamples))
	//if(!readData4File ("train_heart-1.txt", &elist, &numTrainingExamples))
	{
		cout<<"Error in Reading Training Data File."<<endl;
		return 0;
	}
	cout<<"Training Data Uploaded."<<endl;

	if(!readData4File ("test_heart.txt", &qlist, &numTestingExamples))
	//if(!readData4File ("test_hillValley.txt", &qlist, &numTestingExamples))
	//if(!readData4File ("test_wine.txt", &qlist, &numTestingExamples))
	//if(!readData4File ("test_heart-1.txt", &qlist, &numTestingExamples))
	{
		cout<<"Error in Reading Testing Data File."<<endl;
		return 0;
	}
	cout<<"Testing Data Uploaded."<<endl;

	/* Simple KNN */
	SimpleKNN (&elist, numTrainingExamples, &qlist, numTestingExamples);
	/* Attribute Weighted KNN with gradient descent and cross validation */
	AttributeWKNN (&elist, numTrainingExamples, &qlist, numTestingExamples);
	/* KNN with Backward Elimination */
	BackwardElimination (&elist, numTrainingExamples, 
						 &qlist, numTestingExamples);
	/* Instance Weighted KNN with gradient descent and cross validation */
	InstanceWKNN (&elist, numTrainingExamples, &qlist, numTestingExamples);

	//getch();
	return 0;
}

/*-----------------------------------------------------------------*/
/* filename - File from which the training/testing data is read    */
/* rlist - The data structure that holds the training/test data    */
/* rlistExamples - # of training/test examples                     */
/*-----------------------------------------------------------------*/
bool readData4File (const char *filename, TRAINING_EXAMPLES_LIST *rlist, 
					int *rlistExamples)
{
	FILE *fp = NULL;
	int len = 0;
	char line[LINE_MAX+1];
	int lineSize = LINE_MAX;
	TrainingExample *TEObj;
	int index = 0;
	int numExamples = 0;

	*rlistExamples = 0;

	line[0] = 0;

	if((fp = fopen (filename, "r")) == NULL)
	{
		cout<<"Error in opening file."<<endl;
		return false;
	}

	//Initialize weights to random values
	srand (time(NULL));

	char *tmp;
	int tmpParams = 0; //NO_OF_ATT;
	double cd = 0.0;

	/* Read the data file line by line */
    while((len = GetLine (line, lineSize, fp))!=0) 
	{
		TEObj = new TrainingExample ();
		tmp = strtok (line,",");
		while (tmp != NULL)
		{
			cd = atof (tmp);
			TEObj->Value[tmpParams] = cd;
			tmpParams ++;

			tmp = strtok (NULL, ",");

			if(tmpParams == NO_OF_ATT)
			{
				tmpParams = 0;
				cd = 0.0;
				line[0] = 0;
				numExamples ++;

				//Not using this normalization anymore. 
				// N(y) = y/(1+y)
				// Doing normalization by standard deviation and mean
				//TEObj->NormalizeVals ();
				
				/* Generating random weights for instances. */
				/* These weights are used in instance WKNN  */
				double rno = (double)(rand () % 100 + 1);
				TEObj->Weight = rno/100;
				TEObj->index = index++;
				TEObj->isNearest2AtleastSome = false;
				break;
			}
		}

		rlist->insert (rlist->end(), *TEObj);

		delete TEObj;
	}

	/* Normalize values using standard deviation */
	NormalizeByStandardDeviation (rlist,numExamples);

	*rlistExamples = numExamples;

	return true;
}

/*-----------------------------------------------------------------*/
/* trainList - List of training examples                           */
/* trainExamples - # of training examples                          */
/* testList - List of testing examples                             */
/* testExamples - # of testing examples                            */
/*-----------------------------------------------------------------*/
float SimpleKNN (TRAINING_EXAMPLES_LIST *trainList, int trainExamples, 
				 TRAINING_EXAMPLES_LIST *testList, int testExamples)
{
	bool isInstanceWKNN = false;
	MODE mode			= TESTING;
	bool isBackwardElim = false;
	bool isAttrWKNN		= false;

	cout<<endl<<"Testing Simple KNN(Without Weights)."<<endl;
	
	/* Test Simple KNN */
	int CCI = TestKNN(trainList, *testList, isInstanceWKNN, mode, 
						isBackwardElim, isAttrWKNN);
	
	float accuracy = (float)(((float)CCI/(float)testExamples)*100);
	
	cout<<"----------------------KNN----------------------"<<endl;
	cout<<"Number of Training Examples      # "<<trainExamples<<endl;
	cout<<"Number of Testing Examples       # "<<testExamples<<endl;
	cout<<"K used                           = "<<K<<endl;
	cout<<"Correctly Classified Instances   # "<<CCI<<endl;
	cout<<"Incorrectly Classified Instances # "<<testExamples - CCI<<endl;
	cout<<"Accuracy (%)                     = "<<accuracy<<endl;
	cout<<"-----------------------------------------------"<<endl<<endl;
	
	return accuracy;
}

/*-----------------------------------------------------------------*/
/* trainList - List of training examples                           */
/* trainExamples - # of training examples                          */
/* testList - List of testing examples                             */
/* testExamples - # of testing examples                            */
/*-----------------------------------------------------------------*/
float InstanceWKNN (TRAINING_EXAMPLES_LIST *trainList, int trainExamples, 
					TRAINING_EXAMPLES_LIST *testList, int testExamples)
{
	bool isInstanceWKNN		= true;
	bool isBackwardElim		= false;
	bool isAttrWKNN			= false;
	int no_of_iterations	= 25;
	int desiredAccuracy		= 85;

	cout<<endl<<"Starting Instance Weighted KNN..."<<endl;

	/* Learn weights by cross validation (3 fold) on training set */
	float accuracy = CrossValidate (trainList, no_of_iterations, 
									trainExamples, isAttrWKNN);
	/* Learn weights on the training set */
	LearnWeights (trainList, *trainList, no_of_iterations, trainExamples,
					TRAINING, desiredAccuracy, isAttrWKNN);
	
	/* Test the trained weights on test set */
	int CCI = TestKNN (trainList, *testList, isInstanceWKNN, 
						TESTING, isBackwardElim, isAttrWKNN);
	
	accuracy = (float)(((float)CCI/(float)testExamples)*100);
	
	cout<<"-------Instance Weighted-KNN----------------------------"<<endl;
	cout<<"Number of Training Examples      # "<<trainExamples<<endl;
	cout<<"Number of Testing Examples       # "<<testExamples<<endl;
	cout<<"K used                           = "<<K<<endl;
	cout<<"Correctly Classified Instances   # "<<CCI<<endl;
	cout<<"Incorrectly Classified Instances # "<<testExamples - CCI<<endl;
	cout<<"Accuracy (%)                     = "<<accuracy<<endl;
	cout<<"--------------------------------------------------------"<<endl;

	return accuracy;
}

/*-----------------------------------------------------------------*/
/* trainList - List of training examples                           */
/* trainExamples - # of training examples                          */
/* testList - List of testing examples                             */
/* testExamples - # of testing examples                            */
/*-----------------------------------------------------------------*/
float BackwardElimination (TRAINING_EXAMPLES_LIST *trainList, 
						   int trainExamples, 
						   TRAINING_EXAMPLES_LIST *testList, 
						   int testExamples)
{
	float accuracy = 0.0f;
	int CCI = 0;
	int noOfAttDeleted = 0;

	bool isInstanceWKNN		= false;
	MODE mode				= TESTING;
	bool isBackwardElim		= true;
	bool isAttrWKNN			= false;

	cout<<endl<<"Starting Backward Elimination..."<<endl;
	cout<<"--------------------Backward Elimination--------------------"<<endl;

	/* Initially all the attributes will be included in KNN */
	BackwardEliminationInit ();

	/* Test KNN with all the attributes */
	CCI = TestKNN (trainList, *trainList, isInstanceWKNN, 
					mode, isBackwardElim, isAttrWKNN);
	accuracy = (float)(((float)CCI/(float)trainExamples)*100);
	
	cout<< "Initial Accuracy on training data with "<<NO_OF_ATT-1
		<<" attributes = "<<accuracy<<"%"<<endl;

	for(int i = 0; i < NO_OF_ATT - 1; i++)
	{
		/* Delete one attribute at a time.                      */
		/* If the accuracy has decreased restore the attribute  */
		/* else let the attribute remain deleted.               */
		isBEAttIncluded[i] = false;
		CCI = TestKNN (trainList, *trainList, isInstanceWKNN, 
						mode, isBackwardElim, isAttrWKNN);
		float tmpAcc = (float)(((float)CCI/(float)trainExamples)*100);
		if(tmpAcc >= accuracy)
		{
			accuracy = tmpAcc;
			noOfAttDeleted++;
		}
		else
			isBEAttIncluded[i] = true;
	}
	
	cout<<"Backward Elimination achieves "<<accuracy<<"% accuracy with "
		<<NO_OF_ATT-1-noOfAttDeleted<<" attributes \n on training data."<<endl;

	/* Test KNN again with eliminated attributes on the test data. */
	CCI = TestKNN (trainList, *testList, isInstanceWKNN, 
					mode, isBackwardElim, isAttrWKNN);
	accuracy = (float)(((float)CCI/(float)testExamples)*100);
	
	cout<<"Number of Testing Examples       # "<<testExamples<<endl;
	cout<<"K used                           = "<<K<<endl;
	cout<<"Correctly Classified Instances   # "<<CCI<<endl;
	cout<<"Incorrectly Classified Instances # "<<testExamples - CCI<<endl;
	cout<<"Backward Elimination achieves "<<accuracy<<"% accuracy with "
		<<NO_OF_ATT-1-noOfAttDeleted<<" attributes \n on testing data."<<endl;
	cout<<"------------------------------------------------------------"<<endl;
	
	return accuracy;
}

/*-----------------------------------------------------------------*/
/* trainList - List of training examples                           */
/* trainExamples - # of training examples                          */
/* testList - List of testing examples                             */
/* testExamples - # of testing examples                            */
/*-----------------------------------------------------------------*/
float AttributeWKNN (TRAINING_EXAMPLES_LIST *trainList, int trainExamples, 
					 TRAINING_EXAMPLES_LIST *testList, int testExamples)
{
	int no_of_iterations	= 25;
	int desiredAccuracy		= 85;
	bool isAttrWKNN			= true;
	bool isInstanceWKNN		= false;
	bool isBackwardElim		= false;

	cout<<endl<<"Starting Attribute Weighted KNN..."<<endl;

	/* Every attribute is associated with a weight. */
	/* Initialize the weights with random values.   */
	InitAttWeights ();

	/* Learn weights by cross validation (3 fold) on training set */
	float accuracy = CrossValidate (trainList, no_of_iterations, 
									trainExamples, isAttrWKNN);
	/* Learn weights on the whole training set */
	no_of_iterations = 100;
	LearnWeights (trainList, *trainList, no_of_iterations, trainExamples, 
					TRAINING, desiredAccuracy, isAttrWKNN);
	
	/* Test the trained weights with the test set. */
	int CCI = TestKNN (trainList, *testList, isInstanceWKNN, TESTING, 
						isBackwardElim, isAttrWKNN);
	accuracy = (float)(((float)CCI/(float)testExamples)*100);
	
	cout<<"-------Attribute Weighted-KNN---------------------------"<<endl;
	cout<<"Number of Training Examples      # "<<trainExamples<<endl;
	cout<<"Number of Testing Examples       # "<<testExamples<<endl;
	cout<<"K used                           = "<<K<<endl;
	cout<<"Correctly Classified Instances   # "<<CCI<<endl;
	cout<<"Incorrectly Classified Instances # "<<testExamples - CCI<<endl;
	cout<<"Accuracy (%)                     = "<<accuracy<<endl;
	cout<<"--------------------------------------------------------"<<endl;
	
	return accuracy;
}

/*------------------------------------------------------------------*/
/* Test Simple KNN, Instance WeightedKNN, Attribute WeightedKNN and */
/* KNN by Backward Elimination)                                     */
/* tlist - training list                                            */
/* data - testing list                                              */
/* isInstanceWeighted - Instance WKNN                               */
/* mode - Training/Testing                                          */
/* isBackwardElimination - KNN by Backward Elimination              */
/* isAttWKNN - Attribute WKNN                                       */
/*------------------------------------------------------------------*/
int TestKNN (TRAINING_EXAMPLES_LIST *tlist, TRAINING_EXAMPLES_LIST data, 
			 bool isInstanceWeighted, MODE mode,
			 bool isBackwardElimination, bool isAttWKNN)
{
	int correctlyClassifiedInstances = 0;
	TRAINING_EXAMPLES_LIST::iterator testIter;
	TrainingExample tmpTestObj;
	uint index[K];
	int size = data.size(), i, predictedClass;

	//#pragma omp parallel for private(testIter, tmpTestObj, predictedClass) shared(correctlyClassifiedInstances, data, index, size)
	for (i = 0; i < size; i++)
	//for(testIter = data.begin(); testIter < size; ++testIter)
	{
		tmpTestObj = data[i];
		//tmpTestObj = *testIter;
		/* Predict the class for the query point */
		predictedClass = PredictByKNN(tlist, tmpTestObj.Value, 
											isInstanceWeighted, 
											index, mode, isBackwardElimination, 
											isAttWKNN);
		/* Count the number of correctly classified instances */
		if(((int)(tmpTestObj.Value[NO_OF_ATT-1])) == predictedClass)
			correctlyClassifiedInstances ++;
	}
	return correctlyClassifiedInstances;
}

/*------------------------------------------------------------------*/
/* Initialize attribute weights to random values                    */
/*------------------------------------------------------------------*/
void InitAttWeights ()
{
	srand (time(NULL));
	for(int i = 0; i < NO_OF_ATT - 1; i++)
		attWeights[i] = ((double)(rand () % 100 + 1))/100;
}


/*------------------------------------------------------------------*/
/* Normalize values by using mean and standard deviation            */
/*------------------------------------------------------------------*/
void NormalizeByStandardDeviation (TRAINING_EXAMPLES_LIST *trainList, 
								   int trainExamples)
{
	for(int i = 0; i < NO_OF_ATT -1; i++)
	{
		/* Find the mean */
		double mean = 0.0;
		TRAINING_EXAMPLES_LIST::iterator Iter;
		TrainingExample tmpTestObj;
		for(Iter = trainList->begin(); Iter != trainList->end(); ++Iter)
		{
			tmpTestObj = *Iter;
			mean += tmpTestObj.Value[i];
		}
		mean = (mean / (double)trainExamples);

		/* Find the deviation */
		double deviation = 0.0;
		for(Iter = trainList->begin(); Iter != trainList->end(); ++Iter)
		{
			tmpTestObj = *Iter;
			double val = mean - tmpTestObj.Value[i];
			deviation += val*val;
		}
		deviation = (deviation / (double)trainExamples);
		deviation = sqrt (deviation);

		/* Normalize the values */
		for(Iter = trainList->begin(); Iter != trainList->end(); ++Iter)
		{
			tmpTestObj = *Iter;
			double val = (tmpTestObj.Value[i] - mean)/deviation;
			(*Iter).Value[i] = val;
		}
	}
}

/*------------------------------------------------*/
/* Initialize the array to include all attributes */
/*------------------------------------------------*/
void BackwardEliminationInit ()
{
	for (int i = 0; i < NO_OF_ATT -1; i++)
		isBEAttIncluded[i] = true;
}

/*---------------------------------------------------------------------------*/
/* Predict Class by KNN (Simple KNN, Instance WeightedKNN,                   */
/*                       Attribute WeightedKNN, KNN by Backward Elimination) */
/* tlist - training list                                                     */
/* query - query instance to be classified                                   */
/* isWeightedKNN - Instance Weighted KNN                                     */
/* index - Indices of the K nearest neighbors will be stored in index array  */
/* mode - Specifies whether we are training or testing                       */
/* isBE - Backward Elimination                                               */
/* isAttWeightedKNN - Attribute WeightedKNN                                  */
/*---------------------------------------------------------------------------*/
int PredictByKNN (TRAINING_EXAMPLES_LIST *tlist, double *query, 
				  bool isWeightedKNN, uint *index, MODE mode, 
				  bool isBE, bool isAttWeightedKNN)
{
	double distance = 0.0;
	TRAINING_EXAMPLES_LIST::iterator iter;
	TrainingExample tmpObj;
	TRAINING_EXAMPLES_LIST elistWithD;

	//int size = (*tlist).size();

	if(!elistWithD.empty())
		elistWithD.clear ();

	/* If we are in for backward elimination or attribute WKNN */
	/* then Instance WKNN has to be false                      */
	if(isBE || isAttWeightedKNN)
		isWeightedKNN = false;

	/* Calculate the distance of the query */
	/* point from all training instances   */
	/* using the euclidean distance        */
	//#pragma omp parallel for private(iter) 
	for(iter = tlist->begin(); iter != tlist->end(); ++iter)
	//for (int i = 0; i < size; i++)
	{
		tmpObj = *iter;
		//tmpObj = tlist + i* sizeof(TrainingExample);
		distance = 0.0;
		int j;

		//#pragma omp parallel for private(j,tmpObj,distance) shared(isAttWeightedKNN)
		for(j = 0; j < NO_OF_ATT - 1; j++)
		{
			if(isAttWeightedKNN)
			{
				distance += (abs(query[j] - tmpObj.Value[j]) * 
							abs(query[j] - tmpObj.Value[j])) * 
								(attWeights[j] * attWeights[j]);
			}
			else
			{
				if(isBE)
				{
					if(isBEAttIncluded[j])
						distance += abs(query[j] - tmpObj.Value[j]) *
									abs(query[j] - tmpObj.Value[j]);
				}
				else
				{
					if(isWeightedKNN)
					{
						if(isBEAttIncluded[j])
							distance += abs(query[j] - tmpObj.Value[j]) *
										abs(query[j] - tmpObj.Value[j]);
					}
					else
						distance += abs(query[j] - tmpObj.Value[j]) * 
									abs(query[j] - tmpObj.Value[j]);
				}
			}
		}
		distance = sqrt(distance);
		/* If the distance is zero then set it to some high value */
		/* since it the query point itself                        */
		if((int)(distance*1000) == 0)
			distance = 999999999999999.9;
		
		tmpObj.Distance = distance; 
		elistWithD.insert (elistWithD.end(), tmpObj);
	}

	/* Sort the points on distance in ascending order */
	//elistWithD.sort(compare);
	__gnu_parallel::sort(elistWithD.begin(), elistWithD.end(), compare);

	if(!isWeightedKNN)
	{
		/* Simple KNN, Attribute Weighted KNN, Backward Elimination */
		int classCount[NO_OF_CLASSES];

		for(int i = 0; i < NO_OF_CLASSES; i++)
			classCount[i] = 0;

		int knn = K;
		for(iter = elistWithD.begin(); iter != elistWithD.end(); ++iter)
		{
			/* Calculate how the K nearest neighbors are classified */
			tmpObj = *iter;
			classCount[(int)tmpObj.Value[NO_OF_ATT-1]]++;
			knn--;
			if(!knn)
				break;
		}

		int maxClass = 0;
		int maxCount = 0;

		/* Find the class represented maximum number of times */
		/* among the k neighbors                              */
		for(int i = 0; i < NO_OF_CLASSES; i++)
		{
			if(classCount[i] > maxCount)
			{
				maxClass = i;
				maxCount = classCount[i];
			}
		}

		return maxClass;
	}
	else
	{
		/*Instance Weighted KNN */
		int knn = K;
		double pclass = 0.0;
		int i = 0;
		int maxClass = 0;
		/* Calulate the class by multiplying the K nearest  */
		/* neighbor weights by the class values.            */
		for(iter = elistWithD.begin(); iter != elistWithD.end(); ++iter)
		{
			tmpObj = *iter;

			/* While testing, do not use the training examples   */
			/* which were not near any instance during training. */
			if(mode == TESTING && tmpObj.isNearest2AtleastSome == false)
				continue;
			
			pclass += tmpObj.Weight * tmpObj.Value[NO_OF_ATT-1];
			/* Store the indices of the K nearest neighbors */
			index[i++] = tmpObj.index;
			knn--;
			if(!knn)
				break;
		}

		/* Mark an instance near when it is near to some query instance */
		for(iter = tlist->begin(); iter != tlist->end(); ++iter)
		{
			tmpObj = *iter;
			for(int i = 0; i < K; i++)
			{
				if(index[i] == tmpObj.index)
				{
					(*iter).isNearest2AtleastSome = true;
					break;
				}
			}
		}

		maxClass = (int)pclass;
		return maxClass;
	}
}

/*---------------------------------------------------------------------------*/
/* 3 Fold Cross Validation                                                   */
/* data - training data                                                      */
/* iterations - learn weights for number of iterations on a given cross fold */
/* numExamples - # of examples in the training set                           */
/* isAttWKNN = true (Learn attribute weights)                                */
/*           = false (Learn instance weights)                                */
/*---------------------------------------------------------------------------*/
float LearnWeights (TRAINING_EXAMPLES_LIST *tlist, //Training Data
					TRAINING_EXAMPLES_LIST data,   //Testing Data
												   //Train data = Test data
					int iterations,				   //Learn for # of iterations
					int numExamples,			   //# of examples
					MODE mode,					   // mode = TRAINING
					int desiredAccuracy,		   //Desired accuracy in %
					bool isAttWeightedKNN)	   //Attribute or Instance Weighted
{
	TRAINING_EXAMPLES_LIST::iterator iter;
	uint index[K];
	int CCI;
	float accuracy;

	/* Learn weights for number of iterations  */
	/* or till the desired accuracy is reached */
	int tmp = 0;
	while(tmp!=iterations)
	{
		for(iter = data.begin(); iter != data.end(); ++iter)
		{
			TrainingExample TEObj = *iter;
			/* Predict the class */
			int predictedClass = PredictByKNN (tlist, TEObj.Value, true, index,
												mode, false, isAttWeightedKNN);
			int actualClass = (int)(TEObj.Value[NO_OF_ATT-1]);
			if(actualClass != predictedClass)
			{
				/* Calculate the error */
				double error = actualClass - predictedClass;
				/* Adjust weights by Gradient Descent */
				AdjustWeightsByGradientDescent (TEObj.Value, tlist, error,
												index,isAttWeightedKNN);
			}
		}

		CCI = TestKNN (tlist, data, true, mode, false, false);
		accuracy = (float)(((float)CCI/(float)numExamples)*100);
		tmp++;

		int iacc = (int)accuracy;
		if(iacc > desiredAccuracy)
			break;
	}
	return accuracy;
}

/*---------------------------------------------------------------------------*/
/* 3 Fold Cross Validation                                                   */
/* data - training data                                                      */
/* iterations - learn weights for number of iterations on a given cross fold */
/* numExamples - # of examples in the training set                           */
/* isAttWKNN = true (Learn attribute weights)                                */
/*           = false (Learn instance weights)                                */
/*---------------------------------------------------------------------------*/
float CrossValidate(TRAINING_EXAMPLES_LIST *data, int iterations, 
					int numExamples, bool isAttWKNN)
{
	/* Divide the data into three equal sets N1,N2,N3   */
	/* First Cross Fold:  Training = N1+N2 Testing = N3 */
	/* Second Cross Fold: Training = N2+N3 Testing = N1 */
	/* Third Cross Fold:  Training = N1+N3 Testing = N2 */
	int N = numExamples/3;
	int first = N;
	int second = 2*N;
	//int third = numExamples;
	int i = 0;
	double accuracy = 0.0;

	TRAINING_EXAMPLES_LIST trainList,testList;
	TRAINING_EXAMPLES_LIST::iterator iter;

	/* first cross fold validation */
	i = 0;
	for(iter = data->begin(); iter != data->end(); ++iter)
	{
		TrainingExample TEObj = *iter;
		if(i < second)
			trainList.insert (trainList.end (), TEObj);
		else
			testList.insert (testList.end (), TEObj);
		i++;
	}

	/* Learn Weights (Test and adjust by gradient descent) */
	accuracy = LearnWeights (&trainList, testList, iterations, 
							second + 1, TRAINING, 95, isAttWKNN);

	data->clear ();
	for(iter = trainList.begin(); iter != trainList.end(); ++iter)
	{
		TrainingExample TEObj = *iter;
		data->insert (data->end (), TEObj);
	}

	for(iter = testList.begin(); iter != testList.end(); ++iter)
	{
		TrainingExample TEObj = *iter;
		data->insert (data->end (), TEObj);
	}

	/* second cross fold validation */
	trainList.clear ();
	testList.clear();
	i = 0;
	for(iter = data->begin(); iter != data->end(); ++iter)
	{
		TrainingExample TEObj = *iter;
		if(i >= first)
			trainList.insert (trainList.end (), TEObj);	
		else
			testList.insert (testList.end (), TEObj);	
		
		i++;
	}

	/* Learn Weights (Test and adjust by gradient descent) */
	accuracy = LearnWeights (&trainList, testList, iterations, 
							numExamples-first + 1, TRAINING, 95, isAttWKNN);
	data->clear ();
	for(iter = testList.begin(); iter != testList.end(); ++iter)
	{
		TrainingExample TEObj = *iter;
		data->insert (data->end (), TEObj);
	}

	for(iter = trainList.begin(); iter != trainList.end(); ++iter)
	{
		TrainingExample TEObj = *iter;
		data->insert (data->end (), TEObj);
	}


	/* third fold cross validation */
	trainList.clear ();
	testList.clear();

	i = 0;
	for(iter = data->begin(); iter != data->end(); ++iter)
	{
		TrainingExample TEObj = *iter;
		if(i < first)
			trainList.insert (trainList.end (), TEObj);
		else if (i >= first && i < second)
			testList.insert (testList.end (), TEObj);
		else if (i >= second && i < numExamples)
			trainList.insert (trainList.end (), TEObj);
		i++;
	}
	
	/* Learn Weights (Test and adjust by gradient descent) */
	accuracy = LearnWeights (&trainList, testList, iterations, 
						first+numExamples-second + 1, TRAINING, 95, isAttWKNN); 
	
	data->clear ();
	for(iter = trainList.begin(); iter != trainList.end(); ++iter)
	{
		TrainingExample TEObj = *iter;
		data->insert (data->end (), TEObj);
	}

	for(iter = testList.begin(); iter != testList.end(); ++iter)
	{
		TrainingExample TEObj = *iter;
		data->insert (data->end (), TEObj);
	}
	
	return (float)accuracy;
}

/*-------------------------------------------------------------------------*/
/* qvalue - Value of a particular attribute                                */
/* tlist  - List of training examples                                      */
/* error  - Error in prediction of class for an instance                   */
/* index  - indices corresponding to the K nearest neighbors(instances)    */
/* isAttWeightedKNN = true (Adjust attribute weights)                      */
/*                  = false (Adjust instance weights)                      */
/*-------------------------------------------------------------------------*/
void AdjustWeightsByGradientDescent (double *qvalue, 
									 TRAINING_EXAMPLES_LIST *tlist, 
									 double error, uint *index, 
									 bool isAttWeightedKNN)
{
	if(isAttWeightedKNN)
	{
		/* Adjust attribute weights by gradient descent*/
		for(int i = 0; i < NO_OF_ATT - 1; i++)
			attWeights[i] = attWeights[i] + LEARNING_RATE * error * qvalue[i];
	}
	else
	{
		/* Adjust instance weights by gradient descent.     */
		/* We adjust the weights of the K nearest neighbors */
		/* for a query instance                             */
		TRAINING_EXAMPLES_LIST::iterator iter;
		int k = K;

		for(iter = tlist->begin(); iter != tlist->end(); ++iter)
		{
			TrainingExample TEObj = *iter;
			for(int i = 0; i < K; i++)
			{
				if(TEObj.index == index[i])
				{
					double wt = TEObj.Weight;
					wt = wt + LEARNING_RATE * error;
					(*iter).Weight = wt;
					k--;
					break;
				}
			}
			if(k == 0)
				break;
		}
	}
}

/*-------------------------------------------------------------------------*/
/* Comparison function used by the sorting function for list objects.      */
/*-------------------------------------------------------------------------*/
bool compare(const TrainingExample t1, const TrainingExample t2)
{
	if (t1.Distance < t2.Distance)
		return true;
	else
		return false;
}

/*------------------------------------------------------------------------*/
/* line - to read file fp line                                            */
/* max - maximum line length to read                                      */
/* fp - file to read from                                                 */
/* Return Parameter: 0 if end of file, line length otherwise.             */
/* Copies a file contents to another file.                                */
/*------------------------------------------------------------------------*/
int GetLine (char *line, int max, FILE *fp)
{
   if(fgets(line, max, fp)==NULL)
      return 0;
   else
      return strlen(line);
}

