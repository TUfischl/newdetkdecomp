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
	int MyRecLevel;

	Hypertree *decomp(HE_VEC &HEdges);
	Hypertree *decompose(HE_VEC *Sep, Superedge *Sup, vector<HE_VEC*> &Parts);

	HE_VEC getNeighborEdges(HE_VEC &Edges);

	bool isBalanced(vector<HE_VEC*> &Parts, int CompSize);
		

public:
	static int MyMaxRecursion;
	static Hypergraph *MyBaseGraph;

	BalKDecomp(Hypergraph *HGraph, int k, int RecLevel = 0);
	virtual ~BalKDecomp();

	// Constructs a hypertree decomposition of width at most MyK (if it exists)
	virtual Hypertree *buildHypertree();

	static void init(Hypergraph *BaseGraph, int MaxRecursion) {
		MyMaxRecursion = MaxRecursion;
		MyBaseGraph = BaseGraph;
	}
};

#endif
