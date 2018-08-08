// FracImproveDecomp.cpp: implementation of the FracImproveDecomp class.
//
//////////////////////////////////////////////////////////////////////


#include <cstdio>
#include <iostream>
#include <cmath>
#include <algorithm>

#include <list>
#include <vector>

using namespace std;

#include "FracImproveDecomp.h"
#include "Hypertree.h"
#include "Hypergraph.h"
#include "Hyperedge.h"
#include "Vertex.h"
#include "Globals.h"

#include "FecCalculator.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


FracImproveDecomp::FracImproveDecomp(const HypergraphSharedPtr &HGraph, int k) : DetKDecomp(HGraph,k), MyFecCalculator(HGraph)
{

}


FracImproveDecomp::~FracImproveDecomp()
{
}



//////////////////////////////////////////////////////////////////////
// Class methods
//////////////////////////////////////////////////////////////////////


/*
***Description***
The method decomposes the hyperedges in a subhypergraph as described in Gottlob and 
Samer: A Backtracking-Based Algorithm for Computing Hypertree-Decompositions.

INPUT:	HEdges: Hyperedges in the subgraph
		Connector: Connector nodes that must be covered
		iRecLevel: Recursion level
OUTPUT: return: Hypertree decomposition of HEdges
*/

bool FracImproveDecomp::verifyFracHypertreeWidth(HypertreeSharedPtr & htree, double &outFW) const
{
	double currentChildFW = -1;
	double bestFW = -1;
	unique_ptr<FractionalEdgeCover> fec = make_unique<FractionalEdgeCover>(MyFecCalculator.computeFEC(htree->getChi(), bestFW));

	double weight = fec->computeWeight();

	if (weight > threshold)
		return false;

	for (auto child : htree->allChildren()) {
		bool value = verifyFracHypertreeWidth(child, currentChildFW);
		if (value) {
			if (bestFW < currentChildFW)
				bestFW = currentChildFW;
		}
		else
			return false;			
	}

	outFW = bestFW;
	htree->setFec(fec);

	return true;
}

HypertreeSharedPtr FracImproveDecomp::decomp(const HyperedgeVector &HEdges, double &outFW, const VertexSet &Connector, int RecLevel) const
{
	int i, j;

	list<Hypertree *>::iterator TreeIter;

	SeparatorSharedPtr separator{ nullptr };
	vector<int> cov_sep_set, cov_weights;
	vector<bool> in_comp;
	size_t cnt_edges{ HEdges.size() };
	int comp_end, nbr_sel_cov, i_add, sep_size;
	size_t nbr_of_parts;
	bool add_edge, fail_sep;
	HypertreeSharedPtr htree{ nullptr };

	double bestFWidth = globalBestFW;

	HyperedgeVector inner_edges, bound_edges, add_edges;

	/*
	for (int k = 0; k <= RecLevel; k++)
	cout << "+";
	cout << " " << *HEdges << endl;
	*/

	if ((htree = decompTrivial(HEdges, Connector)) != nullptr) {
		if (verifyFracHypertreeWidth(htree, outFW))
			return htree;
		else
			htree = nullptr;
	}

	// Divide hyperedges into inner hyperedges and hyperedges containing some connecting nodes
	comp_end = (int)divideCompEdges(HEdges, Connector, inner_edges, bound_edges);

	in_comp.resize(bound_edges.size());
	cov_weights.resize(bound_edges.size(), 0);
	cov_sep_set.resize(MyK + 1);
	for (i = 0; i < bound_edges.size(); i++)
		i < comp_end ? in_comp[i] = true : in_comp[i] = false;

	// Select initial hyperedges to cover the connecting nodes
	nbr_sel_cov = setInitSubset(Connector, bound_edges, cov_sep_set, in_comp, cov_weights);

	// Initialize AddEdges array
	for (i = 0; i < bound_edges.size(); i++)
		if (in_comp[i])
			if (!bound_edges[i]->isHeavy())
				add_edges.push_back(bound_edges[i]);
	for (i = 0; i < inner_edges.size(); i++)
		if (!inner_edges[i]->isHeavy())
			add_edges.push_back(inner_edges[i]);
	if (add_edges.size() <= 0)
		writeErrorMsg("Illegal number of hyperedges.", "DetKDecomp::decomp");

	if (nbr_sel_cov >= 0)
		do {
			// Check whether a covering hyperedge within the component was selected
			add_edge = true;
			for (i = 0; i < nbr_sel_cov; i++)
				if (in_comp[cov_sep_set[i]]) {
					add_edge = false;
					break;
				}

			// Stop if no inner hyperedge can be in the separator
			if (!add_edge || (MyK - nbr_sel_cov > 0)) {

				i_add = 0;
				add_edge ? sep_size = nbr_sel_cov + 1 : sep_size = nbr_sel_cov;

				do {
					// Output the search progress
					// cout << "(" << RecLevel << ")" << endl;

					// Create a separator 
					separator = make_shared<Separator>();

					for (i = 0; i < nbr_sel_cov; i++)
						separator->insert(bound_edges[cov_sep_set[i]]);

					if (add_edge)
						separator->insert(add_edges[i_add]);


					// Check if selected hyperedges were already used before as separator
					auto &reused = getSepParts(separator);

					//Debugging output
					/*
					cout << "+++ Separator: ";
					for (int i = 0; i < separator->size(); i++) {
					cout << (*separator)[i]->getName();
					if (i < separator->size()-1)
					cout << ",";
					}
					cout << endl;
					*/

					// Before seperating check for low fhw
					VertexSet chi = computeChi(HEdges, separator, Connector);
					double chiFWidth;
					unique_ptr<FractionalEdgeCover> fec = make_unique<FractionalEdgeCover>(MyFecCalculator.computeFEC(chi, chiFWidth));
					if (chiFWidth > threshold) 
						continue;
					bestFWidth = chiFWidth;

					vector<DecompComponent> partitions;
					vector<bool> cut_parts;
					vector<double> cut_parts_fw;
					list<HypertreeSharedPtr> Subtrees;

					// Separate hyperedges into partitions with corresponding connector nodes
					nbr_of_parts = separate(separator, HEdges, partitions);

					// Create auxiliary array
					cut_parts.clear();
					cut_parts_fw.clear();
					cut_parts.resize(nbr_of_parts);
					cut_parts_fw.resize(nbr_of_parts);

					// Check partitions for decomposibility and undecomposibility
					fail_sep = false;
					for (i = 0; i < partitions.size(); i++) {
						if (partitions[i].size() >= cnt_edges) {
							//writeErrorMsg("Monotonicity violated.", "DetKDecomp::decomp");
							fail_sep = true;
							reused.failed.push_back(partitions[i].first());
							break;
						}

						// Check for undecomposability
						if (partitions[i].containsOneOf(reused.failed)) {
							fail_sep = true;
							break;
						}

						// Check for decomposibility
						if (HyperedgeSharedPtr help = partitions[i].containsOneOf(reused.succ)) {
							cut_parts[i] = true;
							cut_parts_fw[i] = reused.succFW[help];
						}
						else
							cut_parts[i] = false;
					}

					if (!fail_sep) {
						// Decompose partitions into hypertrees
						for (i = 0; i < partitions.size(); i++) {
							double currentChildFW = -1;
							if (cut_parts[i]) {
								// Prune subtree
								currentChildFW = cut_parts_fw[i];
								htree = getCutNode(RecLevel + 1, partitions[i]);
							}
							else {
								// Decompose component recursively
								htree = decomp(partitions[i], currentChildFW, RecLevel + 1);
								if (htree == nullptr)
									reused.failed.push_back(partitions[i][0]);
								else {
									reused.succ.push_back(partitions[i][0]);
									reused.succFW[partitions[i][0]] = currentChildFW;
								}
							}

							if (currentChildFW > bestFWidth) {
								bestFWidth = currentChildFW;
							}

							if (htree != nullptr)
								Subtrees.push_back(htree);
							else break;
						}

						//Either all components decomposed or some component failed
						if (htree != nullptr) {
							// Create a new hypertree node
							for (i = 0; i < nbr_sel_cov; i++) {
								j = cov_sep_set[i];
								in_comp[j] ? bound_edges[j]->setLabel(-1) : bound_edges[j]->setLabel(0);
							}
							if (add_edge)
								add_edges[i_add]->setLabel(-1);
							htree = getHTNode(HEdges, separator, Connector, Subtrees);
							htree->setFec(fec);
						}
					}

				} while (add_edge && (htree == nullptr) && (++i_add < add_edges.size()));
			}

		} while ((htree == nullptr) && ((nbr_sel_cov = setNextSubset(Connector, bound_edges, cov_sep_set, in_comp, cov_weights)) > 0));

		outFW = bestFWidth;
		return htree;
}


/*
***Description***
The method builds a hypertree decomposition of a given hypergraph as described in Gottlob 
and Samer: A Backtracking-Based Algorithm for Computing Hypertree-Decompositions.

INPUT:	HGraph: Hypergraph that has to be decomposed
		iK: Maximum separator size
OUTPUT: return: Hypertree decomposition of HGraph
*/

HypertreeSharedPtr FracImproveDecomp::buildHypertree(double minImprovement, double &fw)
{
	HypertreeSharedPtr HTree;
	HyperedgeVector HEdges;

	globalBestFW = MyK + 1;
	threshold = MyK - minImprovement;

	// Order hyperedges heuristically
	HEdges = MyHg->getMCSOrder();

	//cout << HEdges << endl;

	// Store initial heuristic order as weight
	//for(int i=0; i < HEdges.size(); i++)
	//	HEdges[i]->setWeight(i);

	// Build hypertree decomposition
	HTree = decomp(HEdges, fw);

	// Expand pruned hypertree nodes
	if ((HTree != nullptr) && (HTree->getCutNode() != nullptr)) {
		cout << "Expanding hypertree ..." << endl;
		expandHTree(HTree);
	}

	return HTree;
}


VertexSet FracImproveDecomp::computeChi(const HyperedgeVector &comp, const shared_ptr<Separator> &Sep, const VertexSet &Connector) const
{
	VertexSet vcomp;
	VertexSet chi;

	for (auto &e : comp)
		for (auto &v : e->allVertices())
			vcomp.insert(v);


	// Insert hyperedges and nodes into the hypertree-node
	for (auto &e : (*Sep)) {
		for (auto &v : e->allVertices())
			if (vcomp.find(v) != vcomp.end())
				chi.insert(v);
	}

	// Insert additional chi-labels to guarantee connectedness
	for (auto &v : Connector)
		chi.insert(v);

	return chi;
}