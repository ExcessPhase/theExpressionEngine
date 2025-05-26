#pragma once
#include <iostream>
#include <string>
#include <memory>
#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/at_c.hpp>  // for tuple element access
#include "expression.h"
#include "factory.h"
namespace theExpressionEngine
{

namespace parser
{
namespace x3 = boost::spirit::x3;

//---------------------------------------------------------------------
// AST Node Definitions
//---------------------------------------------------------------------

// For convenience, define a type alias for a pointer to an AST node.
//---------------------------------------------------------------------
// Parser Grammar Using Boost Spirit X3 (returning an AST)
//---------------------------------------------------------------------

// We declare rules with a synthesized attribute of type ast_ptr.
#if 0
x3::rule<class factor, ast_ptr> const factor("factor");
x3::rule<class term, ast_ptr> const term("term");
x3::rule<class expression, ast_ptr> const expression("expression");
#endif

static const factory::ptr s_pFactory = factory::getFactory();
x3::rule<class NUMBER, expression::ptr> const NUMBER("number");
x3::rule<class factor, expression::ptr> const factor("factor");
x3::rule<class term, expression::ptr> const term("term");
x3::rule<class expr, expression::ptr> const expr("expr");
x3::rule<class addition, expression::ptr> const addition("addition");
x3::rule<class subtraction, expression::ptr> const subtraction("subtraction");
x3::rule<class multiplication, expression::ptr> const multiplication("multiplication");
x3::rule<class division, expression::ptr> const division("division");
x3::rule<class negation, expression::ptr> const negation("negation");
x3::rule<class plus, expression::ptr> const plus("plus");
x3::rule<class x, expression::ptr> const x("x");
#define __unary__(a) x3::rule<class a, expression::ptr> const a(#a);\
auto const a##_def = (x3::lit(#a) >> "(" >> expr >> ")")\
	[(	[](auto& ctx)\
		{	x3::_val(ctx) = s_pFactory->a(x3::_attr(ctx));\
		}\
	)];\
BOOST_SPIRIT_DEFINE(a);
__unary__(sin)
__unary__(cos)
__unary__(tan)
__unary__(sinh)
__unary__(cosh)
__unary__(tanh)
__unary__(asin)
__unary__(acos)
__unary__(atan)
__unary__(asinh)
__unary__(acosh)
__unary__(atanh)
__unary__(exp)
__unary__(log)
__unary__(sqrt)
__unary__(fabs)
__unary__(log10)
__unary__(erf)
__unary__(erfc)
__unary__(tgamma)
__unary__(lgamma)
__unary__(cbrt)
auto const NUMBER_def = x3::double_
	[(
		[](auto& ctx)
		{	x3::_val(ctx) = s_pFactory->realConstant(x3::_attr(ctx));
		}
	)];
BOOST_SPIRIT_DEFINE(NUMBER);

auto const x_def = x3::lit("x")
	[(
		[](auto& ctx)
		{	x3::_val(ctx) = s_pFactory->parameter(0);
		}
	)];
BOOST_SPIRIT_DEFINE(x);

auto const negation_def = '-' >> factor;
BOOST_SPIRIT_DEFINE(negation);

auto const plus_def = '+' >> factor;
BOOST_SPIRIT_DEFINE(plus);

auto const factor_def = NUMBER | ('(' >> expr >> ')') | negation | x
	| sin | cos | tan
	| asin | acos | atan
	| sinh | cosh | tanh
	| asinh | acosh | atanh
	| exp
	| log
	| sqrt
	| fabs
	| log10
	| erf
	| erfc
	| tgamma
	| lgamma
	| cbrt
	| plus;
BOOST_SPIRIT_DEFINE(factor);

auto const term_def = factor | multiplication | division;
BOOST_SPIRIT_DEFINE(term);

auto const expr_def = term | addition | subtraction;
BOOST_SPIRIT_DEFINE(expr);

auto const addition_def = (term >> *(x3::lit('+') >> term))
	[(	[](auto& ctx)
		{	auto const& attr = x3::_attr(ctx);
			expression::ptr result = boost::fusion::at_c<0>(attr);
			auto const& vec = boost::fusion::at_c<1>(attr);
			for (auto const& sub : vec)
				result = s_pFactory->addition(result, sub);
			x3::_val(ctx) = result;
		}
	)];
BOOST_SPIRIT_DEFINE(addition);

auto const multiplication_def = (factor >> *(x3::lit('*') >> factor))
	[(	[](auto& ctx)
		{	auto const& attr = x3::_attr(ctx);
			expression::ptr result = boost::fusion::at_c<0>(attr);
			auto const& vec = boost::fusion::at_c<1>(attr);
			for (auto const& sub : vec)
				result = s_pFactory->multiplication(result, sub);
			x3::_val(ctx) = result;
		}
	)];
BOOST_SPIRIT_DEFINE(multiplication);

auto const division_def = (factor >> *(x3::lit('/') >> factor))
	[(	[](auto& ctx)
		{	auto const& attr = x3::_attr(ctx);
			expression::ptr result = boost::fusion::at_c<0>(attr);
			auto const& vec = boost::fusion::at_c<1>(attr);
			for (auto const& sub : vec)
				result = s_pFactory->division(result, sub);
			x3::_val(ctx) = result;
		}
	)];
BOOST_SPIRIT_DEFINE(division);

auto const subtraction_def = (term >> *(x3::lit('-') >> term))
	[(	[](auto& ctx)
		{	auto const& attr = x3::_attr(ctx);
			expression::ptr result = boost::fusion::at_c<0>(attr);
			auto const& vec = boost::fusion::at_c<1>(attr);
			for (auto const& sub : vec)
				result = s_pFactory->subtraction(result, sub);
			x3::_val(ctx) = result;
		}
	)];
BOOST_SPIRIT_DEFINE(subtraction);


}
}
