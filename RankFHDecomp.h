// Models the algorithm rank-fhdecomp using bounded rank.
//
//////////////////////////////////////////////////////////////////////

#if !defined(CLS_RankFHDecomp)
#define CLS_RankFHDecomp

#pragma once
#include "Globals.h"
#include "FecCalculator.h"
#include "Hypertree.h"
#include "DecompComponent.h"
#include "DetKDecomp.h"
#include "VertexSeparator.h"

class RankFHDecomp
{
private:
	HypergraphSharedPtr MyHg;
	double MyK;
	int MyRank;

	FecCalculator MyFecCalculator;

	mutable unordered_map<VertexSeparatorSharedPtr, CompCache> MyTriedSeps;

	CompCache &getSepParts(VertexSeparatorSharedPtr & sep) const;

	size_t separate(VertexSeparatorSharedPtr bag, const HyperedgeVector &edges, vector<DecompComponent> &partitions) const;

	virtual HypertreeSharedPtr decomp(const HyperedgeVector &HEdges, const VertexSet &Connector = VertexSet(), int RecLevel = 0) const;

	virtual HypertreeSharedPtr decomp(const DecompComponent &comp, int recLevel) const {
		return decomp(comp.component(), comp.connector(), recLevel);
	}

	HypertreeSharedPtr getCutNode(int label, const DecompComponent & comp, const VertexSet & ChiConnect) const;

	void expandHTree(HypertreeSharedPtr &HTree) const;

	HypertreeSharedPtr getHTNode(const HyperedgeVector &lambda, const VertexSet &ChiConnect, const list<HypertreeSharedPtr> &Subtrees = list<HypertreeSharedPtr>()) const;


public:
	RankFHDecomp(const HypergraphSharedPtr &HGraph, double k);
	~RankFHDecomp();

	// Constructs a hypertree decomposition of width at most MyK (if it exists)
	HypertreeSharedPtr buildHypertree();
};

#endif

