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

[0-9]+(\.[0-9]*)?([eE][+-]?[0-9]*)?	{*yylval = factory->realConstant(atof(yytext));return NUMBER;}
