// Models a hypertree node.
//
//////////////////////////////////////////////////////////////////////
#pragma once

#if !defined(CLS_HINGETREE)
#define CLS_HINGETREE


#include <list>
#include <set>
#include <vector>
#include <iostream>

#include "Hyperedge.h"
#include "Globals.h"

using namespace std;

class Hyperedge;
class Hingetree;

struct Hingeedge {
	Hingetree* dest{ nullptr };
	Hyperedge* label{ nullptr };
};

class Hingetree
{
private:
	Hingeedge* MyParent{ nullptr };

	// Set of pointers to the neighbors in the hingetree
	list<Hingeedge *> MyChildren;

	// Set of Hyperedges in this hingetree
	vector<Hyperedge*> MyEdges;

    // Is this hingetree node minimal
	bool IsMinimal{ false };
	
	

public:
	Hingetree();
	Hingetree(vector<Hyperedge *> &edges);
	
	~Hingetree();

	bool isMinimal() { return IsMinimal; };
	void setMinimal() { IsMinimal = true; };

	Hyperedge* selectUnused(const set<Hyperedge*> &used);
	list<Hingeedge*> &getChildren() { return MyChildren;  }
	HE_VEC *getEdges() { return &MyEdges;  }
	Hingeedge* getParent() { return MyParent;  }
	void setParent(Hingeedge* p) { MyParent = p; }
	int nbrOfNodes();
	int sizeOfLargestHinge();

	friend std::ostream& operator<<(std::ostream&, const Hingetree&);
	
	auto allEdges() -> decltype(make_iterable(MyEdges.begin(), MyEdges.end()))
	{
		return make_iterable(MyEdges.begin(), MyEdges.end());
	}
};

#endif

