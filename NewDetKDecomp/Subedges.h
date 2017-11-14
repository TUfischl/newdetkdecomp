#pragma once
// Models a collection of subedges of a hypergraph.
//
//////////////////////////////////////////////////////////////////////

#include<unordered_map>

#include "Globals.h"
#include "Hypergraph.h"

class Hypergraph;
class Hyperedge;


#if !defined(CLS_SUBEDGES)
#define CLS_SUBEDGES

class Subedges
{
private:
	Hypergraph *MyHg;
	int MyK;
	unordered_map<Hyperedge*, HE_VEC*> MySubedges;

public:
	Subedges(Hypergraph* hg, int k) : MyHg{ hg }, MyK{ k } { }
	~Subedges();
	HE_VEC* getSubedges(Hyperedge *he);
};

#endif

