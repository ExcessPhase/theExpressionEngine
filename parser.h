#include <iostream>
#include <string>
#include <memory>
#include <boost/parser/parser.hpp>
#include <boost/fusion/include/at_c.hpp> // for tuple element access
#include "expression.h"
#include "factory.h"
namespace theExpressionEngine
{

namespace __PARSER__
{
namespace bp = boost::parser;
struct lookup_table_tag;
bp::rule<class NUMBER, expression<__BTHREADED__>::ptr> const NUMBER("number");
bp::rule<class DOUBLE, expression<__BTHREADED__>::ptr> const DOUBLE("double");
bp::rule<class DOUBLE_0, expression<__BTHREADED__>::ptr> const DOUBLE_0("double_0");
bp::rule<class DOUBLE_1, expression<__BTHREADED__>::ptr> const DOUBLE_1("double_1");
bp::rule<class DOUBLE_2, expression<__BTHREADED__>::ptr> const DOUBLE_2("double_2");
bp::rule<class INT, expression<__BTHREADED__>::ptr> const INT("int");
bp::rule<class factor, expression<__BTHREADED__>::ptr> const factor("factor");
bp::rule<class term, expression<__BTHREADED__>::ptr> const term("term");
bp::rule<class expr, expression<__BTHREADED__>::ptr> const expr("expr");
bp::rule<class add_or_sub, expression<__BTHREADED__>::ptr> const add_or_sub("add_or_sub");
bp::rule<class less_greater, expression<__BTHREADED__>::ptr> const less_greater("less_greater");
bp::rule<class equality, expression<__BTHREADED__>::ptr> const equality("equality");
bp::rule<class bitwise, expression<__BTHREADED__>::ptr> const bitwise("bitwise");
bp::rule<class logical, expression<__BTHREADED__>::ptr> const logical("logical");
bp::rule<class shift, expression<__BTHREADED__>::ptr> const shift("shift");
bp::rule<class negation_, expression<__BTHREADED__>::ptr> const negation_("negation_");
bp::rule<class plus, expression<__BTHREADED__>::ptr> const plus("plus");
bp::rule<class x, expression<__BTHREADED__>::ptr> const x("x");
bp::rule<class ternary, expression<__BTHREADED__>::ptr> const ternary("ternary");
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
auto const DOUBLE_0_def = bp::lexeme[+bp::digit >> bp::char_("Ee") >> -bp::char_("+-") >> +bp::digit]
[(	[](auto& ctx)
	{	const std::tuple<const factory<__BTHREADED__>::name2int&, const factory<__BTHREADED__>&> &r = bp::_globals(ctx);
		const auto &[s0, s1] = bp::_attr(ctx);
		bp::_val(ctx) = std::get<1>(r).realConstant(
			std::stod(s0 + s1)
		);
	}
)];
BOOST_PARSER_DEFINE_RULES(DOUBLE_0);
auto const DOUBLE_1_def = bp::lexeme[+bp::digit >> bp::char_('.') >> *bp::digit >> -(bp::char_("Ee") >> -bp::char_("+-") >> +bp::digit)]
[(	[](auto& ctx)
	{	const std::tuple<const factory<__BTHREADED__>::name2int&, const factory<__BTHREADED__>&> &r = bp::_globals(ctx);
		const auto &[s0, s1, s2] = bp::_attr(ctx);
		bp::_val(ctx) = std::get<1>(r).realConstant(
			std::stod(
				s0 + s1 + (s2
					? std::get<0>(*s2) + std::get<1>(*s2)
					: std::string()
				)
			)
		);
	}
)];
BOOST_PARSER_DEFINE_RULES(DOUBLE_1);
auto const DOUBLE_2_def = bp::lexeme[*bp::digit >> bp::char_('.') >> +bp::digit >> -(bp::char_("Ee") >> -bp::char_("+-") >> +bp::digit)]
[(	[](auto& ctx)
	{	const std::tuple<const factory<__BTHREADED__>::name2int&, const factory<__BTHREADED__>&> &r = bp::_globals(ctx);
		const auto &[s0, s1, s2] = bp::_attr(ctx);
		bp::_val(ctx) = std::get<1>(r).realConstant(
			std::stod(
				s0 + s1 + (s2 ? std::get<0>(*s2) + std::get<1>(*s2) : std::string())
			)
		);
	}
)];
BOOST_PARSER_DEFINE_RULES(DOUBLE_2);
auto const DOUBLE_def = DOUBLE_0 | DOUBLE_1 | DOUBLE_2;
BOOST_PARSER_DEFINE_RULES(DOUBLE);
auto const INT_def = bp::lexeme[+bp::digit]
[(	[](auto& ctx)
	{	const std::tuple<const factory<__BTHREADED__>::name2int&, const factory<__BTHREADED__>&> &r = bp::_globals(ctx);
		const auto &sDigits = bp::_attr(ctx);
		bp::_val(ctx) = std::get<1>(r).intConstant(std::stoi(sDigits));
	}
)];
BOOST_PARSER_DEFINE_RULES(INT);
// Now update your NUMBER rule to use unsigned_double instead of bp::double_
auto const NUMBER_def = (DOUBLE | INT);//unsigned_double
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

auto const ternary_def = (equality >> -('?' >> expr >> ':' >> expr))
[(	[](auto& ctx)
	{	auto const& attr = bp::_attr(ctx);
		const std::tuple<const factory<__BTHREADED__>::name2int&, const factory<__BTHREADED__>&> &r = bp::_globals(ctx);
		expression<__BTHREADED__>::ptr result = std::get<0>(attr);
		auto const& maybe_branch = std::get<1>(attr);
		if (maybe_branch)
		{	expression<__BTHREADED__>::ptr true_branch = std::get<0>(*maybe_branch);
			expression<__BTHREADED__>::ptr false_branch = std::get<1>(*maybe_branch);
			result = std::get<1>(r).conditional(result, true_branch, false_branch);
		}
		bp::_val(ctx) = result;
	}
)];
BOOST_PARSER_DEFINE_RULES(ternary);
auto const expr_def = ternary;
BOOST_PARSER_DEFINE_RULES(expr);

//bp::rule<class plus_minus_pair, char> const plus_minus_pair = "plus_minus_pair";
//auto const plus_minus_pair_def = bp::char_("+-");
//BOOST_PARSER_DEFINE_RULES(plus_minus_pair);

bp::rule<class equality_ops, std::string> const equality_ops = "equality_ops";
auto const equality_ops_def = bp::lexeme[
	bp::string("==")
	| bp::string("!=")
];
BOOST_PARSER_DEFINE_RULES(equality_ops);
auto const equality_def = (less_greater >> -(equality_ops >> less_greater))
	[(	[](auto& ctx)
		{	auto const& attr = bp::_attr(ctx);
			expression<__BTHREADED__>::ptr result = std::get<0>(attr);
			auto const& maybe_rhs = std::get<1>(attr);
			const std::tuple<const factory<__BTHREADED__>::name2int&, const factory<__BTHREADED__>&> &r = bp::_globals(ctx);
			if (maybe_rhs)
			{	auto const &op = std::get<0>(*maybe_rhs); // string_view
				expression<__BTHREADED__>::ptr rhs = std::get<1>(*maybe_rhs);
				if (op == "==")
					result = std::get<1>(r).equal_to(result, rhs);
				else
					result = std::get<1>(r).not_equal_to(result, rhs);
			}
			bp::_val(ctx) = result;
		}
	)];
BOOST_PARSER_DEFINE_RULES(equality);
bp::rule<class relational_ops, std::string> const relational_ops = "relational_ops";
auto const relational_ops_def = bp::lexeme[
	bp::string("<=")
	| bp::string(">=")
	| bp::string("<")
	| bp::string(">")
];
BOOST_PARSER_DEFINE_RULES(relational_ops);
auto const less_greater_def = (add_or_sub >> *(relational_ops >> add_or_sub))
	[(	[](auto& ctx)
		{	auto const& attr = bp::_attr(ctx);
			expression<__BTHREADED__>::ptr result = std::get<0>(attr);
			auto const& ops = std::get<1>(attr);
			const std::tuple<const factory<__BTHREADED__>::name2int&, const factory<__BTHREADED__>&> &r = bp::_globals(ctx);
			for (auto const& op_pair : ops)
			{
				auto& op = std::get<0>(op_pair); // '+' or '-'
				expression<__BTHREADED__>::ptr rhs = std::get<1>(op_pair);
				if (op == "<=")
					result = std::get<1>(r).less_equal(result, rhs);
				else
				if (op == ">=")
					result = std::get<1>(r).greater_equal(result, rhs);
				else
				if (op == "<")
					result = std::get<1>(r).less(result, rhs);
				else
					result = std::get<1>(r).greater(result, rhs);
			}
			bp::_val(ctx) = result;
		}
	)];
BOOST_PARSER_DEFINE_RULES(less_greater);
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
