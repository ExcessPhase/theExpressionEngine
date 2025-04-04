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
[0-9]+(\.[0-9]*)?([eE][+-]?[0-9]*)?	{*yylval = factory->realConstant(atof(yytext));return NUMBER;}
"/"	{return '/';}
"*"	{return '*';}
"+"	{return '+';}
"-"	{return '-';}
"("	{return '(';}
")"	{return ')';}
","	{return ',';}
"sin"	{return SIN;}
"cos"	{return COS;}
"tan"	{return TAN;}
"asin"	{return ASIN;}
"acos"	{return ACOS;}
"atan"	{return ATAN;}
"sinh"	{return SINH;}
"cosh"	{return COSH;}
"tanh"	{return TANH;}
"asinh"	{return ASINH;}
"acosh"	{return ACOSH;}
"atanh"	{return ATANH;}
"sqrt"	{return SQRT;}
"abs"	{return ABS;}
"exp"	{return EXP;}
"log"	{return LOG;}
"log10"	{return LOG10;}
"erf"	{return ERF;}
"erfc"	{return ERFC;}
"tgamma"	{return TGAMMA;}
"lgamma"	{return LGAMMA;}
"cbrt"	{return CBRT;}
"pow"	{return POW;}
"atan2"	{return ATAN2;}
"max"	{return MAX;}
"min"	{return MIN;}
"x"	{return X;}
