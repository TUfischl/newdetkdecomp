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


int BalKDecomp::MyMaxRecursion{ 0 };
Hypergraph *BalKDecomp::MyBaseGraph{ nullptr };
list<Hypergraph *> BalKDecomp::sFailedHg;
list<Hypergraph *> BalKDecomp::sSuccHg;

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
	VE_SET vertices;
	int cnt_bal{ 0 };

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

	for (auto he : Edges)
		for (auto v : he->allVertices())
			vertices.insert(v);

	//cout << "Decomposing: " << vertices << endl;

	// Find balanced separators
	while ((indices = comb.next()) != nullptr && htree == nullptr) {
		MyHg->resetEdgeLabels();
		MyHg->resetVertexLabels();

		sep = new HE_VEC();

		for (int i = 0; i < MyK; i++) {
			auto he = sep_edges[indices[i]];
			he->labelAll(-1);
			sep->push_back(he);
		}

		sep_edge = Superedge::getSuperedge(sep,&vertices);

		// super edge must be new and
		// super edge from separator must not be part of current component
		if (checked.find(sep_edge) == checked.end() &&
			(MyHg->getNbrOfHeavyEdges() == 0 || find(Edges.begin(), Edges.end(), sep_edge) == Edges.end())) {

			checked.insert(sep_edge);

			nbr_parts = separate(&Edges, partitions, connectors);

			if (isBalanced(partitions, Edges.size())) {
				cnt_bal++;
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

		
		HE_VEC *sub_separator;
		// All separators failed, try subedge separators
		while (!bal_seps.empty() && htree == nullptr) {
			SubedgeSeparatorFactory sub_sep_fac;
			
			sep = bal_seps.back();
			bal_seps.pop_back();

			sub_sep_fac.init(MyHg, &Edges, sep, MySubedges);

			while (htree == nullptr && (sub_separator = sub_sep_fac.next()) != nullptr) {
				MyHg->resetEdgeLabels();
				MyHg->resetVertexLabels();

				for (auto he : *sub_separator)
					he->labelAll(-1);

				sep_edge = Superedge::getSuperedge(sub_separator,&vertices);

				// super edge must be new and
				// super edge from separator must not be part of current component
				if (checked.find(sep_edge) == checked.end() &&
					(MyHg->getNbrOfHeavyEdges() == 0 || find(Edges.begin(), Edges.end(), sep_edge) == Edges.end())) {

					checked.insert(sep_edge);

					nbr_parts = separate(&Edges, partitions, connectors);

					if (isBalanced(partitions, Edges.size())) {
						//Now try to decompose 
						if ((htree = decompose(sub_separator, sep_edge, partitions)) != nullptr)
							delete sub_separator;
					}
					else
						delete sub_separator;

					for (auto part : partitions)
						delete part;
					partitions.clear();
					for (auto conn : connectors)
						delete conn;
					connectors.clear();
				}
				else
					delete sub_separator;
			}

			if (htree == nullptr)
				delete sep;
		}
	}

	//if (htree != nullptr)
	//	htree->reduceChi(&vertices);

	if (MyRecLevel == 0)
		cout << cnt_bal << " balanced separators tried." << endl;

	return htree;
}

Hypertree * BalKDecomp::decompose(HE_VEC *Sep, Superedge *Sup, vector<HE_VEC*>& Parts)
{
	Hypergraph *hypergraph;
	BalKDecomp *baldecomp;
	Hypertree *htree{ nullptr };
	list<Hypertree *> subtrees;
	vector<Hypergraph *> hypergraphs;
	vector<bool> v_succ;
	bool succ;
	bool failed = false;

	for (auto part : Parts) {
		failed = getHypergraph(&hypergraph, &succ, part, Sup);
		v_succ.push_back(succ);

		if (failed)
			break;
		else
			hypergraphs.push_back(hypergraph);
	}

	if (!failed) {
		for (auto hg : hypergraphs) {
			baldecomp = new BalKDecomp(hg, MyK, MyRecLevel + 1);
			htree = baldecomp->buildHypertree();
			delete baldecomp;

			if (htree == nullptr) {
				sFailedHg.push_back(hg);
				break;
			}
			else {
				sSuccHg.push_back(hg);
				subtrees.push_back(htree);
			}
		}
	}

	if (htree != nullptr) {
		for (auto e : *Sep)
			e->labelAll(-1);

		/*
		for (int i = 0; i < MyRecLevel; i++)
			cout << "+";
		cout << " " << *Sep << ": " << *Sup  << endl;
		*/

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
		if (v->getLabel() == 1) {
			for (auto he : MyBaseGraph->allVertexNeighbors(v))
				if (he->getLabel() == 0) {
					he->setLabel(1);
					neighbors.push_back(he);
				}
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
	MySubedges = new Subedges(HGraph, k);
}

BalKDecomp::~BalKDecomp()
{
	delete MySubedges;
}

bool BalKDecomp::getHypergraph(Hypergraph ** Hg, bool * Succ, HE_VEC * Part, Hyperedge * Sup)
{
	size_t cnt = Part->size() + 1;

	for (auto hg : sFailedHg) {
		bool found = true;

		if (hg->getNbrOfEdges() == cnt) {
			if (!hg->hasEdge(Sup))
				found = false;
			if (found && !hg->hasAllEdges(Part))
				found = false;
		}
		else
			found = false;

		if (found) {
			*Hg = hg;
			return true;
		}
	}

	for (auto hg : sSuccHg) {
		bool found = true;

		if (hg->getNbrOfEdges() == cnt) {
			if (!hg->hasEdge(Sup))
				found = false;
			if (found && !hg->hasAllEdges(Part))
				found = false;
		}
		else
			found = false;

		if (found) {
			*Hg = hg;
			*Succ = true;
			return false;
		}
	}

	(*Hg) = new Hypergraph();
	(*Hg)->setParent(MyBaseGraph);

	for (auto he : *Part)
		(*Hg)->insertEdge(he);
	(*Hg)->insertEdge(Sup);
	*Succ = false;
	return false;
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
