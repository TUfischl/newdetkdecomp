#include <iostream>
#include <unordered_set>

#include "Globals.h"
#include "Hyperedge.h"


int G_EdgeID = 0;

Hyperedge::Hyperedge(const string & name) : Hyperedge(++G_EdgeID, name) 
{
}

Hyperedge::Hyperedge(const string & name, const VertexSet& vertices) : Hyperedge(++G_EdgeID, name, vertices)
{
}

Hyperedge::~Hyperedge()
{
}

void Hyperedge::add(const VertexSharedPtr &v)
{
	//if (find(v->getId()) == nullptr)
		Vertices.insert(v);
	//else
	//	writeErrorMsg("Hyperedge " + getName() + " already contains vertex with id " + to_string(v->getId()), "Hyperedge::addVertex");
}

VertexSharedPtr Hyperedge::find(const VertexSharedPtr &v)
{
	VertexSet::const_iterator it = Vertices.find(v);

	if (it == Vertices.cend())
		return nullptr;
	else
		return *it;
}

VertexSharedPtr Hyperedge::find(int id)
{
	for (auto v : Vertices)
		if (v->getId() == id)
			return v;

	return nullptr;
}

VertexSharedPtr Hyperedge::find(const string & name)
{
	for (auto v : Vertices)
		if (v->getName() == name)
			return v;

	return nullptr;
}

bool Hyperedge::isCoveredBy(const VertexSet & v) const
{
	for (auto edge_vertex : Vertices)
		if (v.find(edge_vertex) == v.cend())
			return false;

	return true;
}


void Hyperedge::setAllLabels(int label) const
{
	NamedEntity::setLabel(label);

	for (auto v : Vertices)
		v->setLabel(label);
}

std::ostream & operator<<(std::ostream & out, const Hyperedge & he)
{
	out << he.getName() << "(";
	for (auto v_it = he.Vertices.cbegin(); v_it != he.Vertices.cend();) {
		out << *(*v_it);
		if ((++v_it) != he.Vertices.cend())
			out << ",";
	}
	out << ")";

	return out;
}

std::ostream & operator<<(std::ostream & out, const HyperedgeVector & he)
{
	out << "(";
	for (auto it = he.cbegin(); it != he.cend();) {
		out << (*it)->getName();
		if (++it != he.cend())
			out << ",";
	}
	out << ")";

	return out;
}