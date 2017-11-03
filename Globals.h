#pragma once

#if !defined(CLS_GLOBALS)
#define CLS_GLOBALS

#include<string>
#include<unordered_set>
#include<vector>

//#include "Hyperedge.h"

class Vertex;
class Hyperedge;

using namespace std;

using uint = unsigned int;
using VE_VEC = vector<Vertex *>;
using VE_SET = unordered_set<Vertex *>;
using HE_VEC = vector<Hyperedge *>;
using HE_SET = unordered_set<Hyperedge *>;

// Writes an error message to the standard error output stream
void writeErrorMsg(const string& cMessage, const string& cLocation, bool bExitProgram = true);

// Sorts an array of pointers in non-decreasing order according to a given int array
void sortPointers(void **Ptr, int *iEval, int iL, int iR);

// Checks whether Set1 is a subset of Set2
bool isSubset(VE_SET &Set1, VE_SET &Set2);

// Checks whether Set1 is a subset of Set2
bool isSubset(HE_SET &Set1, HE_SET &Set2);

// Returns a random integer between iLB and iUB
int random_range(int iLB, int iUB);

// Converts an unsigned integer number into a string
char *uitoa(unsigned int iNumber, char *cString);

template<class T>
struct Iterable
{
	T _begin;
	T _end;

	Iterable(T begin, T end)
		: _begin(begin), _end(end)
	{}

	T begin()
	{
		return _begin;
	}

	T end()
	{
		return _end;
	}
};

template<class T>
Iterable<T> make_iterable(T t, T u)
{
	return Iterable<T>(t, u);
}

template<typename T>
void sortVectors(vector<T> &Ptr, vector<int> &iEval, int iL, int iR)
{
	int i = iL - 1, j = iR;
	T pTmp;
	int iTmp;

	if (iR - iL > 200) {  // Quicksort
		while (true) {
			while (iEval[++i] < iEval[iR]);
			while (iEval[--j] > iEval[iR]);
			if (i >= j) break;

			// Swap valuation entries
			iTmp = iEval[i];
			iEval[i] = iEval[j];
			iEval[j] = iTmp;

			// Swap pointers
			pTmp = Ptr[i];
			Ptr[i] = Ptr[j];
			Ptr[j] = pTmp;
		}

		// Swap valuation entries
		iTmp = iEval[i];
		iEval[i] = iEval[iR];
		iEval[iR] = iTmp;

		// Swap pointers
		pTmp = Ptr[i];
		Ptr[i] = Ptr[iR];
		Ptr[iR] = pTmp;

		sortVectors<T>(Ptr, iEval, iL, i - 1);
		sortVectors<T>(Ptr, iEval, i + 1, iR);
	}
	else {  // Insertion sort
		for (i = iL + 1; i <= iR; i++) {
			iTmp = iEval[i];
			pTmp = Ptr[i];
			for (j = i - 1; (j >= iL) && (iTmp < iEval[j]); j--) {
				iEval[j + 1] = iEval[j];
				Ptr[j + 1] = Ptr[j];
			}
			iEval[j + 1] = iTmp;
			Ptr[j + 1] = pTmp;
		}
	}
}

#endif