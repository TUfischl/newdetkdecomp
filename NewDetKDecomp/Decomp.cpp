#include "Decomp.h"
#include "Hyperedge.h"
#include "Hypergraph.h"

Decomp::~Decomp()
{
}

/*
***Description***
The method partitions a given set of hyperedges into connected components, i.e., into sets
of hyperedges in the same component and sets of nodes connecting the components with the
separator. It is assumed that separating nodes and hyperedges are labeled by -1 and all
other nodes and hyperedges are labeled by 0.

INPUT:	HEdges: Hyperedges to be partitioned
OUTPUT: Partitions: Components consisting of sets of hyperedges
Connectors: Sets of nodes connecting each component with the separator
return: Number of components
*/

int Decomp::separate(HE_VEC *HEdges, vector<HE_VEC*> &Partitions, vector<VE_VEC*> &Connectors)
{
	int label;
	HE_VEC *part;
	VE_VEC *conn;

	for (auto he : *HEdges)
		if (he->getLabel() == 0) {
			part = new HE_VEC;
			conn = new VE_VEC;

			// Search for connected hyperedges
			label = (int)Partitions.size() + 1;
			collectReachEdges(he, label, part, conn);

			Partitions.push_back(part);
			Connectors.push_back(conn);

			//Relabel connecting vertices
			for (auto v : *conn)
				v->setLabel(-1);
		}

	return (int)Partitions.size();
}



/*
***Description***
The method labels all unlabeled hyperedges reachable from Edge with iLabel. It is assumed that
all separating nodes/edges are labeled by -1 and all other nodes/edges are labeled by 0.

INPUT:	Edge: Hyperedge
iLabel: Label of all hyperedges in the same component as Edge
OUTPUT: Egdes: List of all hyperedges in the same component as Edge
Connector: List of nodes connecting the component with the separator
*/

void Decomp::collectReachEdges(Hyperedge *Edge, int Label, HE_VEC *Edges, VE_VEC *Connector)
{
	Edge->setLabel(Label);
	Edges->push_back(Edge);

	for (int i = 0; i < Edges->size(); i++) {
		for (auto v : (*Edges)[i]->allVertices()) {
			switch (v->getLabel()) {
			case 0:	 // Collect hyperedges connected via each node
				v->setLabel(Label);
				for (auto he : MyHg->allVertexNeighbors(v))
					if (he->getLabel() == 0) {
						he->setLabel(Label);
						Edges->push_back(he);
					}
				break;
			case -1: // Node connects the component with the separator
				Connector->push_back(v);
				v->setLabel(-2);
				break;
			}
		}
	}
}