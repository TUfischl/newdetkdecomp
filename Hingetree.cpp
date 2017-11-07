
#include "Hingetree.h"
#include "Hyperedge.h"
#include "Globals.h"

Hingetree::Hingetree()
{
	//edges = new Hyperedge*[1];
	//edges[0] = nullptr;
}


Hingetree::~Hingetree()
{
	//delete[] edges;
}

Hingetree::Hingetree(vector<Hyperedge *> &edges) : MyEdges(edges)
{

}

Hyperedge * Hingetree::selectUnused(const set<Hyperedge*> &used)
{
	if (!IsMinimal) {
		for (auto he : MyEdges)
			if (used.find(he) == used.end())
				return he;
	}
	IsMinimal = true;
	return nullptr;
}

int Hingetree::nbrOfNodes()
{
	int sum = 0;

	for (auto child : MyChildren) {
		sum += child->dest->nbrOfNodes();
	}

	return sum + 1;
}

int Hingetree::sizeOfLargestHinge()
{
	int hingeSize = MyEdges.size();
	
	for (auto child : MyChildren) {
		int newSize = child->dest->sizeOfLargestHinge();
		if (newSize > hingeSize)
			hingeSize = newSize;
	}

	return hingeSize;
}

std::ostream & operator<<(std::ostream & os, const Hingetree & t)
{
	os << "{ [";
	for (auto it = t.MyEdges.cbegin(); it != t.MyEdges.cend(); ) {
		os << (*it)->getName();
		if (++it != t.MyEdges.cend())
			os << ",";
	}
	os << "]";
	for (auto child : t.MyChildren) {
		os << ", " << child->label->getName() << ": " << (*(child->dest));
	}
	os << " }";

	return os;
}
