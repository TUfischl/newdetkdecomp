// Models the algorithm det-k-decomp.
//
//////////////////////////////////////////////////////////////////////

#if !defined(CLS_BalKDecomp)
#define CLS_BalKDecomp

#include "Globals.h"
#include "Decomp.h"

class Hypertree;
class Subedges;

class BalKDecomp :
	public Decomp
{
private:
	static list<Hypergraph *> sFailedHg;
	static list<Hypergraph *> sSuccHg;

	int MyRecLevel;
	Subedges *MySubedges;

	Hypertree *decomp(HE_VEC &HEdges);
	Hypertree *decompose(HE_VEC *Sep, Superedge *Sup, vector<HE_VEC*> &Parts);
	
	HE_VEC getNeighborEdges(HE_VEC &Edges);

	bool isBalanced(vector<HE_VEC*> &Parts, int CompSize);
		

public:
	static int MyMaxRecursion;
	static Hypergraph *MyBaseGraph;

	BalKDecomp(Hypergraph *HGraph, int k, int RecLevel = 0);
	virtual ~BalKDecomp();

	// Finds or constructs a hypergraph from a list of edges and a superedge
	// Returns false if hypergraph is in sFailedHg
	bool getHypergraph(Hypergraph **Hg, bool *Succ, HE_VEC *Part, Hyperedge *Sup);

	// Constructs a hypertree decomposition of width at most MyK (if it exists)
	virtual Hypertree *buildHypertree();

	static void init(Hypergraph *BaseGraph, int MaxRecursion) {
		MyMaxRecursion = MaxRecursion;
		MyBaseGraph = BaseGraph;
	}
};

#endif
