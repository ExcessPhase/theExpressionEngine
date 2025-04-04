#pragma once

typedef void* yyscan_t;
#include "bison.h"
int yylex(YYSTYPE *const, yyscan_t, theExpressionEngine::factory::ptr const &);
#define YY_DECL int yylex (YYSTYPE*const yylval, yyscan_t yyscanner, theExpressionEngine::factory::ptr const &factory)
