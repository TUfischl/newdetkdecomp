
#include <algorithm>

#include "Globals.h"
#include "Hypergraph.h"

void Hypergraph::labelReachEdges(Hyperedge * edge)
{
	edge->setLabel(1);

	for (auto e : MyEdgeNeighbors[edge])
		if (e->getLabel() == 0)
			labelReachEdges(e);
}

Hypergraph::Hypergraph()
{
}


Hypergraph::~Hypergraph()
{
	if (MyParent == nullptr) {
		for (auto e : MyEdges)
			delete e;
		for (auto v : MyVertices)
			delete v;
	}
}

void Hypergraph::buildHypergraph(Parser * P)
{
	vector<Vertex  *> vertices;
	Hyperedge* edge;

	for (int i = 0; i < P->getNbrOfVars(); i++)
		vertices.push_back(new Vertex(P->getVariable(i)));

	for (int i = 0; i < P->getNbrOfAtoms(); i++) {
		edge = new Hyperedge(P->getAtom(i));
		for (int j = 0; j < P->getNbrOfVars(i); j++) 
			edge->add(vertices[P->getNextAtomVar()]);
		this->insertEdge(edge);
	}
}

Hyperedge * Hypergraph::getEdgeByID(int id)
{
	for (auto he : MyEdges)
		if (he->getId() == id)
			return he;
	return nullptr;
}

Vertex * Hypergraph::getVertexByID(int id)
{
	for (auto v : MyVertices)
		if (v->getId() == id)
			return v;
	return nullptr;
}

void Hypergraph::resetEdgeLabels(int val)
{
	for (auto he : MyEdges)
		he->setLabel(val);
}

void Hypergraph::resetVertexLabels(int val)
{
	for (auto v : MyVertices)
		v->setLabel(val);
}

Hyperedge * Hypergraph::getEdgeById(int id)
{
	for (auto e : MyEdges)
		if (e->getId() == id)
			return e;
	return nullptr;
}


void Hypergraph::insertEdge(Hyperedge * edge)
{
	if (getEdgeById(edge->getId()) != nullptr)
		writeErrorMsg("This hypergraph already contains a Hyperedge with id " + to_string(edge->getId()), "Hypergraph::insertEdge");

	MyEdges.push_back(edge);

	if (edge->isHeavy())
		MyCntHeavy++;

	for (auto v : edge->allVertices()) {
		if (find(MyVertices.begin(), MyVertices.end(), v) == MyVertices.end())
			MyVertices.push_back(v);
		MyEdgeNeighbors[edge].insert(MyVertexNeighbors[v].cbegin(), MyVertexNeighbors[v].cend());
		for (auto e : MyVertexNeighbors[v])
			MyEdgeNeighbors[e].insert(edge);
		MyVertexNeighbors[v].insert(edge);
	}
}

bool Hypergraph::isConnected()
{
	resetEdgeLabels();

	if (getNbrOfEdges() > 0)
		labelReachEdges(MyEdges[0]);
	
	for (auto e : MyEdges)
		if (e->getLabel() == 0)
			return false;

	return true;
}

void Hypergraph::makeDual(Hypergraph & hg)
{
	unordered_map<uint, Vertex*> newVertices;

	//Every edge becomes a vertex
	for (auto e : MyEdges)
		newVertices[e->getId()] = new Vertex(e->getId(), e->getName());

	//Every vertex becomes an edge
	for (auto v : MyVertices) {
		Hyperedge* e = new Hyperedge(v->getId(), v->getName());
		for (auto n : MyVertexNeighbors[v])
			e->add(newVertices[n->getId()]);
		hg.insertEdge(e);
	}
	
}

HE_VEC Hypergraph::getMCSOrder()
{
	HE_VEC order;
	HE_VEC candidates;
	HE_VEC::iterator he_iter;
	uint max_card{ 0 };
	uint tmp_card{ 0 };
	
	Hyperedge *he;

	//Reset edge labels (used to determine which hyperedges have been removed)
	resetEdgeLabels();

	//Select randomly an initial hyperedge
	he = MyEdges[random_range(0, MyEdges.size() - 1)];
	he->setLabel(1);
	order.push_back(he);
	
	//Remove nodes with highest connectivity iteratively
	do {
		he = nullptr;
		he_iter = MyEdges.begin();
		max_card = 0 ;
		//find first vertex that has not been removed yet
		for (; he_iter != MyEdges.end() && (*he_iter)->getLabel() != 0; he_iter++);
		if (he_iter != MyEdges.end()) {
			//count its connectivity
			he = *he_iter;
			for (auto n : allEdgeNeighbors(he))
				max_card += n->getLabel();
			candidates.push_back(he);

			// Search for the node with highest connectivity, 
			// i.e., with highest number of neighbours in the set of nodes already removed
			for (; he_iter != MyEdges.end(); he_iter++) {
				he = *he_iter;
				if (he->getLabel() == 0) {
					tmp_card = 0;
					for (auto n : allEdgeNeighbors(he))
						tmp_card += n->getLabel();
					if (tmp_card >= max_card) {
						if (tmp_card > max_card) {
							candidates.clear();
							max_card = tmp_card;
						}
						candidates.push_back(he);
					}
				}
			}

			//randomly select the next node with highest connectivity
			he = candidates[random_range(0, candidates.size() - 1)];
			candidates.clear();
			he->setLabel(1);
			order.push_back(he);
		}
	} while (he != nullptr);

	return order;
}
