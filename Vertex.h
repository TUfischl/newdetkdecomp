#pragma once
// Models a vertex of a hypergraph.
//
//////////////////////////////////////////////////////////////////////


#if !defined(CLS_VERTEX)
#define CLS_VERTEX


#include <string>

#include "Globals.h"
#include "Component.h"

using namespace std;

class Vertex : public Component
{
public:
	Vertex(uint id, const string& name) : Component(id,name) { }
	Vertex(const string& name);
	virtual~Vertex();
};

#endif
