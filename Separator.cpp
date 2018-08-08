#include "Separator.h"

void Separator::label(int lbl) const
{
	for (auto e : MySep)
		e->setAllLabels(lbl);
}

bool Separator::contains(const VertexSharedPtr & v) const
{
	for (auto e : MySep) {
		if (e->find(v) != nullptr)
			return true;
	}

	return false;
}

VertexSet Separator::covers()
{
	VertexSet vs;

	for (auto e : MySep)
		for (auto v : e->allVertices())
			vs.insert(v);

	return vs;
}

bool operator==(const  std::shared_ptr<Separator> & lhs, const  std::shared_ptr<Separator> & rhs)
{
	if (lhs->size() != rhs->size()) 
		return false;
	
	for (auto e : lhs->edges())
		if (find(rhs->begin(), rhs->end(), e) == rhs->end())
			return false;

	return true;
}
