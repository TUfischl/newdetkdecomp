#pragma once
// Models a superedge of a hypergraph, an edge consisting of edges.
//
//////////////////////////////////////////////////////////////////////


#if !defined(CLS_SUPEREDGE)
#define CLS_SUPEREDGE


#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>

#include "NamedEntity.h"
#include "Vertex.h"
#include "Globals.h"
#include "Hyperedge.h"

using namespace std;

class Superedge :
	public Hyperedge
{
private:
	HyperedgeSet Edges;

public:
	//Real Constructors
	Superedge(const string& name) : Hyperedge(name) { 
		
	}

	void add(const HyperedgeVector &Edges, const VertexSet &VertComp);

	// Removes Vertices from this edge that are not in vertices
	void reduce(const VertexSet &vertices);

	static std::shared_ptr<Superedge> getSuperedge(const HyperedgeVector &Edges, const VertexSet &VertComp);

	virtual bool isHeavy() const { return true; }
	virtual size_t getWeight() const { return Edges.size(); }

	virtual void setAllLabels(int label = 0) const;

	friend std::ostream& operator<< (std::ostream& stream, const std::shared_ptr<Superedge>& super);
};

using SuperedgeSharedPtr = std::shared_ptr<Superedge>;



#endif
