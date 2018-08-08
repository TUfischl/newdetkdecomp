#include <cmath>

#include "Decomp.h"
#include "Hyperedge.h"
#include "Hypergraph.h"
#include "Hypertree.h"
#include "Superedge.h"
#include "Separator.h"
#include "DecompComponent.h"

Decomp::~Decomp()
{
}

/*
***Description***
The method partitions a given set of hyperedges into connected components, i.e., into sets
of hyperedges in the same component and sets of nodes connecting the components with the
separator. 

INPUT:	HEdges: Hyperedges to be partitioned
OUTPUT: Partitions: Components consisting of sets of hyperedges
Connectors: Sets of nodes connecting each component with the separator
return: Number of components
*/
 
size_t Decomp::separate(const SeparatorSharedPtr &sep, const HyperedgeVector &edges, vector<DecompComponent> &partitions) const
{
	int label = 0;
	unordered_map<HyperedgeSharedPtr, int, NamedEntityHash> eLabels;
	unordered_map<VertexSharedPtr, int, NamedEntityHash> vLabels;

	partitions.clear();

	//First set the label for all seperating vertices and edges
	for (auto &e : sep->allEdges()) {
		eLabels[e] = -1;
		for (auto &v : e->allVertices())
			vLabels[v] = -1;
	}

	for (auto &he : edges)
		if (eLabels[he] == 0) {
			DecompComponent comp(static_pointer_cast<BaseSeparator>(sep));
			label++;

			comp.add(he);
			eLabels[he] = label;
			
			//find all edges reachable from the current component
			for (int i = 0; i < comp.size(); i++) {
				for (auto &v : comp[i]->allVertices()) {
					if (vLabels[v] == 0) {
						vLabels[v] = label;
						for (auto &he : MyHg->allVertexNeighbors(v))
							if (eLabels[he] == 0) {
								eLabels[he] = label;
								comp.add(he);
							}
					}
				}
			}

			partitions.push_back(comp);
		}

	return partitions.size();
}

HypertreeSharedPtr Decomp::decompTrivial(const HyperedgeVector &edges, const VertexSet &connector) const
{
	HypertreeSharedPtr htree{ nullptr };
	size_t total_weight = 0;

	for (auto &e : edges)
		total_weight += e->getWeight();
	
	// Stop if the hypergraph can be decomposed into a single hypertree-node
	if (total_weight <= MyK)
		return getHTNode(edges, edges, connector);

	if (connector.size() == 0) {
		// check if there are heavy edges (weight != cnt)
		if (edges.size() == total_weight) {
			// Stop if the hypergraph can be decomposed into two hypertree-nodes 
			if ((edges.size() > 1) && ((int)ceil(edges.size() / 2.0) <= MyK)) {
				HyperedgeVector part;
				int half = (int)edges.size() / 2;
				int i = 0;
				for (auto &he : edges) {
					part.push_back(he);
					if (i == half) {
						htree = getHTNode(edges, part);
						part.clear();
					}
					i++;
				}

				htree->insChild(getHTNode(edges, part));

				return htree;
			}
		}
		else {
			// decompose in 2 hypertree nodes if possible
			if (total_weight <= MyK * 2) {
				HyperedgeVector part;
				SuperedgeSharedPtr heavy{ nullptr };
				// find the first heavy edge
				for (auto it = edges.begin(); heavy == nullptr && it != edges.end(); it++)
					if ((*it)->isHeavy())
						heavy = dynamic_pointer_cast<Superedge>(*it);

				// put all other edges into the first bag
				for (auto &he : edges)
					if (he != heavy) 
						part.push_back(he);

				
				// create htree node
				htree = getHTNode(edges, part);

				// create second htree node
				part.clear();
				part.push_back(heavy);
				htree->insChild(getHTNode(edges,part));

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

INPUT:	lambda: A separator (hyperedges) to be inserted into the lambda-set
ChiConnect: Connector nodes that must be a subset of the chi-set
Subtrees: Subtrees of the new hypertree-node
OUTPUT: return: Labeled hypertree-node
*/

HypertreeSharedPtr Decomp::getHTNode(const HyperedgeVector &comp, const HyperedgeVector &lambda, const VertexSet &ChiConnect, const list<HypertreeSharedPtr> &Subtrees, const SuperedgeSharedPtr &Super) const
{
	HypertreeSharedPtr HTree = make_shared<Hypertree>(MyHg);
	VertexSet vcomp;

	for (auto &e : comp)
		for (auto &v : e->allVertices())
			vcomp.insert(v);


	// Insert hyperedges and nodes into the hypertree-node
	for (auto &e : lambda) {
		HTree->insLambda(e);
		for (auto &v : e->allVertices())
			if (vcomp.find(v) != vcomp.end() && (Super == nullptr || Super->find(v) != nullptr))
				HTree->insChi(v);
	}

	// Insert additional chi-labels to guarantee connectedness
	for (auto &v : ChiConnect)
		HTree->insChi(v);

	// Insert children into the hypertree-node
	for (auto &subtree : Subtrees)
		//Super Edges are for the balanced separator algorithm
		if (Super != nullptr) {
			//cout << Super << " --- " << comp << endl;
			HypertreeSharedPtr root = (subtree)->findNodeByLambda(Super);
			if (root != nullptr) {
				root->setRoot();
				for (auto &child : root->allChildren())
					HTree->insChild(child);
				root->remChildren(false);
			}
			else
				writeErrorMsg("Superedge not found in the subtree.", "BalKDecomp::getHTNode");
		}
		else
			HTree->insChild(subtree);

	return HTree;
}

HypertreeSharedPtr Decomp::getCutNode(int label, const HyperedgeVector & lambda, const VertexSet & ChiConnect) const
{
	HypertreeSharedPtr htree = getHTNode(lambda, lambda, ChiConnect);
	htree->setCut();
	htree->setLabel(label);
	return htree;
}

