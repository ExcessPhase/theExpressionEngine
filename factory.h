#pragma once
#include <memory>
namespace theExpessionEngine
{
struct expression;
struct factory:std::enable_shared_from_this<const factory>
{	typedef std::shared_ptr<const expression> exprPtr;
	virtual exprPtr realConstant(const double) const = 0;
};
}