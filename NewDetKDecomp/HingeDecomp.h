// Models the algorithm hinge-decomp.
//
//////////////////////////////////////////////////////////////////////


#pragma once
#if !defined(CLS_HingeDecomp)
#define CLS_HingeDecomp

#include <set>
#include "Decomp.h"

using namespace std;

class Hypergraph;
class Hingetree;
class Hyperedge;


class HingeDecomp : Decomp
{
private:
	Hingetree *MyHinge{ nullptr };

	set<Hyperedge*> MyUsed;

	void decomp(Hingetree **hinge);
	void decomp() { decomp(&MyHinge); };

	Hypertree* buildHypertree(Hingetree *hinge);

public:
	HingeDecomp(Hypergraph *hg, int k) : Decomp(hg, k) { }
	~HingeDecomp();

	// Constructs a hinge decomposition of HGraph
	Hingetree *buildHingetree();

	virtual Hypertree *buildHypertree() {
		if (MyHinge == nullptr)
			writeErrorMsg("No Hingetree built!", "HingeDecomp::buildHypertree");

		return buildHypertree(MyHinge);
	}
	
};

#endif
