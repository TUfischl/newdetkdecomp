#include <list>

#include "Superedge.h"
#include "Globals.h"

list<Superedge *> MySuperedges;


void Superedge::add(HE_VEC *Edges, VE_SET *Vertices)
{
	for (auto e : *Edges)
		if (!e->isHeavy()) 
			MyEdges.insert(e);
		else
		writeErrorMsg("Superedges only from normal edges.", "Superedge::add");	

	for (auto v : *Vertices) 
		Hyperedge::add(v);

	MyGravity = Edges->size();
}


void Superedge::reduce(VE_SET * vertices)
{
	for (auto it = MyVertices.begin(); it != MyVertices.end(); )
		if (vertices->find(*it) == vertices->end())
			it = MyVertices.erase(it);
		else
			it++;
}

Superedge * Superedge::getSuperedge(HE_VEC * Edges, VE_SET *VetComp)
{
	VE_SET vertices;
	Superedge *sup{ nullptr };
	bool found;

	for (auto he : *Edges) {
		for (auto v : he->allVertices()) {
			if (VetComp->find(v) != VetComp->end())
				vertices.insert(v);
		}
	}

	//cout << *Edges << ";" << vertices << endl;

	for (auto se : MySuperedges)
		if (se->getNbrOfVertices() == vertices.size()) {
			found = true;
			for (auto it = se->allVertices().begin(); it != se->allVertices().end() && found; it++)
				if ((*it)->getLabel() != -1 || VetComp->find(*it) == VetComp->end())
					found = false;

			if (found)
				return se;
		}

	string name = "SE";
	name += to_string(MySuperedges.size() + 1);
	sup = new Superedge(name);
	sup->add(Edges,&vertices);
	MySuperedges.push_back(sup);

	return sup;
}

Superedge::~Superedge()
{
}
