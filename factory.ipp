#include "factory.h"
#include "expression.h"
#include "unique.h"
#include "onDestroy.h"
#define __PARSER__ parser_mt
#define __BTHREADED__ true
#include "parser.h"
#undef __PARSER__
#undef __BTHREADED__
#define __PARSER__ parser_st
#define __BTHREADED__ false
#include "parser.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <stdexcept>
#include <regex>
#include <iostream>
#include <cmath>
#include <thread>
#include <functional>
#include <mutex>
#include <algorithm>
#include <execution>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>

namespace theExpressionEngine
{
template<bool BTHREADED>
struct realConstant:expression<BTHREADED>
{	const double m_d;
	realConstant(const double _d)
		:m_d(_d)
	{
	}
	virtual bool isSmaller(const expression<BTHREADED>&_r) const override
	{	const auto &r = dynamic_cast<const realConstant&>(_r);
		if (m_d < r.m_d)
			return true;
		else
			return false;
	}
	virtual double evaluate(const double *const, const int*const, const double*const, const int*const) const override
	{	return m_d;
	}
	virtual llvm::Value* generateCode(
		const expression<BTHREADED>*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const,
		llvm::Value*const,
		llvm::Value*const,
		llvm::Value*const
	) const override
	{	return llvm::ConstantFP::get(context, llvm::APFloat(m_d));
	}
	virtual typename expression<BTHREADED>::ptr recreateFromChildren(typename expression<BTHREADED>::children, const factory<BTHREADED>&) const override
	{	return this;
	}
	virtual std::size_t getWeight(void) const override
	{	return 1;
	}
	virtual std::ostream &print(std::ostream&_r) const override
	{	return _r << m_d;
	}
};
template<bool BTHREADED>
typename expression<BTHREADED>::ptr expression<BTHREADED>::collapse(const factory<BTHREADED>&_rF) const
{	if (!this->getPtr(dummy<realConstant<BTHREADED> >()) && !m_sChildren.empty() && std::all_of(
		m_sChildren.begin(),
		m_sChildren.end(),
		[](const ptr&_p)
		{	return _p->getPtr(dummy<realConstant<BTHREADED> >()) != nullptr;
		}
	))
		return _rF.realConstant(evaluate(nullptr, nullptr, nullptr, nullptr));
	else
		return this;
}
template<bool BTHREADED, const char ac[], double(*PFCT)(double, double), typename factory<BTHREADED>::exprPtr (factory<BTHREADED>::*CREATE)(const typename factory<BTHREADED>::exprPtr&, const typename factory<BTHREADED>::exprPtr&) const>
struct binary:expression<BTHREADED>
{	binary(const typename expression<BTHREADED>::ptr&_p0, const typename expression<BTHREADED>::ptr&_p1)
		:expression<BTHREADED>(
			typename expression<BTHREADED>::children({_p0, _p1})
		)
	{
	}
	virtual std::ostream &print(std::ostream&_r) const override
	{	_r << ac << "(";
		this->m_sChildren[0]->print(_r) << ",";
		return this->m_sChildren[1]->print(_r) << ")";
	}
	virtual double evaluate(const double *const _p, const int*const _pI, const double*const _pT, const int*const _pIT) const override
	{	return PFCT(this->m_sChildren[0]->evaluate(_p, _pI, _pT, _pIT), this->m_sChildren[1]->evaluate(_p, _pI, _pT, _pIT));
	}
	virtual llvm::Value* generateCode(
		const expression<BTHREADED>*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP,
		llvm::Value*const _pIP,
		llvm::Value*const _pT,
		llvm::Value*const _pIT
	) const override
	{		// Create the function prototype for std::sqrt
		using namespace llvm;
		//Value* const Input = ConstantFP::get(llvm::Type::getDoubleTy(context), m_sChildren.at(0)->generateCodeW(context, builder, M));
		// Call the tan function
		return builder.CreateCall(
			M->getOrInsertFunction(
				ac,
				FunctionType::get(
					Type::getDoubleTy(context), // Return type: double
					{	Type::getDoubleTy(context),
						Type::getDoubleTy(context)
					}, // Argument type: double
					false // Not variadic
				)
			),
			{	this->m_sChildren[0]->generateCodeW(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
				this->m_sChildren[1]->generateCodeW(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT)
			}
		);
	}
	virtual typename expression<BTHREADED>::ptr recreateFromChildren(typename expression<BTHREADED>::children _s, const factory<BTHREADED>&_rF) const override
	{	return (_rF.*CREATE)(_s[0], _s[1]);
	}
	virtual std::size_t getWeight(void) const override
	{	return 10;
	}
};
template<bool BTHREADED>
struct max:expression<BTHREADED>
{	max(const typename expression<BTHREADED>::ptr&_p0, const typename expression<BTHREADED>::ptr&_p1)
		:expression<BTHREADED>(
			typename expression<BTHREADED>::children({_p0, _p1})
		)
	{
	}
	virtual std::ostream &print(std::ostream&_r) const override
	{	_r << "max" << "(";
		this->m_sChildren[0]->print(_r) << ",";
		return this->m_sChildren[1]->print(_r) << ")";
	}
	virtual double evaluate(const double *const _p, const int *const _pI, const double*const _pT, const int *const _pIT) const override
	{	return std::max(this->m_sChildren[0]->evaluate(_p, _pI, _pT, _pIT), this->m_sChildren[1]->evaluate(_p, _pI, _pT, _pIT));
	}
	virtual llvm::Value* generateCode(
		const expression<BTHREADED>*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP,
		llvm::Value*const _pIP,
		llvm::Value*const _pT,
		llvm::Value*const _pIT
	) const override
	{	// Create the function prototype for std::sqrt
		using namespace llvm;
			// Call the tan function
		Type* const doubleType = Type::getDoubleTy(context);
		return builder.CreateCall(
			Intrinsic::getOrInsertDeclaration(M, Intrinsic::maxnum, {doubleType}),
			{	this->m_sChildren[0]->generateCodeW(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
				this->m_sChildren[1]->generateCodeW(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT)
			},
			"maxnum"
		);
	}
	virtual typename expression<BTHREADED>::ptr recreateFromChildren(typename expression<BTHREADED>::children _s, const factory<BTHREADED>&_rF) const override
	{	return _rF.max(_s[0], _s[1]);
	}
	virtual std::size_t getWeight(void) const override
	{	return 2;
	}
};
template<bool BTHREADED>
struct min:expression<BTHREADED>
{	min(const typename expression<BTHREADED>::ptr&_p0, const typename expression<BTHREADED>::ptr&_p1)
		:expression<BTHREADED>(
			typename expression<BTHREADED>::children({_p0, _p1})
		)
	{
	}
	virtual std::ostream &print(std::ostream&_r) const override
	{	_r << "min" << "(";
		this->m_sChildren[0]->print(_r) << ",";
		return this->m_sChildren[1]->print(_r) << ")";
	}
	virtual double evaluate(const double *const _p, const int *const _pI, const double*const _pT, const int *const _pIT) const override
	{	return std::min(this->m_sChildren[0]->evaluate(_p, _pI, _pT, _pIT), this->m_sChildren[1]->evaluate(_p, _pI, _pT, _pIT));
	}
	virtual llvm::Value* generateCode(
		const expression<BTHREADED>*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP,
		llvm::Value*const _pIP,
		llvm::Value*const _pT,
		llvm::Value*const _pIT
	) const override
	{	// Create the function prototype for std::sqrt
		using namespace llvm;
		//Value* const Input = ConstantFP::get(llvm::Type::getDoubleTy(context), m_sChildren.at(0)->generateCodeW(context, builder, M));
		// Call the tan function
		Type* const doubleType = Type::getDoubleTy(context);
		return builder.CreateCall(
			Intrinsic::getOrInsertDeclaration(M, Intrinsic::minnum, {doubleType}),
			{	this->m_sChildren[0]->generateCodeW(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
				this->m_sChildren[1]->generateCodeW(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
			},
			"minnum"
		);
	}
	virtual typename expression<BTHREADED>::ptr recreateFromChildren(typename expression<BTHREADED>::children _s, const factory<BTHREADED>&_rF) const override
	{	return _rF.min(_s[0], _s[1]);
	}
	virtual std::size_t getWeight(void) const override
	{	return 2;
	}
};
template<bool BTHREADED>
struct multiplication:expression<BTHREADED>
{	multiplication(const typename expression<BTHREADED>::ptr&_p0, const typename expression<BTHREADED>::ptr&_p1)
		:expression<BTHREADED>(
			typename expression<BTHREADED>::children({_p0, _p1})
		)
	{
	}
	virtual std::ostream &print(std::ostream&_r) const override
	{	_r << "(";
		this->m_sChildren[0]->print(_r) << "*";
		return this->m_sChildren[1]->print(_r) << ")";
	}
	virtual double evaluate(const double *const _p, const int *const _pI, const double*const _pT, const int*const _pIT) const override
	{	return this->m_sChildren[0]->evaluate(_p, _pI, _pT, _pIT) * this->m_sChildren[1]->evaluate(_p, _pI, _pT, _pIT);
	}
	virtual llvm::Value* generateCode(
		const expression<BTHREADED>*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP,
		llvm::Value*const _pIP,
		llvm::Value*const _pT,
		llvm::Value*const _pIT
	) const override
	{	return builder.CreateFMul(
			this->m_sChildren[0]->generateCodeW(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
			this->m_sChildren[1]->generateCodeW(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
			"plus"
		);
	}
	virtual typename expression<BTHREADED>::ptr recreateFromChildren(typename expression<BTHREADED>::children _s, const factory<BTHREADED>&_rF) const override
	{	return _rF.multiplication(_s[0], _s[1]);
	}
	virtual std::size_t getWeight(void) const override
	{	return 4;
	}
};
template<bool BTHREADED>
struct division:expression<BTHREADED>
{	division(const typename expression<BTHREADED>::ptr&_p0, const typename expression<BTHREADED>::ptr&_p1)
		:expression<BTHREADED>(
			typename expression<BTHREADED>::children({_p0, _p1})
		)
	{
	}
	virtual std::ostream &print(std::ostream&_r) const override
	{	_r << "(";
		this->m_sChildren[0]->print(_r) << "/";
		return this->m_sChildren[1]->print(_r) << ")";
	}
	virtual double evaluate(const double *const _p, const int *const _pI, const double*const _pT, const int*const _pIT) const override
	{	return this->m_sChildren[0]->evaluate(_p, _pI, _pT, _pIT) / this->m_sChildren[1]->evaluate(_p, _pI, _pT, _pIT);
	}
	virtual llvm::Value* generateCode(
		const expression<BTHREADED>*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP,
		llvm::Value*const _pIP,
		llvm::Value*const _pT,
		llvm::Value*const _pIT
	) const override
	{	return builder.CreateFDiv(
			this->m_sChildren[0]->generateCodeW(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
			this->m_sChildren[1]->generateCodeW(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
			"plus"
		);
	}
	virtual typename expression<BTHREADED>::ptr recreateFromChildren(typename expression<BTHREADED>::children _s, const factory<BTHREADED>&_rF) const override
	{	return _rF.division(_s[0], _s[1]);
	}
	virtual std::size_t getWeight(void) const override
	{	return 10;
	}
};
template<bool BTHREADED>
struct addition:expression<BTHREADED>
{	addition(const typename expression<BTHREADED>::ptr&_p0, const typename expression<BTHREADED>::ptr&_p1)
		:expression<BTHREADED>(
			typename expression<BTHREADED>::children({_p0, _p1})
		)
	{
	}
	virtual std::ostream &print(std::ostream&_r) const override
	{	_r << "(";
		this->m_sChildren[0]->print(_r) << "+";
		return this->m_sChildren[1]->print(_r) << ")";
	}
	virtual double evaluate(const double *const _p, const int *const _pI, const double*const _pT, const int*const _pIT) const override
	{	return this->m_sChildren[0]->evaluate(_p, _pI, _pT, _pIT) + this->m_sChildren[1]->evaluate(_p, _pI, _pT, _pIT);
	}
	virtual llvm::Value* generateCode(
		const expression<BTHREADED>*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP,
		llvm::Value*const _pIP,
		llvm::Value*const _pT,
		llvm::Value*const _pIT
	) const override
	{	return builder.CreateFAdd(
			this->m_sChildren[0]->generateCodeW(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
			this->m_sChildren[1]->generateCodeW(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
			"plus"
		);
	}
	virtual typename expression<BTHREADED>::ptr recreateFromChildren(typename expression<BTHREADED>::children _s, const factory<BTHREADED>&_rF) const override
	{	return _rF.addition(_s[0], _s[1]);
	}
	virtual std::size_t getWeight(void) const override
	{	return 2;
	}
};
template<bool BTHREADED>
struct subtraction:expression<BTHREADED>
{	subtraction(const typename expression<BTHREADED>::ptr&_p0, const typename expression<BTHREADED>::ptr&_p1)
		:expression<BTHREADED>(
			typename expression<BTHREADED>::children({_p0, _p1})
		)
	{
	}
	virtual std::ostream &print(std::ostream&_r) const override
	{	_r << "(";
		this->m_sChildren[0]->print(_r) << "-";
		return this->m_sChildren[1]->print(_r) << ")";
	}
	virtual double evaluate(const double *const _p, const int *const _pI, const double*const _pT, const int*const _pIT) const override
	{	return this->m_sChildren[0]->evaluate(_p, _pI, _pT, _pIT) - this->m_sChildren[1]->evaluate(_p, _pI, _pT, _pIT);
	}
	virtual llvm::Value* generateCode(
		const expression<BTHREADED>*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP,
		llvm::Value*const _pIP,
		llvm::Value*const _pT,
		llvm::Value*const _pIT
	) const override
	{	return builder.CreateFSub(
			this->m_sChildren[0]->generateCodeW(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
			this->m_sChildren[1]->generateCodeW(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
			"plus"
		);
	}
	virtual typename expression<BTHREADED>::ptr recreateFromChildren(typename expression<BTHREADED>::children _s, const factory<BTHREADED>&_rF) const override
	{	return _rF.subtraction(_s[0], _s[1]);
	}
	virtual std::size_t getWeight(void) const override
	{	return 2;
	}
};
template<bool BTHREADED, double(*PMATH)(double), llvm::Intrinsic::ID EID, const char AC[], typename factory<BTHREADED>::exprPtr (factory<BTHREADED>::*CREATE)(const typename factory<BTHREADED>::exprPtr&) const>
struct unary:expression<BTHREADED>
{	unary(const typename expression<BTHREADED>::ptr&_p)
		:expression<BTHREADED>(
			typename expression<BTHREADED>::children({_p})
		)
	{
	}
	virtual std::ostream &print(std::ostream&_r) const override
	{	_r << AC << "(";
		return this->m_sChildren[0]->print(_r) << ")";
	}
	virtual double evaluate(const double *const _p, const int *const _pI, const double*const _pT, const int*const _pIT) const override
	{	return PMATH(this->m_sChildren[0]->evaluate(_p, _pI, _pT, _pIT));
	}
	virtual llvm::Value* generateCode(
		const expression<BTHREADED>*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP,
		llvm::Value*const _pIP,
		llvm::Value*const _pT,
		llvm::Value*const _pIT
	) const override
	{	// Create the function prototype for std::sqrt
		using namespace llvm;
		return builder.CreateCall(
			Intrinsic::getOrInsertDeclaration(M, EID, {Type::getDoubleTy(context)}),
			this->m_sChildren[0]->generateCodeW(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT)
		);
	}
	virtual typename expression<BTHREADED>::ptr recreateFromChildren(typename expression<BTHREADED>::children _s, const factory<BTHREADED>&_rF) const override
	{	return (_rF.*CREATE)(_s[0]);
	}
	virtual std::size_t getWeight(void) const override
	{	return 10;
	}
};
template<bool BTHREADED, double(*PMATH)(double), const char AC[], typename factory<BTHREADED>::exprPtr (factory<BTHREADED>::*CREATE)(const typename factory<BTHREADED>::exprPtr&) const>
struct unaryF:expression<BTHREADED>
{	unaryF(const typename expression<BTHREADED>::ptr&_p)
		:expression<BTHREADED>(
			typename expression<BTHREADED>::children({_p})
		)
	{
	}
	virtual std::ostream &print(std::ostream&_r) const override
	{	_r << AC << "(";
		return this->m_sChildren[0]->print(_r) << ")";
	}
	virtual double evaluate(const double *const _p, const int *const _pI, const double*const _pT, const int*const _pIT) const override
	{	return PMATH(this->m_sChildren[0]->evaluate(_p, _pI, _pT, _pIT));
	}
	virtual llvm::Value* generateCode(
		const expression<BTHREADED>*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP,
		llvm::Value*const _pIP,
		llvm::Value*const _pT,
		llvm::Value*const _pIT
	) const override
	{		// Create the function prototype for std::sqrt
		using namespace llvm;
		//Value* const Input = ConstantFP::get(llvm::Type::getDoubleTy(context), this->m_sChildren.at(0)->generateCodeW(context, builder, M));
		// Call the tan function
		return builder.CreateCall(
			M->getOrInsertFunction(
				AC,
				FunctionType::get(
					llvm::Type::getDoubleTy(context), // Return type: double
					{	//llvm::Type::getDoubleTy(context),
						llvm::Type::getDoubleTy(context)
					}, // Argument type: double
					false // Not variadic
				)
			),
			{	this->m_sChildren[0]->generateCodeW(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT)
			}
		);
	}
	virtual typename expression<BTHREADED>::ptr recreateFromChildren(typename expression<BTHREADED>::children _s, const factory<BTHREADED>&_rF) const override
	{	return (_rF.*CREATE)(_s[0]);
	}
	virtual std::size_t getWeight(void) const override
	{	return 10;
	}
};
template<bool BTHREADED>
struct negation:expression<BTHREADED>
{	negation(const typename expression<BTHREADED>::ptr&_p)
		:expression<BTHREADED>(
			typename expression<BTHREADED>::children({_p})
		)
	{
	}
	virtual std::ostream &print(std::ostream&_r) const override
	{	_r << "-(";
		return this->m_sChildren[0]->print(_r) << ")";
	}
	virtual double evaluate(const double *const _p, const int *const _pI, const double*const _pT, const int*const _pIT) const override
	{	return -this->m_sChildren[0]->evaluate(_p, _pI, _pT, _pIT);
	}
	virtual llvm::Value* generateCode(
		const expression<BTHREADED>*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP,
		llvm::Value*const _pIP,
		llvm::Value*const _pT,
		llvm::Value*const _pIT
	) const override
	{	// Create the function prototype for std::sqrt
		using namespace llvm;
		llvm::Type* doubleType = Type::getDoubleTy(M->getContext());
		return builder.CreateFSub(
			ConstantFP::get(doubleType, 0.0),
			this->m_sChildren[0]->generateCodeW(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT)
		);
	}
	virtual typename expression<BTHREADED>::ptr recreateFromChildren(typename expression<BTHREADED>::children _s, const factory<BTHREADED>&_rF) const override
	{	return _rF.negation(_s[0]);
	}
	virtual std::size_t getWeight(void) const override
	{	return 1;
	}
};
template<bool BTHREADED>
struct variable:expression<BTHREADED>
{	const std::size_t m_i;
	variable(const std::size_t _i)
		:
		m_i(_i)
	{
	}
	virtual std::ostream &print(std::ostream&_r) const override
	{	return _r << "var" << m_i;
	}
	virtual double evaluate(const double *const _p, const int *const _pI, const double*const _pT, const int*const _pIT) const override
	{	return _pT[m_i];
	}
	virtual bool isSmaller(const expression<BTHREADED>&_r) const override
	{	const auto &r = dynamic_cast<const variable&>(_r);
		if (m_i < r.m_i)
			return true;
		else
			return false;
	}
	virtual llvm::Value* generateCode(
		const expression<BTHREADED>*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const,
		llvm::Value*const,
		llvm::Value*const _pP,
		llvm::Value*const
	) const override
	{
		using namespace llvm;
		// Step 1: Retrieve the contained integer (index)
		// Assume `index` is a member of the `parameter` object
		llvm::Value* indexVal = llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), m_i);

		// Step 2: Perform bounds checking or validation if required (optional)

		// Step 3: Use GetElementPtr (GEP) to calculate the address at the given index
		llvm::Value* indexedPointer = builder.CreateGEP(
			llvm::Type::getDoubleTy(context), // The type of elements (double)
			_pP,	// The pointer to be indexed
			indexVal	// The index (as a Value*)
		);

		// Step 4: Return the indexed pointer
		return builder.CreateLoad(
			Type::getDoubleTy(context), // The type to load
			indexedPointer	// The pointer to load from
		);
	}
	virtual typename expression<BTHREADED>::ptr recreateFromChildren(typename expression<BTHREADED>::children, const factory<BTHREADED>&) const override
	{	return this;
	}
	virtual std::size_t getWeight(void) const override
	{	return 1;
	}
};
template<bool BTHREADED>
struct parameter:expression<BTHREADED>
{	const std::size_t m_i;
	parameter(const std::size_t _i)
		:
		m_i(_i)
	{
	}
	virtual std::ostream &print(std::ostream&_r) const override
	{	return _r << "x" << m_i;
	}
	virtual double evaluate(const double *const _p, const int *const _pI, const double*const _pT, const int*const _pIT) const override
	{	return _p[m_i];
	}
	virtual bool isSmaller(const expression<BTHREADED>&_r) const override
	{	const auto &r = dynamic_cast<const parameter&>(_r);
		if (m_i < r.m_i)
			return true;
		else
			return false;
	}
	virtual llvm::Value* generateCode(
		const expression<BTHREADED>*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP,
		llvm::Value*const,
		llvm::Value*const,
		llvm::Value*const
	) const override
	{
		using namespace llvm;
		// Step 1: Retrieve the contained integer (index)
		// Assume `index` is a member of the `parameter` object
		llvm::Value* indexVal = llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), m_i);

		// Step 2: Perform bounds checking or validation if required (optional)

		// Step 3: Use GetElementPtr (GEP) to calculate the address at the given index
		llvm::Value* indexedPointer = builder.CreateGEP(
			llvm::Type::getDoubleTy(context), // The type of elements (double)
			_pP,	// The pointer to be indexed
			indexVal	// The index (as a Value*)
		);

		// Step 4: Return the indexed pointer
		return builder.CreateLoad(
			Type::getDoubleTy(context), // The type to load
			indexedPointer	// The pointer to load from
		);
	}
	virtual typename expression<BTHREADED>::ptr recreateFromChildren(typename expression<BTHREADED>::children, const factory<BTHREADED>&) const override
	{	return this;
	}
	virtual std::size_t getWeight(void) const override
	{	return 1;
	}
};
template<bool BTHREADED>
struct expressionSetImpl:expressionSet<BTHREADED>
{
	typedef typename expression<BTHREADED>::children children;
	typedef std::tuple<
		children,
		std::vector<std::vector<std::size_t> >,
		std::vector<std::size_t>
	> TUPLE;
	const TUPLE m_sChildren;
	virtual bool operator<(const expressionSet<BTHREADED> &_r) const override
	{	return m_sChildren < dynamic_cast<const expressionSetImpl&>(_r).m_sChildren;
	}
	virtual const std::vector<std::size_t> &getOrder(void) const override
	{	return std::get<2>(m_sChildren);
	}
	static auto get(
		const children& _rChildren,
		const factory<BTHREADED>&_rF
	)
	{	typedef std::set<const expression<BTHREADED>*> ESET;
			/// expressions used more than once
			/// all expressions
		const auto sSetRepeated = [&](void)
		{	ESET sSetAll, sSetRepeated;
			for (const auto &p : _rChildren)
				p->DFS(
					[&](const expression<BTHREADED>*const _p)
					{	if (_p->m_sChildren.empty())
							return false;
						if (sSetAll.emplace(_p).second)
							return true;
						sSetRepeated.emplace(_p);
						return false;
					}
				);
			return sSetRepeated;
		}();
		const auto sExpr2Deps = [&](void)
		{	std::map<
				const expression<BTHREADED>*,
				ESET
			> sExpr2Deps;
			auto sLeft = sSetRepeated;
			for (const auto &p : _rChildren)
				sLeft.insert(p.get());
			for (const auto p : sLeft)
			{	auto &rDeps = sExpr2Deps[p];
				ESET sSet;
				p->DFS(
					[&](const expression<BTHREADED>*const _p)
					{	if (_p->m_sChildren.empty())
							return false;
						if (!sSet.emplace(_p).second)
							return false;
						if (sSetRepeated.count(_p) && _p != p)
							rDeps.emplace(_p);
						return true;
					}
				);
			}
			return sExpr2Deps;
		}();
		const auto sG2Set = [&](void)
		{	ESET sDep;
			auto sLeft = sSetRepeated;
			for (const auto &p : _rChildren)
				sLeft.insert(p.get());
			std::map<ESET, ESET> sG2Set;
			while (!sLeft.empty())
			{	auto &r = sG2Set[sDep];
				for (const auto p : sLeft)
				{	auto &rC = sExpr2Deps.at(p);
					if (rC.size() <= sDep.size() && std::includes(
						sDep.begin(),
						sDep.end(),
						rC.begin(),
						rC.end()
					))
						r.insert(p);
				}
				for (const auto p : r)
				{	sLeft.erase(p);
					sDep.insert(p);
				}
			}
			return sG2Set;
		}();
		std::map<const expression<BTHREADED>*, std::size_t> sExpr2Id;
		children sChildren;
		ESET sSet;
		std::vector<std::vector<std::size_t> > sGroups;
		auto sLeft = sSetRepeated;
		for (const auto &p : _rChildren)
			sLeft.insert(p.get());
		while (!sLeft.empty())
		{	auto &r = sG2Set.at(sSet);
			sGroups.emplace_back();
			sGroups.back().reserve(r.size());
			for (const auto p : r)
			{	const auto sInsert = sExpr2Id.emplace(p, sExpr2Id.size());
				sLeft.erase(p);
				sSet.insert(p);
				sGroups.back().push_back(sInsert.first->second);
				sChildren.push_back(p);
			}
		}
		sChildren.shrink_to_fit();
		sGroups.shrink_to_fit();
		std::vector<std::size_t> sOrder(_rChildren.size());
		for (std::size_t i = 0; i < _rChildren.size(); ++i)
			sOrder[i] = sExpr2Id.at(_rChildren[i].get());
		typename expression<BTHREADED>::ptr2ptr sMap;
		for (const auto p : sSetRepeated)
			sMap[p] = _rF.variable(sExpr2Id.at(p));
		for (auto &p : sChildren)
			if (sSetRepeated.count(p.get()))
			{	auto pF = sMap.find(p);
				auto pR = pF->second;
				sMap.erase(pF);
				p = p->replace(sMap, _rF);
				sMap[p] = pR;
			}
			else
				p = p->replace(sMap, _rF);
		return TUPLE(std::move(sChildren), std::move(sGroups), std::move(sOrder));
	}
	expressionSetImpl(
		const children& _rChildren,
		const factory<BTHREADED>&_rF
	)
		:m_sChildren(get(_rChildren, _rF))
	{
	}
	template<double(expression<BTHREADED>::*EVALUATE)(const double *const, const int*const, const double*const, const int*const) const>
	void run_evaluate(
		std::vector<double>&_rChildren,
		std::vector<int>&_rIChildren,
		const double *const _pParams,
		const int *const _pIParams//,
		//typename expressionSet<BTHREADED>::atomicVec &_rC,
		//boost::asio::thread_pool &_rPool
	) const
	{	const auto &[rChildren, rG2Set, rOrder] = m_sChildren;
		_rChildren.resize(rChildren.size());
		if constexpr (!BTHREADED)
			std::transform(
				rChildren.begin(),
				rChildren.end(),
				_rChildren.begin(),
				[&](const typename expression<BTHREADED>::ptr&_p)
				{	return ((*_p).*EVALUATE)(_pParams, _pIParams, _rChildren.data(), _rIChildren.data());
				}
			);
		else
			for (const auto &r : rG2Set)
				std::for_each(
					std::execution::par,
					r.begin(),
					r.end(),
					[&](const std::size_t _i)
					{	_rChildren[_i] = ((*rChildren[_i]).*EVALUATE)(_pParams, _pIParams, _rChildren.data(), _rIChildren.data());
					}
				);
	}
	virtual void evaluate(
		std::vector<double>&_rChildren,
		std::vector<int>&_rIChildren,
		const double *const _pParams,
		const int *const _pIParams//,
		//typename expressionSet<BTHREADED>::atomicVec &_rC,
		//boost::asio::thread_pool &_rPool
	) const override
	{	run_evaluate<&expression<BTHREADED>::evaluate>(_rChildren, _rIChildren, _pParams, _pIParams);
	}
	virtual void evaluateLLVM(
		std::vector<double>&_rChildren,
		std::vector<int>&_rIChildren,
		const double *const _pParams,
		const int *const _pIParams
		//typename expressionSet<BTHREADED>::atomicVec &_rC,
		//boost::asio::thread_pool &_rPool
	) const override
	{	run_evaluate<&expression<BTHREADED>::evaluateLLVM>(_rChildren, _rIChildren, _pParams, _pIParams);
	}
	virtual const typename expression<BTHREADED>::children &getChildren(void) const override
	{	return std::get<0>(m_sChildren);
	}
	virtual void onDestroy(void) const
	{
	}
};
template<bool BTHREADED>
struct factoryImpl:factory<BTHREADED>
{	using typename factory<BTHREADED>::exprPtr;
	virtual exprPtr realConstant(const double _d) const override
	{	return theExpressionEngine::expression<BTHREADED>::template create<dynamic_cast_implementation<theExpressionEngine::realConstant<BTHREADED> > >(*this, _d);
	}
	virtual exprPtr parameter(const std::size_t _i) const override
	{	return theExpressionEngine::expression<BTHREADED>::template create<theExpressionEngine::parameter<BTHREADED> >(*this, _i);
	}
	virtual exprPtr variable(const std::size_t _i) const override
	{	return theExpressionEngine::expression<BTHREADED>::template create<theExpressionEngine::variable<BTHREADED> >(*this, _i);
	}
#define __COMMA__
#define __COMMA2__
#define __MAKE_ENTRY2__(a)\
	static constexpr const char s_ac_##a[] = #a;\
	virtual exprPtr a(const exprPtr&_p) const override\
	{	return theExpressionEngine::expression<BTHREADED>::template create<theExpressionEngine::unaryF<BTHREADED, std::a, s_ac_##a, &factory<BTHREADED>::a> >(*this, _p);\
	}
#define __MAKE_ENTRY__(a)\
	static constexpr const char s_ac_##a[] = #a;\
	virtual exprPtr a(const exprPtr&_p) const override\
	{	return theExpressionEngine::expression<BTHREADED>::template create<theExpressionEngine::unary<BTHREADED, std::a, llvm::Intrinsic::a, s_ac_##a, &factory<BTHREADED>::a> >(*this, _p);\
	}
#include "unary.h"
#define __MAKE_ENTRY__(pow)\
	virtual exprPtr pow(const exprPtr&_p0, const exprPtr&_p1) const override\
	{	static const char ac[] = #pow;\
		return theExpressionEngine::expression<BTHREADED>::template create<theExpressionEngine::binary<BTHREADED, ac, &std::pow, &factory<BTHREADED>::pow> >(*this, _p0, _p1);\
	}
#include "binary.h"
	virtual exprPtr max(const exprPtr&_p0, const exprPtr&_p1) const override
	{	return theExpressionEngine::expression<BTHREADED>::template create<theExpressionEngine::max<BTHREADED> >(*this, _p0, _p1);
	}
	virtual exprPtr min(const exprPtr&_p0, const exprPtr&_p1) const override
	{	return theExpressionEngine::expression<BTHREADED>::template create<theExpressionEngine::min<BTHREADED> >(*this, _p0, _p1);
	}
	virtual exprPtr addition(const exprPtr&_p0, const exprPtr&_p1) const override
	{	if (const auto p = _p0->getPtr(dummy<theExpressionEngine::realConstant<BTHREADED> >()); p && p->m_d == 0)
			return _p1;
		else
		if (const auto p = _p1->getPtr(dummy<theExpressionEngine::realConstant<BTHREADED> >()); p && p->m_d == 0)
			return _p0;
		else
			return theExpressionEngine::expression<BTHREADED>::template create<theExpressionEngine::addition<BTHREADED> >(*this, _p0, _p1);
	}
	virtual exprPtr subtraction(const exprPtr&_p0, const exprPtr&_p1) const override
	{	if (const auto p = _p0->getPtr(dummy<theExpressionEngine::realConstant<BTHREADED> >()); p && p->m_d == 0)
			return negation(_p1);
		else
		if (const auto p = _p1->getPtr(dummy<theExpressionEngine::realConstant<BTHREADED> >()); p && p->m_d == 0)
			return _p0;
		else
		return theExpressionEngine::expression<BTHREADED>::template create<theExpressionEngine::subtraction<BTHREADED> >(*this, _p0, _p1);
	}
	virtual exprPtr multiplication(const exprPtr&_p0, const exprPtr&_p1) const override
	{	if (const auto p = _p0->getPtr(dummy<theExpressionEngine::realConstant<BTHREADED> >()))
			if (p->m_d == 1)
				return _p1;
			else
			if (p->m_d == 0)
				return _p0;
		if (const auto p = _p1->getPtr(dummy<theExpressionEngine::realConstant<BTHREADED> >()))
			if (p->m_d == 1)
				return _p0;
			else
			if (p->m_d == 0)
				return _p1;
		return theExpressionEngine::expression<BTHREADED>::template create<theExpressionEngine::multiplication<BTHREADED> >(*this, _p0, _p1);
	}
	virtual exprPtr division(const exprPtr&_p0, const exprPtr&_p1) const override
	{	if (const auto p = _p0->getPtr(dummy<theExpressionEngine::realConstant<BTHREADED> >()); p && p->m_d == 0)
			return _p0;
		else
		if (const auto p = _p1->getPtr(dummy<theExpressionEngine::realConstant<BTHREADED> >()); p && p->m_d == 1)
			return _p0;
		else
		if (_p0 == _p1)
			return realConstant(1.0);
		else
			return theExpressionEngine::expression<BTHREADED>::template create<theExpressionEngine::division<BTHREADED> >(*this, _p0, _p1);
	}
	virtual exprPtr negation(const exprPtr&_r) const override
	{	return theExpressionEngine::expression<BTHREADED>::template create<theExpressionEngine::negation<BTHREADED> >(*this, _r);
	}
	virtual exprPtr parse(const char *const _r, const typename factory<BTHREADED>::name2int&_rP) const override;
	virtual boost::intrusive_ptr<const expressionSet<BTHREADED> > createExpressionSet(const std::vector<exprPtr>&_r) const override
	{	return expressionSet<BTHREADED>::template create<expressionSetImpl<BTHREADED> >(*this, _r, *this);
	}
};
template<>
typename factoryImpl<true>::exprPtr factoryImpl<true>::parse(const char *const _r, const typename factoryImpl<true>::name2int&_rP) const
{	exprPtr pRet;
	auto sParam = std::tie(_rP, static_cast<const factory<true>&>(*this));
	auto const sParser = parser_st::bp::with_globals(parser_mt::expr, sParam);

	if (parser_mt::bp::parse(
		std::string(_r),
		sParser,
		parser_mt::bp::ws,
		pRet
	))
		return pRet;
	else
		throw std::runtime_error(_r);
}
template<>
typename factoryImpl<false>::exprPtr factoryImpl<false>::parse(const char *const _r, const typename factoryImpl<false>::name2int&_rP) const
{	exprPtr pRet;
	auto sParam = std::tie(_rP, static_cast<const factory<false>&>(*this));
	auto const sParser = parser_st::bp::with_globals(parser_st::expr, sParam);

	if (parser_st::bp::parse(
		std::string(_r),
		sParser,
		parser_st::bp::ws,
		pRet
	))
		return pRet;
	else
		throw std::runtime_error(_r);
}
template<bool BTHREADED>
const typename factory<BTHREADED>::ptr &factory<BTHREADED>::getFactory(void)
{	static const typename factory<BTHREADED>::ptr s(std::make_shared<const factoryImpl<BTHREADED> >());
	return s;
}
namespace operators
{
template<typename T>
std::enable_if_t<
	is_expression_ptr<T>::value,
	T
>
operator+(const T&_r0, const T&_r1)
{	return T::element_type::FACTORY::getFactory()->addition(_r0, _r1);
}
template<typename T>
std::enable_if_t<
	is_expression_ptr<T>::value,
	T
>
operator-(const T&_r0, const T&_r1)
{	return T::element_type::FACTORY::getFactory()->subtraction(_r0, _r1);
}
template<typename T>
std::enable_if_t<
	is_expression_ptr<T>::value,
	T
>
operator*(const T&_r0, const T&_r1)
{	return T::element_type::FACTORY::getFactory()->multiplication(_r0, _r1);
}
template<typename T>
std::enable_if_t<
	is_expression_ptr<T>::value,
	T
>
operator/(const T&_r0, const T&_r1)
{	return T::element_type::FACTORY::getFactory()->division(_r0, _r1);
}
#define __MAKE_ENTRY__(sin)\
template<typename T>\
std::enable_if_t<\
	is_expression_ptr<T>::value,\
	T\
>\
sin(const T&_r)\
{	return T::element_type::FACTORY::getFactory()->sin(_r);\
}
#define __MAKE_ENTRY2__(sin) __MAKE_ENTRY__(sin)
#define __COMMA__
#define __COMMA2__
#include "unary.h"
#define __MAKE_ENTRY__(pow)\
template<typename T>\
std::enable_if_t<\
	is_expression_ptr<T>::value,\
	T\
>\
pow(const T&_r0, const T&_r1)\
{	return T::element_type::FACTORY::getFactory()->pow(_r0, _r1);\
}
#include "binary.h"
}
}
