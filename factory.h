#pragma once
#include <memory>
#include <string>
#include <map>
#include "unique.h"
#include "expression.h"
namespace theExpressionEngine
{
template<bool BTHREADED>
struct expression;
template<bool BTHREADED>
struct expressionSet;
template<bool BTHREADED>
struct factory:std::enable_shared_from_this<const factory<BTHREADED> >
{	typedef boost::intrusive_ptr<const expression<BTHREADED> > exprPtr;
	virtual exprPtr realConstant(const double) const = 0;
	virtual exprPtr intConstant(const int) const = 0;
	//virtual exprPtr sqrt(const exprPtr&) const = 0;
#define __COMMA2__
#define __MAKE_ENTRY2__(a)\
	virtual exprPtr a(const exprPtr&_p) const = 0;
#define __COMMA__
#define __MAKE_ENTRY__(a)\
	virtual exprPtr a(const exprPtr&_p) const = 0;
#include "unary.h"
	virtual exprPtr parameter(const std::size_t) const = 0;
	virtual exprPtr variable(const std::size_t) const = 0;
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
	virtual exprPtr less_equal(const exprPtr&, const exprPtr&) const = 0;
	virtual exprPtr greater_equal(const exprPtr&, const exprPtr&) const = 0;
	virtual exprPtr less(const exprPtr&, const exprPtr&) const = 0;
	virtual exprPtr greater(const exprPtr&, const exprPtr&) const = 0;
	virtual boost::intrusive_ptr<const expressionSet<BTHREADED> > createExpressionSet(const std::vector<exprPtr>&) const = 0;
	typedef std::map<std::string, exprPtr> name2int;
	virtual exprPtr parse(const char *const, const name2int&) const = 0;
	typedef std::shared_ptr<const factory<BTHREADED> > ptr;
	static const ptr &getFactory(void);
};
}
#include "factory.ipp"
