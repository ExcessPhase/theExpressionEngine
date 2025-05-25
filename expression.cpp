#include <iostream>
#include <any>
#include <stack>
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
{	std::unique_lock<MUTEX> sLock(getMutex());
	m_sOnDestroyList.emplace_back(std::move(_s));
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
	explicit llvmData(const expression *const _p, const expression*const _pRoot)
		:Context(),
		M(std::make_unique<Module>("top", Context))
		//EE(std::unique_ptr<ExecutionEngine>(EngineBuilder(std::move(M)).setErrorStr(&ErrStr).setEngineKind(EngineKind::JIT).create()))
	{
		IRBuilder<> Builder(Context);
		FunctionType* FT = FunctionType::get(
			Type::getDoubleTy(Context),
			{	Type::getDoubleTy(Context)->getPointerTo()
			},
			false
		);
		Function* GetValueFunc = Function::Create(FT, Function::ExternalLinkage, "getValue", M.get());
		BasicBlock* BB = BasicBlock::Create(Context, "EntryBlock", GetValueFunc);
		Builder.SetInsertPoint(BB);

		Function::arg_iterator args = GetValueFunc->arg_begin();
		Value* doublePtrArg = &(*args);
		// Create a constant double value
		Value* const ConstantVal = _p->generateCodeW(_pRoot, Context, Builder, M.get(), doublePtrArg);//ConstantFP::get(Context, APFloat(3.14));

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
double expression::evaluateLLVM(const double *const _p, const expression*const _pRoot) const
{	std::unique_lock<MUTEX> sLock(getMutex());
	const auto sInsert = m_sAttachedData.emplace(_pRoot, ARRAY());
	std::any &r = sInsert.first->second[eLLVMdata];
	if (sInsert.second)
	{	_pRoot->addOnDestroy(
			[_pRoot, this](void)
			{	std::unique_lock<MUTEX> sLock(getMutex());
				m_sAttachedData.erase(_pRoot);
			}
		);
		r = std::make_shared<const llvmData>(this, _pRoot);
	}
	sLock.unlock();
	return std::any_cast<const std::shared_ptr<const llvmData>&>(r)->jitFunction(_p);
}
expression::ptr expression::replace(const ptr2ptr&_r, const factory&_rF) const
{	std::vector<std::tuple<ptr, size_t, children, bool> > sStack;
	sStack.push_back({this, std::numeric_limits<std::size_t>::max(), children(), true});
	while (!sStack.empty())
	{	const auto [pThis, iParentPos, sChildren, b] = sStack.back();
		sStack.pop_back();
		if (b)
		{	const auto pFind = _r.find(pThis);
			if (pFind != _r.end())
				sStack.push_back({pFind->second, iParentPos, pFind->second->m_sChildren, false});
			else
			{	sStack.push_back({pThis, iParentPos, children(), false});
				const auto iPos = sStack.size() - 1;
				for (const auto &p : pThis->m_sChildren)
					sStack.push_back({p, iPos, children(), true});
			}
		}
		else
			if (iParentPos == std::numeric_limits<std::size_t>::max())
				if (pThis->m_sChildren != sChildren)
					return pThis->recreateFromChildren(sChildren, _rF);
				else
					return pThis;
			else
				if (pThis->m_sChildren != sChildren)
					std::get<2>(sStack.at(iParentPos)).push_back(
						pThis->recreateFromChildren(sChildren, _rF)
					);
				else
					std::get<2>(sStack.at(iParentPos)).push_back(
						pThis
					);
	}
	std::abort();
	return nullptr;
}
std::size_t expression::getWeightW(void) const
{	std::stack<ptr> s;
	s.push(this);
	std::size_t i = 0;
	while (!s.empty())
	{	const auto p = s.top();
		i += p->getWeight();
		s.pop();
		for (const auto &p : m_sChildren)
			s.push(p);
	}
	return i;
}
llvm::Value *expression::generateCodeW(
	const expression *const _pRoot,
	llvm::LLVMContext& context,
	llvm::IRBuilder<>& builder,
	llvm::Module *const M,
	llvm::Value*const _pP
) const
{	std::unique_lock<MUTEX> sLock(getMutex());
	const auto sInsert = m_sAttachedData.emplace(_pRoot, ARRAY());
	if (sInsert.second)
		_pRoot->addOnDestroy(
			[_pRoot, this](void)
			{	std::unique_lock<MUTEX> sLock(getMutex());
				m_sAttachedData.erase(_pRoot);
			}
		);
	auto &rAny = sInsert.first->second[eLLVMValuePtr];
	if (!rAny.has_value())
		rAny = generateCode(
			_pRoot ? _pRoot : this,
			context,
			builder,
			M,
			_pP
		);
	return std::any_cast<llvm::Value*>(rAny);
}
}
