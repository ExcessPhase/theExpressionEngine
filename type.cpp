#include "type.h"


namespace theExpessionEngine
{
type::type(const enumType _eType)
	:m_eType(_eType)
{
}
bool type::operator<(const type&_r) const
{	return m_eType < _r.m_eType;
}
}