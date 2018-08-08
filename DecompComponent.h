#if !defined(CLS_DECOMPCOMP)
#define CLS_DECOMPCOMP

#include <list>

#include "Hypergraph.h"
#include "BaseSeparator.h"

#pragma once
class DecompComponent
{
	HyperedgeVector MyComp;
	VertexSet MyConnector;
	BaseSeparatorSharedPtr MySep;

public:
	DecompComponent(const BaseSeparatorSharedPtr &sep) : MySep{ sep } {};
	DecompComponent() {};
	~DecompComponent();

	void add(const HyperedgeSharedPtr &edge);
	auto size() const { return MyComp.size(); }

	bool contains(const HyperedgeSharedPtr &edge) const { return find(MyComp.begin(),MyComp.end(),edge) != MyComp.end(); }
	HyperedgeSharedPtr containsOneOf(const list<HyperedgeSharedPtr> &edges) const;
	
	// Labels the component with label and the seperator and connector with -1
	void label(int label, int sepLabel = -1) const;

	HyperedgeSharedPtr first() const { if (MyComp.size() > 0) return *(MyComp.begin()); else return HyperedgeSharedPtr(nullptr); }

	const HyperedgeVector &component() const { return MyComp; }
	const VertexSet &connector() const { return MyConnector; }

	HyperedgeSharedPtr& operator[](std::size_t idx) { return MyComp[idx]; }
	const HyperedgeSharedPtr& operator[](std::size_t idx) const { return MyComp[idx]; }
};

#endif