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

#include "Component.h"
#include "Vertex.h"

using namespace std;





#pragma once
class Hyperedge : public Component
{
private:
	VE_SET MyVertices;

protected:
	uint MyGravity{ 1 };

public:
	//Helper Constructors
	Hyperedge(const string& name);
	Hyperedge(const string& name, const unordered_set<Vertex *> &vertices);
	Hyperedge(int id, const string& name) : Hyperedge(id, name, unordered_set<Vertex*>()) { }
	Hyperedge(int id, const string& name, VE_SET::const_iterator begin, VE_SET::const_iterator end) : Hyperedge(id,name, VE_SET(begin, end)) { }

	//Real Constructor
	Hyperedge(int id, const string& name, const unordered_set<Vertex *> &vertices) : Component(id, name), MyVertices(vertices) {};
	virtual~Hyperedge();

	virtual void add(Vertex* v);
		
	size_t getNbrOfVertices() { return MyVertices.size();  }

	Vertex* find(Vertex* v);
	Vertex* find(int id);
	Vertex* find(const string& name);

	bool sameVertices(const Hyperedge &e) { return MyVertices == e.MyVertices;  };
	bool sameVertices(const VE_SET &v) { return MyVertices == v; };

	//Vertex* getVertex(int pos) { return MyVertices[pos]; }

	//Labels hyperedge and all vertices
	void labelAll(int label = 0);

	/*
	vector<Vertex *>::iterator getVerticesBegin() { return MyVertices.begin();  }
	vector<Vertex *>::iterator getVerticesEnd() { return MyVertices.end(); }
	vector<Vertex *>::const_iterator getVerticesCBegin() const { return MyVertices.cbegin(); } 
	vector<Vertex *>::const_iterator getVerticesCEnd() const { return MyVertices.cend(); } 
	*/

	auto allVertices() -> decltype(make_iterable(MyVertices.begin(), MyVertices.end()))
	{
		return make_iterable(MyVertices.begin(), MyVertices.end());
	}

	uint isHeavy() { return MyGravity > 1; }
	uint getGravity() { return MyGravity; }

	friend std::ostream& operator<< (std::ostream &out, const Hyperedge &he);
	friend int totalGravity(HE_VEC &HEdges);
};




#endif


