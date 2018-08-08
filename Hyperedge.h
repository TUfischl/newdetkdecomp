#pragma once
// Models a hyperedge of a hypergraph.
//
//////////////////////////////////////////////////////////////////////


#if !defined(CLS_HYPEREDGE)
#define CLS_HYPEREDGE

#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <memory>

#include "NamedEntity.h"
#include "Vertex.h"

using namespace std;


#pragma once
class Hyperedge : public NamedEntity
{

protected:
	VertexSet Vertices;

public:
	//Helper Constructors
	Hyperedge(const string& name);
	Hyperedge(const string& name, const VertexSet &vertices);
	Hyperedge(int id, const string& name) : Hyperedge(id, name, VertexSet()) { }
	//Hyperedge(int id, const string& name, const VertexSet::const_iterator &begin, const VertexSet::const_iterator &end) : Hyperedge(id,name, VertexSet(begin, end)) { }

	//Real Constructor
	Hyperedge(int id, const string& name, const VertexSet &vertices) : NamedEntity(id, name), Vertices{ vertices } {};
	virtual~Hyperedge();

	//virtual void add(Vertex* v);
	virtual void add(const VertexSharedPtr &v);
		
	size_t getNbrOfVertices() { return Vertices.size();  }

	VertexSharedPtr find(const VertexSharedPtr &v);
	VertexSharedPtr find(int id);
	VertexSharedPtr find(const string& name);

	bool sameVertices(const Hyperedge &e) const { return Vertices == e.Vertices;  };
	bool sameVertices(const VertexSet &v) const { return Vertices == v; };
	bool isCoveredBy(const VertexSet &v) const;

	//Vertex* getVertex(int pos) { return MyVertices[pos]; }

	//Labels hyperedge and all vertices
	void setAllLabels(int label = 0) const;

	/*
	vector<Vertex *>::iterator getVerticesBegin() { return MyVertices.begin();  }
	vector<Vertex *>::iterator getVerticesEnd() { return MyVertices.end(); }
	vector<Vertex *>::const_iterator getVerticesCBegin() const { return MyVertices.cbegin(); } 
	vector<Vertex *>::const_iterator getVerticesCEnd() const { return MyVertices.cend(); } 
	*/

	auto allVertices() -> decltype(make_iterable(Vertices.begin(), Vertices.end()))
	{
		return make_iterable(Vertices.begin(), Vertices.end());
	}

	auto allVertices() const -> decltype(make_iterable(Vertices.begin(), Vertices.end()))
	{
		return make_iterable(Vertices.begin(), Vertices.end());
	}

	//const VE_SET &getVertices() { return MyVertices;  }

	virtual bool isHeavy() const { return false; }
	virtual size_t getWeight() const { return 1; }

	friend std::ostream& operator<< (std::ostream &out, const Hyperedge &he);
};


using HyperedgeSharedPtr = std::shared_ptr<Hyperedge>;
using HyperedgeSet = std::unordered_set<HyperedgeSharedPtr, NamedEntityHash>;
using HyperedgeVector = std::vector<HyperedgeSharedPtr>;

// Outputs a set of hyperedges
std::ostream& operator<< (std::ostream &out, const HyperedgeVector &he);
#endif


