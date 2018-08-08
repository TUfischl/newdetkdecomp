// Models the algorithm det-k-decomp.
//
//////////////////////////////////////////////////////////////////////

#if !defined(CLS_DetKDecomp)
#define CLS_DetKDecomp

#include "Globals.h"
#include "Decomp.h"
#include "Separator.h"

class Hypergraph;
class Hyperedge;
class Hypertree;
class Vertex;
class Subedges;

struct CompCache {
	// Separator component already successfully decomposed
	list<HyperedgeSharedPtr> succ;
	// fractional width for succ components
	unordered_map<HyperedgeSharedPtr,double> succFW;
	// Separator component not decomposable
	list<HyperedgeSharedPtr> failed;
};

class DetKDecomp : public Decomp
{
protected:
	mutable unordered_map<SeparatorSharedPtr,CompCache> MyTriedSeps;

	// Run BIP algorithm
	bool MyBIP;
	std::unique_ptr<Subedges> MySubedges;

	// Initializes a Boolean array representing a subset selection
	virtual int setInitSubset(const VertexSet &Vertices, HyperedgeVector &Edges,  vector<int> &Set, vector<bool> &InComp, vector<int> &CovWeights) const;

	// Selects the next subset in a Boolean array representing a subset selection
	virtual int setNextSubset(const VertexSet &Vertices, HyperedgeVector &Edges, vector<int> &Set, vector<bool> &InComp, vector<int> &CovWeights) const;

	// Covers a set of nodes by a set of edges
	int coverNodes(HyperedgeVector &Edges, vector<int> &Set, vector<bool> &InComp, vector<int> &CovWeights, size_t Uncovered, bool Reconstr) const;

	// Orders hyperedges according to maximum cardinality search
	//void orderMCS(Hyperedge **HEdges, int iNbrOfEdges);

	// Divides a set of hyperedges into inner hyperedges and those containing given vertices
	size_t divideCompEdges(const HyperedgeVector &HEdges, const VertexSet &Vertices, HyperedgeVector &Inner, HyperedgeVector &Bound) const;

	// Returns the partitions to a given separator that are known to be decomposable or undecomposable
	CompCache &getSepParts(SeparatorSharedPtr &sep) const;

	// Checks whether HEdges contains an edge labeled with iLabel
	bool containsLabel(list<Hyperedge *> *HEdges, int iLabel);

	// Checks whether the parent connector nodes are distributed to different components
	//bool isSplitSep(Node **Connector, Node ***ChildConnectors);

	// Checks whether a set of edges or a separator covers a set of vertices
	bool covers(const HyperedgeVector &Edges, const VertexSet &Vertices) const;
	bool covers(const SeparatorSharedPtr &Edges, const VertexSet &Vertices) const {
		return covers(Edges->edges(),Vertices);
	}
	

	// Builds a hypertree decomposition according to k-decomp by covering connector nodes
	virtual HypertreeSharedPtr decomp(const HyperedgeVector &HEdges, const VertexSet &Connector=VertexSet(), int RecLevel = 0) const;
	virtual HypertreeSharedPtr decomp(const DecompComponent &comp, int recLevel) const {
		return decomp(comp.component(), comp.connector(), recLevel);
	}

	// Expands cut hypertree nodes
	void expandHTree(HypertreeSharedPtr &HTree);

public:
	// Constructor
	DetKDecomp(const HypergraphSharedPtr &HGraph, int k, bool bip = false);

	// Destructor
	virtual ~DetKDecomp();

	// Constructs a hypertree decomposition of width at most MyK (if it exists)
	virtual HypertreeSharedPtr buildHypertree();
};


#endif // !defined(CLS_DetKDecomp)

