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


%token NUMBER SIN COS
%left '+' '-'
%left '*' '/'

%%

input: expr {*root = $1;}
	;

expr: term { $$ = $1;}
	| expr '+' term {$$ = factory->addition($1, $3);}
	| expr '-' term {$$ = factory->subtraction($1, $3);}
	;

term: factor {$$=$1;}
	| term '*' factor {$$ = factory->multiplication($1, $3);}
	| term '/' factor {$$ = factory->division($1, $3);}
	;
factor: NUMBER {$$ = $1;}
	| '(' expr ')' {$$ = $2;}
	| '+' factor {$$ = $2;}
	| '-' factor {$$ = factory->negation($2);}
	;
%%
void yyerror(yyscan_t, theExpressionEngine::expression::ptr *const, theExpressionEngine::factory::ptr const &, const char*const _p)
{	throw std::logic_error(_p);
}
