#include "factory.h"
#include "environment.h"
#include "expression.h"
#include "type.h"
#include "unique.h"
#include "onDestroy.h"
#include "dynamic_cast.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include <stdexcept>

namespace theExpessionEngine
{
struct realConstant:expression
{	const double m_d;
	realConstant(const double _d)
		:expression(unique<type>::create(type::eReal)),
		m_d(_d)
	{
	}
	virtual bool isSmaller(const expression&_r) const override
	{	const auto &r = dynamic_cast<const realConstant&>(_r);
		if (m_d < r.m_d)
			return true;
		else
			return false;
	}
	virtual double evaluateThis(environment&) const override
	{	return m_d;
	}
	virtual llvm::Value* generateCode(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module *const M) const override
	{	return llvm::ConstantFP::get(context, llvm::APFloat(m_d));
	}
};
namespace
{
struct plusImpl:expression
{	plusImpl(const ptr&_p0, const ptr&_p1)
		:expression(
			_p0->m_sType,
			children({_p0, _p1})
		)
	{
	}
	virtual double evaluateThis(environment&_r) const override
	{	return m_sChildren.at(0)->evaluate(_r) + m_sChildren.at(1)->evaluate(_r);
	}
	virtual llvm::Value* generateCode(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module *const M) const override
	{	return builder.CreateFAdd(
			m_sChildren.at(0)->generateCodeW(context, builder, M),
			m_sChildren.at(1)->generateCodeW(context, builder, M),
			"plus"
		);
	}
};
struct _sqrt:expression
{	_sqrt(const ptr&_p)
		:expression(
			_p->m_sType,
			children({_p})
		)
	{
	}
	virtual double evaluateThis(environment&_r) const override
	{	return std::sqrt(m_sChildren.at(0)->evaluate(_r));
	}
	mutable llvm::FunctionCallee SqrtFunc;
	mutable llvm::FunctionType *SqrtTy = nullptr;
	virtual llvm::Value* generateCode(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module *const M) const override
	{	    // Create the function prototype for std::sqrt
		using namespace llvm;
		llvm::Function* const sqrtFunc = llvm::Intrinsic::getDeclaration(M, llvm::Intrinsic::sqrt, builder.getDoubleTy());
		//llvm::Value* sqrtResult = builder.CreateCall(sqrtFunc, sum);	    
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
	{	return unique<onDestroy<dynamic_cast_implementation<theExpessionEngine::realConstant> > >::create(_d);
	}
	virtual exprPtr sqrt(const exprPtr&_p) const override
	{	return unique<onDestroy<_sqrt> >::create(_p);
	}
	virtual exprPtr plus(const exprPtr&_p0, const exprPtr&_p1) const override
	{	return unique<onDestroy<plusImpl> >::create(_p0, _p1);
	}
	virtual exprPtr minus(const exprPtr&, const exprPtr&) const override
	{	return nullptr;
	}
	virtual exprPtr multiply(const exprPtr&, const exprPtr&) const override
	{	return nullptr;
	}
	virtual exprPtr divide(const exprPtr&, const exprPtr&) const override
	{	return nullptr;
	}
};
}
factory::ptr factory::getFactory(environment&_r)
{	auto &rAny = _r.m_s[environment::eFactory];
	if (!rAny.has_value())
		rAny = std::make_shared<const factoryImpl>();
	return std::any_cast<const std::shared_ptr<const factoryImpl>&>(rAny);
}
}
