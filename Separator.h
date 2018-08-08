#pragma once

#if !defined(CLS_SEPARATOR)
#define CLS_SEPARATOR

#include<string>

#include "Hyperedge.h"
#include "BaseSeparator.h"

class Separator : public BaseSeparator
{
private:
	HyperedgeVector MySep;

public:
	Separator() {};
	Separator(const HyperedgeVector &sep) : MySep{ sep } {}

	void label(int lbl = -1) const;

	void insert(const HyperedgeSharedPtr &ptr) { MySep.push_back(ptr); }
	void push_back(const HyperedgeSharedPtr &ptr) { insert(ptr); }

	bool contains(const HyperedgeSharedPtr &ptr) const { return find(MySep.begin(),MySep.end(),ptr) != MySep.end(); }
	bool contains(const VertexSharedPtr &v) const;

	auto allEdges() -> decltype(make_iterable(MySep.begin(), MySep.end()))
	{
		return make_iterable(MySep.begin(), MySep.end());
	}

	auto allEdges() const -> decltype(make_iterable(MySep.begin(), MySep.end()))
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

	VertexSet covers();

	const HyperedgeVector &edges() const { return MySep; }

	friend bool operator==(const  std::shared_ptr<Separator>& lhs, const  std::shared_ptr<Separator>& rhs);

};

using SeparatorSharedPtr = std::shared_ptr<Separator>;

#endif
