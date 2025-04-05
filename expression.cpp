#include <iostream>
#include <any>
#include <typeinfo>
#include <algorithm>
#include "expression.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/Support/TargetSelect.h>

//#include <set>
//#include "environment.h"


namespace theExpressionEngine
{
expression::ptr collapse(const expression&_r, const factory&_rF)
{	return _r.collapse(_rF);
}
expression::expression(
	children&&_rChildren
)
	:m_sChildren(std::move(_rChildren))
{
}
bool expression::operator<(const expression&_r) const
{	if (m_sChildren.size() < _r.m_sChildren.size())
		return true;
	else
	if (m_sChildren.size() > _r.m_sChildren.size())
		return false;
	else
	{	const auto s = std::mismatch(
			m_sChildren.begin(),
			m_sChildren.end(),
			_r.m_sChildren.begin()
		);
		if (s.first != m_sChildren.end())
			if (*s.first < *s.second)
				return true;
			else
				return false;
		else
		{	const auto &r0 = typeid(*this);
			const auto &r1 = typeid(_r);
			if (r0.before(r1))
				return true;
			else
			if (r1.before(r0))
				return false;
			else
			return isSmaller(_r);
		}
	}
}
bool expression::isSmaller(const expression&) const
{	return false;
}
void expression::onDestroy(void) const
{	for (auto &r : m_sOnDestroyList)
		r();
}
void expression::addOnDestroy(onDestroyFunctor _s) const
{	m_sOnDestroyList.emplace_back(std::move(_s));
}
namespace
{
using namespace llvm;
struct llvmData
{
	LLVMContext Context;
	std::unique_ptr<Module> M;// = std::make_unique<Module>("top", Context);
	std::unique_ptr<ExecutionEngine> EE;// = std::unique_ptr<ExecutionEngine>(EngineBuilder(std::move(M)).setErrorStr(&ErrStr).setEngineKind(EngineKind::JIT).create());
	using JITFunctionType = double(*)(const double*);
	JITFunctionType jitFunction;// = reinterpret_cast<JITFunctionType>(funcAddress);
	explicit llvmData(const expression *_p)
		:Context(),
		M(std::make_unique<Module>("top", Context))
		//EE(std::unique_ptr<ExecutionEngine>(EngineBuilder(std::move(M)).setErrorStr(&ErrStr).setEngineKind(EngineKind::JIT).create()))
	{
		IRBuilder<> Builder(Context);
		FunctionType* FT = FunctionType::get(
			Type::getDoubleTy(Context),
			{	Type::getDoublePtrTy(Context)
			},
			false
		);
		Function* GetValueFunc = Function::Create(FT, Function::ExternalLinkage, "getValue", M.get());
		BasicBlock* BB = BasicBlock::Create(Context, "EntryBlock", GetValueFunc);
		Builder.SetInsertPoint(BB);

		Function::arg_iterator args = GetValueFunc->arg_begin();
		Value* doublePtrArg = &(*args);
		// Create a constant double value
		Value* const ConstantVal = _p->generateCodeW(nullptr, Context, Builder, M.get(), doublePtrArg);//ConstantFP::get(Context, APFloat(3.14));

		// Return the constant value
		Builder.CreateRet(ConstantVal);

		if (verifyModule(*M, &errs()))
		{	throw std::runtime_error("module verification failed");
			//std::cerr << "Error: module verification failed" << std::endl;
			//M->print(errs(), nullptr);
			//return 1;
		}

		// Create the execution engine
		std::string ErrStr;
		EE = std::unique_ptr<ExecutionEngine>(EngineBuilder(std::move(M)).setErrorStr(&ErrStr).setEngineKind(EngineKind::JIT).create());
		if (!EE)
		{	//errs() << "Failed to create ExecutionEngine: " << ErrStr << "\n";
			//return 1;
			throw std::runtime_error("Failed to create ExecutionEngine");
		}

		// Add the module and compile the function
		EE->finalizeObject();

		auto funcAddress = EE->getFunctionAddress("getValue");
		if (!funcAddress) {
		    //std::cerr << "Error: Failed to get function address for 'getValue'" << std::endl;
		    //return 1;
			throw std::runtime_error("Failed to get function address for 'getValue'");
		}

		// Cast the function address to a function pointer with the correct signature
		using JITFunctionType = double(*)(const double*);
		jitFunction = reinterpret_cast<JITFunctionType>(funcAddress);
	}
};
}
double expression::evaluateLLVM(const double *const _p) const
{	const auto sInsert = m_sAttachedData.emplace(this, ARRAY());
	std::any &r = sInsert.first->second[eLLVMdata];
	if (sInsert.second)
	{	addOnDestroy(
			[this](void)
			{	m_sAttachedData.erase(this);	
			}
		);
		r = std::make_shared<const llvmData>(this);
	}
	return std::any_cast<const std::shared_ptr<const llvmData>&>(r)->jitFunction(_p);
}
}
