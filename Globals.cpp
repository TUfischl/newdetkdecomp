#include <iostream>
#include <vector>

#include "Globals.h"

void writeErrorMsg(const string & cMessage, const string & cLocation, bool bExitProgram)
{
	if (bExitProgram) {
		cerr << "ERROR in " << cLocation << ": " << cMessage << endl;
		exit(EXIT_FAILURE);
	}
	else
		cerr << "WARNING in " << cLocation << ": " << cMessage << endl;
}

void sortPointers(void **Ptr, int *iEval, int iL, int iR)
{
	int i = iL - 1, j = iR;
	void *pTmp;
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

		sortPointers(Ptr, iEval, iL, i - 1);
		sortPointers(Ptr, iEval, i + 1, iR);
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

char *uitoa(unsigned int iNumber, char *cString)
{
	int iPos = 0;
	char cTmp;

	// Compute the string representation in reverse order
	do {
		cString[iPos++] = (char)((iNumber % 10) + 48);
		iNumber /= 10;
	} while (iNumber != 0);
	cString[iPos] = '\0';

	// Invert the order of the digits
	for (int i = 0, j = iPos - 1; i < iPos / 2; i++, j--) {
		cTmp = cString[i];
		cString[i] = cString[j];
		cString[j] = cTmp;
	}

	return cString;
}






int random_range(int iLB, int iUB)
{
	int iRange;

	if (iLB > iUB)
		writeErrorMsg("Lower bound larger than upper bound.", "random_range");

	iRange = (iUB - iLB) + 1;
	return iLB + (int)(iRange * (rand() / (RAND_MAX + 1.0)));
}
