#pragma once

#if !defined(CLS_VERTEXSEPARATOR)
#define CLS_VERTEXSEPARATOR

#include<string>

#include "Hyperedge.h"
#include "BaseSeparator.h"

class VertexSeparator : public BaseSeparator
{
private:
	VertexSet MySep;

public:
	VertexSeparator() {};
	VertexSeparator(const VertexSet &sep) : MySep{ sep } {}

	void label(int lbl = -1) const { for (auto v : MySep) v->setAllLabels(lbl); }

	void insert(const VertexSharedPtr &ptr) { MySep.insert(ptr); }
	void erase(const VertexSharedPtr &ptr) { MySep.erase(ptr); }
	void push_back(const VertexSharedPtr &ptr) { insert(ptr); }

	bool contains(const HyperedgeSharedPtr &ptr) const;
	bool contains(const VertexSharedPtr &v) const { return find(MySep.begin(), MySep.end(), v) != MySep.end(); };

	auto allVertices() -> decltype(make_iterable(MySep.begin(), MySep.end()))
	{
		return make_iterable(MySep.begin(), MySep.end());
	}

	auto allVertices() const -> decltype(make_iterable(MySep.begin(), MySep.end()))
	{
		return make_iterable(MySep.begin(), MySep.end());
	}

	bool empty() const { return MySep.size() == 0; }
	auto size() const { return MySep.size(); }

	auto cbegin() const { return MySep.cbegin(); }
	auto cend() const { return MySep.cend(); }

	auto begin() { return MySep.begin(); }
	auto end() { return MySep.end(); }

	auto begin() const { return MySep.begin(); }
	auto end() const { return MySep.end(); }

	VertexSet covers() { return MySep; }

	const VertexSet &vertices() const { return MySep; }

	friend std::ostream& operator<< (std::ostream &out, const shared_ptr<VertexSeparator> &ptr);

};

using VertexSeparatorSharedPtr = std::shared_ptr<VertexSeparator>;

#endif
