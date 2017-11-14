#pragma once

#include <vector>
#include <set>

#include "Hyperedge.h"
#include "CombinationIterator.h"
#include "Hypergraph.h"
#include "Vertex.h"
#include "Subedges.h"

class SubedgeSeparatorFactory
{
	vector<HE_VEC *> MySubSets;
	vector<int> MyState;
	bool MyInit{ false };

public:
	SubedgeSeparatorFactory();
	~SubedgeSeparatorFactory();

	void init(Hypergraph* hg, HE_VEC *comp, HE_VEC *sep, Subedges *subs);
	HE_VEC *next();
};

