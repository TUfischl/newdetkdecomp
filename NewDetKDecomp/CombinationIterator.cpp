#include "CombinationIterator.h"


CombinationIterator::CombinationIterator( int n, int k)
{
	this->indices = new int[k+1];
	this->k = k;
	this->n = n;
	if (k > n)
		this->k = n;
	reset();
}

CombinationIterator::~CombinationIterator()
//CombinationIterator::~CombinationIterator()
{
	delete indices;
}

int* CombinationIterator::next()
{
	int i = 0;
	bool found = false;

	if (k <= n) {
		if (indices[0] == -1) {
			for (i = 0; i<stage; i++)
				indices[i] = i;
			found = true;
		}
		else
		{
			//Search for index that can be incremented
			for (i = stage - 1; i >= 0 && !found; --i) {
				if (indices[i] < n - stage + i) {
					indices[i]++;
					found = true;
				}
			}
			i++;
			if (found)
				for (++i; i < stage; ++i)
					indices[i] = indices[i - 1] + 1;
			else {
				stage++;
				if (stage <= k) {
					found = true;
					for (i = 0; i < stage; i++)
						indices[i] = i;
				}
			}
		}

		//if found create selection and return 
		if (found)
			return indices;
		else
			return nullptr;
	}
	else
		return nullptr;
}

void CombinationIterator::reset()
{
	for (auto i = 0; i <= k; i++)
		indices[i] = -1;
	this->stage = 1;
}

void CombinationIterator::setStage(int stage)
{
	this->stage = stage;
}

/*bool CombinationIterator::hasNext()
{
	for (auto i = 0; i < k; i++)
		if (indices[i] != n - k + i )
			return true;
	return false;
}*/

