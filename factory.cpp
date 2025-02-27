#include "factory.h"
#include "expression.h"
#include "unique.h"
#include "onDestroy.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <stdexcept>
#include <regex>

namespace theExpessionEngine
{
struct realConstant:expression
{	const double m_d;
	realConstant(const double _d)
		:m_d(_d)
	{
	}
	virtual bool isSmaller(const expression&_r) const override
	{	const auto &r = dynamic_cast<const realConstant&>(_r);
		if (m_d < r.m_d)
			return true;
		else
			return false;
	}
	virtual double evaluate(const double *const) const override
	{	return m_d;
	}
	virtual llvm::Value* generateCode(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module *const M) const override
	{	return llvm::ConstantFP::get(context, llvm::APFloat(m_d));
	}
};
namespace
{
struct addition:expression
{	addition(const ptr&_p0, const ptr&_p1)
		:expression(
			children({_p0, _p1})
		)
	{
	}
	virtual double evaluate(const double *const _p) const override
	{	return m_sChildren.at(0)->evaluate(_p) + m_sChildren.at(1)->evaluate(_p);
	}
	virtual llvm::Value* generateCode(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module *const M) const override
	{	return builder.CreateFAdd(
			m_sChildren.at(0)->generateCodeW(context, builder, M),
			m_sChildren.at(1)->generateCodeW(context, builder, M),
			"plus"
		);
	}
};
struct sqrt:expression
{	sqrt(const ptr&_p)
		:expression(
			children({_p})
		)
	{
	}
	virtual double evaluate(const double *const _p) const override
	{	return std::sqrt(m_sChildren.at(0)->evaluate(_p));
	}
	virtual llvm::Value* generateCode(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module *const M) const override
	{	    // Create the function prototype for std::sqrt
		using namespace llvm;
		llvm::Function* const sqrtFunc = llvm::Intrinsic::getDeclaration(M, llvm::Intrinsic::sqrt, builder.getDoubleTy());
	    	return builder.CreateCall(
			sqrtFunc,
			m_sChildren.at(0)->generateCodeW(context, builder, M),
			"sqrt"
		);
	}
};
#if 0
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

using namespace llvm;

int main() {
    LLVMContext Context;
    Module *M = new Module("cmath_function_example", Context);
    IRBuilder<> Builder(Context);

    // Create the function prototype for std::sqrt
    FunctionType *SqrtTy = FunctionType::get(Type::getDoubleTy(Context), {Type::getDoubleTy(Context)}, false);
    FunctionCallee SqrtFunc = M->getOrInsertFunction("sqrt", SqrtTy);

    // Create the function that returns a double and calls std::sqrt
    FunctionType *FuncTy = FunctionType::get(Type::getDoubleTy(Context), false);
    Function *Func = Function::Create(FuncTy, Function::ExternalLinkage, "generateSqrt", M);
    BasicBlock *EntryBB = BasicBlock::Create(Context, "entry", Func);
    Builder.SetInsertPoint(EntryBB);

    // Call std::sqrt with a constant argument and return the result
    Value *Arg = ConstantFP::get(Context, APFloat(16.0));
    Value *SqrtCall = Builder.CreateCall(SqrtFunc, {Arg});
    Builder.CreateRet(SqrtCall);

    // Verify the module
    verifyModule(*M, &errs());

    // Print the module
    M->print(outs(), nullptr);

    delete M;
    return 0;
}
#endif
struct factoryImpl:factory
{	virtual exprPtr realConstant(const double _d) const override
	{	return unique<onDestroy<theExpessionEngine::realConstant> >::create(_d);
	}
	virtual exprPtr sqrt(const exprPtr&_p) const override
	{	return unique<onDestroy<theExpessionEngine::sqrt> >::create(_p);
	}
	virtual exprPtr addition(const exprPtr&_p0, const exprPtr&_p1) const override
	{	return unique<onDestroy<theExpessionEngine::addition> >::create(_p0, _p1);
	}
	virtual exprPtr subtraction(const exprPtr&, const exprPtr&) const override
	{	return nullptr;
	}
	virtual exprPtr multiplication(const exprPtr&, const exprPtr&) const override
	{	return nullptr;
	}
	virtual exprPtr division(const exprPtr&, const exprPtr&) const override
	{	return nullptr;
	}
	exprPtr parseSqrt(std::string::const_iterator &_p, const std::string::const_iterator &_pEnd) const
	{	static const std::regex sRegex(R"(sqrt[ \t]*\([ \t]*(.*)\)[ \t]*)");
		std::smatch sMatch;

		if (std::regex_match(_p, _pEnd, sMatch, sRegex))
		{	_p = sMatch[0].second;
			return sqrt(parseSingle(_p, _pEnd));
		}
		else
			return nullptr;
	}
	exprPtr parseSingle(std::string::const_iterator &_p, const std::string::const_iterator &_pEnd) const
	{	return parseSqrt(_p, _pEnd);
	}
	exprPtr parseMultDiv(std::string::const_iterator &_p, const std::string::const_iterator &_pEnd) const
	{	return parseSingle(_p, _pEnd);
	}
	exprPtr parseAddSub(std::string::const_iterator &_p, const std::string::const_iterator &_pEnd) const
	{	return parseMultDiv(_p, _pEnd);
	}
	virtual exprPtr parse(const std::string&_r) const override
	{	auto p = _r.begin();
		if (const auto pRet = parseAddSub(p, _r.end()))
			if (p != _r.end())
				throw std::runtime_error("Parse error!");
			else
				return pRet;
		else
			throw std::runtime_error("Parse error!");
	}
};
}
factory::ptr factory::getFactory(void)
{	static const factory::ptr s(std::make_shared<const factoryImpl>());
	return s;
}
}
