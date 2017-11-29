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

#include "Component.h"
#include "Vertex.h"
#include "Globals.h"
#include "Hyperedge.h"

using namespace std;


class Superedge :
	public Hyperedge
{
private:
	HE_SET MyEdges;

public:
	//Real Constructors
	Superedge(const string& name) : Hyperedge(name) { 
		MyGravity = 0;
	}

	void add(Hyperedge *e);

	static Superedge *getSuperedge(HE_VEC *Edges);

	~Superedge();
};

#endif
