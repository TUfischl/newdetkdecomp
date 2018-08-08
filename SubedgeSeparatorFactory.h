#pragma once

#include <vector>
#include <set>

#include "Hyperedge.h"
#include "CombinationIterator.h"
#include "Hypergraph.h"
#include "Vertex.h"
#include "Subedges.h"
#include "Separator.h"

class SubedgeSeparatorFactory
{
	vector<HyperedgeVector> MySubSets;
	vector<int> MyState;
	bool MyInit{ false };

public:
	SubedgeSeparatorFactory();
	~SubedgeSeparatorFactory();

	void init(const HypergraphSharedPtr &hg, const HyperedgeVector &comp, const SeparatorSharedPtr &sep, const unique_ptr<Subedges> &subs);
	SeparatorSharedPtr next();
};

