// SetCover.cpp: implementation of the SetCover class.
//
//////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <ctime>
#include <vector>

using namespace std;

#include "SetCover.h"
#include "Vertex.h"
#include "Hyperedge.h"
#include "Hypergraph.h"
#include "Globals.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


SetCover::SetCover(Hypergraph *H) : MyH { H }
{
}


SetCover::~SetCover()
{
}



//////////////////////////////////////////////////////////////////////
// Class methods
//////////////////////////////////////////////////////////////////////


HE_SET SetCover::NodeCover1(VE_SET &Vertices, HE_SET &HEdges, bool bDeterm)
{
	size_t iNbrOfCovEdges, iNbrOfUncovNodes, iMax;
	HE_SET CovEdges{};
	Hyperedge *hedge;
	HE_VEC CovCand;

	iNbrOfCovEdges = 0;
	iNbrOfUncovNodes = Vertices.size();

	// Clear labels on hypergraph (hide all vertices and edges)
	MyH->resetEdgeLabels(-1);
	MyH->resetVertexLabels(-1);

	// Unhide vertices to be covered
	for (auto v : Vertices) {
		/* for (auto it = MyH->getVertexNeighborsBegin(v); it != MyH->getVertexNeighborsEnd(v); it++)
			(*it)->setLabel(-1); */
		v->setLabel(0);
	}
		
	// Unhide edges that can be used to cover the vertices
	for (auto he : HEdges)
		he->setLabel(0);

	// STEP 1: Choose all hyperedges that cover a node which is not contained in any other hyperedge
	for(auto v : Vertices) {
		// Check if vertex is already covered
		if (v->getLabel() == 0) {
			// Check whether there is only one hyperedge that covers the node
			hedge = nullptr;
			for (auto he : MyH->allVertexNeighbors(v))
				if(he->getLabel() != -1)
					if(hedge != nullptr) {
						hedge = nullptr;
						break;
					}
					else
						hedge = he;
		
			if(hedge != nullptr) {
				++iNbrOfCovEdges;
				hedge->setLabel(-1);
				CovEdges.insert(hedge);
			
				// Mark vertices covered by the chosen edge
				for (auto v : hedge->allVertices()) {
					if (v->getLabel() > 0 && Vertices.find(v) != Vertices.end()) {
						v->setLabel(-1);
						--iNbrOfUncovNodes;
					}
				}
			}
		}
	}

	// STEP 2: Choose iteratively hyperedges that cover a maximum number of uncovered nodes
	while(iNbrOfUncovNodes > 0) {
		// Label each hyperedge with the number of uncovered nodes it covers
		for (auto v : Vertices)
			if (v->getLabel() == 0)
				for (auto he : MyH->allVertexNeighbors(v))
					if (he->getLabel() >= 0)
						he->incLabel();

		// Search for the hyperedges with the maximum labeling
		iMax = -1;
		for(auto he : HEdges) {
			if(he->getLabel() > iMax) {
				CovCand.clear();
				CovCand.push_back(he);
				iMax = he->getLabel();
			}
			else
				if(he->getLabel() == iMax)
					CovCand.push_back(he);

			// Reset hyperedge labels
			if(he->getLabel() > 0)
				he->setLabel(0);
		}

		// Select a hyperedge with maximum labeling
		if(bDeterm) {
			hedge = CovCand[0];
			for (auto he : CovCand)
				if (he->getId() < hedge->getId())
					hedge = he;
		}
		else
			hedge = CovCand[rand()%CovCand.size()];
		CovCand.clear();

		++iNbrOfCovEdges;
		hedge->setLabel(-1);
		CovEdges.insert(hedge);

		// Mark vertices covered by the chosen edge
		for (auto v : hedge->allVertices()) {
			if (v->getLabel() > 0 && Vertices.find(v) != Vertices.end()) {
				v->setLabel(-1);
				--iNbrOfUncovNodes;
			}
		}
	}

	return CovEdges;
}


HE_SET SetCover::NodeCover2(VE_SET &Vertices, HE_SET &HEdges, bool bDeterm)
{
	size_t iNbrOfCovEdges, iNbrOfUncovNodes, iContained;
	int i;
	HE_SET CovEdges{};
	double iMax;
	unordered_map<Vertex *, double> NodeWeights;
	unordered_map<Hyperedge*, double> EdgeWeights;
	Hyperedge *hedge;
	HE_VEC CovCand;

	iNbrOfCovEdges = 0;
	iNbrOfUncovNodes = Vertices.size();

	/*
	// Allocate memory for the weights
	NodeWeights.reserve(Vertices.size());
	EdgeWeights.reserve(Nodes.size());
	*/

	MyH->resetEdgeLabels(-1);
	MyH->resetVertexLabels(-1);

	// Compute the node weights
	for(auto v : Vertices) {
		iContained = 0;
		for (auto he : MyH->allVertexNeighbors(v)) {
			he->setLabel(-1);
			if(HEdges.find(he)!=HEdges.end())
				++iContained;
		}
		v->setLabel(0);
		NodeWeights[v] = 1.0 - ((double)iContained / (double)HEdges.size());
	}

	// Label each edge with its index in the EdgeWeights array
	i = 0;
	for(auto he : HEdges) {
		he->setLabel(i++);
		EdgeWeights[he] = 0;
	}

	// STEP 1: Choose all hyperedges that cover a node which is not contained in any other hyperedge
	for (auto v : Vertices) {
		// Check if vertex is already covered
		if (v->getLabel() == 0) {
			// Check whether there is only one hyperedge that covers the node
			hedge = nullptr;
			for (auto he : MyH->allVertexNeighbors(v))
				if (he->getLabel() != -1)
					if (hedge != nullptr) {
						hedge = nullptr;
						break;
					}
					else
						hedge = he;

			if (hedge != nullptr) {
				++iNbrOfCovEdges;
				hedge->setLabel(-1);
				CovEdges.insert(hedge);

				// Mark vertices covered by the chosen edge
				for (auto v : hedge->allVertices()) {
					if (v->getLabel() > 0 && Vertices.find(v) != Vertices.end()) {
						NodeWeights[v] = 0;
						v->setLabel(-1);
						--iNbrOfUncovNodes;
					}
				}
			}
		}
	}

	// Compute the hyperedge weights
	for(auto v : Vertices)
		if(NodeWeights[v] > 0)
			for(auto he : MyH->allVertexNeighbors(v))
				if(he->getLabel() >= 0)
					EdgeWeights[he] += NodeWeights[v];

	// STEP 2: Choose iteratively hyperedges that have the highest weight
	while(iNbrOfUncovNodes > 0) {
		// Search for the hyperedge with the highest weight
		iMax = -1;
		for(auto he : HEdges)
			if (he->getLabel() >= 0)
				if (EdgeWeights[he] > iMax) {
					CovCand.clear();
					CovCand.push_back(he);
					iMax = EdgeWeights[he];
				}
				else
					if (EdgeWeights[he] == iMax)
						CovCand.push_back(he);

		// Select a hyperedge with maximum labeling
		if(bDeterm) {
			hedge = CovCand[0];
			for (auto he : CovCand)
				if (he->getId() < hedge->getId())
					hedge = he;
		}
		else
			hedge = CovCand[rand()%CovCand.size()];
		CovCand.clear();

		++iNbrOfCovEdges;
		EdgeWeights[hedge] = 0;
		hedge->setLabel(-1);
		CovEdges.insert(hedge);

		// Reset the node weight of covered nodes and update the hyperedge weights
		for (auto v : hedge->allVertices()) {
			if (v->getLabel() > 0 && Vertices.find(v) != Vertices.end()) {
				for (auto he : MyH->allVertexNeighbors(v))
					if (he->getLabel() >= 0)
						EdgeWeights[he] -= NodeWeights[v];
				NodeWeights[v] = 0;
				v->setLabel(-1);
				--iNbrOfUncovNodes;
			}
		}
	}

	return CovEdges;
}


bool SetCover::covers(VE_SET &Vertices, HE_SET &HEdges)
{
	// Reset node labels
	for(auto v : Vertices)
		v->setLabel(0);

	// Mark all nodes that can be covered
	for(auto he : HEdges)
		for(auto v : he->allVertices())
			v->setLabel(1);

	// Check whether there are unmarked nodes
	for(auto v : Vertices)
		if(v->getLabel() == 0)
			return false;

	return true;
}

/*
bool SetCover::covers(Node **Nodes, Hyperedge **HEdges)
{
	int i, j;

	// Reset node labels
	for(i=0; Nodes[i] != NULL; i++)
		Nodes[i]->setLabel(0);

	// Mark all nodes that can be covered
	for(i=0; HEdges[i] != NULL; i++)
		for(j=0; j < HEdges[i]->getNbrOfNodes(); j++)
			HEdges[i]->getNode(j)->setLabel(1);

	// Check whether there are unmarked nodes
	for(i=0; Nodes[i] != NULL; i++)
		if(Nodes[i]->getLabel() == 0)
			return false;

	return true;
}
*/


HE_SET SetCover::cover(VE_SET &Nodes, HE_SET &HEdges)
{
	HE_SET CovEdges1, CovEdges2;

	if(!covers(Nodes, HEdges))
		writeErrorMsg("Covering not possible.", "SetCover::cover");

	// Apply set covering
	CovEdges1 = NodeCover1(Nodes, HEdges, true);

	CovEdges2 = NodeCover1(Nodes, HEdges, false);
	if(CovEdges2.size() < CovEdges1.size())
		CovEdges1 = CovEdges2;

	CovEdges2 = NodeCover2(Nodes, HEdges, true);
	if(CovEdges2.size() < CovEdges1.size()) 
		CovEdges1 = CovEdges2;

	CovEdges2 = NodeCover2(Nodes, HEdges, false);
	if(CovEdges2.size() < CovEdges1.size()) 
		CovEdges1 = CovEdges2;

	return CovEdges1;
}

