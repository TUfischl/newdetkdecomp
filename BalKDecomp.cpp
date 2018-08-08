#include <unordered_map>

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
HypergraphSharedPtr BalKDecomp::MyBaseGraph{ nullptr };
list<HypergraphSharedPtr> BalKDecomp::sFailedHg;
unordered_map<HypergraphSharedPtr, HypertreeSharedPtr> BalKDecomp::sSuccHg;

HypertreeSharedPtr BalKDecomp::decomp(const HyperedgeVector &Edges)
{
	int nbr_parts;
	list<SeparatorSharedPtr> bal_seps;
	unordered_set<SuperedgeSharedPtr> checked;
	vector<DecompComponent> partitions;
	HypertreeSharedPtr htree;
	SeparatorSharedPtr sep;
	int *indices;
	HyperedgeVector sep_edges; //Edges to consider for the separator
	SuperedgeSharedPtr sep_edge;
	VertexSet vertices;
	int cnt_bal{ 0 }, cnt_sub_bal{ 0 };

	if ((htree = decompTrivial(Edges, VertexSet())) != nullptr)
		return htree;

	// reset all labels
	MyBaseGraph->setAllLabels();
	MyHg->setAllLabels();

	sep_edges = getNeighborEdges(Edges);

	CombinationIterator comb(sep_edges.size(), MyK);
	comb.setStage(MyK);

	for (auto he : Edges)
		for (auto v : he->allVertices())
			vertices.insert(v);

	//cout << "Decomposing: " << vertices << endl;

	// Find balanced separators
	while ((indices = comb.next()) != nullptr && htree == nullptr) {
		MyHg->setAllLabels();

		sep = make_shared<Separator>();

		for (int i = 0; i < MyK; i++) {
			auto he = sep_edges[indices[i]];
			he->setAllLabels(-1);
			sep->push_back(he);
		}

		nbr_parts = separate(sep,Edges,partitions);

		if (isBalanced(partitions, Edges.size())) {
			sep_edge = Superedge::getSuperedge(sep->edges(), vertices);
			// super edge must be new and
			// super edge from separator must not be part of current component
			if (checked.find(sep_edge) == checked.end() &&
				(MyHg->getNbrOfSuperEdges() == 0 || find(Edges.begin(), Edges.end(), sep_edge) == Edges.end())) {

				checked.insert(sep_edge);

				cnt_bal++;
				//Now try to decompose 
				list<HypertreeSharedPtr> subtrees = decompose(sep, sep_edge, partitions);

				if (subtrees.size() > 0) {
					for (auto e : sep->allEdges())
						e->setAllLabels(-1);

					htree = getHTNode(Edges, sep, VertexSet(), subtrees, sep_edge);
				}
				else {
					bal_seps.push_back(sep);
				}
			}
		}

		partitions.clear();
	}

	if (MyRecLevel == 0)
		cout << cnt_bal << " balanced separators tried." << endl;

	// Now we are trying subedge separators
	if (htree == nullptr) {

		// All separators failed, try subedge separators
		while (!bal_seps.empty() && htree == nullptr) {
			SubedgeSeparatorFactory sub_sep_fac;
			
			sep = bal_seps.back();
			bal_seps.pop_back();

			sub_sep_fac.init(MyHg, Edges, sep, MySubedges);

			do {
				MyHg->setAllLabels();

				for (auto he : sep->allEdges())
					he->setAllLabels(-1);

				sep_edge = Superedge::getSuperedge(sep->edges(), vertices);

				// super edge must be new and
				// super edge from separator must not be part of current component
				if (checked.find(sep_edge) == checked.end() &&
					(MyHg->getNbrOfSuperEdges() == 0 || find(Edges.begin(), Edges.end(), sep_edge) == Edges.end())) {

					checked.insert(sep_edge);

					nbr_parts = separate(sep,Edges, partitions);

					if (isBalanced(partitions, Edges.size())) {


						//Now try to decompose 
						list<HypertreeSharedPtr> subtrees = decompose(sep, sep_edge, partitions);

						if (subtrees.size() > 0) {
							for (auto e : sep->allEdges())
								e->setAllLabels(-1);

							/*
							for (int i = 0; i < MyRecLevel; i++)
							cout << "+";
							cout << " " << *Sep << ": " << *Sup  << endl;
							*/

							htree = getHTNode(Edges, sep, VertexSet(), subtrees, sep_edge);
						}

						cnt_sub_bal++;
					}

					partitions.clear();
				}


			} while (htree == nullptr && (sep = sub_sep_fac.next())->size() != 0);
		}
	}

	//if (htree != nullptr)
	//	htree->reduceChi(&vertices);

	if (MyRecLevel == 0)
		cout << cnt_sub_bal << " subedge balanced separators tried." << endl;

	return htree;
}

list<HypertreeSharedPtr> BalKDecomp::decompose(const SeparatorSharedPtr &Sep, const SuperedgeSharedPtr &Sup, const vector<DecompComponent> &Parts)
{
	HypergraphSharedPtr hypergraph;
	unique_ptr<BalKDecomp> baldecomp;
	HypertreeSharedPtr htree{ nullptr };
	list<HypertreeSharedPtr> subtrees;
	vector<HypergraphSharedPtr> hypergraphs;
	vector<bool> v_succ;
	bool succ;
	bool failed = false;

	for (auto part : Parts) {
		failed = getHypergraph<HyperedgeVector>(hypergraph, succ, part.component(), Sup);
		v_succ.push_back(succ);

		if (failed)
			break;
		else
			hypergraphs.push_back(hypergraph);
	}

	if (!failed) {
		int i = 0;
		for (auto hg : hypergraphs) {
			//hypergraph has been succesfully decomposed previously
			if (v_succ[i]) {
				htree = sSuccHg[hg]->clone();
				subtrees.push_back(htree);
			}
			else {
				baldecomp = make_unique<BalKDecomp>(hg, MyK, MyRecLevel + 1);
				htree = baldecomp->buildHypertree();

				if (htree == nullptr) {
					sFailedHg.push_back(hg);
					break;
				}
				else {
					sSuccHg[hg] = htree->clone();
					subtrees.push_back(htree);
				}
			}
			i++;
		}
	}

	if (htree == nullptr)
		subtrees.clear();


	return subtrees;
}

HyperedgeVector BalKDecomp::getNeighborEdges(const HyperedgeVector &Edges) const
{
	HyperedgeVector neighbors;

	for (auto he : Edges) {
		if (!he->isHeavy())
			neighbors.push_back(he);
		for (auto v : he->allVertices())
			for (auto n : MyBaseGraph->allVertexNeighbors(v))
				if (find(neighbors.begin(),neighbors.end(),n) == neighbors.end())
					neighbors.push_back(n);
	}

	return neighbors;
}

bool BalKDecomp::isBalanced(const vector<DecompComponent>& Parts, int CompSize)
{
	if (Parts.size() > 1) {
		for (auto p : Parts)
			if (p.size() > CompSize / 2)
				return false;

		return true;
	}

	return false;
}

BalKDecomp::BalKDecomp(const HypergraphSharedPtr &HGraph, int k, int RecLevel) : Decomp(HGraph, k), MyRecLevel{ RecLevel }
{
	MySubedges = make_unique<Subedges>(HGraph, k);
}

BalKDecomp::~BalKDecomp()
{
}

template<typename T>
bool BalKDecomp::getHypergraph(HypergraphSharedPtr &Hg, bool &Succ, const T &Part, const SuperedgeSharedPtr &Sup) const
{
	size_t cnt = Part.size() + (Sup != nullptr ? 1 : 0);

	for (auto hg : sFailedHg) {
		bool found = true;

		if (hg->getNbrOfEdges() == cnt) {
			if (Sup != nullptr && !hg->hasEdge(Sup))
				found = false;
			if (found && !hg->hasAllEdges(Part))
				found = false;
		}
		else
			found = false;

		if (found) {
			Hg = hg;
			return true;
		}
	}

	for (auto pair : sSuccHg) {
		HypergraphSharedPtr hg = pair.first;
		bool found = true;

		if (hg->getNbrOfEdges() == cnt) {
			if (Sup != nullptr && !hg->hasEdge(Sup))
				found = false;
			if (found && !hg->hasAllEdges(Part))
				found = false;
		}
		else
			found = false;

		if (found) {
			Hg = hg;
			Succ = true;
			return false;
		}
	}


	Hg = make_shared<Hypergraph>();
	Hg->setParent(MyBaseGraph);

	for (auto he : Part)
		Hg->insertEdge(he);
	if (Sup != nullptr)
		Hg->insertEdge(Sup);
	Succ = false;
	return false;
}

/*
***Description***
The method expands pruned hypertree nodes, i.e., subgraphs which were not decomposed but are
known to be decomposable are decomposed.

INPUT:	HTree: Hypertree that has to be expanded
OUTPUT: HTree: Expanded hypertree
*/

void BalKDecomp::expandHTree(const HypertreeSharedPtr &HTree)
{
	HypertreeSharedPtr cut_node;
	HypertreeSharedPtr subtree;
	HyperedgeSet lambda;
	HypergraphSharedPtr hg;
	bool succ;
	unique_ptr<BalKDecomp> baldecomp;
	//set<Hyperedge *>::iterator SetIter1;
	//set<Node *>::iterator SetIter2;

	/*
	ParentSep = new Hyperedge*[iMyK+1];
	if(ParentSep == NULL)
	writeErrorMsg("Error assigning memory.", "DetKDecomp::expandHTree");
	*/

	while ((cut_node = HTree->getCutNode()) != nullptr) {
		// Store subgraph in an array
		lambda = cut_node->getLambda();

		// Get subgraph
		getHypergraph<HyperedgeSet>(hg, succ, lambda);

		if (!succ)
			writeErrorMsg("Hypergraph was not successfully decomposed!", "BalKDecomp::expandHTree");
		
		// Decompose subgraph
		baldecomp = make_unique<BalKDecomp>(hg, MyK, cut_node->getLabel());
		subtree = baldecomp->buildHypertree();
		baldecomp = nullptr;

		
		if (subtree == nullptr)
			writeErrorMsg("Illegal decomposition pruning.", "BalKDecomp::expandHTree");

		
		HypertreeSharedPtr parent = cut_node->getParent().lock();

		// Replace the pruned node by the corresponding subtree
		parent->insChild(subtree);
		parent->remChild(cut_node);

	}
}

HypertreeSharedPtr BalKDecomp::buildHypertree()
{
	HypertreeSharedPtr HTree;
	
	if (MyMaxRecursion == 0 || MyRecLevel < MyMaxRecursion) {
		HyperedgeVector HEdges;

		// Order hyperedges heuristically
		HEdges = MyHg->getMCSOrder();

		// Store initial heuristic order as weight
		//for (int i = 0; i < HEdges.size(); i++)
		//	HEdges[i]->setWeight(i);


		// Build hypertree decomposition
		HTree = decomp(HEdges);
	}
	else {
		// Use normal DetKDecomp
		DetKDecomp Decomp(MyHg, MyK, true);

		HTree = Decomp.buildHypertree();
	}

	if (MyRecLevel == 0 && HTree != nullptr && HTree->getCutNode() != nullptr) {
		cout << "Expanding hypertree ..." << endl;
		expandHTree(HTree);
	}


	return HTree;
}
