#include "BaseSeparator.h"



BaseSeparator::BaseSeparator()
{
}


BaseSeparator::~BaseSeparator()
{
}

bool operator==(const  std::shared_ptr<BaseSeparator> & lhs, const  std::shared_ptr<BaseSeparator> & rhs)
{
	return lhs->covers() == rhs->covers();
}