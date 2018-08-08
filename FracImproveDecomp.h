// Models the algorithm det-k-decomp.
//
//////////////////////////////////////////////////////////////////////

#if !defined(CLS_FracImproveDecomp)
#define CLS_FracImproveDecomp

#include <set>
#include <vector>

#include "FractionalEdgeCover.h"
#include "DetKDecomp.h"
#include "FecCalculator.h"

class Hypergraph;
class Hyperedge;
class Hypertree;
class Vertex;
class CompSet;


class FracImproveDecomp : DetKDecomp
{
private:
	FecCalculator MyFecCalculator;

	mutable double globalBestFW;
	double threshold;

	bool verifyFracHypertreeWidth(HypertreeSharedPtr &htree, double &outFW) const;

	VertexSet computeChi(const HyperedgeVector &comp, const shared_ptr<Separator> &Sep, const VertexSet &Connector) const;

	// Builds a hypertree decomposition according to k-decomp by covering connector nodes
	virtual HypertreeSharedPtr decomp(const HyperedgeVector &HEdges, double &outFW, const VertexSet &Connector = VertexSet(), int RecLevel = 0) const;

	virtual HypertreeSharedPtr decomp(const DecompComponent &comp, double &outFW, int recLevel) const {
		return decomp(comp.component(), outFW, comp.connector(), recLevel);
	}

public:
	// Constructor
	FracImproveDecomp(const HypergraphSharedPtr &HGraph, int k);

	// Destructor
	virtual ~FracImproveDecomp();

	// Constructs a hypertree decomposition of width at most iK (if it exists)
	HypertreeSharedPtr buildHypertree(double minImprovement, double &fw);
};


#endif // !defined(CLS_DetKDecomp)

