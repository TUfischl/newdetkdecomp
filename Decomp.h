#pragma once

#pragma once
#if !defined(CLS_Decomp)
#define CLS_Decomp

#include "Globals.h"

class Hypergraph;

class Decomp
{
private:
	// Collects connected hyperedges and the corresponding boundary nodes
	void collectReachEdges(Hyperedge *Edge, int Label, HE_VEC *Edges, VE_VEC *Connector);

protected:
	Hypergraph* MyHg;

	// Separates a set of hyperedges into partitions with corresponding connecting nodes
	int separate(HE_VEC *HEdges, vector<HE_VEC*> &Partitions, vector<VE_VEC*> &Connectors);

public:
	Decomp(Hypergraph* hg) : MyHg{ hg } { }
	~Decomp();
};

#endif