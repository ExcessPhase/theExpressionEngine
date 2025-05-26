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
using ast_ptr = expression::ptr;

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
auto const NUMBER_def = x3::double_
	[(
		[](auto& ctx)
		{	x3::_val(ctx) = s_pFactory->realConstant(x3::_attr(ctx));
		}
	)];
BOOST_SPIRIT_DEFINE(NUMBER);

auto const factor_def = NUMBER | ('(' >> expr >> ')');
BOOST_SPIRIT_DEFINE(factor);

auto const term_def = factor | multiplication | division;
BOOST_SPIRIT_DEFINE(term);

auto const expr_def = term | addition | subtraction;
BOOST_SPIRIT_DEFINE(expr);

auto const addition_def = term >> *(x3::lit('+') >> term)
	[(	[](auto& ctx)
		{	auto const& attr = x3::_attr(ctx);
			ast_ptr result = boost::fusion::at_c<0>(attr);
			auto const& vec = boost::fusion::at_c<1>(attr);
			for (auto const& sub : vec)
				result = s_pFactory->addition(result, sub);
			x3::_val(ctx) = result;
		}
	)];
BOOST_SPIRIT_DEFINE(addition);

auto const multiplication_def = factor >> *(x3::lit('*') >> factor)
	[(	[](auto& ctx)
		{	auto const& attr = x3::_attr(ctx);
			ast_ptr result = boost::fusion::at_c<0>(attr);
			auto const& vec = boost::fusion::at_c<1>(attr);
			for (auto const& sub : vec)
				result = s_pFactory->multiplication(result, sub);
			x3::_val(ctx) = result;
		}
	)];
BOOST_SPIRIT_DEFINE(multiplication);

auto const division_def = factor >> *(x3::lit('/') >> factor)
	[(	[](auto& ctx)
		{	auto const& attr = x3::_attr(ctx);
			ast_ptr result = boost::fusion::at_c<0>(attr);
			auto const& vec = boost::fusion::at_c<1>(attr);
			for (auto const& sub : vec)
				result = s_pFactory->division(result, sub);
			x3::_val(ctx) = result;
		}
	)];
BOOST_SPIRIT_DEFINE(division);

auto const subtraction_def = term >> *(x3::lit('-') >> term)
	[(	[](auto& ctx)
		{	auto const& attr = x3::_attr(ctx);
			ast_ptr result = boost::fusion::at_c<0>(attr);
			auto const& vec = boost::fusion::at_c<1>(attr);
			for (auto const& sub : vec)
				result = s_pFactory->subtraction(result, sub);
			x3::_val(ctx) = result;
		}
	)];
BOOST_SPIRIT_DEFINE(subtraction);

#if 0
// A factor is either a literal number or a parenthesized expression.
auto const factor_def = x3::double_
	[
		[](auto& ctx)
		{
			// Create a literal AST node for the parsed number.
			x3::_val(ctx) = std::make_shared<literal_node>(x3::_attr(ctx));
		}
	] | ('(' >> expression >> ')');

// A term is a factor followed by zero or more multiplicative operations.
auto const term_def = factor >> *(x3::lit('*') >> factor)
	[
		[](auto& ctx)
		{
			// The attribute of "factor >> *(lit('*') >> factor)" is a tuple:
			//    (first_factor, vector<ast_ptr>)
			auto const& attr = x3::_attr(ctx);
			ast_ptr result = boost::fusion::at_c<0>(attr);
			auto const& vec = boost::fusion::at_c<1>(attr);
			// For each parsed '*' operator and its right factor, create a binary node.
			for (auto const& sub : vec)
			{
			result = std::make_shared<binary_node>('*', result, sub);
			}
			// Set the overall attribute to our newly formed AST.
			x3::_val(ctx) = result;
		}
	];

// An expression is a term followed by zero or more additive operations.
auto const expression_def =
	term >> *(x3::lit('+') >> term)
	[	[](auto& ctx)
		{	auto const& attr = x3::_attr(ctx);
			ast_ptr result = boost::fusion::at_c<0>(attr);
			auto const& vec = boost::fusion::at_c<1>(attr);
			for (auto const& sub : vec)
				result = std::make_shared<binary_node>('+', result, sub);
			x3::_val(ctx) = result;
		}
	];

// Associate the rule implementations.
BOOST_SPIRIT_DEFINE(factor, term, expression);

// Define a top-level rule alias if desired.
x3::rule<class ast_expression, ast_ptr> const ast_expression("ast_expression");
auto const ast_expression_def = expression;
BOOST_SPIRIT_DEFINE(ast_expression);
#endif

//---------------------------------------------------------------------
// Helper Function to Print the AST (for demonstration purposes)
//---------------------------------------------------------------------
#if 0
void print_ast(const ast_ptr& node, int indent = 0)
{
    std::string padding(indent, ' ');
    if (!node)
    {
        std::cout << padding << "null\n";
        return;
    }
    // Try to cast to literal_node.
    if (auto lit = std::dynamic_pointer_cast<literal_node>(node))
    {
        std::cout << padding << "Literal: " << lit->value << "\n";
    }
    // Otherwise, try to cast to binary_node.
    else if (auto bin = std::dynamic_pointer_cast<binary_node>(node))
    {
        std::cout << padding << "Binary Node: " << bin->op << "\n";
        std::cout << padding << "Left:\n";
        print_ast(bin->left, indent + 2);
        std::cout << padding << "Right:\n";
        print_ast(bin->right, indent + 2);
    }
    else
    {
        std::cout << padding << "Unknown node type.\n";
    }
}
#endif

//---------------------------------------------------------------------
// Main: Parse an expression and print its AST.
//---------------------------------------------------------------------
#if 0
int main()
{
    std::string input;
    std::cout << "Enter an arithmetic expression (using + and *): ";
    std::getline(std::cin, input);

    auto iter = input.begin();
    auto end  = input.end();

    ast_ptr ast;
    bool success = phrase_parse(iter, end, ast_expression, x3::space, ast);
    if (success && iter == end)
    {
        std::cout << "Parsing succeeded.\n";
        print_ast(ast);
    }
    else
    {
        std::string rest(iter, end);
        std::cout << "Parsing failed. Remaining unparsed: \"" << rest << "\"\n";
    }
    return 0;
}
#endif

}
}
