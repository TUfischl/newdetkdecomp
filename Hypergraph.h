#pragma once

#if !defined(CLS_HYPERGRAPH)
#define CLS_HYPERGRAPH

#include<memory>
#include<vector>
#include<unordered_map>
#include<unordered_set>

#include "Globals.h"
#include "Superedge.h"
#include "Hyperedge.h"
#include "Parser.h"
#include "NamedEntity.h"

class Hypergraph : NamedEntity
{ 
private:
	// Stores a parent hypergraph (hence, this hypergraph is a subgraph/copy of another graph
	// The desctructor then keeps edges and vertices
	HyperedgeSet Edges;
	VertexSet Vertices;
	std::shared_ptr<Hypergraph> Parent;

	unordered_map<VertexSharedPtr, HyperedgeSet> VertexNeighbors;
	unordered_map<HyperedgeSharedPtr, HyperedgeSet> EdgeNeighbors;

	//Nbr of heavy edges (weight > 1)
	int CntSuperedges{ 0 };

	// Labels all edges reachable from Edge
	void labelReachEdges(const HyperedgeSharedPtr &edge, int label = 1) const;

public:
	Hypergraph() : NamedEntity() {}

	// Build Hypergraph from a parser object
	void buildHypergraph(Parser &P);

	// Sets the parent of this hypergraph
	void setParent(const std::shared_ptr<Hypergraph> &hg) { Parent = hg;  }

	// Returns the number of edges in the hypergraph
	size_t getNbrOfEdges() const { return Edges.size();  }

	// Returns the number of heavy edges (with weight > 1)
	int getNbrOfSuperEdges() const { return CntSuperedges; }

	// Returns the number of nodes in the hypergraph
	size_t getNbrOfVertices() const { return Vertices.size();  }

	// Returns true if hyperedge is stored in hypergraph
	bool hasEdge(const HyperedgeSharedPtr &he) const { return Edges.find(he) != Edges.end(); }

	bool hasAllEdges(const HyperedgeSet &edges) const;
	bool hasAllEdges(const HyperedgeVector &edges) const;

	// Returns the hyperedge stored at position iPos
	//Hyperedge *getEdge(int pos) { return MyEdges[pos]; }

	// Returns the vertex stored at position iPos
	//Vertex *getVertex(int pos) { return MyVertices[pos]; }

	// Returns the hyperedge with ID iID
	HyperedgeSharedPtr getEdgeByID(int id) const;

	// Returns the node with ID iID
	VertexSharedPtr getVertexByID(int id) const;

	void setVertexLabels(int value = 0) const;
	void setEdgeLabels(int value = 0) const;
	void setAllLabels(int value = 0) const;

	// Sets labels of all edges to zero
	//void resetEdgeLabels(int val = 0);

	// Sets labels of all vertices to zero
	//void resetVertexLabels(int val = 0);

	// Inserts an hyperedge into the hypergraph
	void insertEdge(const HyperedgeSharedPtr &edge);

	// Checks whether the hypergraph is connected
	bool isConnected() const;

	// Transforms the hypergraph into its dual hypergraph
	void makeDual(Hypergraph& hg) const;

	auto allEdges() -> decltype(make_iterable(Edges.begin(), Edges.end()))
	{
		return make_iterable(Edges.begin(), Edges.end());
	}

	auto allEdges() const -> decltype(make_iterable(Edges.begin(), Edges.end()))
	{
		return make_iterable(Edges.begin(), Edges.end());
	}

	auto allVertices() -> decltype(make_iterable(Vertices.begin(), Vertices.end()))
	{
		return make_iterable(Vertices.begin(), Vertices.end());
	}

	auto allVertices() const -> decltype(make_iterable(Vertices.begin(), Vertices.end()))
	{
		return make_iterable(Vertices.begin(), Vertices.end());
	}

	size_t nbrOfVertexNeighbors(const VertexSharedPtr &v) const { return VertexNeighbors.at(v).size(); }

	auto allVertexNeighbors(const VertexSharedPtr &v) -> decltype(make_iterable(VertexNeighbors[v].begin(), VertexNeighbors[v].end()))
	{
		return make_iterable(VertexNeighbors[v].begin(), VertexNeighbors[v].end());
	}

	auto allVertexNeighbors(const VertexSharedPtr &v) const -> decltype(make_iterable(VertexNeighbors.at(v).begin(), VertexNeighbors.at(v).end()))
	{
		return make_iterable(VertexNeighbors.at(v).begin(), VertexNeighbors.at(v).end());
	}

	/*
	HE_SET::iterator getVertexNeighborsBegin(Vertex * v) {
		return MyVertexNeighbors[v].begin();
	}

	HE_SET::iterator getVertexNeighborsEnd(Vertex *v) {
		return MyVertexNeighbors[v].end();
	}
	*/

	size_t nbrOfEdgeNeighbors(HyperedgeSharedPtr &e) const { return EdgeNeighbors.at(e).size(); }

	auto allEdgeNeighbors(const HyperedgeSharedPtr &e) -> decltype(make_iterable(EdgeNeighbors[e].begin(), EdgeNeighbors[e].end()))
	{
		return make_iterable(EdgeNeighbors[e].begin(), EdgeNeighbors[e].end());
	}

	auto allEdgeNeighbors(const HyperedgeSharedPtr &e) const -> decltype(make_iterable(EdgeNeighbors.at(e).begin(), EdgeNeighbors.at(e).end()))
	{
		return make_iterable(EdgeNeighbors.at(e).begin(), EdgeNeighbors.at(e).end());
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
	HyperedgeVector getMCSOrder();

	/*
	 * Hypergraph Properties
	 */
	
	 /* 
	  * Calculates the degree of the hypergraph.
	  */
	int degree() const;

	/*
	* Calculates the k-BIP  of the hypergraph.
	*/
	int bip(int k) const;

	/*
	* Calculates the k-BIP  of the hypergraph.
	*/
	int vcDimension() const;

	/*
	* Calculates the arity of the hypergraph.
	*/
	int arity() const;


};

using HypergraphSharedPtr = std::shared_ptr<Hypergraph>;

#endif
