// Models set cover algorithms.
//
//////////////////////////////////////////////////////////////////////


#if !defined(CLS_SETCOVER)
#define CLS_SETCOVER

#include <set>

#include "Globals.h"

class Vertex;
class Hyperedge;
class Hypergraph;

class SetCover  
{
private:
	Hypergraph *MyH;

	// Covers a set of nodes by a set of hyperedges
	HE_SET NodeCover1(VE_SET &Vertices, HE_SET &HEdges, bool bDeterm);

	// Covers a set of nodes by a set of hyperedges
	HE_SET NodeCover2(VE_SET &Vertices, HE_SET &HEdges, bool bDeterm);

public:
	// Constructor
	SetCover(Hypergraph *H);

	// Destructor
	virtual ~SetCover();

	// Checks whether a set of nodes can be covered by a set of hyperedges
	bool covers(VE_SET &Vertices, HE_SET &HEdges);

	// Checks whether a set of nodes can be covered by a set of hyperedges
	//bool covers(Vertex **Nodes, Hyperedge **HEdges);

	// Covers a set of nodes by a set of hyperedges
	HE_SET cover(VE_SET &Vertices, HE_SET &HEdges);
};


#endif // !defined(CLS_SETCOVER)
