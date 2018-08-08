#include <string>

#include "Vertex.h"
#include "Hyperedge.h"
#include "FractionalEdgeCover.h"

using namespace std;

FractionalEdgeCover::FractionalEdgeCover(const VertexSet &bag, const HyperedgeVector &edges, const vector<double> &weights)
{
	this->bag = bag;
	this->edges = edges;
	this->weights = weights;
	this->weight = -1.0;
}

FractionalEdgeCover::~FractionalEdgeCover()
{
}

const VertexSet &FractionalEdgeCover::getBag()
{
	return bag;
}

const HyperedgeVector &FractionalEdgeCover::getEdges()
{
	return edges;
}

const vector<double> &FractionalEdgeCover::getWeights()
{
	return weights;
}

double FractionalEdgeCover::computeWeight()
{
	if (weight < 0) {
		weight = 0;
		for (auto w : weights)
		{
			weight += w;
		}
	}
	return weight;
}

string FractionalEdgeCover::toString() const
{
	// build bag's string
	string sBag = "{ ";
	auto bagIt = bag.begin();
	while (bagIt != bag.end())
	{
		sBag += (*bagIt)->getName();
		bagIt++;
		if (bagIt != bag.end())
		{
			sBag += ", ";
		}
	}
	sBag += " }";

	// build covers' string
	string sCover = "{ ";
	auto edgesIt = edges.begin();
	auto weightsIt = weights.begin();
	while (edgesIt != edges.end())
	{
		sCover += (*edgesIt)->getName();
		sCover += "=";
		sCover += to_string(*weightsIt);
		edgesIt++;
		weightsIt++;
		if (edgesIt != edges.end())
		{
			sCover += ", ";
		}
	}
	sCover += " }";

	return sBag + " " + sCover;
}