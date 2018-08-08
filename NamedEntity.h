#pragma once

#if !defined(CLS_NAMEDENTITY)
#define CLS_NAMEDENTITY

#include <memory>
#include <string>
#include <unordered_set>
#include <algorithm>

#include "Globals.h"

#pragma once
class NamedEntity
{
private:
	uint Id;
	string Name;
	mutable int Label;
		
public:
	NamedEntity(uint id = 0, const string &name = "") : Id{ id }, Name{ name } { Label = 0; }

	virtual uint getId() const { return Id; }
	virtual string getName() const { return Name; }

	virtual int getLabel() const { return Label; }
	virtual void setLabel(int label = 0) const { Label = label; }
	virtual void setAllLabels(int label = 0) const = 0;

	virtual void incLabel() const { setLabel(Label +1); } 
	virtual void decLabel() const { setLabel(Label -1); }
};

using NamedEntitySharedPtr = std::shared_ptr<NamedEntity>;

struct NamedEntityHash {
	size_t operator() (const NamedEntitySharedPtr &entity) const {
		return std::hash<uint>()(entity->getId());
	}
};

// Checks whether Set1 is a subset of Set2
template <typename T>
bool isSubset(const std::unordered_set<T, NamedEntityHash>& a, const std::unordered_set<T, NamedEntityHash>& b)
{
	// return true if all members of a are also in b
	auto const is_in_b = [&b](auto const& x) { return b.find(x) != b.end(); };

	return a.size() <= b.size() && std::all_of(a.begin(), a.end(), is_in_b);
}

#endif

