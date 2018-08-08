
#include <algorithm>
#include <cmath>
#include <memory>

#include "Globals.h"
#include "Hypergraph.h"
#include "CombinationIterator.h"
#include "Parser.h"

void Hypergraph::labelReachEdges(const HyperedgeSharedPtr &edge, int label) const
{
	edge->setLabel(label);

	for (auto e : EdgeNeighbors.at(edge))
		if (e->getLabel() == 0)
			labelReachEdges(e, label);
}


void Hypergraph::buildHypergraph(Parser &P)
{
	VertexVector vertices;
	HyperedgeSharedPtr edge;

	for (int i = 0; i < P.getNbrOfVars(); i++)
		vertices.push_back(std::make_shared<Vertex>(P.getVariable(i)));

	for (int i = 0; i < P.getNbrOfAtoms(); i++) {
		edge = std::make_shared<Hyperedge>(P.getAtom(i));
		for (int j = 0; j < P.getNbrOfVars(i); j++) 
			edge->add(vertices[P.getNextAtomVar()]);
		this->insertEdge(edge);
	}
}

bool Hypergraph::hasAllEdges(const HyperedgeVector &edges) const
{
	for (auto he : edges)
		if (!hasEdge(he))
			return false;

	return true;
}

bool Hypergraph::hasAllEdges(const HyperedgeSet &edges) const
{
	for (auto he : edges)
		if (!hasEdge(he))
			return false;

	return true;
}

HyperedgeSharedPtr Hypergraph::getEdgeByID(int id) const
{
	for (auto he : Edges)
		if (he->getId() == id)
			return he;

	return nullptr;
}

VertexSharedPtr Hypergraph::getVertexByID(int id) const
{
	for (auto v : Vertices)
		if (v->getId() == id)
			return v;
	return nullptr;
}

void Hypergraph::setVertexLabels(int value) const
{
	for (auto v : Vertices)
		v->setLabel(value);
}

void Hypergraph::setEdgeLabels(int value) const
{
	for (auto he : Edges)
		he->setLabel(value);
}

void Hypergraph::setAllLabels(int value) const
{
	setVertexLabels(value);
	setEdgeLabels(value);
}

/*
void Hypergraph::resetEdgeLabels(int val)
{
	for (auto he : Edges)
		he->setLabel(val);
}

void Hypergraph::resetVertexLabels(int val)
{
	for (auto v : Vertices)
		v->setLabel(val);
}
*/

void Hypergraph::insertEdge(const HyperedgeSharedPtr &edge)
{
	if (getEdgeByID(edge->getId()) != nullptr )
		writeErrorMsg("This hypergraph already contains a Hyperedge with id " + to_string(edge->getId()), "Hypergraph::insertEdge");

	if (dynamic_pointer_cast<Superedge>(edge) != nullptr)
		CntSuperedges++;

	Edges.insert(edge);

	for (auto v : edge->allVertices()) {
		if (find(Vertices.begin(), Vertices.end(), v) == Vertices.end())
			Vertices.insert(v);
		EdgeNeighbors[edge].insert(VertexNeighbors[v].cbegin(), VertexNeighbors[v].cend());
		for (auto e : VertexNeighbors[v])
			EdgeNeighbors[e].insert(edge);
	    VertexNeighbors[v].insert(edge);
	}
}


bool Hypergraph::isConnected() const
{
	setAllLabels();

	if (getNbrOfEdges() > 0)
		labelReachEdges(*Edges.begin());
	
	for (auto e : Edges)
		if (e->getLabel() == 0)
			return false;

	return true;
}

void Hypergraph::makeDual(Hypergraph & hg) const
{
	unordered_map<uint, VertexSharedPtr> newVertices;

	//Every edge becomes a vertex
	for (auto e : Edges)
		newVertices[e->getId()] = make_shared<Vertex>(e->getId(), e->getName());

	//Every vertex becomes an edge
	for (auto v : Vertices) {
		HyperedgeSharedPtr e = make_shared<Hyperedge>(v->getId(), v->getName());
		for (auto n : allVertexNeighbors(v))
			e->add(newVertices[n->getId()]);
		hg.insertEdge(e);
	}
	
}

HyperedgeVector Hypergraph::getMCSOrder()
{
	HyperedgeVector order;
	HyperedgeVector candidates;
	HyperedgeSet::iterator he_iter;
	uint max_card{ 0 };
	uint tmp_card{ 0 };
	
	HyperedgeSharedPtr he;

	//Reset edge labels (used to determine which hyperedges have been removed)
	setEdgeLabels();

	//Select randomly an initial hyperedge
	auto it = Edges.begin();
	for (int i = 0; i < random_range(0, Edges.size() - 1); i++, it++);
	he = *it;
	he->setLabel(1);
	order.push_back(he);
	
	//Remove nodes with highest connectivity iteratively
	do {
		he = nullptr;
		he_iter = Edges.begin();
		max_card = 0 ;
		//find first vertex that has not been removed yet
		for (; he_iter != Edges.end() && (*he_iter)->getLabel() != 0; he_iter++);
		if (he_iter != Edges.end()) {
			//count its connectivity
			he = *he_iter;
			for (auto n : allEdgeNeighbors(he))
				max_card += n->getLabel();
			candidates.push_back(he);

			// Search for the node with highest connectivity, 
			// i.e., with highest number of neighbours in the set of nodes already removed
			for (; he_iter != Edges.end(); he_iter++) {
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

int Hypergraph::degree() const
{
	int maxDegree = 0;

	for (const VertexSharedPtr v : Vertices) {
		int degree;
		try {
			degree = VertexNeighbors.at(v).size();
		}
		catch (const std::out_of_range& oor) {
			degree = 0;
		}
		if (degree > maxDegree) maxDegree = degree;
	}

	return maxDegree;
}

int Hypergraph::bip(int k) const
{
	int maxBip = 0;
	HyperedgeVector edges(Edges.begin(), Edges.end());

	if (k <= Edges.size()) {
		CombinationIterator comb(Edges.size(), k);
		comb.setStage(k);
		int *indices;
		while ((indices = comb.next()) != nullptr) {
			VertexVector vertices;

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
		VertexVector vertices;

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

int Hypergraph::vcDimension() const
{
	int maxVC = (int)floor(log(Edges.size()) / log(2));
	int i;
	VertexVector vertices(Vertices.begin(), Vertices.end());

	//Find the maximum cardinality of a shattered subset of V
	for (i = 1; i <= maxVC; i++) {
		bool shattered = false;

		//For each subset X of size vc check if it is shattered, if X is shattered then vc is at least i
		int *indices;
		CombinationIterator cit(vertices.size(), i);
		cit.setStage(i);
		while ((indices = cit.next()) != nullptr && !shattered) {
			bool check_x = true;
			set_type set_x;
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
				for (auto e_it = Edges.begin(); e_it != Edges.end() && !edge_found; e_it++) {
					set_type help_x;

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
	for (auto e : Edges)
		if ((help = e->getNbrOfVertices()) > ari)
			ari = help;

	return ari;
}



