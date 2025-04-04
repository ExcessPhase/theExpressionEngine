%define api.pure
%define api.value.type {theExpressionEngine::expression::ptr}
%parse-param {yyscan_t scanner}
%parse-param {theExpressionEngine::expression::ptr *const root}
%parse-param {theExpressionEngine::factory::ptr const &factory}
%lex-param {yyscan_t scanner}
%lex-param {theExpressionEngine::factory::ptr const &factory}
%{
#include "factory.h"
#include "expression.h"
#include "flex.h"
#include <stdexcept>
typedef void* yyscan_t;
void yyerror(yyscan_t scanner, theExpressionEngine::expression::ptr *const, theExpressionEngine::factory::ptr const &, const char*const);
int yylex(YYSTYPE *, yyscan_t, theExpressionEngine::factory::ptr const &factory);
#define YYSTYPE theExpressionEngine::expression::ptr
%}


%token NUMBER
%left '+' '-'
%left '*' '/'

%%

expr: NUMBER {$$=$1; *root = $1;}
	| expr '+' expr {$$ = $1;}
	| expr '-' expr {$$ = $1;}
	| expr '*' expr {$$ = $1;}
	| expr '/' expr {$$ = $1;}
	;
%%
void yyerror(yyscan_t, theExpressionEngine::expression::ptr *const, theExpressionEngine::factory::ptr const &, const char*const _p)
{	throw std::logic_error(_p);
}
