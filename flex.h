#pragma once

typedef void* yyscan_t;
#include "bison.h"
//int yylex(YYSTYPE *const, yyscan_t, theExpressionEngine::factory::ptr const &, theExpressionEngine::factory::name2int const &);
int yylex(yy::parser::value_type* yylval, void* scanner);
//#define YY_DECL int yylex (YYSTYPE*const yylval, yyscan_t yyscanner, theExpressionEngine::factory::ptr const &factory, theExpressionEngine::factory::name2int const &_rN2I)
#define YY_DECL int yylex(yy::parser::value_type* const yylval, yyscan_t yyscanner, theExpressionEngine::factory::ptr const factory, theExpressionEngine::factory::name2int const& _rN2I)
