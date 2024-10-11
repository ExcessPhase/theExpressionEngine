#include "expression.h"
#include <typeinfo>
#include <algorithm>
#include <set>
#include "environment.h"


namespace theExpessionEngine
{
expression::expression(
	const std::shared_ptr<const type> &_rType,
	children&&_rChildren
)
	:m_sChildren(std::move(_rChildren)),
	m_sType(_rType)
{
}
bool expression::operator<(const expression&_r) const
{	if (m_sType < _r.m_sType)
		return true;
	else
	if (m_sType > _r.m_sType)
		return false;
	else
	if (m_sChildren.size() < _r.m_sChildren.size())
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
const expression::value&expression::evaluate(environment&_r) const
{	const auto sInsert = m_sValues.emplace(&_r, value());
	if (sInsert.second)
		sInsert.first->second = evaluateThis(_r);
	return sInsert.first->second;
}
void expression::onDestroy(void) const
{	for (const auto &r : m_sValues)
		std::any_cast<std::set<const expression*>&>(r.first->m_s.at(environment::eExpressionValueRegistration)).erase(this);
}
}