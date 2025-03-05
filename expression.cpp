#include "expression.h"
#include <typeinfo>
#include <algorithm>
//#include <set>
//#include "environment.h"


namespace theExpessionEngine
{
expression::expression(
	children&&_rChildren
)
	:m_sChildren(std::move(_rChildren))
{
}
bool expression::operator<(const expression&_r) const
{	if (m_sChildren.size() < _r.m_sChildren.size())
		return true;
	else
	if (m_sChildren.size() > _r.m_sChildren.size())
		return false;
	else
	{	const auto s = std::mismatch(
			m_sChildren.begin(),
			m_sChildren.end(),
			_r.m_sChildren.begin()
		);
		if (s.first != m_sChildren.end())
			if (*s.first < *s.second)
				return true;
			else
				return false;
		else
		{	const auto &r0 = typeid(*this);
			const auto &r1 = typeid(_r);
			if (r0.before(r1))
				return true;
			else
			if (r1.before(r0))
				return false;
			else
			return isSmaller(_r);
		}
	}
}
bool expression::isSmaller(const expression&) const
{	return false;
}
void expression::onDestroy(void) const
{	for (auto &r : m_sOnDestroyList)
		r();
}
void expression::addOnDestroy(onDestroyFunctor _s) const
{	m_sOnDestroyList.emplace_back(std::move(_s));
}
}
