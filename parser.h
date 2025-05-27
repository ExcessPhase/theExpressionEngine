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

struct lookup_table_tag;
x3::rule<class NUMBER, expression::ptr> const NUMBER("number");
x3::rule<class factor, expression::ptr> const factor("factor");
x3::rule<class term, expression::ptr> const term("term");
x3::rule<class expr, expression::ptr> const expr("expr");
x3::rule<class addition, expression::ptr> const addition("addition");
x3::rule<class negation, expression::ptr> const negation("negation");
x3::rule<class plus, expression::ptr> const plus("plus");
x3::rule<class x, expression::ptr> const x("x");
#define __unary__(a) x3::rule<class a, expression::ptr> const a(#a);\
auto const a##_def = (x3::lit(#a) >> "(" >> expr >> ")")\
	[(	[](auto& ctx)\
		{	const std::tuple<const factory::name2int&, const factory&> &r = x3::get<lookup_table_tag>(ctx);\
			x3::_val(ctx) = std::get<1>(r).a(x3::_attr(ctx));\
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
// Custom policies disabling the optional sign.
struct unsigned_double_policies : x3::real_policies<double>
{
	static bool const allow_leading_sign = false;
};

// Define a parser that uses the custom policies.
x3::real_parser<double, unsigned_double_policies> const unsigned_double = {};

// Now update your NUMBER rule to use unsigned_double instead of x3::double_
auto const NUMBER_def = unsigned_double
	[([](auto& ctx)
	{
		const std::tuple<const factory::name2int&, const factory&> &r = x3::get<lookup_table_tag>(ctx);
		x3::_val(ctx) = std::get<1>(r).realConstant(x3::_attr(ctx));
	}
	)];
BOOST_SPIRIT_DEFINE(NUMBER);
x3::rule<class identifier, std::string> const identifier = "identifier";
auto const identifier_def =
    x3::lexeme[
        (x3::alpha | x3::char_('_')) >> *(x3::alnum | x3::char_('_'))
    ];
BOOST_SPIRIT_DEFINE(identifier);
auto const x_def = identifier
	[(
		[](auto& ctx)
		{	const std::tuple<const factory::name2int&, const factory&> &r = x3::get<lookup_table_tag>(ctx);
			x3::_val(ctx) = std::get<0>(r).at(x3::_attr(ctx));
		}
	)];
BOOST_SPIRIT_DEFINE(x);

auto const negation_def = ('-' >> factor)
	[(
		[](auto& ctx)
		{	const std::tuple<const factory::name2int&, const factory&> &r = x3::get<lookup_table_tag>(ctx);
			x3::_val(ctx) = std::get<1>(r).negation(x3::_attr(ctx));
		}
	)];
BOOST_SPIRIT_DEFINE(negation);

auto const plus_def = '+' >> factor;
BOOST_SPIRIT_DEFINE(plus);

auto const factor_def = NUMBER 
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
	| plus
	| ('(' >> expr >> ')') | negation | x;
BOOST_SPIRIT_DEFINE(factor);

auto const expr_def = addition;
BOOST_SPIRIT_DEFINE(expr);

//x3::rule<class plus_minus_pair, char> const plus_minus_pair = "plus_minus_pair";
//auto const plus_minus_pair_def = x3::char_("+-");
//BOOST_SPIRIT_DEFINE(plus_minus_pair);

auto const addition_def = (term >> *(x3::char_("+-") >> term))
	[(	[](auto& ctx)
		{	auto const& attr = x3::_attr(ctx);
			expression::ptr result = boost::fusion::at_c<0>(attr);
			auto const& ops = boost::fusion::at_c<1>(attr);
			const std::tuple<const factory::name2int&, const factory&> &r = x3::get<lookup_table_tag>(ctx);
			for (auto const& op_pair : ops)
			{
				char op = boost::fusion::at_c<0>(op_pair); // '+' or '-'
				expression::ptr rhs = boost::fusion::at_c<1>(op_pair);
				if (op == '+')
					result = std::get<1>(r).addition(result, rhs);
				else
					result = std::get<1>(r).subtraction(result, rhs);
			}
			x3::_val(ctx) = result;
		}
	)];
BOOST_SPIRIT_DEFINE(addition);


//x3::rule<class mult_div_pair, char> const mult_div_pair = "mult_div_pair";
//auto const mult_div_pair_def = x3::attr('*') | x3::attr('/');
//BOOST_SPIRIT_DEFINE(mult_div_pair);

auto const term_def = (factor >> *(x3::char_("*/") >> factor))
	[(	[](auto& ctx)
		{	auto const& attr = x3::_attr(ctx);
			expression::ptr result = boost::fusion::at_c<0>(attr);
			auto const& ops = boost::fusion::at_c<1>(attr);
			const std::tuple<const factory::name2int&, const factory&> &r = x3::get<lookup_table_tag>(ctx);
			for (auto const& op_pair : ops)
			{
				char op = boost::fusion::at_c<0>(op_pair); // '+' or '-'
				expression::ptr rhs = boost::fusion::at_c<1>(op_pair);
				if (op == '*')
					result = std::get<1>(r).multiplication(result, rhs);
				else
					result = std::get<1>(r).division(result, rhs);
			}
			x3::_val(ctx) = result;
		}
	)];
BOOST_SPIRIT_DEFINE(term);


}
}
