#include "VertexSeparator.h"


bool VertexSeparator::contains(const HyperedgeSharedPtr & ptr) const
{
	for (auto &v : ptr->allVertices())
		if (MySep.find(v) == MySep.end())
			return false;

	return true;
}

std::ostream & operator<<(std::ostream & out, const shared_ptr<VertexSeparator> & ptr)
{
	out << "{ ";
	for (auto it = ptr->vertices().begin(); it != ptr->vertices().end();) {
		out << (*it)->getName();
		it++;
		if (it != ptr->vertices().end())
			out << ", ";
		else
			out << " }";
	}
	return out;
}
