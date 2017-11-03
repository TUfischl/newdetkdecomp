#pragma once
// Models a hyperedge of a hypergraph.
//
//////////////////////////////////////////////////////////////////////


#if !defined(CLS_HYPEREDGE)
#define CLS_HYPEREDGE


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
	vector<Vertex *> MyVertices;

public:
	//Helper Constructors
	Hyperedge(const string& name);
	Hyperedge(int id, const string& name) : Hyperedge(id, name, vector<Vertex*>()) { }
	Hyperedge(int id, const string& name, vector<Vertex *>::const_iterator begin, vector<Vertex *>::const_iterator end) : Hyperedge(id,name, vector<Vertex *>(begin, end)) { }

	//Real Constructor
	Hyperedge(int id, const string& name, const vector<Vertex *> vertices);
	virtual~Hyperedge();

	void add(Vertex* v);
		
	size_t getNbrOfVertices() { return MyVertices.size();  }

	Vertex* find(Vertex* v);
	Vertex* find(int id);
	Vertex* find(const string& name);

	Vertex* getVertex(int pos) { return MyVertices[pos]; }

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
};




#endif


