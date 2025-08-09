#include "factory.h"
#include "expression.h"
#define BOOST_TEST_MODULE mytests
#include <boost/test/included/unit_test.hpp>
#include <llvm/Support/TargetSelect.h>

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
	const theExpressionEngine::factory<true>::name2int s = {{"x", pFactory->parameter(0, true)}, {"y", pFactory->parameter(1, true)}};\
	const auto pExpr = pFactory->parse(#sin "(" #val0 "," #val1 ")", s);\
	BOOST_CHECK(pExpr == pFactory->realConstant(std::sin(val0, val1)));\
	BOOST_CHECK_CLOSE(pExpr->evaluate(nullptr, nullptr, nullptr, nullptr), std::sin(val0, val1), 0.001);\
	BOOST_CHECK_CLOSE(pExpr->evaluateLLVM(nullptr, nullptr, nullptr, nullptr), std::sin(val0, val1), 0.001);\
	BOOST_CHECK(pFactory->parse(#sin "(" "x" "," "y" ")", s)->replace({{pFactory->parameter(0, true), pFactory->realConstant(val0)},{pFactory->parameter(1, true), pFactory->realConstant(val1)}}, *pFactory) == pExpr);\
	const double adX[] = {val0, val1};\
	BOOST_CHECK_CLOSE(pFactory->parse(#sin "(" "x" "," "y" ")", s)->evaluateLLVM(adX, nullptr, nullptr, nullptr), std::sin(val0, val1), 0.001);\
	BOOST_CHECK_CLOSE(pFactory->parse(#sin "(" "x" "," "y" ")", s)->evaluate(adX, nullptr, nullptr, nullptr), std::sin(val0, val1), 0.001);\
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
	const theExpressionEngine::factory<true>::name2int s = {{"x", pFactory->parameter(0, true)}};\
	const auto pExpr = pFactory->parse(#sin "(" #val ")", s);\
	BOOST_CHECK_CLOSE(pExpr->evaluate(nullptr, nullptr, nullptr, nullptr), std::sin(val), 0.001);\
	BOOST_CHECK_CLOSE(pExpr->evaluateLLVM(nullptr, nullptr, nullptr, nullptr), std::sin(val), 0.001);\
	BOOST_CHECK(pFactory->parse(#sin "(" "x" ")", s)->replace({{pFactory->parameter(0, true), pFactory->realConstant(val)}}, *pFactory) == pExpr);\
	const double dX = val;\
	BOOST_CHECK_CLOSE(pFactory->parse(#sin "(" "x" ")", s)->evaluateLLVM(&dX, nullptr, nullptr, nullptr), std::sin(val), 0.001);\
	BOOST_CHECK_CLOSE(pFactory->parse(#sin "(" "x" ")", s)->evaluate(&dX, nullptr, nullptr, nullptr), std::sin(val), 0.001);\
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
	const theExpressionEngine::factory<true>::name2int s = {{"x", pFactory->parameter(0, true)}};
	BOOST_CHECK(pFactory->parse("0*x", s) == pFactory->parse("0", s));
	BOOST_CHECK(pFactory->parse("x*0", s) == pFactory->parse("0", s));
}
BOOST_AUTO_TEST_CASE(zero_001)
{	const auto pFactory = theExpressionEngine::factory<true>::getFactory();
	const theExpressionEngine::factory<true>::name2int s = {{"x", pFactory->parameter(0, true)}};
	BOOST_CHECK(pFactory->parse("1*x", s) == pFactory->parse("x", s));
	BOOST_CHECK(pFactory->parse("x*1", s) == pFactory->parse("x", s));
}
BOOST_AUTO_TEST_CASE(zero_002)
{	const auto pFactory = theExpressionEngine::factory<true>::getFactory();
	const theExpressionEngine::factory<true>::name2int s = {{"x", pFactory->parameter(0, true)}};
	BOOST_CHECK(pFactory->parse("0/x", s) == pFactory->parse("0", s));
}
BOOST_AUTO_TEST_CASE(zero_003)
{	const auto pFactory = theExpressionEngine::factory<true>::getFactory();
	const theExpressionEngine::factory<true>::name2int s = {{"x", pFactory->parameter(0, true)}};
	BOOST_CHECK(pFactory->parse("x/1", s) == pFactory->parse("x", s));
}
BOOST_AUTO_TEST_CASE(zero_004)
{	const auto pFactory = theExpressionEngine::factory<true>::getFactory();
	const theExpressionEngine::factory<true>::name2int s = {{"x", pFactory->parameter(0, true)}};
	BOOST_CHECK(pFactory->parse("0+x", s) == pFactory->parse("x", s));
	BOOST_CHECK(pFactory->parse("x+0", s) == pFactory->parse("x", s));
	BOOST_CHECK(pFactory->parse("x-0", s) == pFactory->parse("x", s));
	BOOST_CHECK(pFactory->parse("0-x", s) == pFactory->parse("-x", s));
}
BOOST_AUTO_TEST_CASE(zero_005)
{	const auto pFactory = theExpressionEngine::factory<true>::getFactory();
	const theExpressionEngine::factory<true>::name2int s = {{"x", pFactory->parameter(0, true)}};
	const auto s1 = pFactory->parse("1+x", s);
	using namespace theExpressionEngine::operators;
	const auto s2 = s1*s1;
	const double d = 1.1;
	const auto d1 = (d + 1.0)*(d + 1.0);
	BOOST_CHECK_CLOSE(s2->evaluate(&d, nullptr, nullptr, nullptr), d1, 0.001);
	BOOST_CHECK_CLOSE(s2->evaluateLLVM(&d, nullptr, nullptr, nullptr), d1, 0.001);
	std::vector<double> sC;
	std::vector<int> sCI;
	const auto sES = pFactory->createExpressionSet({s2});
	BOOST_CHECK(sES->getChildren().size() == 2);
	BOOST_CHECK(sES->getChildren().at(1) == pFactory->variable(0, true)*pFactory->variable(0, true));
	BOOST_CHECK(sES->getChildren().at(0) == s1);
	sES->evaluate(sC, sCI, &d, nullptr);
	BOOST_CHECK_CLOSE(sC.at(sES->getOrder().at(0)), d1, 0.001);
	sES->evaluateLLVM(sC, sCI, &d, nullptr);
	BOOST_CHECK_CLOSE(sC.at(sES->getOrder().at(0)), d1, 0.001);
}
BOOST_AUTO_TEST_CASE(zero_006)
{	const auto pFactory = theExpressionEngine::factory<true>::getFactory();
	const theExpressionEngine::factory<true>::name2int s = {{"x", pFactory->parameter(0, true)}};
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
	BOOST_CHECK_CLOSE(s1->evaluate(&x, nullptr, nullptr, nullptr), d1, 0.001);
	BOOST_CHECK_CLOSE(s2->evaluate(&x, nullptr, nullptr, nullptr), d2, 0.001);
	BOOST_CHECK_CLOSE(s3->evaluate(&x, nullptr, nullptr, nullptr), d3, 0.001);
	BOOST_CHECK_CLOSE(s1->evaluateLLVM(&x, nullptr, nullptr, nullptr), d1, 0.001);
	BOOST_CHECK_CLOSE(s2->evaluateLLVM(&x, nullptr, nullptr, nullptr), d2, 0.001);
	BOOST_CHECK_CLOSE(s3->evaluateLLVM(&x, nullptr, nullptr, nullptr), d3, 0.001);

	std::vector<double> sC;
	std::vector<int> sCI;
	const auto sES = pFactory->createExpressionSet({s1, s2, s3});
	sES->evaluate(sC, sCI, &x, nullptr);
	BOOST_CHECK_CLOSE(sC.at(sES->getOrder().at(2)), d3, 0.001);
	sES->evaluateLLVM(sC, sCI, &x, nullptr);
	BOOST_CHECK_CLOSE(sC.at(sES->getOrder().at(2)), d3, 0.001);
}
BOOST_AUTO_TEST_CASE(zero_007)
{	const auto pFactory = theExpressionEngine::factory<true>::getFactory();
	const auto pX = pFactory->parameter(0, true);
	const auto pY = pFactory->parameter(1, true);
	//using namespace theExpressionEngine;
	using namespace theExpressionEngine::operators;
	BOOST_CHECK(pX + pY == pFactory->addition(pX, pY));
	BOOST_CHECK(pX - pY == pFactory->subtraction(pX, pY));
	BOOST_CHECK(pX * pY == pFactory->multiplication(pX, pY));
	BOOST_CHECK(pX / pY == pFactory->division(pX, pY));
#define __MAKE_ENTRY__(sin) BOOST_CHECK(sin(pX) == pFactory->sin(pX));
#define __MAKE_ENTRY2__(sin) __MAKE_ENTRY__(sin)
#define __COMMA__
#define __COMMA2__
#include "unary.h"
#define __MAKE_ENTRY__(pow) BOOST_CHECK(pow(pX, pY) == pFactory->pow(pX, pY));
#include "binary.h"
}
BOOST_AUTO_TEST_CASE(zero_008)
{	const auto pFactory = theExpressionEngine::factory<true>::getFactory();
	const auto p1 = pFactory->realConstant(1.0);
	const auto p0 = pFactory->realConstant(0.0);
	//using namespace theExpressionEngine;
	using namespace theExpressionEngine::operators;
	BOOST_CHECK(pFactory->greater_equal(p1, p1) == pFactory->intConstant(1));
	BOOST_CHECK(pFactory->less_equal(p1, p1) == pFactory->intConstant(1));
	BOOST_CHECK(pFactory->greater(p1, p0) == pFactory->intConstant(1));
	BOOST_CHECK(pFactory->greater_equal(p1, p0) == pFactory->intConstant(1));
	BOOST_CHECK(pFactory->less(p0, p1) == pFactory->intConstant(1));
	BOOST_CHECK(pFactory->less_equal(p0, p1) == pFactory->intConstant(1));
	BOOST_CHECK(pFactory->less(p1, p0) == pFactory->intConstant(0));
	BOOST_CHECK(pFactory->greater(p0, p1) == pFactory->intConstant(0));
	BOOST_CHECK(pFactory->greater_equal(p0, p1) == pFactory->intConstant(0));
	const auto pX = pFactory->parameter(0, true);
	const auto pY = pFactory->parameter(1, true);
	const double aXY[2] = {1.0, 1.1};
	BOOST_CHECK(pFactory->greater(pX, pY)->evaluateInt(aXY, nullptr, nullptr, nullptr) == 0);
	BOOST_CHECK(pFactory->greater_equal(pX, pY)->evaluateInt(aXY, nullptr, nullptr, nullptr) == 0);
	BOOST_CHECK(pFactory->less(pX, pY)->evaluateInt(aXY, nullptr, nullptr, nullptr) == 1);
	BOOST_CHECK(pFactory->less_equal(pX, pY)->evaluateInt(aXY, nullptr, nullptr, nullptr) == 1);
	BOOST_CHECK(pFactory->greater(pX, pY)->evaluateIntLLVM(aXY, nullptr, nullptr, nullptr) == 0);
	BOOST_CHECK(pFactory->greater_equal(pX, pY)->evaluateIntLLVM(aXY, nullptr, nullptr, nullptr) == 0);
	BOOST_CHECK(pFactory->less(pX, pY)->evaluateIntLLVM(aXY, nullptr, nullptr, nullptr) == 1);
	BOOST_CHECK(pFactory->less_equal(pX, pY)->evaluateIntLLVM(aXY, nullptr, nullptr, nullptr) == 1);
	const double aXY1[2] = {1.2, 1.1};
	BOOST_CHECK(pFactory->greater(pX, pY)->evaluateInt(aXY1, nullptr, nullptr, nullptr) == 1);
	BOOST_CHECK(pFactory->greater_equal(pX, pY)->evaluateInt(aXY1, nullptr, nullptr, nullptr) == 1);
	BOOST_CHECK(pFactory->less(pX, pY)->evaluateInt(aXY1, nullptr, nullptr, nullptr) == 0);
	BOOST_CHECK(pFactory->less_equal(pX, pY)->evaluateInt(aXY1, nullptr, nullptr, nullptr) == 0);
	BOOST_CHECK(pFactory->greater(pX, pY)->evaluateIntLLVM(aXY1, nullptr, nullptr, nullptr) == 1);
	BOOST_CHECK(pFactory->greater_equal(pX, pY)->evaluateIntLLVM(aXY1, nullptr, nullptr, nullptr) == 1);
	BOOST_CHECK(pFactory->less(pX, pY)->evaluateIntLLVM(aXY1, nullptr, nullptr, nullptr) == 0);
	BOOST_CHECK(pFactory->less_equal(pX, pY)->evaluateIntLLVM(aXY1, nullptr, nullptr, nullptr) == 0);
	const theExpressionEngine::factory<true>::name2int s = {{"x", pFactory->parameter(0, true)}, {"y", pFactory->parameter(1, true)}};
	BOOST_CHECK(pFactory->parse("x<y", s)->evaluateInt(aXY, nullptr, nullptr, nullptr) == 1);
	BOOST_CHECK(pFactory->parse("x<=y", s)->evaluateInt(aXY, nullptr, nullptr, nullptr) == 1);
	BOOST_CHECK(pFactory->parse("x<y", s)->evaluateIntLLVM(aXY, nullptr, nullptr, nullptr) == 1);
	BOOST_CHECK(pFactory->parse("x<=y", s)->evaluateIntLLVM(aXY, nullptr, nullptr, nullptr) == 1);
	BOOST_CHECK(pFactory->parse("x>y", s)->evaluateInt(aXY, nullptr, nullptr, nullptr) == 0);
	BOOST_CHECK(pFactory->parse("x>=y", s)->evaluateInt(aXY, nullptr, nullptr, nullptr) == 0);
	BOOST_CHECK(pFactory->parse("x>y", s)->evaluateIntLLVM(aXY, nullptr, nullptr, nullptr) == 0);
	BOOST_CHECK(pFactory->parse("x>=y", s)->evaluateIntLLVM(aXY, nullptr, nullptr, nullptr) == 0);
}
BOOST_AUTO_TEST_CASE(zero_009)
{	const auto pFactory = theExpressionEngine::factory<true>::getFactory();
	const auto p1 = pFactory->realConstant(1.0);
	const auto p0 = pFactory->realConstant(0.0);
	//using namespace theExpressionEngine;
	using namespace theExpressionEngine::operators;
	BOOST_CHECK(pFactory->equal_to(p1, p1) == pFactory->intConstant(1));
	BOOST_CHECK(pFactory->not_equal_to(p1, p1) == pFactory->intConstant(0));
	BOOST_CHECK(pFactory->equal_to(p1, p0) == pFactory->intConstant(0));
	BOOST_CHECK(pFactory->not_equal_to(p1, p0) == pFactory->intConstant(1));
	const auto pX = pFactory->parameter(0, true);
	const auto pY = pFactory->parameter(1, true);
	const double aXY[2] = {1.0, 1.1};
	BOOST_CHECK(pFactory->equal_to(pX, pY)->evaluateInt(aXY, nullptr, nullptr, nullptr) == 0);
	BOOST_CHECK(pFactory->not_equal_to(pX, pY)->evaluateInt(aXY, nullptr, nullptr, nullptr) == 1);
	BOOST_CHECK(pFactory->equal_to(pX, pY)->evaluateIntLLVM(aXY, nullptr, nullptr, nullptr) == 0);
	BOOST_CHECK(pFactory->not_equal_to(pX, pY)->evaluateIntLLVM(aXY, nullptr, nullptr, nullptr) == 1);
	const theExpressionEngine::factory<true>::name2int s = {{"x", pFactory->parameter(0, true)}, {"y", pFactory->parameter(1, true)}};
	BOOST_CHECK(pFactory->parse("x==y", s)->evaluateInt(aXY, nullptr, nullptr, nullptr) == 0);
	BOOST_CHECK(pFactory->parse("x!=y", s)->evaluateInt(aXY, nullptr, nullptr, nullptr) == 1);
	BOOST_CHECK(pFactory->parse("x==y", s)->evaluateIntLLVM(aXY, nullptr, nullptr, nullptr) == 0);
	BOOST_CHECK(pFactory->parse("x!=y", s)->evaluateIntLLVM(aXY, nullptr, nullptr, nullptr) == 1);
}
BOOST_AUTO_TEST_CASE(zero_010)
{	const auto pFactory = theExpressionEngine::factory<true>::getFactory();
	//using namespace theExpressionEngine;
	using namespace theExpressionEngine::operators;
	const auto pX = pFactory->parameter(0, true);
	const auto pY = pFactory->parameter(1, true);
	const double aXY[2] = {1.0, 1.1};
	const theExpressionEngine::factory<true>::name2int s = {{"x", pFactory->parameter(0, true)}, {"y", pFactory->parameter(1, true)}};
	BOOST_CHECK_CLOSE(pFactory->parse("x>y ? x : y", s)->evaluate(aXY, nullptr, nullptr, nullptr), 1.1, 1e-6);
	BOOST_CHECK_CLOSE(pFactory->parse("x<y ? x : y", s)->evaluate(aXY, nullptr, nullptr, nullptr), 1.0, 1e-6);
	BOOST_CHECK(pFactory->less(pFactory->intConstant(1), pFactory->intConstant(2)) == pFactory->intConstant(1));
	BOOST_CHECK(pFactory->less(pFactory->intConstant(2), pFactory->intConstant(2)) == pFactory->intConstant(0));
	BOOST_CHECK(
		pFactory->conditional(
			pFactory->less(pFactory->intConstant(1), pFactory->intConstant(2)),
			pFactory->intConstant(3),
			pFactory->intConstant(4)
		) == pFactory->intConstant(3)
	);
	BOOST_CHECK(pFactory->parse("1<2 ? 3 : 4", s) == pFactory->intConstant(3));
	BOOST_CHECK(pFactory->parse("1<2 ? 3.0 : 4.0", s) == pFactory->realConstant(3.0));
}
