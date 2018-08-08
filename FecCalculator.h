#if !defined(CLS_FEC_CALCULATOR)
#define CLS_FEC_CALCULATOR

#include <set>

#include "NamedEntity.h"
#include "FractionalEdgeCover.h"
#include "Hypergraph.h"
#include "Hyperedge.h"
#include "Vertex.h"

struct Map {
	NamedEntitySharedPtr comp;
	int index;

	bool compareMaps(Map *m1, Map *m2)
	{
		NamedEntitySharedPtr c1 = m1->comp;
		NamedEntitySharedPtr c2 = m2->comp;
		return c1->getId() < c2->getId();
	}
};

struct classcomp {
	bool operator() (const Map *m1, const Map *m2) const
	{
		NamedEntitySharedPtr c1 = m1->comp;
		NamedEntitySharedPtr c2 = m2->comp;
		return c1->getId() < c2->getId();
	}
};

class FecCalculator
{
private:
	HypergraphSharedPtr MyHg;

	HyperedgeSet getIncidentEdges(const VertexSet &chi, set<Map *, classcomp> &nodeToindexMap,
		set<Map *> &edgeToIndexMap, vector<HyperedgeSharedPtr> &indexToEdgeMap) const;

	void computeElements(const VertexSet &, std::set<Map *, classcomp>&,
		const HyperedgeSet &, std::set<Map *>&, double *) const;

public:
	FecCalculator(const HypergraphSharedPtr &hg);

	virtual ~FecCalculator();

	FractionalEdgeCover computeFEC(const VertexSet &chi, double &weight) const;
};

#endif // !defined(CLS_FEC_CALCULATOR)