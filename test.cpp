#include "factory.h"
#include "expression.h"
#define BOOST_TEST_MODULE mytests
#include <boost/test/included/unit_test.hpp>
#include "llvm/Support/TargetSelect.h"

void initializeLLVM() {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
    // Add other LLVM initialization code as needed.
}
struct LLVMSetup {
    LLVMSetup() {
        initializeLLVM();
    }
    ~LLVMSetup() {
        // Cleanup if necessary.
    }
};
BOOST_GLOBAL_FIXTURE(LLVMSetup);

#define __TEST__(sin, val, name)\
BOOST_AUTO_TEST_CASE(name)\
{	const auto pFactory = theExpressionEngine::factory::getFactory();\
	const theExpressionEngine::factory::name2int s = {{"x", pFactory->parameter(0)}};\
	const auto pExpr = pFactory->parse(#sin "(" #val ")", s);\
	BOOST_CHECK(pExpr->evaluate(nullptr) == std::sin(val));\
	BOOST_CHECK(pExpr->evaluateLLVM(nullptr) == std::sin(val));\
	BOOST_CHECK(pFactory->parse(#sin "(" "x" ")", s)->replace({{pFactory->parameter(0), pFactory->realConstant(val)}}, *pFactory) == pExpr);\
}

__TEST__(sin, 1.0, expression_000)
__TEST__(cos, 1.0, expression_001)
__TEST__(tan, 1.0, expression_002)
__TEST__(sinh, 1.0, expression_003)
__TEST__(cosh, 1.0, expression_004)
__TEST__(tanh, 1.0, expression_005)
__TEST__(asin, 0.1, expression_006)
__TEST__(acos, 0.1, expression_007)
__TEST__(atan, 0.1, expression_008)
__TEST__(asinh, 0.1, expression_009)
__TEST__(acosh, 2.0, expression_010)
__TEST__(atanh, 0.1, expression_011)
__TEST__(exp, 1.0, expression_012)
__TEST__(log, 2.0, expression_013)
__TEST__(sqrt, 2.0, expression_014)
__TEST__(abs, -2.0, expression_015)
__TEST__(log10, 2.0, expression_016)
__TEST__(erf, 2.0, expression_017)
__TEST__(erfc, 2.0, expression_018)
__TEST__(tgamma, 2.0, expression_019)
__TEST__(lgamma, 2.0, expression_020)
__TEST__(cbrt, 2.0, expression_021)
BOOST_AUTO_TEST_CASE(zero_000)
{	const auto pFactory = theExpressionEngine::factory::getFactory();
	const theExpressionEngine::factory::name2int s = {{"x", pFactory->parameter(0)}};
	BOOST_CHECK(pFactory->parse("0*x", s) == pFactory->parse("0", s));
	BOOST_CHECK(pFactory->parse("x*0", s) == pFactory->parse("0", s));
}
BOOST_AUTO_TEST_CASE(zero_001)
{	const auto pFactory = theExpressionEngine::factory::getFactory();
	const theExpressionEngine::factory::name2int s = {{"x", pFactory->parameter(0)}};
	BOOST_CHECK(pFactory->parse("1*x", s) == pFactory->parse("x", s));
	BOOST_CHECK(pFactory->parse("x*1", s) == pFactory->parse("x", s));
}
BOOST_AUTO_TEST_CASE(zero_002)
{	const auto pFactory = theExpressionEngine::factory::getFactory();
	const theExpressionEngine::factory::name2int s = {{"x", pFactory->parameter(0)}};
	BOOST_CHECK(pFactory->parse("0/x", s) == pFactory->parse("0", s));
}
BOOST_AUTO_TEST_CASE(zero_003)
{	const auto pFactory = theExpressionEngine::factory::getFactory();
	const theExpressionEngine::factory::name2int s = {{"x", pFactory->parameter(0)}};
	//const auto sDiv = pFactory->parse("x/1", s);
	//const auto sX = pFactory->parse("x", s);
	BOOST_CHECK(pFactory->parse("x/1", s) == pFactory->parse("x", s));
}
