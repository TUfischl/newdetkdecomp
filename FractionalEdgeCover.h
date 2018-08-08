#if !defined(CLS_FractionalEdgeCover)
#define CLS_FractionalEdgeCover

#include <string>
#include <set>
#include <vector>

#include "Vertex.h"
#include "Hyperedge.h"

class FractionalEdgeCover
{
private:
	VertexSet bag;
	HyperedgeVector edges;
	std::vector<double> weights;
	double weight;

public:
	FractionalEdgeCover(const VertexSet &bag, const HyperedgeVector &edges, const vector<double> &weights);
	virtual ~FractionalEdgeCover();

	const VertexSet &getBag();
	const HyperedgeVector &getEdges();
	const std::vector<double> &getWeights();

	double computeWeight();

	std::string toString() const;
};

#endif

