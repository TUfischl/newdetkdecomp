#include <memory>
#include <list>

#include "Superedge.h"
#include "Globals.h"

list<SuperedgeSharedPtr> MySuperedges;


void Superedge::add(const HyperedgeVector &Edges, const VertexSet &Vertices)
{
	for (auto e : Edges)
		if (!e->isHeavy()) 
			this->Edges.insert(e);
		else
		writeErrorMsg("Superedges only from normal edges.", "Superedge::add");	

	for (auto v : Vertices) 
		Hyperedge::add(v);
}


void Superedge::reduce(const VertexSet &Vertices)
{
	for (auto it = this->Vertices.begin(); it != this->Vertices.end(); )
		if (Vertices.find(*it) == Vertices.end())
			it = this->Vertices.erase(it);
		else
			it++;
}

SuperedgeSharedPtr Superedge::getSuperedge(const HyperedgeVector &Edges, const VertexSet &VetComp)
{
	VertexSet vertices;
	SuperedgeSharedPtr sup;
	bool found;

	for (auto he : Edges) {
		for (auto v : he->allVertices()) {
			if (VetComp.find(v) != VetComp.end())
				vertices.insert(v);
		}
	}

	//cout << *Edges << ";" << vertices << endl;

	for (auto se : MySuperedges)
		if (se->getNbrOfVertices() == vertices.size()) {
			found = true;
			for (auto it = se->allVertices().begin(); it != se->allVertices().end() && found; it++)
				if ((*it)->getLabel() != -1 || VetComp.find(*it) == VetComp.end())
					found = false;

			if (found)
				return se;
		}

	string name = "SE";
	name += to_string(MySuperedges.size() + 1);
	sup = std::make_shared<Superedge>(name);
	sup->add(Edges,vertices);
	MySuperedges.push_back(sup);

	return sup;
}

void Superedge::setAllLabels(int label) const
{
	Hyperedge::setAllLabels(label);

	for (auto e : Edges)
		e->setLabel(label);
}

std::ostream & operator<<(std::ostream & stream, const std::shared_ptr<Superedge>& super)
{
	stream << "Superedge: ";
	for (auto v_it = super->Vertices.begin(); v_it != super->Vertices.end();) {
		stream << (*v_it)->getName();
		v_it++;
		if (v_it != super->Vertices.end()) cout << ",";
	}
	stream << "(";
	for (auto e_it = super->Edges.begin(); e_it != super->Edges.end();) {
		stream << (*e_it)->getName();
		e_it++;
		if (e_it != super->Edges.end()) cout << ",";
	}
	stream << ")";

	return stream;
}
