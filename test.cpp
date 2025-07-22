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

#define __TEST2__(sin, val0, val1, name)\
BOOST_AUTO_TEST_CASE(name)\
{	const auto pFactory = theExpressionEngine::factory<true>::getFactory();\
	const theExpressionEngine::factory<true>::name2int s = {{"x", pFactory->parameter(0)}, {"y", pFactory->parameter(1)}};\
	const auto pExpr = pFactory->parse(#sin "(" #val0 "," #val1 ")", s);\
	BOOST_CHECK_CLOSE(pExpr->evaluate(nullptr, nullptr), std::sin(val0, val1), 0.001);\
	BOOST_CHECK_CLOSE(pExpr->evaluateLLVM(nullptr, nullptr), std::sin(val0, val1), 0.001);\
	BOOST_CHECK(pFactory->parse(#sin "(" "x" "," "y" ")", s)->replace({{pFactory->parameter(0), pFactory->realConstant(val0)},{pFactory->parameter(1), pFactory->realConstant(val1)}}, *pFactory) == pExpr);\
	const double adX[] = {val0, val1};\
	BOOST_CHECK_CLOSE(pFactory->parse(#sin "(" "x" "," "y" ")", s)->evaluateLLVM(adX, nullptr), std::sin(val0, val1), 0.001);\
	BOOST_CHECK_CLOSE(pFactory->parse(#sin "(" "x" "," "y" ")", s)->evaluate(adX, nullptr), std::sin(val0, val1), 0.001);\
}
__TEST2__(pow, 2.1, 3.1, expression_2_000)
__TEST2__(atan2, 2.1, 3.1, expression_2_001)
__TEST2__(hypot, 2.1, 3.1, expression_2_002)
__TEST2__(fmod, 2.1, 1.1, expression_2_003)
__TEST2__(max, 2.1, 1.1, expression_2_004)
__TEST2__(min, 2.1, 1.1, expression_2_005)
__TEST2__(max, 1.1, 2.1, expression_2_006)
__TEST2__(min, 1.1, 2.1, expression_2_007)
#define __TEST__(sin, val, name)\
BOOST_AUTO_TEST_CASE(name)\
{	const auto pFactory = theExpressionEngine::factory<true>::getFactory();\
	const theExpressionEngine::factory<true>::name2int s = {{"x", pFactory->parameter(0)}};\
	const auto pExpr = pFactory->parse(#sin "(" #val ")", s);\
	BOOST_CHECK_CLOSE(pExpr->evaluate(nullptr, nullptr), std::sin(val), 0.001);\
	BOOST_CHECK_CLOSE(pExpr->evaluateLLVM(nullptr, nullptr), std::sin(val), 0.001);\
	BOOST_CHECK(pFactory->parse(#sin "(" "x" ")", s)->replace({{pFactory->parameter(0), pFactory->realConstant(val)}}, *pFactory) == pExpr);\
	const double dX = val;\
	BOOST_CHECK_CLOSE(pFactory->parse(#sin "(" "x" ")", s)->evaluateLLVM(&dX, nullptr), std::sin(val), 0.001);\
	BOOST_CHECK_CLOSE(pFactory->parse(#sin "(" "x" ")", s)->evaluate(&dX, nullptr), std::sin(val), 0.001);\
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
__TEST__(fabs, -2.0, expression_015)
__TEST__(log10, 2.0, expression_016)
__TEST__(erf, 2.0, expression_017)
__TEST__(erfc, 2.0, expression_018)
__TEST__(tgamma, 2.0, expression_019)
__TEST__(lgamma, 2.0, expression_020)
__TEST__(cbrt, 2.0, expression_021)
BOOST_AUTO_TEST_CASE(zero_000)
{	const auto pFactory = theExpressionEngine::factory<true>::getFactory();
	const theExpressionEngine::factory<true>::name2int s = {{"x", pFactory->parameter(0)}};
	BOOST_CHECK(pFactory->parse("0*x", s) == pFactory->parse("0", s));
	BOOST_CHECK(pFactory->parse("x*0", s) == pFactory->parse("0", s));
}
BOOST_AUTO_TEST_CASE(zero_001)
{	const auto pFactory = theExpressionEngine::factory<true>::getFactory();
	const theExpressionEngine::factory<true>::name2int s = {{"x", pFactory->parameter(0)}};
	BOOST_CHECK(pFactory->parse("1*x", s) == pFactory->parse("x", s));
	BOOST_CHECK(pFactory->parse("x*1", s) == pFactory->parse("x", s));
}
BOOST_AUTO_TEST_CASE(zero_002)
{	const auto pFactory = theExpressionEngine::factory<true>::getFactory();
	const theExpressionEngine::factory<true>::name2int s = {{"x", pFactory->parameter(0)}};
	BOOST_CHECK(pFactory->parse("0/x", s) == pFactory->parse("0", s));
}
BOOST_AUTO_TEST_CASE(zero_003)
{	const auto pFactory = theExpressionEngine::factory<true>::getFactory();
	const theExpressionEngine::factory<true>::name2int s = {{"x", pFactory->parameter(0)}};
	BOOST_CHECK(pFactory->parse("x/1", s) == pFactory->parse("x", s));
}
BOOST_AUTO_TEST_CASE(zero_004)
{	const auto pFactory = theExpressionEngine::factory<true>::getFactory();
	const theExpressionEngine::factory<true>::name2int s = {{"x", pFactory->parameter(0)}};
	BOOST_CHECK(pFactory->parse("0+x", s) == pFactory->parse("x", s));
	BOOST_CHECK(pFactory->parse("x+0", s) == pFactory->parse("x", s));
	BOOST_CHECK(pFactory->parse("x-0", s) == pFactory->parse("x", s));
	BOOST_CHECK(pFactory->parse("0-x", s) == pFactory->parse("-x", s));
}
BOOST_AUTO_TEST_CASE(zero_005)
{	const auto pFactory = theExpressionEngine::factory<true>::getFactory();
	const theExpressionEngine::factory<true>::name2int s = {{"x", pFactory->parameter(0)}};
	const auto s1 = pFactory->parse("1+x", s);
	const auto s2 = pFactory->multiplication(s1, s1);
	const double d = 1.1;
	const auto d1 = (d + 1.0)*(d + 1.0);
	BOOST_CHECK_CLOSE(s2->evaluate(&d, nullptr), d1, 0.001);
	BOOST_CHECK_CLOSE(s2->evaluateLLVM(&d, nullptr), d1, 0.001);
	std::vector<double> sC;
	const auto sES = pFactory->createExpressionSet({s2});
	theExpressionEngine::expressionSet<true>::atomicVec sCount;
	sCount.resize(sES->getChildren().size());
	BOOST_CHECK(sES->getChildren().size() == 2);
	BOOST_CHECK(sES->getTempSize() == 1);
	BOOST_CHECK(sES->getChildren().at(1) == pFactory->multiplication(pFactory->variable(0), pFactory->variable(0)));
	BOOST_CHECK(sES->getChildren().at(0) == s1);
	boost::asio::thread_pool sPool(std::thread::hardware_concurrency());
	sES->evaluate(sC, &d, sCount, sPool);
	BOOST_CHECK_CLOSE(sC.at(sES->getTempSize()), d1, 0.001);
	sES->evaluateLLVM(sC, &d, sCount, sPool);
	BOOST_CHECK_CLOSE(sC.at(sES->getTempSize()), d1, 0.001);
}
BOOST_AUTO_TEST_CASE(zero_006)
{	const auto pFactory = theExpressionEngine::factory<true>::getFactory();
	const theExpressionEngine::factory<true>::name2int s = {{"x", pFactory->parameter(0)}};
#define str(x) #x
#define expand(x) str(x)
#define expr(x) ((1+x)*(1-x)*(2+x)*(2-x)*(3+x)*(3-x))
	const auto s1 = pFactory->parse(expand(expr(x)), s);
	const auto s2 = pFactory->parse(expand(expr(x)) "*" expand(expr(x)), s);
	const auto s3 = pFactory->parse(expand(expr(x)) "*" expand(expr(x)) "*" expand(expr(x)), s);
	const double x = 1.1;
	const auto d1 = expr(x);
	const auto d2 = expr(x)*expr(x);
	const auto d3 = expr(x)*expr(x)*expr(x);
#undef str
#undef expand
#undef expr
	BOOST_CHECK_CLOSE(s1->evaluate(&x, nullptr), d1, 0.001);
	BOOST_CHECK_CLOSE(s2->evaluate(&x, nullptr), d2, 0.001);
	BOOST_CHECK_CLOSE(s3->evaluate(&x, nullptr), d3, 0.001);
	BOOST_CHECK_CLOSE(s1->evaluateLLVM(&x, nullptr), d1, 0.001);
	BOOST_CHECK_CLOSE(s2->evaluateLLVM(&x, nullptr), d2, 0.001);
	BOOST_CHECK_CLOSE(s3->evaluateLLVM(&x, nullptr), d3, 0.001);

	std::vector<double> sC;
	const auto sES = pFactory->createExpressionSet({s1, s2, s3});
	theExpressionEngine::expressionSet<true>::atomicVec sCount;
	sCount.resize(sES->getChildren().size());
	boost::asio::thread_pool sPool(std::thread::hardware_concurrency());
	sES->evaluate(sC, &x, sCount, sPool);
	BOOST_CHECK_CLOSE(sC.at(sES->getTempSize() + 2), d3, 0.001);
	sES->evaluateLLVM(sC, &x, sCount, sPool);
	BOOST_CHECK_CLOSE(sC.at(sES->getTempSize() + 2), d3, 0.001);
}
BOOST_AUTO_TEST_CASE(zero_007)
{	const auto pFactory = theExpressionEngine::factory<true>::getFactory();
	const auto pX = pFactory->parameter(0);
	using namespace theExpressionEngine;
	BOOST_CHECK(pX + pX == pFactory->addition(pX, pX));
	BOOST_CHECK(pX - pX == pFactory->subtraction(pX, pX));
	BOOST_CHECK(pX * pX == pFactory->multiplication(pX, pX));
	BOOST_CHECK(pX / pX == pFactory->division(pX, pX));
	BOOST_CHECK(sin(pX) == pFactory->sin(pX));
}
