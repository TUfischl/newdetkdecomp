// Models set cover algorithms.
//
//////////////////////////////////////////////////////////////////////


#if !defined(CLS_SETCOVER)
#define CLS_SETCOVER

#include <set>

#include "Globals.h"
#include "Vertex.h"
#include "Hyperedge.h"
#include "Hypergraph.h"

class SetCover  
{
private:
	HypergraphSharedPtr MyH;

	// Covers a set of nodes by a set of hyperedges
	HyperedgeSet NodeCover1(const VertexSet &Vertices, const HyperedgeSet &HEdges, bool bDeterm);

	// Covers a set of nodes by a set of hyperedges
	HyperedgeSet NodeCover2(const VertexSet &Vertices, const HyperedgeSet &HEdges, bool bDeterm);

public:
	// Constructor
	SetCover(const HypergraphSharedPtr &H);
	  


	// Destructor
	virtual ~SetCover();

	// Checks whether a set of nodes can be covered by a set of hyperedges
	bool covers(const VertexSet &Vertices, const HyperedgeSet &HEdges);

	// Checks whether a set of nodes can be covered by a set of hyperedges
	//bool covers(Vertex **Nodes, Hyperedge **HEdges);

	// Covers a set of nodes by a set of hyperedges
	HyperedgeSet cover(const VertexSet &Vertices, const HyperedgeSet &HEdges);
};


#endif // !defined(CLS_SETCOVER)
