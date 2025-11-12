/*
 * MainMST.cpp
 *
 *  Created on: Jun 26, 2014
 *      Author: Abdullah-Al Mamun
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <climits>
#include <cfloat>
#include <ctime>
#include <cmath>

using namespace std;

// typedef
typedef struct EdgePack
{
	unsigned int verU, verV;
	float weight;
	EdgePack(unsigned int u=0, unsigned int v=0, float weightUV=0.0):verU(u),verV(v),weight(weightUV){};
}edgePack;
typedef unsigned int uint;

// macros
#define INVALID_ID		INT_MAX

// function declaration
uint doPartition(edgePack *edgeArr, uint left, uint right, uint pivotInd);
uint doPartitionVal(edgePack *edgeArr, uint left, uint right, edgePack pivotVal);
uint doRandomSelectInd(edgePack *edgeArr, uint edgeSampleExpectedTotal);
uint doSelect(edgePack *edgeArr, uint left, uint right, uint selectInd);

void doQuickSort(edgePack *edgeArr, uint left, uint right);
void doKruskal(edgePack *edgeArr, uint *parentArr, uint left, uint right, uint verTotal);
uint findRoot(uint pointID, uint *parentArr);
void makeEquivalent(uint rootU, uint rootV, uint *parentArr, uint *weightArr);

uint createSuperVertex(uint *parentArr, uint *verRootArr);
uint createNewCost(edgePack **costSVArr, edgePack *edgeArr, edgePack *edgeSVArr, uint *verRootArr, uint svTotal);

// global variable numPoint (total points)
unsigned int verTotal, edgeTotal, edgeNeeded;
double costMin;

edgePack *treeEdgeArr;

int main(int argc, char *argv[])
{
	cout << "---EPMST---" << endl;

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


	clock_t startT	= clock();

	// start -- select a list of sample edges
	clock_t selectStartT	= clock();
	uint edgeSampleExpectedTotal	= uint(atof(argv[2]) * ceil(log2f((float)verTotal)) * verTotal);	// n log(n)
//	uint edgeSampleExpectedTotal2	= uint(ceil(0.1 * edgeTotal)); // 0.1m
//	if (edgeSampleExpectedTotal > edgeSampleExpectedTotal2) // we choose the smallest one
//		edgeSampleExpectedTotal	= edgeSampleExpectedTotal2;
	if (edgeSampleExpectedTotal < 2 * verTotal)
	{
		edgeSampleExpectedTotal	= 2 * verTotal;	// at least 2n
	}

	if (edgeTotal < edgeSampleExpectedTotal)	// not more than total edges
		edgeSampleExpectedTotal	= edgeTotal;

	uint edgeForKruskalTotal	= doRandomSelectInd(edgeArr, edgeSampleExpectedTotal);
	clock_t selectTotalT		= clock() - selectStartT;
	// << edgeSampleExpectedTotal << "\t" << edgeForKruskalTotal << endl;
	// end -- selection of edges for kruskal


	// start -- kruskal algorithm for selected edges
	clock_t kruskalStartT	= clock();
	edgeNeeded	= verTotal - 1;
	treeEdgeArr	= (edgePack *) malloc(edgeNeeded * sizeof(edgePack));
	uint *parentArr	= (uint *) malloc(verTotal * sizeof(uint));
	for (uint i = 0; i < verTotal; ++i)
		parentArr[i]	= i;

	costMin	= 0.0;
	doKruskal(edgeArr, parentArr, 0, edgeForKruskalTotal - 1, verTotal);
	clock_t kruskalTotalT	= clock() - kruskalStartT;
	// end -- kruskal algorithm

	// start -- find the rest MST
	if (edgeNeeded > 0)
	{
		// start -- construct supervertices
		clock_t svStartT	= clock();
		uint *verRootArr	= (uint *) malloc(verTotal * sizeof(uint));
		for (uint i = 0; i < verTotal; ++i)
			verRootArr[i]	= INVALID_ID;
		uint superVertexTotal	= createSuperVertex(parentArr, verRootArr);

		edgePack **costSVArr	= (edgePack **) malloc(superVertexTotal * sizeof(edgePack *));
		for (uint i = 0; i < superVertexTotal; ++i)
		{
			costSVArr[i]	= (edgePack *) malloc(superVertexTotal * sizeof(edgePack));
			for (uint j = 0; j < superVertexTotal; ++j)
				costSVArr[i][j]	= EdgePack(INT_MAX, INT_MAX, FLT_MAX);
		}

		edgePack *edgeSVArr		= (edgePack *) malloc(superVertexTotal * superVertexTotal * sizeof(edgePack));
		uint edgeSVForKruskal	= createNewCost(costSVArr, edgeArr, edgeSVArr, verRootArr, superVertexTotal);
		clock_t svTotalT		= (clock() - svStartT);
		// end -- construction of super vertices

		// start -- kruskal over modified graph
		clock_t mkruskalStartT	= clock();
		uint *parentSVArr	= (uint *) malloc(superVertexTotal * sizeof(uint));
		for (uint i = 0; i < superVertexTotal; ++i)
			parentSVArr[i]	= i;

		doKruskal(edgeSVArr, parentSVArr, 0, edgeSVForKruskal - 1, superVertexTotal);
		clock_t mkruskalTotalT	= clock() - mkruskalStartT;
		// end -- kruskal over modified graph

		free(verRootArr);
		for (uint i = 0; i < superVertexTotal; ++i)
			free(costSVArr[i]);
		free(costSVArr);
		free(edgeSVArr);
		free(parentSVArr);

		cout << "Supervertices time: " << svTotalT << "\tModified kruskal time: " << mkruskalTotalT << endl;
	}

	// end -- finish finding MST


	free(edgeArr);
	free(treeEdgeArr);
	free(parentArr);

	clock_t totalT	= (clock() - startT);

	// As we use undirected graph, each edge has been counted twice. So we divide the total time by 2. This implementation requires each edge only once. If input file includes each edge twice, then we should not divide total time by 2. Then we can use only one edge of them into edgeArr when reading edge list from input file. Either way is fine.
	cout << fixed << "Cost: " << costMin << "\tTotal time: " << totalT << "\t Time(sec): " << ((double)totalT / CLOCKS_PER_SEC) << "\tTime per edge: " << (totalT * 1000 / (double)(2 * edgeTotal)) << endl;

	return 0;
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


uint doRandomSelectInd(edgePack *edgeArr, uint edgeSampleExpectedTotal)
{
	uint edgePerRange			= (uint)(floor)(edgeTotal / edgeSampleExpectedTotal);
	edgePack *sampleEdgeArr		= (edgePack *) malloc(edgeSampleExpectedTotal * sizeof(edgePack));
	srand(time(NULL));
	for (unsigned int i = 0; i < edgeSampleExpectedTotal; ++i)
		sampleEdgeArr[i]	= edgeArr[i * edgePerRange + rand() % edgePerRange];

	uint selectedInd	= ((long int)edgeSampleExpectedTotal * edgeSampleExpectedTotal) / edgeTotal;
	selectedInd			= doPartitionVal(edgeArr, 0, edgeTotal - 1, sampleEdgeArr[doSelect(sampleEdgeArr, 0, edgeSampleExpectedTotal - 1, selectedInd - 1)]);
	free(sampleEdgeArr);

	return selectedInd;
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


void doKruskal(edgePack *edgeArr, uint *parentArr, uint left, uint right, uint verTotal)
{
	doQuickSort(edgeArr, left, right);

	uint *weightArr	= (uint *) malloc(verTotal * sizeof(uint));
	for (uint i = 0; i < verTotal; ++i)
		weightArr[i]	= 1;

	uint rootU, rootV;
	for (uint i = left; i <= right; ++i)
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


// make connected components as supervertices
uint createSuperVertex(uint *parentArr, uint *verRootArr)
{
	uint currRoot, rootInd;

	rootInd			= 0;
	uint *tempPRootArr		= (uint *) malloc(verTotal * sizeof(uint));

	for (uint i = 0; i < verTotal; ++i)
		tempPRootArr[i]	= INVALID_ID;

	for (uint i = 0; i < verTotal; ++i)
	{
		if (verRootArr[i] == INVALID_ID)
		{
			currRoot	= findRoot(i, parentArr);
			if (tempPRootArr[currRoot] == INVALID_ID)
			{
				tempPRootArr[currRoot]	= rootInd;
				verRootArr[i]			= rootInd;
				++rootInd;
			}
			else
				verRootArr[i]			= tempPRootArr[currRoot];
		}
	}
	free(tempPRootArr);

	return (rootInd);
}


// find edge cost among super vertices
uint createNewCost(edgePack **costSVArr, edgePack *edgeArr, edgePack *edgeSVArr, uint *verRootArr, uint svTotal)
{
	uint rootU, rootV;

	for (uint i = 0; i < edgeTotal; ++i)
	{
		rootU	= verRootArr[edgeArr[i].verU];
		rootV	= verRootArr[edgeArr[i].verV];
		if( rootU == rootV)
			continue;
		if ((costSVArr[rootU][rootV].verU == INT_MAX) || (edgeArr[i].weight < costSVArr[rootU][rootV].weight))
		{
			costSVArr[rootU][rootV]	= edgeArr[i];
			costSVArr[rootV][rootU]	= edgeArr[i];
		}
	}

	uint k = 0;
	for (uint i = 0; i < svTotal; ++i)
	{
		for (uint j = i + 1; j < svTotal; ++j)
		{
			if (i == j)
				continue;
			if (costSVArr[i][j].verU != INT_MAX)
				edgeSVArr[k++]	= EdgePack(i, j, costSVArr[i][j].weight);
		}
	}

	return k;
}
