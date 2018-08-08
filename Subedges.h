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
	HypergraphSharedPtr MyHg;
	int MyK;
	unordered_map<HyperedgeSharedPtr, HyperedgeVector, NamedEntityHash> MySubedges;

public:
	Subedges(const HypergraphSharedPtr &hg, int k) : MyHg{ hg }, MyK{ k } { }
	//HE_VEC* getSubedges(Hyperedge *he);

	void init();
	void init(const HyperedgeSharedPtr &he);

	void push_back(const HyperedgeSharedPtr &orig, const HyperedgeSharedPtr &subedge);
	
	auto size() const { return MySubedges.size(); }

	size_t size(const HyperedgeSharedPtr &orig) const;

	auto allSubedges(const HyperedgeSharedPtr &orig) -> decltype(make_iterable(MySubedges.at(orig).begin(), MySubedges.at(orig).end()))
	{
		return make_iterable(MySubedges.at(orig).begin(), MySubedges.at(orig).end());
	}

	auto allSubedges(const HyperedgeSharedPtr &orig) const -> decltype(make_iterable(MySubedges.at(orig).begin(), MySubedges.at(orig).end()))
	{
		return make_iterable(MySubedges.at(orig).end(), MySubedges.at(orig).end());
	}

	HyperedgeSharedPtr get(const HyperedgeSharedPtr &orig, size_t index) const;

	auto begin(const HyperedgeSharedPtr &orig);
	auto end(const HyperedgeSharedPtr &orig);

	auto &subedges(const HyperedgeSharedPtr &orig) const { return MySubedges.at(orig);  }

	
};

#endif

