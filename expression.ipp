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
template<bool BTHREADED>
boost::intrusive_ptr<const expression<BTHREADED> > collapse(const expression<BTHREADED>&_r, const factory<BTHREADED>&_rF)
{	return _r.collapse(_rF);
}
template<bool BTHREADED>
expression<BTHREADED>::expression(
	children&&_rChildren,
	const enumType _e
)
	:m_sChildren(std::move(_rChildren)),
	m_eType(_e)
{
}
template<bool BTHREADED>
bool expression<BTHREADED>::operator<(const expression<BTHREADED>&_r) const
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
template<bool BTHREADED>
bool expression<BTHREADED>::isSmaller(const expression<BTHREADED>&) const
{	return false;
}
template<bool BTHREADED>
void expression<BTHREADED>::onDestroy(void) const
{	for (auto &r : m_sOnDestroyList)
		r();
}
template<bool BTHREADED>
void expression<BTHREADED>::addOnDestroy(onDestroyFunctor _s) const
{	std::unique_lock<MUTEX> sLock(m_sMutex);
	m_sOnDestroyList.emplace_back(std::move(_s));
}
static void createTernaryFunctions(llvm::Module& module, llvm::LLVMContext& context)
{
	llvm::IRBuilder<> builder(context);
	llvm::Type* int32Ty = llvm::Type::getInt32Ty(context);
	llvm::Type* doubleTy = llvm::Type::getDoubleTy(context);

	// --- ternaryInt: i32(i32)
	llvm::FunctionType* intFuncTy = llvm::FunctionType::get(int32Ty, {int32Ty}, false);
	llvm::Function* ternaryInt = llvm::Function::Create(
		intFuncTy,
		llvm::Function::InternalLinkage,
		"ternaryInt",
		module
	);

	llvm::BasicBlock* intEntry = llvm::BasicBlock::Create(context, "entry", ternaryInt);
	llvm::BasicBlock* intThen  = llvm::BasicBlock::Create(context, "then", ternaryInt);
	llvm::BasicBlock* intElse  = llvm::BasicBlock::Create(context, "else", ternaryInt);
	llvm::BasicBlock* intEnd   = llvm::BasicBlock::Create(context, "end", ternaryInt);

	builder.SetInsertPoint(intEntry);
	llvm::Argument* testExprInt = ternaryInt->getArg(0);
	testExprInt->setName("testExpr");

	llvm::Value* condInt = builder.CreateICmpNE(testExprInt, llvm::ConstantInt::get(int32Ty, 0));
	builder.CreateCondBr(condInt, intThen, intElse);

	builder.SetInsertPoint(intThen);
	llvm::Value* thenValInt = llvm::ConstantInt::get(int32Ty, 100);
	builder.CreateBr(intEnd);

	builder.SetInsertPoint(intElse);
	llvm::Value* elseValInt = llvm::ConstantInt::get(int32Ty, -100);
	builder.CreateBr(intEnd);

	builder.SetInsertPoint(intEnd);
	llvm::PHINode* phiInt = builder.CreatePHI(int32Ty, 2, "result");
	phiInt->addIncoming(thenValInt, intThen);
	phiInt->addIncoming(elseValInt, intElse);
	builder.CreateRet(phiInt);

	// --- ternaryDouble: double(i32)
	llvm::FunctionType* doubleFuncTy = llvm::FunctionType::get(doubleTy, {int32Ty}, false);
	llvm::Function* ternaryDouble = llvm::Function::Create(
		doubleFuncTy,
		llvm::Function::InternalLinkage,
		"ternaryDouble",
		module
	);

	llvm::BasicBlock* dblEntry = llvm::BasicBlock::Create(context, "entry", ternaryDouble);
	llvm::BasicBlock* dblThen  = llvm::BasicBlock::Create(context, "then", ternaryDouble);
	llvm::BasicBlock* dblElse  = llvm::BasicBlock::Create(context, "else", ternaryDouble);
	llvm::BasicBlock* dblEnd   = llvm::BasicBlock::Create(context, "end", ternaryDouble);

	builder.SetInsertPoint(dblEntry);
	llvm::Argument* testExprDbl = ternaryDouble->getArg(0);
	testExprDbl->setName("testExpr");

	llvm::Value* condDbl = builder.CreateICmpNE(testExprDbl, llvm::ConstantInt::get(int32Ty, 0));
	builder.CreateCondBr(condDbl, dblThen, dblElse);

	builder.SetInsertPoint(dblThen);
	llvm::Value* thenValDbl = llvm::ConstantFP::get(doubleTy, 3.14);
	builder.CreateBr(dblEnd);

	builder.SetInsertPoint(dblElse);
	llvm::Value* elseValDbl = llvm::ConstantFP::get(doubleTy, -2.71);
	builder.CreateBr(dblEnd);

	builder.SetInsertPoint(dblEnd);
	llvm::PHINode* phiDbl = builder.CreatePHI(doubleTy, 2, "result");
	phiDbl->addIncoming(thenValDbl, dblThen);
	phiDbl->addIncoming(elseValDbl, dblElse);
	builder.CreateRet(phiDbl);
}
using namespace llvm;
template<bool BTHREADED>
struct llvmData
{
	LLVMContext Context;
	std::unique_ptr<Module> M;// = std::make_unique<Module>("top", Context);
	std::unique_ptr<ExecutionEngine> EE;// = std::unique_ptr<ExecutionEngine>(EngineBuilder(std::move(M)).setErrorStr(&ErrStr).setEngineKind(EngineKind::JIT).create());
	using JITFunctionType = double(*)(const double*, const int*, const double*, const int*);
	JITFunctionType jitFunction;// = reinterpret_cast<JITFunctionType>(funcAddress);
	explicit llvmData(const expression<BTHREADED> *const _p)
		:Context(),
		M(std::make_unique<Module>("top", Context))
		//EE(std::unique_ptr<ExecutionEngine>(EngineBuilder(std::move(M)).setErrorStr(&ErrStr).setEngineKind(EngineKind::JIT).create()))
	{
		IRBuilder<> Builder(Context);
		createTernaryFunctions(*M, Context);
		FunctionType* FT = FunctionType::get(
			Type::getDoubleTy(Context),
			{	PointerType::get(Type::getDoubleTy(Context), 0),
				PointerType::get(Type::getInt32Ty(Context), 0),
				PointerType::get(Type::getDoubleTy(Context), 0),
				PointerType::get(Type::getInt32Ty(Context), 0)
			},
			false
		);
		Function* GetValueFunc = Function::Create(FT, Function::ExternalLinkage, "getValue", M.get());
		BasicBlock* BB = BasicBlock::Create(Context, "EntryBlock", GetValueFunc);
		Builder.SetInsertPoint(BB);

		Function::arg_iterator args = GetValueFunc->arg_begin();
		Value* const doublePtrArg0 = &(*args++);
		Value* const intPtrArg1 = &(*args++);
		Value* const doublePtrArg2 = &(*args++);
		Value* const intPtrArg3 = &(*args);
		// Create a constant double value
		Value* const ConstantVal = _p->generateCodeW(_p, Context, Builder, M.get(), doublePtrArg0, intPtrArg1, doublePtrArg2, intPtrArg3);//ConstantFP::get(Context, APFloat(3.14));

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
		//using JITFunctionType = double(*)(const double*, const double*);
		jitFunction = reinterpret_cast<JITFunctionType>(funcAddress);
	}
};
template<bool BTHREADED>
struct llvmDataInt
{
	LLVMContext Context;
	std::unique_ptr<Module> M;// = std::make_unique<Module>("top", Context);
	std::unique_ptr<ExecutionEngine> EE;// = std::unique_ptr<ExecutionEngine>(EngineBuilder(std::move(M)).setErrorStr(&ErrStr).setEngineKind(EngineKind::JIT).create());
	using JITFunctionType = int(*)(const double*, const int*, const double*, const int*);
	JITFunctionType jitFunction;// = reinterpret_cast<JITFunctionType>(funcAddress);
	explicit llvmDataInt(const expression<BTHREADED> *const _p)
		:Context(),
		M(std::make_unique<Module>("top", Context))
		//EE(std::unique_ptr<ExecutionEngine>(EngineBuilder(std::move(M)).setErrorStr(&ErrStr).setEngineKind(EngineKind::JIT).create()))
	{
		IRBuilder<> Builder(Context);
		createTernaryFunctions(*M, Context);
		FunctionType* FT = FunctionType::get(
			Type::getInt32Ty(Context),
			{	PointerType::get(Type::getDoubleTy(Context), 0),
				PointerType::get(Type::getInt32Ty(Context), 0),
				PointerType::get(Type::getDoubleTy(Context), 0),
				PointerType::get(Type::getInt32Ty(Context), 0)
			},
			false
		);
		Function* GetValueFunc = Function::Create(FT, Function::ExternalLinkage, "getValue", M.get());
		BasicBlock* BB = BasicBlock::Create(Context, "EntryBlock", GetValueFunc);
		Builder.SetInsertPoint(BB);

		Function::arg_iterator args = GetValueFunc->arg_begin();
		Value* const doublePtrArg0 = &(*args++);
		Value* const intPtrArg1 = &(*args++);
		Value* const doublePtrArg2 = &(*args++);
		Value* const intPtrArg3 = &(*args);
		// Create a constant double value
		Value* const ConstantVal = _p->generateCodeW(_p, Context, Builder, M.get(), doublePtrArg0, intPtrArg1, doublePtrArg2, intPtrArg3);//ConstantFP::get(Context, APFloat(3.14));

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
		//using JITFunctionType = double(*)(const double*, const double*);
		jitFunction = reinterpret_cast<JITFunctionType>(funcAddress);
	}
};
template<bool BTHREADED>
void expression<BTHREADED>::initializeLLVM(void) const
{	std::unique_lock<MUTEX> sLock(m_sMutex);
	const auto sInsert = m_sAttachedData.emplace(this, ARRAY());
	if (sInsert.second)
	{	addOnDestroy(
			[this](void)
			{	std::unique_lock<MUTEX> sLock(m_sMutex);
				m_sAttachedData.erase(this);
			}
		);
		if (m_eType == eFloatingPoint)
			sInsert.first->second[eLLVMdata] = std::make_shared<const llvmData<BTHREADED> >(this);
		else
			sInsert.first->second[eLLVMdata] = std::make_shared<const llvmDataInt<BTHREADED> >(this);
	}
}
template<bool BTHREADED>
double expression<BTHREADED>::evaluate(const double *const, const int*const, const double*const, const int*const) const
{	throw std::logic_error("not an floating point expression!");
}
template<bool BTHREADED>
int expression<BTHREADED>::evaluateInt(const double *const, const int*const, const double*const, const int*const) const
{	throw std::logic_error("not an integer expression!");
}
template<bool BTHREADED>
double expression<BTHREADED>::evaluateLLVM(const double *const _p, const int*const _pI, const double *const _pT, const int*const _pTI) const
{	std::unique_lock<MUTEX> sLock(m_sMutex);
	initializeLLVM();
	auto &p = std::any_cast<const std::shared_ptr<const llvmData<BTHREADED> >&>(m_sAttachedData.at(this).at(eLLVMdata));
	sLock.unlock();
	return p->jitFunction(_p, _pI, _pT, _pTI);
}
template<bool BTHREADED>
int expression<BTHREADED>::evaluateIntLLVM(const double *const _p, const int*const _pI, const double *const _pT, const int*const _pTI) const
{	std::unique_lock<MUTEX> sLock(m_sMutex);
	initializeLLVM();
	auto &p = std::any_cast<const std::shared_ptr<const llvmDataInt<BTHREADED> >&>(m_sAttachedData.at(this).at(eLLVMdata));
	sLock.unlock();
	return p->jitFunction(_p, _pI, _pT, _pTI);
}
template<bool BTHREADED>
typename expression<BTHREADED>::ptr expression<BTHREADED>::replace(const ptr2ptr&_r, const factory<BTHREADED>&_rF) const
{	std::vector<std::tuple<ptr, size_t, children, bool> > sStack;
	sStack.push_back({this, std::numeric_limits<std::size_t>::max(), children(), true});
	while (!sStack.empty())
	{	const auto [pThis, iParentPos, sChildren, b] = std::move(sStack.back());
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
				if (!std::equal(
					pThis->m_sChildren.begin(),
					pThis->m_sChildren.end(),
					sChildren.rbegin(),
					sChildren.rend()
				))
					return pThis->recreateFromChildren({sChildren.rbegin(), sChildren.rend()}, _rF);
				else
					return pThis;
			else
				if (!std::equal(
					pThis->m_sChildren.begin(),
					pThis->m_sChildren.end(),
					sChildren.rbegin(),
					sChildren.rend()
				))
					std::get<2>(sStack.at(iParentPos)).push_back(
						pThis->recreateFromChildren({sChildren.rbegin(), sChildren.rend()}, _rF)
					);
				else
					std::get<2>(sStack.at(iParentPos)).push_back(
						pThis
					);
	}
	std::abort();
	return nullptr;
}
template<bool BTHREADED>
std::size_t expression<BTHREADED>::getWeightW(void) const
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
template<bool BTHREADED>
llvm::Value *expression<BTHREADED>::generateCodeW(
	const expression<BTHREADED> *const _pRoot,
	llvm::LLVMContext& context,
	llvm::IRBuilder<>& builder,
	llvm::Module *const M,
	llvm::Value*const _pP,
	llvm::Value*const _pIP,
	llvm::Value*const _pT,
	llvm::Value*const _pIT
) const
{	std::unique_lock<MUTEX> sLock(m_sMutex);
	const auto sInsert = m_sAttachedData.emplace(_pRoot, ARRAY());
	if (sInsert.second)
		_pRoot->addOnDestroy(
			[_pRoot, this](void)
			{	std::unique_lock<MUTEX> sLock(m_sMutex);
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
			_pP,
			_pIP,
			_pT,
			_pIT
		);
	return std::any_cast<llvm::Value*>(rAny);
}
}
