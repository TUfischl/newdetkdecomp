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
	
}


void SubedgeSeparatorFactory::init(const HypergraphSharedPtr &hg, const HyperedgeVector &comp, const SeparatorSharedPtr &sep, const unique_ptr<Subedges> &subs)
{
	hg->setAllLabels();

	//Label the component
	for (auto he : comp)
		he->setAllLabels(1);

	//Now initialize sub edges of this separator
	for (auto &he : sep->edges()) {
		MyState.push_back(0);
		HyperedgeVector edges;
		edges.push_back(he);

		subs->init(he);
		for (auto sub : subs->subedges(he)) {
			//check if sub edge covers part of component
			bool found = false;
			for (auto v : sub->allVertices())
				if (v->getLabel() == 1) {
					found = true;
					break;
				}

			if (found)
				edges.push_back(sub);
		}

		MySubSets.push_back(edges);
	}

	MyInit = true;
}

SeparatorSharedPtr SubedgeSeparatorFactory::next() {
	bool inc = false;
	SeparatorSharedPtr sep = make_shared<Separator>();

	if (!MyInit)
		writeErrorMsg("SubedgeSeparatorFactory not initialized!", "SubedgeSeparatorFactory::next()");

	for (int i = 0; i < MyState.size() && !inc; i++) {
		MyState[i]++;
		if (MyState[i] < MySubSets[i].size())
			inc = true;
		else
			MyState[i] = 0;
	}

	if (inc) {
		for (int i = 0; i < MyState.size(); i++)
			sep->push_back(MySubSets[i][MyState[i]]);
	}

	return sep;
}



