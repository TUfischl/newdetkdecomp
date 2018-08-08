#include <algorithm>
#include<unordered_set>
#include<set>
#include<vector>
#include<string>
#include<set>
#include<iterator>

#include "Vertex.h"


int G_VertexID = 0;

Vertex::Vertex(const string & name) : Vertex(++G_VertexID, name)
{
}

Vertex::~Vertex()
{
}

std::ostream & operator<<(std::ostream & out, const Vertex & v)
{
	out << v.getName();

	return out;
}

std::ostream & operator<<(std::ostream & out, const VertexSet & v)
{
	out << "(";
	for (auto it = v.cbegin(); it != v.cend();) {
		out << (*it)->getName();
		if (++it != v.cend())
			out << ",";
	}
	out << ")";

	return out;
}


powerset_type powerset(set_type const& set)
{
	typedef set_type::const_iterator set_iter;
	typedef std::vector<set_iter> vec;
	typedef vec::iterator vec_iter;

	struct local
	{
		static VertexSharedPtr dereference(set_iter v) { return *v; }
	};

	powerset_type result;

	vec elements;
	do
	{
		set_type tmp;
		transform(elements.begin(), elements.end(),
			std::inserter(tmp, tmp.end()),
			local::dereference);
		result.insert(tmp);
		if (!elements.empty() && ++elements.back() == set.end())
		{
			elements.pop_back();
		}
		else
		{
			set_iter iter;
			if (elements.empty())
			{
				iter = set.begin();
			}
			else
			{
				iter = elements.back();
				++iter;
			}
			for (; iter != set.end(); ++iter)
			{
				elements.push_back(iter);
			}
		}
	} while (!elements.empty());

	return result;
}
