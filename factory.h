#pragma once
#include <memory>
namespace theExpessionEngine
{
struct expression;
struct environment;
struct factory//:std::enable_shared_from_this<const factory>
{	typedef std::shared_ptr<const expression> exprPtr;
	virtual exprPtr realConstant(const double) const = 0;
	virtual exprPtr plus(const exprPtr&, const exprPtr&) const = 0;
	virtual exprPtr minus(const exprPtr&, const exprPtr&) const = 0;
	virtual exprPtr multiply(const exprPtr&, const exprPtr&) const = 0;
	virtual exprPtr divide(const exprPtr&, const exprPtr&) const = 0;
	typedef std::shared_ptr<const factory> ptr;
	static ptr getFactory(environment&);
};
}