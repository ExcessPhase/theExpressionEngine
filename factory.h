#pragma once
#include <memory>
#include <string>
#include <map>
namespace theExpressionEngine
{
struct expression;
struct factory:std::enable_shared_from_this<const factory>
{	typedef std::shared_ptr<const expression> exprPtr;
	virtual exprPtr realConstant(const double) const = 0;
	//virtual exprPtr sqrt(const exprPtr&) const = 0;
#define __COMMA2__
#define __MAKE_ENTRY2__(a)\
	virtual exprPtr a(const exprPtr&_p) const = 0;
#define __COMMA__
#define __MAKE_ENTRY__(a)\
	virtual exprPtr a(const exprPtr&_p) const = 0;
#include "unary.h"
	virtual exprPtr parameter(const std::size_t) const = 0;
	virtual exprPtr pow(const exprPtr&, const exprPtr&) const = 0;
	virtual exprPtr hypot(const exprPtr&, const exprPtr&) const = 0;
	virtual exprPtr fmod(const exprPtr&, const exprPtr&) const = 0;
	virtual exprPtr atan2(const exprPtr&, const exprPtr&) const = 0;
	virtual exprPtr max(const exprPtr&, const exprPtr&) const = 0;
	virtual exprPtr min(const exprPtr&, const exprPtr&) const = 0;
	virtual exprPtr addition(const exprPtr&, const exprPtr&) const = 0;
	virtual exprPtr subtraction(const exprPtr&, const exprPtr&) const = 0;
	virtual exprPtr multiplication(const exprPtr&, const exprPtr&) const = 0;
	virtual exprPtr division(const exprPtr&, const exprPtr&) const = 0;
	virtual exprPtr negation(const exprPtr&) const = 0;
	typedef std::map<std::string, exprPtr> name2int;
	virtual exprPtr parse(const char *const, const name2int&) const = 0;
	typedef std::shared_ptr<const factory> ptr;
	static ptr getFactory(void);
};
}
