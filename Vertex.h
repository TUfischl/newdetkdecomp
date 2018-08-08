#pragma once
// Models a vertex of a hypergraph.
//
//////////////////////////////////////////////////////////////////////


#if !defined(CLS_VERTEX)
#define CLS_VERTEX


#include <string>
#include <memory>
#include <set>

#include "Globals.h"
#include "NamedEntity.h"

using namespace std;

class Vertex : public NamedEntity
{
public:
	Vertex(uint id, const string& name) : NamedEntity(id,name) { }
	Vertex(const string& name);
	virtual~Vertex();

	void setAllLabels(int label = 0) const { setLabel(0); }

	friend std::ostream& operator<< (std::ostream &out, const Vertex &v);
};

using VertexSharedPtr = std::shared_ptr<Vertex>;
using VertexSet = unordered_set<VertexSharedPtr, NamedEntityHash>;
using VertexVector = vector<VertexSharedPtr>;

// Outputs a set of vertices
std::ostream& operator<< (std::ostream &out, const VertexSet &v);


using set_type = set<std::shared_ptr<Vertex>>;
using powerset_type = set<set_type>;
powerset_type powerset(set_type const& set);


#endif
