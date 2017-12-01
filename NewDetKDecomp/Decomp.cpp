#include "Decomp.h"
#include "Hyperedge.h"
#include "Hypergraph.h"
#include "Hypertree.h"
#include "Superedge.h"

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

Hypertree * Decomp::decompTrivial(HE_VEC * HEdges, VE_VEC * Connector)
{
	Hypertree *htree{ nullptr };
	size_t cnt_edges{ HEdges->size() };
	int total_gravity{ totalGravity(*HEdges) };
	
	// Stop if the hypergraph can be decomposed into a single hypertree-node
	if (total_gravity <= MyK) {
		for (auto he : *HEdges)
			he->setLabel(-1);
		return getHTNode(HEdges, Connector, nullptr);
	}

	if (Connector == nullptr || Connector->size() == 0) {
		// check if there are heavy edges (weight != cnt)
		if (cnt_edges == total_gravity) {
			// Stop if the hypergraph can be decomposed into two hypertree-nodes 
			if ((cnt_edges > 1) && ((int)ceil(cnt_edges / 2.0) <= MyK)) {
				HE_VEC part;
				int half = (int)cnt_edges / 2;
				int i = 0;
				for (auto he : *HEdges) {
					part.push_back(he);
					he->setLabel(-1);
					if (i == half) {
						htree = getHTNode(&part);
						part.clear();
					}
					i++;
				}

				htree->insChild(getHTNode(&part));

				return htree;
			}
		}
		else {
			// decompose in 2 hypertree nodes if possible
			if (total_gravity <= MyK * 2) {
				HE_VEC part;
				Hyperedge* heavy{ nullptr };
				// find the first heavy edge
				for (auto it = HEdges->begin(); heavy == nullptr && it != HEdges->end(); it++)
					if ((*it)->isHeavy())
						heavy = *it;

				// put all other edges into the first bag
				for (auto he : *HEdges)
					if (he != heavy) {
						part.push_back(he);
						he->setLabel(-1);
					}
				
				// create htree node
				htree = getHTNode(&part);

				// create second htree node
				part.clear();
				part.push_back(heavy);
				htree->insChild(getHTNode(&part));

				return htree;
			}
		}
	}

	

	return htree;
}


/*
***Description***
The method creates a new hypertree-node, inserts the given hyperedges into the lambda-set,
inserts the nodes of hyperedges labeled by -1 to the chi-set, inserts the nodes in
ChiConnect to the chi-set, and adds the given hypertrees as subtrees. It is assumed that
hyperedges that should be covered by the chi-set are labeled by -1.

INPUT:	HEdges: Hyperedges to be inserted into the lambda-set
ChiConnect: Connector nodes that must be a subset of the chi-set
Subtrees: Subtrees of the new hypertree-node
OUTPUT: return: Labeled hypertree-node
*/

Hypertree *Decomp::getHTNode(HE_VEC *HEdges, VE_VEC *ChiConnect, list<Hypertree *> *Subtrees, Superedge* Super)
{
	Hypertree *HTree;

	// Create a new hypertree-node
	HTree = new Hypertree(MyHg);
	if (HTree == nullptr)
		writeErrorMsg("Error assigning memory.", "DetKDecomp::getHTNode");

	// Insert hyperedges and nodes into the hypertree-node
	for (auto it = HEdges->cbegin(); it != HEdges->cend(); it++) {
		HTree->insLambda(*it);
		if ((*it)->getLabel() == -1)
			for (auto v : (*it)->allVertices())
				if (Super == nullptr || Super->find(v) != nullptr)
					HTree->insChi(v);
	}

	if (ChiConnect != nullptr)
		// Insert additional chi-labels to guarantee 
		for (auto it = ChiConnect->cbegin(); it != ChiConnect->cend(); it++)
			HTree->insChi(*it);

	if (Subtrees != nullptr)
		// Insert children into the hypertree-node
		for (auto it = Subtrees->cbegin(); it != Subtrees->cend(); it++)
			//Super Edges are for the balanced separator algorithm
			if (Super != nullptr) {
				Hypertree* root = (*it)->findNodeByLambda(Super);
				if (root != nullptr) {
					root->setRoot();
					for (Hypertree* child : root->allChildren())
						HTree->insChild(child);
					root->remChildren(false);
					delete root;
				}
				else
					writeErrorMsg("Superedge not found in the subtree.", "BalKDecomp::getHTNode");
			}
			else
				HTree->insChild(*it);

	return HTree;
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
