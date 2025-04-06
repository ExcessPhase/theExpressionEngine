%option noyywrap
%option reentrant
%{
#include <iostream>
#include <string>
//#include "tokens.h"
#include "flex.h"
#include "bison.h"
%}

%%

[ \t\r\n]+	{}
[0-9]+(\.[0-9]*)?([eE][+-]?[0-9]*)?	{*yylval = factory->realConstant(atof(yytext));return yy::parser::token::NUMBER;}
"/"	{return '/';}
"*"	{return '*';}
"+"	{return '+';}
"-"	{return '-';}
"("	{return '(';}
")"	{return ')';}
","	{return ',';}
"sin"	{return yy::parser::token::SIN;}
"cos"	{return yy::parser::token::COS;}
"tan"	{return yy::parser::token::TAN;}
"asin"	{return yy::parser::token::ASIN;}
"acos"	{return yy::parser::token::ACOS;}
"atan"	{return yy::parser::token::ATAN;}
"sinh"	{return yy::parser::token::SINH;}
"cosh"	{return yy::parser::token::COSH;}
"tanh"	{return yy::parser::token::TANH;}
"asinh"	{return yy::parser::token::ASINH;}
"acosh"	{return yy::parser::token::ACOSH;}
"atanh"	{return yy::parser::token::ATANH;}
"sqrt"	{return yy::parser::token::SQRT;}
"fabs"	{return yy::parser::token::FABS;}
"exp"	{return yy::parser::token::EXP;}
"log"	{return yy::parser::token::LOG;}
"log10"	{return yy::parser::token::LOG10;}
"erf"	{return yy::parser::token::ERF;}
"erfc"	{return yy::parser::token::ERFC;}
"tgamma"	{return yy::parser::token::TGAMMA;}
"lgamma"	{return yy::parser::token::LGAMMA;}
"cbrt"	{return yy::parser::token::CBRT;}
"pow"	{return yy::parser::token::POW;}
"atan2"	{return yy::parser::token::ATAN2;}
"max"	{return yy::parser::token::MAX;}
"min"	{return yy::parser::token::MIN;}
"hypot"	{return yy::parser::token::HYPOT;}
"fmod"	{return yy::parser::token::FMOD;}
[a-zA-Z_][a-zA-Z_0-9]*	{*yylval = _rN2I.at(yytext); return yy::parser::token::X;}
