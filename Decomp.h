#pragma once

#pragma once
#if !defined(CLS_Decomp)
#define CLS_Decomp

#include <list>

#include "Globals.h"
#include "Hypergraph.h"
#include "Hypertree.h"
#include "Separator.h"
#include "DecompComponent.h"

class Decomp
{
protected:
	HypergraphSharedPtr MyHg;
	int MyK;

	// Separates a set of hyperedges into partitions with corresponding connecting nodes
	size_t separate(const SeparatorSharedPtr &sep, const HyperedgeVector &edges, vector<DecompComponent> &partitions) const;

	// Creates a hypertree node
	HypertreeSharedPtr getHTNode(const HyperedgeVector &comp, const HyperedgeVector &lambda, const VertexSet &ChiConnect = VertexSet(), const list<HypertreeSharedPtr> &Subtrees = list<HypertreeSharedPtr>(), const SuperedgeSharedPtr &Super = nullptr) const;
	HypertreeSharedPtr getHTNode(const HyperedgeVector &comp, const SeparatorSharedPtr &lambda, const VertexSet &ChiConnect = VertexSet(), const list<HypertreeSharedPtr> &Subtrees = list<HypertreeSharedPtr>(), const SuperedgeSharedPtr &Super = nullptr) const {
		HyperedgeVector edges;

		for (auto e : lambda->allEdges())
			edges.push_back(e);
		
		return getHTNode(comp, edges, ChiConnect, Subtrees, Super);
	}


	// Creates a hypertree cutnode
	HypertreeSharedPtr getCutNode(int label, const HyperedgeVector &lambda, const VertexSet &ChiConnect = VertexSet()) const;
	HypertreeSharedPtr getCutNode(int label, const DecompComponent &decomp) const {
		return getCutNode(label, decomp.component(), decomp.connector());
	}

	// Decompose trivial cases
	HypertreeSharedPtr decompTrivial(const HyperedgeVector &edges, const VertexSet &connector) const;

public:
	Decomp(const HypergraphSharedPtr &hg, int k) : MyHg{ hg }, MyK{ k } { 
		if (MyK <= 0)
			writeErrorMsg("Illegal hypertree-width.", "Decomp::Decomp");
	}
	virtual~Decomp();

	// Constructs a hypertree decomposition of width at most MyK (if it exists)
	virtual HypertreeSharedPtr buildHypertree() = 0;
};

#endif
