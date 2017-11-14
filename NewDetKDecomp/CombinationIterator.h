#if !defined(CLS_COMBIT)
#define CLS_COMBIT

//This class gives indices for all k subsets of n
class CombinationIterator
{
private:
	int k;
	int n;
	int* indices = nullptr;
	int stage;


public:
	CombinationIterator(int n, int k);
	~CombinationIterator();
	//bool hasNext();
	int* next();
	void reset();
	int getStage() { return stage;  }

	void setStage(int);
};

#endif
