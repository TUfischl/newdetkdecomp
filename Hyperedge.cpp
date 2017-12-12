#include <iostream>
#include <unordered_set>

#include "Globals.h"
#include "Hyperedge.h"


int G_EdgeID = 0;

Hyperedge::Hyperedge(const string & name) : Hyperedge(++G_EdgeID, name) 
{
}

Hyperedge::Hyperedge(const string & name, const VE_SET& vertices) : Hyperedge(++G_EdgeID, name, vertices)
{
}

Hyperedge::~Hyperedge()
{
}

void Hyperedge::add(Vertex * v)
{
	//if (find(v->getId()) == nullptr)
		MyVertices.insert(v);
	//else
	//	writeErrorMsg("Hyperedge " + getName() + " already contains vertex with id " + to_string(v->getId()), "Hyperedge::addVertex");
}

Vertex * Hyperedge::find(Vertex * v)
{
	VE_SET::const_iterator it = MyVertices.find(v);

	if (it == MyVertices.cend())
		return nullptr;
	else
		return *it;
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

int totalGravity(HE_VEC & HEdges)
{
	int total{ 0 };

	for (auto he : HEdges)
		total += he->MyGravity;

	return total;

}

std::ostream & operator<<(std::ostream & out, const Hyperedge & he)
{
	out << he.getName() << "(";
	for (auto v_it = he.MyVertices.cbegin(); v_it != he.MyVertices.cend();) {
		out << *(*v_it);
		if ((++v_it) != he.MyVertices.cend())
			out << ",";
	}
	out << ")";

	return out;
}
