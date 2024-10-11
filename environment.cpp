#include "environment.h"
#include <set>
#include "expression.h"


namespace theExpessionEngine
{
struct expression;
environment::~environment(void)
{	if (const auto pSet = std::any_cast<std::set<const expression*> >(&m_s.at(eExpressionValueRegistration)))
	for (const auto p : *pSet)
		p->m_sValues.erase(this);
}
}