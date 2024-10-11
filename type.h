#pragma once
#include <memory>
namespace theExpessionEngine
{
/// a type object.
/// can become more complicated for arrays or tuples
/// immutable
/// no two with the same content are guaranteed to exist
struct type:std::enable_shared_from_this<const type>
{	enum enumType
	{	eReal,
		eInteger,
		eString
	};
	const enumType m_eType;
	typedef std::shared_ptr<const type> ptr;
	type(const enumType _eType)
		:m_eType(_eType)
	{
	}
	bool operator<(const type&_r) const
	{	return m_eType < _r.m_eType;
	}
};
}