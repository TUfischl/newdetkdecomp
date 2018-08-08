
#include <list>

#include "Subedges.h"
#include "Hypergraph.h"
#include "CombinationIterator.h"


list<HyperedgeSharedPtr> AllSubedges;
int iSub{ 0 };


void Subedges::init(const HyperedgeSharedPtr &he)
{
	auto entry = MySubedges.find(he);
	
	if (entry == MySubedges.end()) {
		//Generate all subedges of he and store in MySubedges
		int i{ 0 }, n, k{ MyK };
		//HE_VEC* sub_edges = new HE_VEC();
		MySubedges[he];
		HyperedgeVector neighbors;

		for (auto e : MyHg->allEdgeNeighbors(he))
			if (!e->isHeavy())
			   neighbors.push_back(e);
		n = neighbors.size();
		if (n < k) k = n;

		if (n > 0)
		{
			VertexSet new_vertices;
			HyperedgeSharedPtr new_edge;
			int *indices;

			CombinationIterator comb(n, k);
			indices = comb.next();
			do {
				new_edge = nullptr;
				new_vertices.clear();
				//Build the intersection of a union of up to k edges
				//i.e. for k edges, find the vertices that are also in he.
				for (i = 0; indices[i] != -1; i++)
					for (auto v : neighbors[indices[i]]->allVertices())
						if (find(he->allVertices().begin(), he->allVertices().end(), v) != he->allVertices().end())
							new_vertices.insert(v);

				//Subedge is different then original edge
				if (!he->sameVertices(new_vertices)) {

					//Only create new subedge if it doesn't yet exist
					for (auto e : MySubedges[he])
						if (e->sameVertices(new_vertices))
							new_edge = e;

					if (new_edge == nullptr) {
						//Now check if subedge was already created from
						//a different edge
						for (auto e : AllSubedges)
							if (e->sameVertices(new_vertices))
								new_edge = e;

						//Create a new subedge
						if (new_edge == nullptr) {
							string s = he->getName();
							s = s + "-" + std::to_string(++iSub);
							new_edge = std::make_shared<Hyperedge>(s, new_vertices);
							AllSubedges.push_back(new_edge);
						}

						MySubedges[he].push_back(new_edge);
					}
				}
			} while ((indices = comb.next()) != nullptr);
		}
	}
}

void Subedges::init()
{
	for (auto e : MyHg->allEdges())
		init(e);
}

void Subedges::push_back(const HyperedgeSharedPtr &orig, const HyperedgeSharedPtr &subedge)
{
	MySubedges[orig].push_back(subedge);
}

size_t Subedges::size(const HyperedgeSharedPtr &orig) const
{
	try {
		return MySubedges.at(orig).size();
	}
	catch (const std::out_of_range& oor) {
		return 0;
	}
}

HyperedgeSharedPtr Subedges::get(const HyperedgeSharedPtr &orig, size_t index) const
{
	try {
		return MySubedges.at(orig).at(index);
	}
	catch (const std::out_of_range& oor) {
		return nullptr;
	}
}

auto Subedges::begin(const HyperedgeSharedPtr &orig)
{
	init(orig);
	return MySubedges[orig].begin();
}

auto Subedges::end(const HyperedgeSharedPtr &orig)
{
	init(orig);
	return MySubedges[orig].end();
}

