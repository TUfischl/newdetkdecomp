#include "Globals.h"
#include "Hyperedge.h"

int G_EdgeID = 0;


Hyperedge::Hyperedge(const string & name) : Hyperedge(++G_EdgeID, name)
{
}

Hyperedge::Hyperedge(int id, const string & name, const vector<Vertex*> vertices) : Component( id , name ), MyVertices(vertices)
{
}

Hyperedge::~Hyperedge()
{
}

void Hyperedge::add(Vertex * v)
{
	if (find(v->getId()) == nullptr)
		MyVertices.push_back(v);
	else
		writeErrorMsg("Hyperedge " + getName() + " already contains vertex with id " + to_string(v->getId()), "Hyperedge::addVertex");
}

Vertex * Hyperedge::find(Vertex * v)
{
	auto item = std::find(MyVertices.cbegin(), MyVertices.cend(), v);
	if (item != MyVertices.cend())
		return *item;
	else
		return nullptr;
}

Vertex * Hyperedge::find(int id)
{
	for (auto v : MyVertices)
		if (v->getId() == id)
			return v;

	return nullptr;
}

Vertex * Hyperedge::find(const string & name)
{
	for (auto v : MyVertices)
		if (v->getName() == name)
			return v;

	return nullptr;
}

void Hyperedge::labelAll(int label)
{
	this->setLabel(label);
	for (auto v : MyVertices)
		v->setLabel(label);
}
