#include <iostream>
#include <string>
#include <memory>
#include <boost/parser/parser.hpp>
#include <boost/fusion/include/at_c.hpp>  // for tuple element access
#include "expression.h"
#include "factory.h"
namespace theExpressionEngine
{

namespace __PARSER__
{
namespace bp = boost::parser;

struct lookup_table_tag;
bp::rule<class NUMBER, expression<__BTHREADED__>::ptr> const NUMBER("number");
bp::rule<class factor, expression<__BTHREADED__>::ptr> const factor("factor");
bp::rule<class term, expression<__BTHREADED__>::ptr> const term("term");
bp::rule<class expr, expression<__BTHREADED__>::ptr> const expr("expr");
bp::rule<class add_or_sub, expression<__BTHREADED__>::ptr> const add_or_sub("add_or_sub");
bp::rule<class less, expression<__BTHREADED__>::ptr> const less("less");
bp::rule<class greater, expression<__BTHREADED__>::ptr> const greater("greater");
bp::rule<class relational, expression<__BTHREADED__>::ptr> const relational("relational");
bp::rule<class negation_, expression<__BTHREADED__>::ptr> const negation_("negation_");
bp::rule<class plus, expression<__BTHREADED__>::ptr> const plus("plus");
bp::rule<class x, expression<__BTHREADED__>::ptr> const x("x");
#define __unary__(a) bp::rule<class a, expression<__BTHREADED__>::ptr> const a(#a);\
auto const a##_def = (bp::lit(#a) >> "(" >> expr >> ")")\
	[(	[](auto& ctx)\
		{	const std::tuple<const factory<__BTHREADED__>::name2int&, const factory<__BTHREADED__>&> &r = bp::_globals(ctx);\
			bp::_val(ctx) = std::get<1>(r).a(bp::_attr(ctx));\
		}\
	)];\
BOOST_PARSER_DEFINE_RULES(a);
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

#define __binary__(a) bp::rule<class a, expression<__BTHREADED__>::ptr> const a(#a);\
auto const a##_def = (bp::lit(#a) >> "(" >> expr >> "," >> expr >> ")")\
	[(	[](auto& ctx)\
		{	const std::tuple<const factory<__BTHREADED__>::name2int&, const factory<__BTHREADED__>&> &r = bp::_globals(ctx);\
			auto &rA = bp::_attr(ctx);\
			bp::_val(ctx) = std::get<1>(r).a(std::get<0>(rA), std::get<1>(rA));\
		}\
	)];\
BOOST_PARSER_DEFINE_RULES(a);
__binary__(pow)
__binary__(max)
__binary__(min)
__binary__(atan2)
__binary__(fmod)
__binary__(hypot)
// Custom policies disabling the optional sign.
#if 0
struct unsigned_double_policies : bp::real_policies<double>
{
	static bool const allow_leading_sign = false;
};

// Define a parser that uses the custom policies.
bp::real_parser<double, unsigned_double_policies> const unsigned_double = {};
#endif

// Now update your NUMBER rule to use unsigned_double instead of bp::double_
auto const NUMBER_def = !bp::char_('-') >> bp::double_//unsigned_double
	[([](auto& ctx)
	{
		const std::tuple<const factory<__BTHREADED__>::name2int&, const factory<__BTHREADED__>&> &r = bp::_globals(ctx);
		bp::_val(ctx) = std::get<1>(r).realConstant(bp::_attr(ctx));
	}
	)];
BOOST_PARSER_DEFINE_RULES(NUMBER);
bp::rule<class identifier, std::string> const identifier = "identifier";
auto const identifier_def =
    bp::lexeme[
        (bp::char_('a', 'z') | bp::char_('A', 'Z') | bp::char_('_')) >> *(bp::char_('a', 'z') | bp::char_('A', 'Z') | bp::char_('0', '9') | bp::char_('_'))
    ];
BOOST_PARSER_DEFINE_RULES(identifier);
auto const x_def = identifier
	[(
		[](auto& ctx)
		{	const std::tuple<const factory<__BTHREADED__>::name2int&, const factory<__BTHREADED__>&> &r = bp::_globals(ctx);
			bp::_val(ctx) = std::get<0>(r).at(bp::_attr(ctx));
		}
	)];
BOOST_PARSER_DEFINE_RULES(x);

auto const negation__def = ('-' >> factor)
	[(
		[](auto& ctx)
		{	const std::tuple<const factory<__BTHREADED__>::name2int&, const factory<__BTHREADED__>&> &r = bp::_globals(ctx);
			bp::_val(ctx) = std::get<1>(r).negation(bp::_attr(ctx));
		}
	)];
BOOST_PARSER_DEFINE_RULES(negation_);

auto const plus_def = '+' >> factor;
BOOST_PARSER_DEFINE_RULES(plus);

auto const trig_def = sin | cos | tan | asin | acos | atan;
auto const hyperbolic_def = sinh | cosh | tanh | asinh | acosh | atanh;
auto const math1_def = exp | log | sqrt | fabs | log10;
auto const math2_def = erf | erfc | tgamma | lgamma | cbrt;
auto const binary_def = pow | max | min | atan2 | fmod | hypot;
auto const primary_def = NUMBER | ('(' >> expr >> ')') | negation_ | x;

auto const factor_def =
    trig_def
    | hyperbolic_def
    | math1_def
    | math2_def
    | binary_def
    | plus
    | primary_def;
BOOST_PARSER_DEFINE_RULES(factor);

auto const expr_def = relational;
BOOST_PARSER_DEFINE_RULES(expr);

//bp::rule<class plus_minus_pair, char> const plus_minus_pair = "plus_minus_pair";
//auto const plus_minus_pair_def = bp::char_("+-");
//BOOST_PARSER_DEFINE_RULES(plus_minus_pair);

auto const relational_def = less | greater;
BOOST_PARSER_DEFINE_RULES(relational);
auto const less_def = (add_or_sub >> -(bp::char_("<") >> add_or_sub))
	[(	[](auto& ctx)
		{	auto const& attr = bp::_attr(ctx);
			expression<__BTHREADED__>::ptr result = std::get<0>(attr);
			auto const& maybe_rhs = std::get<1>(attr);
			const std::tuple<const factory<__BTHREADED__>::name2int&, const factory<__BTHREADED__>&> &r = bp::_globals(ctx);
			if (maybe_rhs)
			{	expression<__BTHREADED__>::ptr rhs = std::get<1>(*maybe_rhs);
				result = std::get<1>(r).less(result, rhs);
			}
			bp::_val(ctx) = result;
		}
	)];
BOOST_PARSER_DEFINE_RULES(less);
auto const greater_def = (add_or_sub >> -(bp::char_(">") >> add_or_sub))
	[(	[](auto& ctx)
		{	auto const& attr = bp::_attr(ctx);
			expression<__BTHREADED__>::ptr result = std::get<0>(attr);
			auto const& maybe_rhs = std::get<1>(attr);
			const std::tuple<const factory<__BTHREADED__>::name2int&, const factory<__BTHREADED__>&> &r = bp::_globals(ctx);
			if (maybe_rhs)
			{	expression<__BTHREADED__>::ptr rhs = std::get<1>(*maybe_rhs);
				result = std::get<1>(r).greater(result, rhs);
			}
			bp::_val(ctx) = result;
		}
	)];
BOOST_PARSER_DEFINE_RULES(greater);
auto const add_or_sub_def = (term >> *(bp::char_("+-") >> term))
	[(	[](auto& ctx)
		{	auto const& attr = bp::_attr(ctx);
			expression<__BTHREADED__>::ptr result = std::get<0>(attr);
			auto const& ops = std::get<1>(attr);
			const std::tuple<const factory<__BTHREADED__>::name2int&, const factory<__BTHREADED__>&> &r = bp::_globals(ctx);
			for (auto const& op_pair : ops)
			{
				char op = std::get<0>(op_pair); // '+' or '-'
				expression<__BTHREADED__>::ptr rhs = std::get<1>(op_pair);
				if (op == '+')
					result = std::get<1>(r).addition(result, rhs);
				else
					result = std::get<1>(r).subtraction(result, rhs);
			}
			bp::_val(ctx) = result;
		}
	)];
BOOST_PARSER_DEFINE_RULES(add_or_sub);


//bp::rule<class mult_div_pair, char> const mult_div_pair = "mult_div_pair";
//auto const mult_div_pair_def = bp::attr('*') | bp::attr('/');
//BOOST_PARSER_DEFINE_RULES(mult_div_pair);

auto const term_def = (factor >> *(bp::char_("*/") >> factor))
	[(	[](auto& ctx)
		{	auto const& attr = bp::_attr(ctx);
			expression<__BTHREADED__>::ptr result = std::get<0>(attr);
			auto const& ops = std::get<1>(attr);
			const std::tuple<const factory<__BTHREADED__>::name2int&, const factory<__BTHREADED__>&> &r = bp::_globals(ctx);
			for (auto const& op_pair : ops)
			{
				char op = std::get<0>(op_pair); // '+' or '-'
				expression<__BTHREADED__>::ptr rhs = std::get<1>(op_pair);
				if (op == '*')
					result = std::get<1>(r).multiplication(result, rhs);
				else
					result = std::get<1>(r).division(result, rhs);
			}
			bp::_val(ctx) = result;
		}
	)];
BOOST_PARSER_DEFINE_RULES(term);


}
}
