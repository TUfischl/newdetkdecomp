#include "DecompComponent.h"
#include "Hyperedge.h"


DecompComponent::~DecompComponent()
{
}

void DecompComponent::add(const HyperedgeSharedPtr & edge)
{
	if (!contains(edge)) {
		for (const auto v : edge->allVertices())
			if (MySep->contains(v))
				MyConnector.insert(v);
		MyComp.push_back(edge);
	}
}

HyperedgeSharedPtr DecompComponent::containsOneOf(const list<HyperedgeSharedPtr>& edges) const
{
	for (auto &e : edges)
		if (contains(e))
			return e;

	return nullptr;
}

void DecompComponent::label(int label, int sepLabel) const
{
	for (auto e : MyComp) 
		e->setAllLabels(label);

	for (auto v : MyConnector) 
		v->setLabel(sepLabel);

	MySep->label(sepLabel);
}
