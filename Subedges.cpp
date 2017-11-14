#include "Subedges.h"
#include "Hypergraph.h"
#include "CombinationIterator.h"

list<Hyperedge *> AllSubedges;
int iSub{ 0 };


Subedges::~Subedges()
{
	for (auto pair : MySubedges)
		delete pair.second;
}

HE_VEC* Subedges::getSubedges(Hyperedge *he)
{
	auto entry = MySubedges.find(he);
	
	if (entry == MySubedges.end()) {
		//Generate all subedges of he and store in MySubedges
		int i{ 0 }, n{ MyHg->nbrOfEdgeNeighbors(he) }, k{ MyK };
		HE_VEC* sub_edges = new HE_VEC();
		HE_VEC neighbors(n);
		VE_SET new_vertices;
		Hyperedge *new_edge;
		int *indices;

		for (auto e : MyHg->allEdgeNeighbors(he))
			neighbors[i++] = e;
		if (n < k) k = n;

		if (neighbors.size() == 0)
			return (MySubedges[he] = sub_edges);

		MyHg->resetVertexLabels();
		he->labelAll(1);

		CombinationIterator comb(n, k);
		indices = comb.next();
		do {
			new_edge = nullptr;
			new_vertices.clear();
			//Build the intersection of a union of up to k edges
			//i.e. for k edges, find the vertices that are labelled with 1.
			for (i = 0; indices[i] != -1; i++)
				for (auto v : neighbors[indices[i]]->allVertices())
					if (v->getLabel() == 1)
						new_vertices.insert(v);

			//Subedge is different then original edge
			if (!he->sameVertices(new_vertices)) {

				//Only create new subedge if it doesn't yet exist
				for (auto e : *sub_edges)
					if (e->sameVertices(new_vertices))
						new_edge = e;

				if (new_edge == nullptr) {
					//Now check if subedge was already created from
					//a different edge
					for (auto e : AllSubedges)
						if (e->sameVertices(new_vertices))
							new_edge = e;

					//Create a new subedge
					if (new_edge == nullptr) {
						string s = he->getName();
						s = s + "-" + std::to_string(++iSub);
						new_edge = new Hyperedge(s, new_vertices);
						AllSubedges.push_back(new_edge);
					}

					sub_edges->push_back(new_edge);
				}
			}
		} while ((indices = comb.next()) != nullptr);

		return (MySubedges[he] = sub_edges);

	}
	else
		return entry->second;
	
	return nullptr;
}
