
#include <algorithm>
#include <cmath>

#include "Globals.h"
#include "Hypergraph.h"
#include "CombinationIterator.h"

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

bool Hypergraph::hasAllEdges(HE_VEC * edges)
{
	for (auto he : *edges)
		if (!hasEdge(he))
			return false;

	return true;
}

bool Hypergraph::hasAllEdges(HE_SET * edges)
{
	for (auto he : *edges)
		if (!hasEdge(he))
			return false;

	return true;
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

void Hypergraph::insertEdge(Hyperedge * edge)
{
	if (getEdgeByID(edge->getId()) != nullptr )
		writeErrorMsg("This hypergraph already contains a Hyperedge with id " + to_string(edge->getId()), "Hypergraph::insertEdge");


	MyEdges.insert(edge);

	if (edge->isHeavy())
		MyCntHeavy++;

	for (auto v : edge->allVertices()) {
		if (find(MyVertices.begin(), MyVertices.end(), v) == MyVertices.end())
			MyVertices.insert(v);
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
		labelReachEdges(*MyEdges.begin());
	
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
	HE_SET::iterator he_iter;
	uint max_card{ 0 };
	uint tmp_card{ 0 };
	
	Hyperedge *he;

	//Reset edge labels (used to determine which hyperedges have been removed)
	resetEdgeLabels();

	//Select randomly an initial hyperedge
	auto it = MyEdges.begin();
	for (int i = 0; i < random_range(0, MyEdges.size() - 1); i++, it++);
	he = *it;
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

int Hypergraph::degree()
{
	int maxDegree = 0;

	for (auto v : MyVertices) {
		int degree = MyVertexNeighbors[v].size();
		if (MyVertexNeighbors[v].size() > maxDegree) maxDegree = degree;
	}

	return maxDegree;
}

int Hypergraph::bip(int k)
{
	int maxBip = 0;
	HE_VEC edges(MyEdges.begin(), MyEdges.end());

	if (k <= MyEdges.size()) {
		CombinationIterator comb(MyEdges.size(), k);
		comb.setStage(k);
		int *indices;
		while ((indices = comb.next()) != nullptr) {
			VE_VEC vertices;

			for (auto v : edges[indices[0]]->allVertices()) {
				bool found = true;
				for (int i = 1; indices[i] != -1 && found; i++)
					if (find(edges[indices[i]]->allVertices().begin(), edges[indices[i]]->allVertices().end(), v) == edges[indices[i]]->allVertices().end())
						found = false;

				if (found)
					vertices.push_back(v);
			}

			if (vertices.size() > maxBip)
				maxBip = vertices.size();
		}
	}
	else {
		VE_VEC vertices;

		for (auto v : edges[0]->allVertices()) {
			bool found = true;
			for (int i = 1; i < edges.size() && found; i++)
				if (find(edges[i]->allVertices().begin(), edges[i]->allVertices().end(), v) == edges[i]->allVertices().end())
					found = false;

			if (found)
				vertices.push_back(v);
		}

		return vertices.size();
	}

	return maxBip;
}

int Hypergraph::vcDimension()
{
	int maxVC = (int)floor(log(MyEdges.size()) / log(2));
	int i;
	VE_VEC vertices(MyVertices.begin(), MyVertices.end());

	//Find the maximum cardinality of a shattered subset of V
	for (i = 1; i <= maxVC; i++) {
		bool shattered = false;

		//For each subset X of size vc check if it is shattered, if X is shattered then vc is at least i
		int *indices;
		CombinationIterator cit(vertices.size(), i);
		cit.setStage(i);
		while ((indices = cit.next()) != nullptr && !shattered) {
			bool check_x = true;
			set<Vertex*> set_x;
			for (int j = 0; indices[j] != -1; j++)
				set_x.insert(vertices[indices[j]]);
			//Collection<String> setX = cit.next();
			//PowerSetIterator<String> itPSetX = new PowerSetIterator<String>(setX);
			powerset_type pow_set_x = powerset(set_x);

			//For each A \subseteq X check if there is an edge s.t. A = X \cap e
			//if there is a subset such that this check fails (checkX = false), then X is not shattered.
			for (powerset_type::iterator iter = pow_set_x.begin();
				iter != pow_set_x.end() && check_x;
				++iter) {

				bool edge_found = false;
				for (auto e_it = MyEdges.begin(); e_it != MyEdges.end() && !edge_found; e_it++) {
					set<Vertex*> help_x;

					for (auto v : set_x) {
						if (find((*e_it)->allVertices().begin(), (*e_it)->allVertices().end(), v) != (*e_it)->allVertices().end())
							help_x.insert(v);
					}

					if (help_x == *iter)
						edge_found = true;
				}

				if (!edge_found)
					check_x = false;

			}

			if (check_x)
				shattered = true;

		}

		if (!shattered)
			return i - 1;

	}
			

	return i - 1;
}

int Hypergraph::arity() const
{
	int ari = 0;
	int help = 0;
	for (auto e : MyEdges)
		if ((help = e->getNbrOfVertices()) > ari)
			ari = help;

	return ari;
}

