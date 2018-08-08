#include "RankFHDecomp.h"
#include "VertexSeparator.h"
#include "BaseSeparator.h"


RankFHDecomp::RankFHDecomp(const HypergraphSharedPtr &HGraph, double k) : MyHg { HGraph }, MyK{k}, MyFecCalculator{HGraph}
{
	MyRank = MyHg->arity();
}


RankFHDecomp::~RankFHDecomp()
{
}

CompCache & RankFHDecomp::getSepParts(VertexSeparatorSharedPtr & sep) const
{
	for (auto t : MyTriedSeps)
		if (t.first == sep) {
			sep = t.first;
			break;
		}

	return MyTriedSeps[sep];
}

size_t RankFHDecomp::separate(VertexSeparatorSharedPtr bag, const HyperedgeVector &edges, vector<DecompComponent> &partitions) const
{
	int label = 0;
	unordered_map<HyperedgeSharedPtr, int, NamedEntityHash> eLabels;
	unordered_map<VertexSharedPtr, int, NamedEntityHash> vLabels;

	partitions.clear();

	//First set the label for all seperating vertices
	for (auto &v : bag->allVertices())
	    vLabels[v] = -1;

	for (auto &he : edges)
		if (eLabels[he] == 0) {
			//don't add he to the component if it is covered by the bag
			if (!he->isCoveredBy(bag->vertices())) {
				DecompComponent comp(static_pointer_cast<BaseSeparator>(bag));
				label++;

				comp.add(he);
				eLabels[he] = label;

				//find all edges reachable from the current component
				for (int i = 0; i < comp.size(); i++) {
					for (auto &v : comp[i]->allVertices()) {
						if (vLabels[v] == 0) {
							vLabels[v] = label;
							for (auto &reach_he : MyHg->allVertexNeighbors(v))
								if (eLabels[reach_he] == 0) {
									if (!reach_he->isCoveredBy(bag->vertices())) {
										eLabels[reach_he] = label;
										comp.add(reach_he);
									}
									else
										eLabels[reach_he] = -1;
								}
						}
					}
				}

				partitions.push_back(comp);
			}
			else
				eLabels[he] = -1;

			
		}

	return partitions.size();
}

HypertreeSharedPtr RankFHDecomp::getCutNode(int label, const DecompComponent & comp, const VertexSet & Chi) const
{
	HypertreeSharedPtr htree = getHTNode(comp.component(), Chi);
	htree->setCut();
	htree->setLabel(label);
	return htree;
}

HypertreeSharedPtr RankFHDecomp::getHTNode(const HyperedgeVector &lambda, const VertexSet &Chi, const list<HypertreeSharedPtr> &Subtrees) const
{
	HypertreeSharedPtr HTree = make_shared<Hypertree>(MyHg);

	// Insert hyperedges and nodes into the hypertree-node
	for (auto &e : lambda) {
		HTree->insLambda(e);
	}

	// Insert additional chi-labels to guarantee connectedness
	for (auto &v : Chi)
		HTree->insChi(v);

	// Insert children into the hypertree-node
	for (auto &subtree : Subtrees) {
		//cout << "Calling insChild" << endl;
		HTree->insChild(subtree);
	}

	return HTree;
}

HypertreeSharedPtr RankFHDecomp::decomp(const HyperedgeVector & HEdges, const VertexSet & Connector, int RecLevel) const
{
	//Possible set of vertices
	VertexSet vertices;
	//Bag of current node
	VertexSeparatorSharedPtr bag;
	//Output Hypertree
	HypertreeSharedPtr htree{ nullptr };

	if (Connector.size()+1 > MyK*MyRank)
		return nullptr;

	//Initialize set of possible vertices
	for (auto e : HEdges)
		for (auto v : e->allVertices())
			if (Connector.find(v) == Connector.end())
				vertices.insert(v);

	// Main Loop: Try to add a vertex to the bag
	for (auto it = vertices.begin(); htree == nullptr && it != vertices.end(); it++) {
		VertexSharedPtr v = (*it);
		bag = make_shared<VertexSeparator>(Connector);
		bag->insert(v);

		/*
		for (int i = 0; i <= RecLevel; i++)
			cout << "+";
		cout << " " << bag << endl;
		*/

		// Can we cover the new bag with MyK
		double width;
		unique_ptr<FractionalEdgeCover> fec = make_unique<FractionalEdgeCover>(MyFecCalculator.computeFEC(bag->vertices(), width));

		if (width > MyK) {
			bag->erase(v);
			continue;
		}

		vector<DecompComponent> partitions;
		vector<bool> cut_parts;
		//vector<double> cut_parts_fw;
		list<HypertreeSharedPtr> subtrees;
		bool fail_sep;

		auto &reused = getSepParts(bag);

		int nbr_of_parts = separate(bag, HEdges, partitions);

		// end if no partitions are left
		if (nbr_of_parts == 0) {
			htree = getHTNode(fec->getEdges(), bag->vertices(), subtrees);
			htree->setFec(fec);
			return htree;
		}

		// Create auxiliary array
		cut_parts.clear();
		//cut_parts_fw.clear();
		cut_parts.resize(nbr_of_parts);
		//cut_parts_fw.resize(nbr_of_parts);

		// Check partitions for decomposibility and undecomposibility
		fail_sep = false;
		for (int i = 0; i < partitions.size(); i++) {
			if (partitions[i].size() > HEdges.size()) {
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
				//cut_parts_fw[i] = reused.succFW[help];
			}
			else
				cut_parts[i] = false;
		}

		if (!fail_sep) {
			// Decompose partitions into hypertrees
			for (int i = 0; i < partitions.size(); i++) {
				double currentChildFW = -1;
				if (cut_parts[i]) {
					// Prune subtree
					//currentChildFW = cut_parts_fw[i];
					htree = getCutNode(RecLevel + 1, partitions[i], bag->vertices());
				}
				else {
					// Decompose component recursively
					htree = decomp(partitions[i], RecLevel + 1);
					if (htree == nullptr)
						reused.failed.push_back(partitions[i][0]);
					else {
						reused.succ.push_back(partitions[i][0]);
						reused.succFW[partitions[i][0]] = currentChildFW;
					}
				}
							

				if (htree != nullptr)
					subtrees.push_back(htree);
				else break;
			}

			//Either all components decomposed or some component failed
			if (htree != nullptr) {
				// Create a new hypertree node
				// Build a separator (edges that are covered by the bag)
				htree = getHTNode(fec->getEdges(), bag->vertices(), subtrees);
				htree->setFec(fec);
			}
		}

	}


	return htree;
}

/*
***Description***
The method builds a hypertree decomposition of a given hypergraph.

INPUT:	HGraph: Hypergraph that has to be decomposed
iK: Maximum separator size
OUTPUT: return: Hypertree decomposition of HGraph
*/

HypertreeSharedPtr RankFHDecomp::buildHypertree()
{
	HypertreeSharedPtr HTree;
	HyperedgeVector HEdges;


	// Order hyperedges heuristically
	HEdges = MyHg->getMCSOrder();

	//cout << HEdges << endl;

	// Store initial heuristic order as weight
	//for(int i=0; i < HEdges.size(); i++)
	//	HEdges[i]->setWeight(i);

	// Build hypertree decomposition
	HTree = decomp(HEdges);

	// Expand pruned hypertree nodes
	if ((HTree != nullptr) && (HTree->getCutNode() != nullptr)) {
		cout << "Expanding hypertree ..." << endl;
		expandHTree(HTree);
	}

	return HTree;
}

/*
***Description***
The method expands pruned hypertree nodes, i.e., subgraphs which were not decomposed but are
known to be decomposable are decomposed.

INPUT:	HTree: Hypertree that has to be expanded
OUTPUT: HTree: Expanded hypertree
*/

void RankFHDecomp::expandHTree(HypertreeSharedPtr &HTree) const
{
	HypertreeSharedPtr cut_node, subtree;

	while ((cut_node = HTree->getCutNode()) != nullptr) {

		// Store subgraph in an array
		auto &lambda = cut_node->getLambda();
		HyperedgeVector edges(lambda.begin(), lambda.end());

		// Reconstruct connector vertices
		VertexSet comp;
		auto &chi = cut_node->getChi();
		VertexSet connector;

		for (auto &e : edges)
			for (auto &v : e->allVertices())
				comp.insert(v);

		for (auto &v : chi)
			if (chi.find(v) != chi.end())
				connector.insert(v);

		

		// Decompose subgraph
		subtree = decomp(edges, connector, cut_node->getLabel());
		if (subtree == nullptr)
			writeErrorMsg("Illegal decomposition pruning.", "H_DetKDecomp::expandHTree");

		// Replace the pruned node by the corresponding subtree
		auto parent = cut_node->getParent().lock();
		parent->insChild(subtree);
		parent->remChild(cut_node);
	}
}