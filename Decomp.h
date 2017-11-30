#pragma once

#pragma once
#if !defined(CLS_Decomp)
#define CLS_Decomp

#include <list>

#include "Globals.h"

class Hypergraph;
class Hypertree;
class Superedge;

class Decomp
{
private:
	// Collects connected hyperedges and the corresponding boundary nodes
	void collectReachEdges(Hyperedge *Edge, int Label, HE_VEC *Edges, VE_VEC *Connector);

protected:
	Hypergraph* MyHg;
	int MyK;

	// Separates a set of hyperedges into partitions with corresponding connecting nodes
	int separate(HE_VEC *HEdges, vector<HE_VEC*> &Partitions, vector<VE_VEC*> &Connectors);

	// Creates a hypertree node
	Hypertree *getHTNode(HE_VEC *HEdges, VE_VEC *ChiConnect = nullptr, list<Hypertree *> *Subtrees = nullptr, Superedge* Super = nullptr);

	// Decompose trivial cases
	Hypertree *decompTrivial(HE_VEC *HEdges, VE_VEC *Connector);

public:
	Decomp(Hypergraph* hg, int k) : MyHg{ hg }, MyK{ k } { 
		if (MyK <= 0)
			writeErrorMsg("Illegal hypertree-width.", "Decomp::Decomp");
	}
	virtual~Decomp();

	// Constructs a hypertree decomposition of width at most MyK (if it exists)
	virtual Hypertree *buildHypertree() = 0;
};

#endif
