#include "Globals.h"
#include "DetKDecomp.h"
#include "BalKDecomp.h"
#include "Hypergraph.h"
#include "Hyperedge.h"
#include "Superedge.h"
#include "Hypertree.h"
#include "CombinationIterator.h"
#include "Subedges.h"
#include "SubedgeSeparatorFactory.h"


Subedges* G_Subedges{ nullptr };
int BalKDecomp::MyMaxRecursion{ 0 };
Hypergraph *BalKDecomp::MyBaseGraph{ nullptr }; 


Hypertree * BalKDecomp::decomp(HE_VEC & Edges)
{
	int nbr_parts;
	list<HE_VEC*> bal_seps;
	unordered_set<Superedge *> checked;
	vector<HE_VEC*> partitions;
	vector<VE_VEC*> connectors;
	Hypertree *htree;
	HE_VEC *sep;
	int *indices;
	HE_VEC sep_edges; //Edges to consider for the separator
	Superedge *sep_edge;

	if ((htree = decompTrivial(&Edges, nullptr)) != nullptr)
		return htree;

	// reset all labels
	MyBaseGraph->resetEdgeLabels();
	MyBaseGraph->resetVertexLabels();
	MyHg->resetEdgeLabels();
	MyHg->resetVertexLabels();

	sep_edges = getNeighborEdges(Edges);

	CombinationIterator comb(sep_edges.size(), MyK);
	comb.setStage(MyK);		

	// Find balanced separators
	while ((indices = comb.next()) != nullptr && htree==nullptr) {
		MyHg->resetEdgeLabels();
		MyHg->resetVertexLabels();

		sep = new HE_VEC();

		for (int i = 0; i < MyK; i++) {
			auto he = sep_edges[indices[i]];
			he->labelAll(-1);
			sep->push_back(he);
		}

		sep_edge = Superedge::getSuperedge(sep);

		// super edge must be new and
		// super edge from separator must not be part of current component
		if (checked.find(sep_edge) == checked.end() &&
			(MyHg->getNbrOfHeavyEdges() == 0 || find(Edges.begin(), Edges.end(), sep_edge) == Edges.end())) {

			checked.insert(sep_edge);

			nbr_parts = separate(&Edges, partitions, connectors);

			if (isBalanced(partitions, Edges.size())) {
				//Now try to decompose 
				if ((htree = decompose(sep, sep_edge, partitions)) == nullptr)
					bal_seps.push_back(sep);
				else
					delete sep;
			}
			else
				delete sep;

			for (auto part : partitions)
				delete part;
			partitions.clear();
			for (auto conn : connectors)
				delete conn;
			connectors.clear();
		}
		else
			delete sep;
	}

	if (htree == nullptr) {
		/*
		SubedgeSeparatorFactory sub_sep_fac;
		// All separators failed, try subedge separators
		while (!bal_seps.empty() && htree == nullptr) {
			sep = bal_seps.back();
			bal_seps.pop_back();

			sub_sep_fac.init(MyHg, &Edges, sep, G_Subedges);
		}
		*/
	}

	return htree;
}

Hypertree * BalKDecomp::decompose(HE_VEC *Sep, Superedge *Sup, vector<HE_VEC*>& Parts)
{
	Hypergraph *hg;
	BalKDecomp *baldecomp;
	Hypertree *htree{ nullptr };
	list<Hypertree *> subtrees;


	for (auto part : Parts) {
		hg = new Hypergraph();
		hg->setParent(MyBaseGraph);

		for (auto he : *part)
			hg->insertEdge(he);
		hg->insertEdge(Sup);

		baldecomp = new BalKDecomp(hg, MyK, MyRecLevel + 1);
		htree = baldecomp->buildHypertree();
		delete baldecomp;
		delete hg;

		if (htree == nullptr)
			break;
		else
			subtrees.push_back(htree);
	}

	if (htree != nullptr) {
		htree = getHTNode(Sep, nullptr, &subtrees, Sup);
		
	}
	else {
		for (auto t : subtrees)
			delete t;
		subtrees.clear();
	}

	return htree;
}

HE_VEC BalKDecomp::getNeighborEdges(HE_VEC & Edges)
{
	HE_VEC neighbors;

	for (auto he : Edges) {
		he->labelAll(1);
		if (!he->isHeavy())
			neighbors.push_back(he);
	}

	for (auto v : MyBaseGraph->allVertices())
		if (v->getLabel()==1)
			for (auto he : MyBaseGraph->allVertexNeighbors(v))
				if (he->getLabel() == 0) {
					he->setLabel(1);
					neighbors.push_back(he);
				}

	return neighbors;
}

bool BalKDecomp::isBalanced(vector<HE_VEC*>& Parts, int CompSize)
{
	if (Parts.size() > 1) {
		for (auto p : Parts)
			if (p->size() > CompSize / 2)
				return false;

		return true;
	}

	return false;
}

BalKDecomp::BalKDecomp(Hypergraph *HGraph, int k, int RecLevel) : Decomp(HGraph, k), MyRecLevel{ RecLevel }
{
	if (G_Subedges == nullptr)
		G_Subedges = new Subedges(HGraph, k);
}

BalKDecomp::~BalKDecomp()
{
	if (G_Subedges != nullptr) {
		delete G_Subedges;
		G_Subedges = nullptr;
	}
}

Hypertree * BalKDecomp::buildHypertree()
{
	Hypertree *HTree;
	
	if (MyMaxRecursion == 0 || MyRecLevel < MyMaxRecursion) {
		HE_VEC HEdges;

		// Order hyperedges heuristically
		HEdges = MyHg->getMCSOrder();

		// Store initial heuristic order as weight
		for (int i = 0; i < HEdges.size(); i++)
			HEdges[i]->setWeight(i);


		// Build hypertree decomposition
		HTree = decomp(HEdges);
	}
	else {
		// Use normal DetKDecomp
		DetKDecomp Decomp(MyHg, MyK, true);

		HTree = Decomp.buildHypertree();
	}


	return HTree;
}
