// Models the algorithm det-k-decomp.
//
//////////////////////////////////////////////////////////////////////

#if !defined(CLS_DetKDecomp)
#define CLS_DetKDecomp

#include "Globals.h"
#include "Decomp.h"

class Hypergraph;
class Hyperedge;
class Hypertree;
class Vertex;

class DetKDecomp : Decomp
{
private:
	// Maximum separator size
	int MyK;

	// Separator component already checked without success
	list<HE_VEC*> MySeps;

	// Separator component already successfully decomposed
	list<list<Hyperedge *> *> MySuccSepParts;

	// Separator component not decomposable
	list<list<Hyperedge *> *> MyFailSepParts;

	// Initializes a Boolean array representing a subset selection
	int setInitSubset(VE_VEC *Vertices, HE_VEC &Edges,  vector<int> &Set, vector<bool> &InComp, vector<int> &CovWeights);

	// Selects the next subset in a Boolean array representing a subset selection
	int setNextSubset(VE_VEC *Vertices, HE_VEC &Edges, vector<int> &Set, vector<bool> &InComp, vector<int> &CovWeights);

	// Covers a set of nodes by a set of edges
	int coverNodes(HE_VEC &Edges, vector<int> &Set, vector<bool> &InComp, vector<int> &CovWeights, int Uncovered, bool Reconstr);

	// Creates a hypertree node
	Hypertree *getHTNode(HE_VEC *HEdges, VE_VEC *ChiConnect, list<Hypertree *> *Subtrees);

	// Orders hyperedges according to maximum cardinality search
	//void orderMCS(Hyperedge **HEdges, int iNbrOfEdges);

	// Divides a set of hyperedges into inner hyperedges and those containing given vertices
	size_t divideCompEdges(HE_VEC *HEdges, VE_VEC *Vertices, HE_VEC &Inner, HE_VEC &Bound);

	// Returns the partitions to a given separator that are known to be decomposable or undecomposable
	bool getSepParts(int SepSize, HE_VEC **Separator, list<Hyperedge *> **SuccParts, list<Hyperedge *> **FailParts);

	// Checks whether HEdges contains an edge labeled with iLabel
	bool containsLabel(list<Hyperedge *> *HEdges, int iLabel);

	// Checks whether the parent connector nodes are distributed to different components
	//bool isSplitSep(Node **Connector, Node ***ChildConnectors);

	// Builds a hypertree decomposition according to k-decomp by covering connector nodes
	Hypertree *decomp(HE_VEC *HEdges, VE_VEC *Connector, int RecLevel);

	// Expands cut hypertree nodes
	void expandHTree(Hypertree *HTree);

public:
	// Constructor
	DetKDecomp(Hypergraph *HGraph);

	// Destructor
	virtual ~DetKDecomp();

	// Constructs a hypertree decomposition of width at most iK (if it exists)
	Hypertree *buildHypertree(int K);
};


#endif // !defined(CLS_DetKDecomp)

