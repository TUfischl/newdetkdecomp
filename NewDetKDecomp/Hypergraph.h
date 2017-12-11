#pragma once

#if !defined(CLS_HYPERGRAPH)
#define CLS_HYPERGRAPH

#include<vector>
#include<unordered_map>
#include<unordered_set>

#include "Hyperedge.h"
#include "Parser.h"

class Hypergraph
{ 
private:
	// Stores a parent hypergraph (hence, this hypergraph is a subgraph/copy of another graph
	// The desctructor then keeps edges and vertices
	Hypergraph *MyParent{ nullptr };

	HE_SET MyEdges;
	VE_SET MyVertices;

	unordered_map<Vertex *, HE_SET> MyVertexNeighbors;
	unordered_map<Hyperedge *, HE_SET> MyEdgeNeighbors;

	//Nbr of heavy edges (weight > 1)
	int MyCntHeavy{ 0 };

	// Labels all edges reachable from Edge
	void labelReachEdges(Hyperedge *edge);

public:
	Hypergraph();
	virtual~Hypergraph();

	// Build Hypergraph from a parser object
	void buildHypergraph(Parser *P);

	// Sets the parent of this hypergraph
	void setParent(Hypergraph* hg) { MyParent = hg;  }

	// Returns the number of edges in the hypergraph
	size_t getNbrOfEdges() { return MyEdges.size();  }

	// Returns the number of heavy edges (with weight > 1)
	int getNbrOfHeavyEdges() { return MyCntHeavy; }

	// Returns the number of nodes in the hypergraph
	size_t getNbrOfVertices() { return MyVertices.size();  }

	// Returns true if hyperedge is stored in hypergraph
	bool hasEdge(Hyperedge *he) { return MyEdges.find(he) != MyEdges.end(); }

	bool hasAllEdges(HE_SET *edges);
	bool hasAllEdges(HE_VEC *edges);

	// Returns the hyperedge stored at position iPos
	//Hyperedge *getEdge(int pos) { return MyEdges[pos]; }

	// Returns the vertex stored at position iPos
	//Vertex *getVertex(int pos) { return MyVertices[pos]; }

	// Returns the hyperedge with ID iID
	Hyperedge *getEdgeByID(int id);

	// Returns the node with ID iID
	Vertex *getVertexByID(int id);

	// Sets labels of all edges to zero
	void resetEdgeLabels(int val = 0);

	// Sets labels of all vertices to zero
	void resetVertexLabels(int val = 0);

	// Inserts an hyperedge into the hypergraph
	void insertEdge(Hyperedge* edge);

	// Checks whether the hypergraph is connected
	bool isConnected();

	// Transforms the hypergraph into its dual hypergraph
	void makeDual(Hypergraph& hg);

	auto allEdges() -> decltype(make_iterable(MyEdges.begin(), MyEdges.end()))
	{
		return make_iterable(MyEdges.begin(), MyEdges.end());
	}

	auto allVertices() -> decltype(make_iterable(MyVertices.begin(), MyVertices.end()))
	{
		return make_iterable(MyVertices.begin(), MyVertices.end());
	}

	int nbrOfVertexNeighbors(Vertex *v) { return MyVertexNeighbors[v].size(); }

	auto allVertexNeighbors(Vertex *v) -> decltype(make_iterable(MyVertexNeighbors[v].begin(), MyVertexNeighbors[v].end()))
	{
		return make_iterable(MyVertexNeighbors[v].begin(), MyVertexNeighbors[v].end());
	}

	/*
	HE_SET::iterator getVertexNeighborsBegin(Vertex * v) {
		return MyVertexNeighbors[v].begin();
	}

	HE_SET::iterator getVertexNeighborsEnd(Vertex *v) {
		return MyVertexNeighbors[v].end();
	}
	*/

	int nbrOfEdgeNeighbors(Hyperedge *e) { return MyEdgeNeighbors[e].size(); }

	auto allEdgeNeighbors(Hyperedge *e) -> decltype(make_iterable(MyEdgeNeighbors[e].begin(), MyEdgeNeighbors[e].end()))
	{
		return make_iterable(MyEdgeNeighbors[e].begin(), MyEdgeNeighbors[e].end());
	}

	/*
	HE_SET::iterator getEdgeNeighborsBegin(Hyperedge * e) {
		return MyEdgeNeighbors[e].begin();
	}

	HE_SET::iterator getEdgeNeighborsEnd(Hyperedge *e) {
		return MyEdgeNeighbors[e].end();
	}

	HE_SET::const_iterator getVertexNeighborsCBegin(Vertex * v) {
		return MyVertexNeighbors[v].cbegin();
	}

	HE_SET::const_iterator getVertexNeighborsCEnd(Vertex *v) {
		return MyVertexNeighbors[v].cend();
	}

	HE_SET::const_iterator getEdgeNeighborsCBegin(Hyperedge * e) {
		return MyEdgeNeighbors[e].cbegin();
	}

	HE_SET::const_iterator getEdgeNeighborsCEnd(Hyperedge *e) {
		return MyEdgeNeighbors[e].cend();
	}
	*/

	/* Calculates an ordering of the hyperedges based on the 
	 * Maximum-Cardinality-Search algorithm. This algorithm 
	 * originally is proposed for vertices, here we have a dual
	 * version for hyperedges.
	 */
	HE_VEC getMCSOrder();

	/*
	 * Hypergraph Properties
	 */
	
	 /* 
	  * Calculates the degree of the hypergraph.
	  */
	int degree();

	/*
	* Calculates the k-BIP  of the hypergraph.
	*/
	int bip(int k);

	/*
	* Calculates the k-BIP  of the hypergraph.
	*/
	int vcDimension();


};

#endif
