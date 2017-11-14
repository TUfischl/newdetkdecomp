#define _CRT_SECURE_NO_DEPRECATE


#include <string>
#include <cstring>


#include "SubedgeSeparatorFactory.h"
#include "Globals.h"

using namespace std;

SubedgeSeparatorFactory::SubedgeSeparatorFactory()
{
	
	
}

SubedgeSeparatorFactory::~SubedgeSeparatorFactory()
{
	for (auto edges : MySubSets)
		delete edges;
}


void SubedgeSeparatorFactory::init(Hypergraph * hg, HE_VEC * comp, HE_VEC * sep, Subedges * subs)
{
	hg->resetEdgeLabels();
	hg->resetVertexLabels();

	//Label the component
	for (auto he : *comp)
		he->labelAll(1);

	//Now initialize sub edges of this separator
	for (auto he : *sep) {
		MyState.push_back(0);
		HE_VEC *edges = new HE_VEC();
		edges->push_back(he);

		for (auto sub : *(subs->getSubedges(he))) {
			//check if sub edge covers part of component
			bool found = false;
			for (auto v : sub->allVertices())
				if (v->getLabel() == 1) {
					found = true;
					break;
				}

			if (found)
				edges->push_back(sub);
		}

		MySubSets.push_back(edges);
	}

	MyInit = true;
}

HE_VEC *SubedgeSeparatorFactory::next() {
	bool inc = false;
	HE_VEC *sep = nullptr;

	if (!MyInit)
		writeErrorMsg("SubedgeSeparatorFactory not initialized!", "SubedgeSeparatorFactory::next()");

	for (int i = 0; i < MyState.size() && !inc; i++) {
		MyState[i]++;
		if (MyState[i] < MySubSets[i]->size())
			inc = true;
		else
			MyState[i] = 0;
	}

	if (inc) {
		sep = new HE_VEC(MyState.size());
		for (int i = 0; i < MyState.size(); i++)
			(*sep)[i] = (*MySubSets[i])[MyState[i]];
	}

	return sep;
}



