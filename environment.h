#pragma once
//#include <memory>
#include <array>
#include <any>
namespace theExpessionEngine
{
/// a catchall type to be passed around nearly everywhere
struct environment//:std::enable_shared_from_this<environment>
{	enum enumBranches
	{	eExpressionValueRegistration,	// std::set<const expression*>
		eFactory,
		SIZE
	};
	std::array<std::any, SIZE> m_s;
	~environment(void);
};
}