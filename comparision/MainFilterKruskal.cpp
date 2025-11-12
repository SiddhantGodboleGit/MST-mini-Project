/*
 * MainFilterKruskal.cpp
 *
 *  Created on: Aug 22, 2014
 *      Author: abdullah-al-mamun
 *
 *
 *  without STL
 */

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <cmath>
#include <ctime>
#include <vector>
#include <cfloat>

using namespace std;

// macros

// typedef
typedef struct EdgePack
{
	unsigned int verU, verV;
	float weight;
	EdgePack(unsigned int u=0, unsigned int v=0, float weightUV=0.0):verU(u),verV(v),weight(weightUV){};
}edgePack;

typedef unsigned int uint;

// function declaration
void doFilterKruskal(edgePack *edgeArr, uint *parentArr, uint indInit, uint indLast);
uint doFilter(edgePack *edgeArr, uint *parentArr, uint indInit, uint indLast);
edgePack pickPivot(edgePack *edgeArr, uint indInit, uint indLast);
uint doSelect(edgePack *edgeArr, uint left, uint right, uint indSelect);
uint doPartition(edgePack *edgeArr, uint left, uint right, uint indPivot);
uint doPartitionVal(edgePack *edgeArr, uint left, uint right, edgePack pivotVal);
void doQuickSort(edgePack *edgeArr, uint left, uint right);
void doKruskal(edgePack *edgeArr, uint *parentArr, uint indInit, uint indLast);
uint findRoot(uint pointID, uint *parentArr);
void makeEquivalent(uint rootU, uint rootV, uint *parentArr, uint *weightArr);

// global variable declaration
edgePack *treeEdgeArr;
uint verTotal, edgeTotal, kruskalThreshold, edgeNeeded;
double costMin;


int main(int argc, char **argv)
{
	cout << "---Filter-Kruskal---" << endl;

	if (argc < 3)
	{
		cout << "Invalid number of arguments.\nCommand Line Argument should be:\nExecutableFile InputFile\nThreshold Constatnt" << endl;
		exit(1);
	}

	// start -- read input data from text file
	clock_t readStartT	= clock();
	ifstream fileIn(argv[1]);
	fileIn >> verTotal >> edgeTotal;

	edgePack *edgeArr			= (edgePack *) malloc(edgeTotal * sizeof(edgePack));

	uint verU, verV;
	float edgeWeight;
	for (uint i = 0; i < edgeTotal; ++i)
	{
		fileIn >> verU >> verV >> edgeWeight;
		edgeArr[i]	= EdgePack(verU, verV, edgeWeight);
	}

	fileIn.close();
	clock_t readTotalT	= clock() - readStartT;
	cout << readTotalT	<< " readtime. point: " << verTotal << " edge: " << edgeTotal << endl;
	// end -- read input data

	// start -- filterKruskal algorithm
	clock_t startT		= clock();
	costMin				= 0.0;
	kruskalThreshold	= atoi(argv[2]) * verTotal; // O(n)
	edgeNeeded			= verTotal - 1;
	treeEdgeArr			= (edgePack *) malloc(edgeNeeded * sizeof(edgePack));
	uint *parentArr	= (uint *) malloc(verTotal * sizeof(uint));
	for (uint i = 0; i < verTotal; ++i)
		parentArr[i]	= i;
	doFilterKruskal(edgeArr, parentArr, 0, edgeTotal - 1);
	clock_t totalT	= clock() - startT;
	// end -- filterKruskal algorithm

	free(edgeArr);
	free(treeEdgeArr);
	free(parentArr);

	// As we use undirected graph, each edge has been counted twice. So we divide the total time by 2. This implementation requires each edge only once. If input file includes each edge twice, then we should not divide total time by 2. Then we can use only one edge of them into edgeArr when reading edge list from input file. Either way is fine.
	cout << fixed << "Cost: " << costMin << "\tTotal time: " << totalT << "\t Time(sec): " << ((double)totalT / CLOCKS_PER_SEC) << "\tTime per edge: " << (totalT * 1000 / (double)(2 * edgeTotal)) << endl;
}

// Filter-Kruskal algorithm
void doFilterKruskal(edgePack *edgeArr, uint *parentArr, uint indInit, uint indLast) // indInit and indLast inclusive
{
	if (indLast < indInit + kruskalThreshold)
		doKruskal(edgeArr, parentArr, indInit, indLast);
	else
	{
		uint indPartition	= doPartitionVal(edgeArr, indInit, indLast, pickPivot(edgeArr, indInit, indLast));
		doFilterKruskal(edgeArr, parentArr, indInit, indPartition);
		uint indNext		= doFilter(edgeArr, parentArr, indPartition + 1, indLast);
		doFilterKruskal(edgeArr, parentArr, indNext, indLast);
	}
}


// filter edges having both end points in the same component and bring them at the beginning
uint doFilter(edgePack *edgeArr, uint *parentArr, uint indInit, uint indLast)
{
	uint rootU, rootV;
	edgePack edgeTemp;
	for (uint i = indInit; i <= indLast; ++i)
	{
		rootU	= findRoot(edgeArr[i].verU, parentArr);
		rootV	= findRoot(edgeArr[i].verV, parentArr);
		if(rootU == rootV)
		{
			edgeTemp	= edgeArr[i];
			edgeArr[i]	= edgeArr[indInit];
			edgeArr[indInit]	= edgeTemp;
			++indInit;
		}
	}

	return indInit;
}


// pick pivot value which is median of sample size sqrt(n)
edgePack pickPivot(edgePack *edgeArr, uint indInit, uint indLast)
{
	double numEdgeLocal	= indLast - indInit + 1;
	uint numSampleEdge	= (uint)floor(sqrt(numEdgeLocal));
	uint edgePerRange	= (uint)floor(numEdgeLocal / numSampleEdge);
	edgePack *sampleEdgeArr		= (edgePack *) malloc(numSampleEdge * sizeof(edgePack));
	srand(time(NULL));
	for (uint i = 0; i < numSampleEdge; ++i)
		sampleEdgeArr[i]	= edgeArr[indInit + i * edgePerRange + rand() % edgePerRange];

	edgePack edgeSelected;
	if (numSampleEdge == 0)
		edgeSelected	= sampleEdgeArr[doSelect(sampleEdgeArr, 0, 0, 0)];
	else
		edgeSelected	= sampleEdgeArr[doSelect(sampleEdgeArr, 0, numSampleEdge - 1, floor(numSampleEdge / 2))];
	free(sampleEdgeArr);

	return edgeSelected;
}

// partition edgeArr into two using pivotInd
uint doPartition(edgePack *edgeArr, uint left, uint right, uint pivotInd)
{
	// find pivot edge
	edgePack pivotVal	= edgeArr[pivotInd];

	// swap pivot value to last value
	edgeArr[pivotInd]	= edgeArr[right];
	edgeArr[right]		= pivotVal;

	int storeInd	= left;
	edgePack tempVal;
	for (unsigned int i = left; i < right; ++i)
	{
		if(edgeArr[i].weight < pivotVal.weight)
		{
			tempVal				= edgeArr[storeInd];
			edgeArr[storeInd]	= edgeArr[i];
			edgeArr[i]			= tempVal;
			++storeInd;
		}
	}

	// swap last value to storeindex value
	tempVal				= edgeArr[storeInd];
	edgeArr[storeInd]	= edgeArr[right];
	edgeArr[right]		= tempVal;

	return storeInd;
}


uint doPartitionVal(edgePack *edgeArr, uint left, uint right, edgePack pivotVal)
{
	edgePack tempVal;
	uint storeInd	= left;
	for (uint i = left; i <= right; ++i)
	{
		if(edgeArr[i].weight <= pivotVal.weight)
		{
			tempVal				= edgeArr[storeInd];
			edgeArr[storeInd]	= edgeArr[i];
			edgeArr[i]			= tempVal;
			++storeInd;
		}
	}

	return storeInd;
}



// quick select to find the selectInd smallest dist edge
uint doSelect(edgePack *edgeArr, uint left, uint right, uint selectInd)
{
	if (left == right)
		return left;

	uint pivotInd;
	while(1)
	{
		pivotInd		= doPartition(edgeArr, left, right, (uint)floor((left + right) / 2));
		if (selectInd == pivotInd)
			return selectInd;
		else if (selectInd < pivotInd)
			right		= pivotInd - 1;
		else
			left		= pivotInd + 1;
	}

	return 0;
}

// sort elements
void doQuickSort(edgePack *edgeArr, uint left, uint right)
{
	if(left < right)
	{
		uint pivotInd		= doPartition(edgeArr, left, right, (uint)floor((left + right) / 2));
		if (pivotInd > 0)
			doQuickSort(edgeArr, left, pivotInd - 1);
		doQuickSort(edgeArr, pivotInd + 1, right);
	}
}


// kruskal algorithm on selected edges
void doKruskal(edgePack *edgeArr, uint *parentArr, uint indInit, uint indLast)
{
	if ((edgeNeeded == 0) || (indInit > indLast))
		return;

	doQuickSort(edgeArr, indInit, indLast);

	uint *weightArr		= (uint *) malloc(verTotal * sizeof(uint));
	for (uint i = 0; i < verTotal; ++i)
		weightArr[i]	= 1;
	uint rootU, rootV;
	for (uint i = indInit; i <= indLast; ++i)
	{
		rootU	= findRoot(edgeArr[i].verU, parentArr);
		rootV	= findRoot(edgeArr[i].verV, parentArr);

		if(rootU != rootV)
		{
			costMin	+= edgeArr[i].weight;
			treeEdgeArr[--edgeNeeded]	= edgeArr[i];
			makeEquivalent(rootU, rootV, parentArr, weightArr);
		}
	}

	free(weightArr);
}

// collapse find function to find the root and make this root as the parent of all ancestors
uint findRoot(uint pointID, uint *parentArr)
{
	if (parentArr[pointID] != pointID)
		parentArr[pointID]	= findRoot(parentArr[pointID], parentArr);

	return parentArr[pointID];
}

// unify two components rooted at rootU and rootV
void makeEquivalent(uint rootU, uint rootV, uint *parentArr, uint *weightArr)
{
	if(weightArr[rootU] < weightArr[rootV])
	{
		parentArr[rootU]	= rootV;
		weightArr[rootV]	= weightArr[rootU] + weightArr[rootV];
	}
	else
	{
		parentArr[rootV]	= rootU;
		weightArr[rootU]	= weightArr[rootU] + weightArr[rootV];
	}
}
