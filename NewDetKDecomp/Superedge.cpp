#include "Superedge.h"

list<Superedge *> MySuperedges;


void Superedge::add(Hyperedge *e)
{
	MyEdges.insert(e);
	for (auto v : e->allVertices())
		Hyperedge::add(v);

	MyGravity++;
}

Superedge * Superedge::getSuperedge(HE_VEC * Edges)
{
	VE_SET vertices;
	Superedge *sup{ nullptr };
	bool found;

	for (auto he : *Edges)
		for (auto v : he->allVertices())
			vertices.insert(v);

	for (auto se : MySuperedges)
		if (se->getNbrOfVertices() == vertices.size()) {
			found = true;
			for (auto it = se->allVertices().begin(); it != se->allVertices().end() && found; it++)
				if ((*it)->getLabel() != -1)
					found = false;

			if (found)
				return se;
		}

	string name = "SE";
	name += to_string(MySuperedges.size() + 1);
	sup = new Superedge(name);
	for (auto he : *Edges)
		sup->add(he);
	MySuperedges.push_back(sup);

	return sup;
}

Superedge::~Superedge()
{
}
