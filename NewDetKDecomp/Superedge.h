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

	void add(HE_VEC *Edges, VE_SET *Vertices);

	// Removes Vertices from this edge that are not in vertices
	void reduce(VE_SET *vertices);

	static Superedge *getSuperedge(HE_VEC *Edges, VE_SET *VetComp);



	~Superedge();
};

#endif
