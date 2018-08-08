#pragma once

#if !defined(CLS_BASESEPARATOR)
#define CLS_BASESEPARATOR

#include<string>

#include "Hyperedge.h"

class BaseSeparator
{
public:
	BaseSeparator();
	virtual ~BaseSeparator();

	virtual void label(int lbl = -1) const = 0;

	virtual bool contains(const VertexSharedPtr &v) const = 0;

	virtual bool empty() const = 0;

	virtual VertexSet covers() = 0;

	friend bool operator==(const  std::shared_ptr<BaseSeparator>& lhs, const  std::shared_ptr<BaseSeparator>& rhs);
};

using BaseSeparatorSharedPtr = std::shared_ptr<BaseSeparator>;

#endif