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
		:expression<BTHREADED>({}, expression<BTHREADED>::eFloatingPoint),
		m_d(_d)
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
	virtual bool isZero(void) const override
	{	return m_d == 0.0;
	}
	virtual bool isOne(void) const override
	{	return m_d == 1.0;
	}
	virtual bool isNonZero(void) const override
	{	return m_d != 0.0;
	}
};
template<bool BTHREADED>
struct intConstant:expression<BTHREADED>
{	const int m_i;
	intConstant(const int _i)
		:expression<BTHREADED>({}, expression<BTHREADED>::eInteger),
		m_i(_i)
	{
	}
	virtual bool isSmaller(const expression<BTHREADED>&_r) const override
	{	const auto &r = dynamic_cast<const intConstant&>(_r);
		if (m_i < r.m_i)
			return true;
		else
			return false;
	}
	virtual int evaluateInt(const double *const, const int*const, const double*const, const int*const) const override
	{	return m_i;
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
	{	return llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), m_i, /*isSigned=*/true);
	}
	virtual typename expression<BTHREADED>::ptr recreateFromChildren(typename expression<BTHREADED>::children, const factory<BTHREADED>&) const override
	{	return this;
	}
	virtual std::size_t getWeight(void) const override
	{	return 1;
	}
	virtual std::ostream &print(std::ostream&_r) const override
	{	return _r << m_i;
	}
	virtual bool isZero(void) const override
	{	return m_i == 0;
	}
	virtual bool isOne(void) const override
	{	return m_i == 1;
	}
	virtual bool isNonZero(void) const override
	{	return m_i != 0;
	}
};
template<bool BTHREADED>
typename expression<BTHREADED>::ptr expression<BTHREADED>::collapse(const factory<BTHREADED>&_rF) const
{	if ((!this->getPtr(dummy<realConstant<BTHREADED> >()) && !this->getPtr(dummy<intConstant<BTHREADED> >())) && !m_sChildren.empty() && std::all_of(
		m_sChildren.begin(),
		m_sChildren.end(),
		[](const ptr&_p)
		{	return _p->getPtr(dummy<realConstant<BTHREADED> >()) != nullptr
				|| _p->getPtr(dummy<intConstant<BTHREADED> >()) != nullptr;
		}
	))
		if (m_eType == eFloatingPoint)
			return _rF.realConstant(evaluate(nullptr, nullptr, nullptr, nullptr));
		else
			return _rF.intConstant(evaluateInt(nullptr, nullptr, nullptr, nullptr));
	else
		return this;
}
template<bool BTHREADED, const char ac[], double(*PFCT)(double, double), typename factory<BTHREADED>::exprPtr (factory<BTHREADED>::*CREATE)(const typename factory<BTHREADED>::exprPtr&, const typename factory<BTHREADED>::exprPtr&) const>
struct binary:expression<BTHREADED>
{	binary(const typename expression<BTHREADED>::ptr&_p0, const typename expression<BTHREADED>::ptr&_p1)
		:expression<BTHREADED>(
			typename expression<BTHREADED>::children({_p0, _p1}),
			expression<BTHREADED>::eFloatingPoint
		)
	{
	}
	virtual std::ostream &print(std::ostream&_r) const override
	{	_r << ac << "(";
		this->m_sChildren[0]->print(_r) << ",";
		return this->m_sChildren[1]->print(_r) << ")";
	}
	virtual double evaluate(const double *const _p, const int*const _pI, const double*const _pT, const int*const _pIT) const override
	{	return PFCT(this->m_sChildren[0]->evaluateW(_p, _pI, _pT, _pIT), this->m_sChildren[1]->evaluateW(_p, _pI, _pT, _pIT));
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
			{	this->m_sChildren[0]->generateCodeWF(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
				this->m_sChildren[1]->generateCodeWF(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT)
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
			typename expression<BTHREADED>::children({_p0, _p1}),
			_p0->m_eType == expression<BTHREADED>::eFloatingPoint
				|| _p1->m_eType == expression<BTHREADED>::eFloatingPoint
				? expression<BTHREADED>::eFloatingPoint
				: expression<BTHREADED>::eInteger
		)
	{
	}
	virtual std::ostream &print(std::ostream&_r) const override
	{	_r << "max" << "(";
		this->m_sChildren[0]->print(_r) << ",";
		return this->m_sChildren[1]->print(_r) << ")";
	}
	virtual double evaluate(const double *const _p, const int *const _pI, const double*const _pT, const int *const _pIT) const override
	{	return std::max(this->m_sChildren[0]->evaluateW(_p, _pI, _pT, _pIT), this->m_sChildren[1]->evaluateW(_p, _pI, _pT, _pIT));
	}
	virtual int evaluateInt(const double *const _p, const int *const _pI, const double*const _pT, const int *const _pIT) const override
	{	return std::max(this->m_sChildren[0]->evaluateInt(_p, _pI, _pT, _pIT), this->m_sChildren[1]->evaluateInt(_p, _pI, _pT, _pIT));
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
			{	this->m_sChildren[0]->generateCodeWF(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
				this->m_sChildren[1]->generateCodeWF(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT)
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
			typename expression<BTHREADED>::children({_p0, _p1}),
			_p0->m_eType == expression<BTHREADED>::eFloatingPoint
				|| _p1->m_eType == expression<BTHREADED>::eFloatingPoint
				? expression<BTHREADED>::eFloatingPoint
				: expression<BTHREADED>::eInteger
		)
	{
	}
	virtual std::ostream &print(std::ostream&_r) const override
	{	_r << "min" << "(";
		this->m_sChildren[0]->print(_r) << ",";
		return this->m_sChildren[1]->print(_r) << ")";
	}
	virtual double evaluate(const double *const _p, const int *const _pI, const double*const _pT, const int *const _pIT) const override
	{	return std::min(this->m_sChildren[0]->evaluateW(_p, _pI, _pT, _pIT), this->m_sChildren[1]->evaluateW(_p, _pI, _pT, _pIT));
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
			{	this->m_sChildren[0]->generateCodeWF(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
				this->m_sChildren[1]->generateCodeWF(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
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
	virtual int evaluateInt(const double *const _p, const int *const _pI, const double*const _pT, const int *const _pIT) const override
	{	return std::min(this->m_sChildren[0]->evaluateInt(_p, _pI, _pT, _pIT), this->m_sChildren[1]->evaluateInt(_p, _pI, _pT, _pIT));
	}
};
template<bool BTHREADED>
struct multiplication:expression<BTHREADED>
{	multiplication(const typename expression<BTHREADED>::ptr&_p0, const typename expression<BTHREADED>::ptr&_p1)
		:expression<BTHREADED>(
			typename expression<BTHREADED>::children({_p0, _p1}),
			_p0->m_eType == expression<BTHREADED>::eFloatingPoint
				|| _p1->m_eType == expression<BTHREADED>::eFloatingPoint
				? expression<BTHREADED>::eFloatingPoint
				: expression<BTHREADED>::eInteger
		)
	{
	}
	virtual std::ostream &print(std::ostream&_r) const override
	{	_r << "(";
		this->m_sChildren[0]->print(_r) << "*";
		return this->m_sChildren[1]->print(_r) << ")";
	}
	virtual double evaluate(const double *const _p, const int *const _pI, const double*const _pT, const int*const _pIT) const override
	{	return this->m_sChildren[0]->evaluateW(_p, _pI, _pT, _pIT) * this->m_sChildren[1]->evaluateW(_p, _pI, _pT, _pIT);
	}
	virtual int evaluateInt(const double *const _p, const int *const _pI, const double*const _pT, const int*const _pIT) const override
	{	return this->m_sChildren[0]->evaluateInt(_p, _pI, _pT, _pIT) * this->m_sChildren[1]->evaluateInt(_p, _pI, _pT, _pIT);
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
			this->m_sChildren[0]->generateCodeWF(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
			this->m_sChildren[1]->generateCodeWF(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
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
			typename expression<BTHREADED>::children({_p0, _p1}),
			_p0->m_eType == expression<BTHREADED>::eFloatingPoint
				|| _p1->m_eType == expression<BTHREADED>::eFloatingPoint
				? expression<BTHREADED>::eFloatingPoint
				: expression<BTHREADED>::eInteger
		)
	{
	}
	virtual std::ostream &print(std::ostream&_r) const override
	{	_r << "(";
		this->m_sChildren[0]->print(_r) << "/";
		return this->m_sChildren[1]->print(_r) << ")";
	}
	virtual double evaluate(const double *const _p, const int *const _pI, const double*const _pT, const int*const _pIT) const override
	{	return this->m_sChildren[0]->evaluateW(_p, _pI, _pT, _pIT) / this->m_sChildren[1]->evaluateW(_p, _pI, _pT, _pIT);
	}
	virtual int evaluateInt(const double *const _p, const int *const _pI, const double*const _pT, const int*const _pIT) const override
	{	return this->m_sChildren[0]->evaluateInt(_p, _pI, _pT, _pIT) / this->m_sChildren[1]->evaluateInt(_p, _pI, _pT, _pIT);
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
			this->m_sChildren[0]->generateCodeWF(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
			this->m_sChildren[1]->generateCodeWF(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
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
			typename expression<BTHREADED>::children({_p0, _p1}),
			_p0->m_eType == expression<BTHREADED>::eFloatingPoint
				|| _p1->m_eType == expression<BTHREADED>::eFloatingPoint
				? expression<BTHREADED>::eFloatingPoint
				: expression<BTHREADED>::eInteger
		)
	{
	}
	virtual std::ostream &print(std::ostream&_r) const override
	{	_r << "(";
		this->m_sChildren[0]->print(_r) << "+";
		return this->m_sChildren[1]->print(_r) << ")";
	}
	virtual double evaluate(const double *const _p, const int *const _pI, const double*const _pT, const int*const _pIT) const override
	{	return this->m_sChildren[0]->evaluateW(_p, _pI, _pT, _pIT) + this->m_sChildren[1]->evaluateW(_p, _pI, _pT, _pIT);
	}
	virtual int evaluateInt(const double *const _p, const int *const _pI, const double*const _pT, const int*const _pIT) const override
	{	return this->m_sChildren[0]->evaluateInt(_p, _pI, _pT, _pIT) + this->m_sChildren[1]->evaluateInt(_p, _pI, _pT, _pIT);
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
			this->m_sChildren[0]->generateCodeWF(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
			this->m_sChildren[1]->generateCodeWF(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
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
			typename expression<BTHREADED>::children({_p0, _p1}),
			_p0->m_eType == expression<BTHREADED>::eFloatingPoint
				|| _p1->m_eType == expression<BTHREADED>::eFloatingPoint
				? expression<BTHREADED>::eFloatingPoint
				: expression<BTHREADED>::eInteger
		)
	{
	}
	virtual std::ostream &print(std::ostream&_r) const override
	{	_r << "(";
		this->m_sChildren[0]->print(_r) << "-";
		return this->m_sChildren[1]->print(_r) << ")";
	}
	virtual double evaluate(const double *const _p, const int *const _pI, const double*const _pT, const int*const _pIT) const override
	{	return this->m_sChildren[0]->evaluateW(_p, _pI, _pT, _pIT) - this->m_sChildren[1]->evaluateW(_p, _pI, _pT, _pIT);
	}
	virtual int evaluateInt(const double *const _p, const int *const _pI, const double*const _pT, const int*const _pIT) const override
	{	return this->m_sChildren[0]->evaluateInt(_p, _pI, _pT, _pIT) - this->m_sChildren[1]->evaluateInt(_p, _pI, _pT, _pIT);
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
			this->m_sChildren[0]->generateCodeWF(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
			this->m_sChildren[1]->generateCodeWF(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
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
			typename expression<BTHREADED>::children({_p}),
			expression<BTHREADED>::eFloatingPoint
		)
	{
	}
	virtual std::ostream &print(std::ostream&_r) const override
	{	_r << AC << "(";
		return this->m_sChildren[0]->print(_r) << ")";
	}
	virtual double evaluate(const double *const _p, const int *const _pI, const double*const _pT, const int*const _pIT) const override
	{	return PMATH(this->m_sChildren[0]->evaluateW(_p, _pI, _pT, _pIT));
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
			this->m_sChildren[0]->generateCodeWF(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT)
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
			typename expression<BTHREADED>::children({_p}),
			expression<BTHREADED>::eFloatingPoint
		)
	{
	}
	virtual std::ostream &print(std::ostream&_r) const override
	{	_r << AC << "(";
		return this->m_sChildren[0]->print(_r) << ")";
	}
	virtual double evaluate(const double *const _p, const int *const _pI, const double*const _pT, const int*const _pIT) const override
	{	return PMATH(this->m_sChildren[0]->evaluateW(_p, _pI, _pT, _pIT));
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
			{	this->m_sChildren[0]->generateCodeWF(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT)
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
			typename expression<BTHREADED>::children({_p}),
			_p->m_eType
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
	virtual int evaluateInt(const double *const _p, const int *const _pI, const double*const _pT, const int*const _pIT) const override
	{	return -this->m_sChildren[0]->evaluateInt(_p, _pI, _pT, _pIT);
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
			this->m_sChildren[0]->generateCodeWF(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT)
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
	const bool m_bFP;
	variable(const std::size_t _i, const bool _b)
		:expression<BTHREADED>({}, _b ? expression<BTHREADED>::eFloatingPoint : expression<BTHREADED>::eInteger),
		m_i(_i),
		m_bFP(_b)
	{
	}
	virtual std::ostream &print(std::ostream&_r) const override
	{	if (m_bFP)
			return _r << "fpvar" << m_i;
		else
			return _r << "var" << m_i;
	}
	virtual double evaluate(const double *const _p, const int *const _pI, const double*const _pT, const int*const _pIT) const override
	{	if (!m_bFP)
			return expression<BTHREADED>::evaluate(_p, _pI, _pT, _pIT);
		else
			return _pT[m_i];
	}
	virtual int evaluateInt(const double *const _p, const int *const _pI, const double*const _pT, const int*const _pIT) const override
	{	if (m_bFP)
			return expression<BTHREADED>::evaluateInt(_p, _pI, _pT, _pIT);
		else
			return _pIT[m_i];
	}
	auto tie(void) const
	{	return std::tie(m_i, m_bFP);
	}
	virtual bool isSmaller(const expression<BTHREADED>&_r) const override
	{	const auto &r = dynamic_cast<const variable&>(_r);
		return tie() < r.tie();
	}
	virtual llvm::Value* generateCode(
		const expression<BTHREADED>*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const,
		llvm::Value*const,
		llvm::Value*const _pP,
		llvm::Value*const _pIP
	) const override
	{
		using namespace llvm;
		// Step 1: Retrieve the contained integer (index)
		// Assume `index` is a member of the `parameter` object
		llvm::Value* indexVal = llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), m_i);

		// Step 2: Perform bounds checking or validation if required (optional)

		// Step 3: Use GetElementPtr (GEP) to calculate the address at the given index
		llvm::Value* indexedPointer = builder.CreateGEP(
			m_bFP
				? llvm::Type::getDoubleTy(context) // The type of elements (double)
				: llvm::Type::getInt32Ty(context), // The type of elements (double)
			m_bFP
				? _pP
				: _pIP,	// The pointer to be indexed
			indexVal	// The index (as a Value*)
		);

		// Step 4: Return the indexed pointer
		return builder.CreateLoad(
			m_bFP
				? Type::getDoubleTy(context) // The type to load
				: Type::getInt32Ty(context), // The type to load
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
	const bool m_bFP;
	parameter(const std::size_t _i, const bool _b)
		:expression<BTHREADED>({}, _b ? expression<BTHREADED>::eFloatingPoint : expression<BTHREADED>::eInteger),
		m_i(_i),
		m_bFP(_b)
	{
	}
	virtual std::ostream &print(std::ostream&_r) const override
	{	if (m_bFP)
			return _r << "fpx" << m_i;
		else
			return _r << "x" << m_i;
	}
	virtual double evaluate(const double *const _p, const int *const _pI, const double*const _pT, const int*const _pIT) const override
	{	if (!m_bFP)
			return expression<BTHREADED>::evaluate(_p, _pI, _pT, _pIT);
		else
			return _p[m_i];
	}
	virtual int evaluateInt(const double *const _p, const int *const _pI, const double*const _pT, const int*const _pIT) const override
	{	if (m_bFP)
			return expression<BTHREADED>::evaluateInt(_p, _pI, _pT, _pIT);
		else
			return _pI[m_i];
	}
	auto tie(void) const
	{	return std::tie(m_i, m_bFP);
	}
	virtual bool isSmaller(const expression<BTHREADED>&_r) const override
	{	const auto &r = dynamic_cast<const parameter&>(_r);
		return tie() < r.tie();
	}
	virtual llvm::Value* generateCode(
		const expression<BTHREADED>*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP,
		llvm::Value*const _pIP,
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
			m_bFP
				? llvm::Type::getDoubleTy(context) // The type of elements (double)
				: llvm::Type::getInt32Ty(context), // The type of elements (double)
			m_bFP
				? _pP
				: _pIP,	// The pointer to be indexed
			indexVal	// The index (as a Value*)
		);

		// Step 4: Return the indexed pointer
		return builder.CreateLoad(
			m_bFP
				? Type::getDoubleTy(context) // The type to load
				: Type::getInt32Ty(context), // The type to load
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
template<
	bool BTHREADED,
	const char ACOP[],
	const char ACNAME[],
	llvm::Value* (*CreateFCmpOLT)(llvm::IRBuilder<>&, llvm::Value*, llvm::Value*, const llvm::Twine&),
	llvm::Value* (llvm::IRBuilderBase::*CreateICmpSLT)(llvm::Value*, llvm::Value*, const llvm::Twine&),
	typename factory<BTHREADED>::exprPtr (factory<BTHREADED>::*less)(const typename factory<BTHREADED>::exprPtr&_p0, const typename factory<BTHREADED>::exprPtr&_p1) const,
	typename FUNCTOR
>
struct relational:expression<BTHREADED>
{
	typedef typename expression<BTHREADED>::ptr ptr;
	relational(const ptr&_p0, const ptr&_p1)
		:expression<BTHREADED>({_p0, _p1}, expression<BTHREADED>::eInteger)
	{
	}
	virtual std::ostream &print(std::ostream&_r) const override
	{	_r << "(";
		this->m_sChildren[0]->print(_r);
		_r << ACOP;
		this->m_sChildren[1]->print(_r);
		return _r << ")";
	}
	virtual double evaluate(const double *const _p, const int *const _pI, const double*const _pT, const int*const _pIT) const override
	{	throw std::logic_error("not a floating point operation!");
		return 0.0;
	}
	virtual int evaluateInt(const double *const _p, const int *const _pI, const double*const _pT, const int*const _pIT) const override
	{	switch (this->m_sChildren[0]->m_eType)
		{	case expression<BTHREADED>::eFloatingPoint:
				switch (this->m_sChildren[0]->m_eType)
				{	case expression<BTHREADED>::eFloatingPoint:
						return FUNCTOR()(this->m_sChildren[0]->evaluate(_p, _pI, _pT, _pIT), this->m_sChildren[1]->evaluate(_p, _pI, _pT, _pIT));
					case expression<BTHREADED>::eInteger:
						return FUNCTOR()(this->m_sChildren[0]->evaluate(_p, _pI, _pT, _pIT), this->m_sChildren[1]->evaluateInt(_p, _pI, _pT, _pIT));
				}
			case expression<BTHREADED>::eInteger:
				switch (this->m_sChildren[0]->m_eType)
				{	case expression<BTHREADED>::eFloatingPoint:
						return FUNCTOR()(this->m_sChildren[0]->evaluateInt(_p, _pI, _pT, _pIT), this->m_sChildren[1]->evaluate(_p, _pI, _pT, _pIT));
					case expression<BTHREADED>::eInteger:
						return FUNCTOR()(this->m_sChildren[0]->evaluateInt(_p, _pI, _pT, _pIT), this->m_sChildren[1]->evaluateInt(_p, _pI, _pT, _pIT));
				}
		}
		std::abort();
		return 0;
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
	{	switch (this->m_sChildren[0]->m_eType)
		{	case expression<BTHREADED>::eFloatingPoint:
				switch (this->m_sChildren[0]->m_eType)
				{	case expression<BTHREADED>::eFloatingPoint:
						return builder.CreateZExt(
							(*CreateFCmpOLT)(
								builder,
								this->m_sChildren[0]->generateCodeW(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
								this->m_sChildren[1]->generateCodeW(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
								ACNAME
							),
							builder.getInt32Ty()
						);
					case expression<BTHREADED>::eInteger:
						return builder.CreateZExt(
							(*CreateFCmpOLT)(
								builder,
								this->m_sChildren[0]->generateCodeW(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
								builder.CreateSIToFP(
									this->m_sChildren[1]->generateCodeW(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
									builder.getDoubleTy(),
									"intToDouble"
								),
								ACNAME
							),
							builder.getInt32Ty()
						);
				}
			case expression<BTHREADED>::eInteger:
				switch (this->m_sChildren[0]->m_eType)
				{	case expression<BTHREADED>::eFloatingPoint:
						return builder.CreateZExt(
							(*CreateFCmpOLT)(
								builder,
								builder.CreateSIToFP(
									this->m_sChildren[0]->generateCodeW(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
									builder.getDoubleTy(),
									"intToDouble"
								),
								this->m_sChildren[1]->generateCodeW(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
								ACNAME
							),
							builder.getInt32Ty()
						);
					case expression<BTHREADED>::eInteger:
						return builder.CreateZExt(
							(builder.*CreateICmpSLT)(
								this->m_sChildren[0]->generateCodeW(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
								this->m_sChildren[1]->generateCodeW(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
								ACNAME
							),
							builder.getInt32Ty()
						);
				}
		}
		std::abort();
		return nullptr;
	}
	virtual typename expression<BTHREADED>::ptr recreateFromChildren(typename expression<BTHREADED>::children _s, const factory<BTHREADED>&_rF) const override
	{	return (_rF.*less)(_s[0], _s[1]);
	}
	virtual std::size_t getWeight(void) const override
	{	return 1;
	}
};
template<bool BTHREADED>
struct conditional:expression<BTHREADED>
{
	typedef typename expression<BTHREADED>::ptr ptr;
	conditional(const ptr&_p0, const ptr&_p1, const ptr&_p2)
		:expression<BTHREADED>({_p0, _p1, _p2}, _p1->m_eType)
	{	if (_p0->m_eType != expression<BTHREADED>::eInteger)
			throw std::logic_error("conditional needs int argument for test expression!");
		if (_p1->m_eType != _p2->m_eType)
			throw std::logic_error("conditional identical types for true and false expression!");
	}
	virtual std::ostream &print(std::ostream&_r) const override
	{	_r << "(";
		this->m_sChildren[0]->print(_r);
		_r << "?";
		this->m_sChildren[1]->print(_r);
		_r << ":";
		this->m_sChildren[2]->print(_r);
		return _r << ")";
	}
	virtual double evaluate(const double *const _p, const int *const _pI, const double*const _pT, const int*const _pIT) const override
	{	return this->m_sChildren[0]->evaluateInt(_p, _pI, _pT, _pIT)
			? this->m_sChildren[1]->evaluate(_p, _pI, _pT, _pIT)
			: this->m_sChildren[2]->evaluate(_p, _pI, _pT, _pIT);
	}
	virtual int evaluateInt(const double *const _p, const int *const _pI, const double*const _pT, const int*const _pIT) const override
	{	return this->m_sChildren[0]->evaluateInt(_p, _pI, _pT, _pIT)
			? this->m_sChildren[1]->evaluateInt(_p, _pI, _pT, _pIT)
			: this->m_sChildren[2]->evaluateInt(_p, _pI, _pT, _pIT);
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
	{	return builder.CreateCall(
			M->getFunction(
				this->m_eType == expression<BTHREADED>::eFloatingPoint
					? "ternaryDouble"
					: "ternaryInt"
			),
			{	this->m_sChildren[0]->generateCodeW(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
				this->m_sChildren[1]->generateCodeW(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
				this->m_sChildren[2]->generateCodeW(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT)
			}
		);
	}
	virtual typename expression<BTHREADED>::ptr recreateFromChildren(typename expression<BTHREADED>::children _s, const factory<BTHREADED>&_rF) const override
	{	return _rF.conditional(_s[0], _s[1], _s[2]);
	}
	virtual std::size_t getWeight(void) const override
	{	return 1;
	}
};
template<
	bool BTHREADED,
	const char ACOP[],
	const char ACNAME[],
	llvm::Value* (llvm::IRBuilderBase::*CreateICmpSLT)(llvm::Value*, llvm::Value*, const llvm::Twine&),
	typename factory<BTHREADED>::exprPtr (factory<BTHREADED>::*less)(const typename factory<BTHREADED>::exprPtr&_p0, const typename factory<BTHREADED>::exprPtr&_p1) const,
	typename FUNCTOR
>
struct bw_logical:expression<BTHREADED>
{
	typedef typename expression<BTHREADED>::ptr ptr;
	bw_logical(const ptr&_p0, const ptr&_p1)
		:expression<BTHREADED>({_p0, _p1}, expression<BTHREADED>::eInteger)
	{	if (!std::all_of(
			this->m_sChildren.begin(),
			this->m_sChildren.end(),
			[](const typename expression<BTHREADED>::ptr&_p)
			{	return _p->m_eType == expression<BTHREADED>::eInteger;
			}
		))
			throw std::logic_error("arguments must be all integers!");
	}
	virtual std::ostream &print(std::ostream&_r) const override
	{	_r << "(";
		this->m_sChildren[0]->print(_r);
		_r << ACOP;
		this->m_sChildren[1]->print(_r);
		return _r << ")";
	}
	virtual double evaluate(const double *const _p, const int *const _pI, const double*const _pT, const int*const _pIT) const override
	{	throw std::logic_error("not a floating point operation!");
		return 0.0;
	}
	virtual int evaluateInt(const double *const _p, const int *const _pI, const double*const _pT, const int*const _pIT) const override
	{	return FUNCTOR()(this->m_sChildren[0]->evaluateInt(_p, _pI, _pT, _pIT), this->m_sChildren[1]->evaluateInt(_p, _pI, _pT, _pIT));
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
	{	return builder.CreateZExt(
			(builder.*CreateICmpSLT)(
				this->m_sChildren[0]->generateCodeW(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
				this->m_sChildren[1]->generateCodeW(_pRoot, context, builder, M, _pP, _pIP, _pT, _pIT),
				ACNAME
			),
			builder.getInt32Ty()
		);
	}
	virtual typename expression<BTHREADED>::ptr recreateFromChildren(typename expression<BTHREADED>::children _s, const factory<BTHREADED>&_rF) const override
	{	return (_rF.*less)(_s[0], _s[1]);
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
		std::vector<std::vector<std::pair<std::size_t, std::size_t> > >,
		std::vector<std::size_t>,
		std::size_t,
		std::size_t
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
		std::size_t iNextIntId = 0, iNextFpId = 0;
		children sChildren;
		ESET sSet;
		std::vector<std::vector<std::pair<std::size_t, std::size_t> > > sGroups;
		auto sLeft = sSetRepeated;
		for (const auto &p : _rChildren)
			sLeft.insert(p.get());
		while (!sLeft.empty())
		{	auto &r = sG2Set.at(sSet);
			sGroups.emplace_back();
			sGroups.back().reserve(r.size());
			for (const auto p : r)
			{	const auto sInsert = sExpr2Id.emplace(
					p,
					p->m_eType == expression<BTHREADED>::eFloatingPoint
						? iNextFpId++
						: iNextIntId++
				);
				sLeft.erase(p);
				sSet.insert(p);
				sGroups.back().emplace_back(sExpr2Id.size() - 1, sInsert.first->second);
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
			sMap[p] = _rF.variable(
				sExpr2Id.at(p),
				p->m_eType == expression<BTHREADED>::eFloatingPoint
			);
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
		return TUPLE(std::move(sChildren), std::move(sGroups), std::move(sOrder), iNextFpId, iNextIntId);
	}
	expressionSetImpl(
		const children& _rChildren,
		const factory<BTHREADED>&_rF
	)
		:m_sChildren(get(_rChildren, _rF))
	{
	}
	template<
		double(expression<BTHREADED>::*EVALUATE)(const double *const, const int*const, const double*const, const int*const) const,
		int(expression<BTHREADED>::*EVALUATE_INT)(const double *const, const int*const, const double*const, const int*const) const
	>
	void run_evaluate(
		std::vector<double>&_rChildren,
		std::vector<int>&_rIChildren,
		const double *const _pParams,
		const int *const _pIParams//,
		//typename expressionSet<BTHREADED>::atomicVec &_rC,
		//boost::asio::thread_pool &_rPool
	) const
	{	const auto &[rChildren, rG2Set, rOrder, iFpSize, iIntSize] = m_sChildren;
		_rChildren.resize(iFpSize);
		_rIChildren.resize(iIntSize);
		if constexpr (!BTHREADED)
			for (std::size_t i = 0; i < rChildren.size(); ++i)
			{	const auto pE = rChildren[i].get();
				if (pE->m_eType == expression<BTHREADED>::eFloatingPoint)
					_rChildren[rOrder[i]] = (pE->*EVALUATE)(_pParams, _pIParams, _rChildren.data(), _rIChildren.data());
				else
					_rIChildren[rOrder[i]] = (pE->*EVALUATE_INT)(_pParams, _pIParams, _rChildren.data(), _rIChildren.data());
			}
		else
			for (const auto &r : rG2Set)
				std::for_each(
					std::execution::par,
					r.begin(),
					r.end(),
					[&](const std::pair<std::size_t, std::size_t> &_r)
					{	const auto pE = rChildren[_r.first].get();
						if (pE->m_eType == expression<BTHREADED>::eFloatingPoint)
							_rChildren[_r.second] = (pE->*EVALUATE)(_pParams, _pIParams, _rChildren.data(), _rIChildren.data());
						else
							_rIChildren[_r.second] = (pE->*EVALUATE_INT)(_pParams, _pIParams, _rChildren.data(), _rIChildren.data());
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
	{	run_evaluate<
			&expression<BTHREADED>::evaluate,
			&expression<BTHREADED>::evaluateInt
		>(_rChildren, _rIChildren, _pParams, _pIParams);
	}
	virtual void evaluateLLVM(
		std::vector<double>&_rChildren,
		std::vector<int>&_rIChildren,
		const double *const _pParams,
		const int *const _pIParams
		//typename expressionSet<BTHREADED>::atomicVec &_rC,
		//boost::asio::thread_pool &_rPool
	) const override
	{	run_evaluate<
			&expression<BTHREADED>::evaluateLLVM,
			&expression<BTHREADED>::evaluateIntLLVM
		>(_rChildren, _rIChildren, _pParams, _pIParams);
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
	virtual exprPtr intConstant(const int _i) const override
	{	return theExpressionEngine::expression<BTHREADED>::template create<dynamic_cast_implementation<theExpressionEngine::intConstant<BTHREADED> > >(*this, _i);
	}
	virtual exprPtr parameter(const std::size_t _i, const bool _b) const override
	{	return theExpressionEngine::expression<BTHREADED>::template create<theExpressionEngine::parameter<BTHREADED> >(*this, _i, _b);
	}
	virtual exprPtr variable(const std::size_t _i, const bool _b) const override
	{	return theExpressionEngine::expression<BTHREADED>::template create<theExpressionEngine::variable<BTHREADED> >(*this, _i, _b);
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
	{	if (_p0->isZero())
			return _p1;
		else
		if (_p1->isZero())
			return _p0;
		else
			return theExpressionEngine::expression<BTHREADED>::template create<theExpressionEngine::addition<BTHREADED> >(*this, _p0, _p1);
	}
	virtual exprPtr subtraction(const exprPtr&_p0, const exprPtr&_p1) const override
	{	if (_p0->isZero())
			return negation(_p1);
		else
		if (_p1->isZero())
			return _p0;
		else
		return theExpressionEngine::expression<BTHREADED>::template create<theExpressionEngine::subtraction<BTHREADED> >(*this, _p0, _p1);
	}
	virtual exprPtr multiplication(const exprPtr&_p0, const exprPtr&_p1) const override
	{	if (_p0->isOne())
			return _p1;
		else
		if (_p0->isZero())
			return _p0;
		else
		if (_p1->isOne())
			return _p0;
		else
		if (_p1->isZero())
			return _p1;
		else
		return theExpressionEngine::expression<BTHREADED>::template create<theExpressionEngine::multiplication<BTHREADED> >(*this, _p0, _p1);
	}
	virtual exprPtr less(const exprPtr&_p0, const exprPtr&_p1) const override
	{	static constexpr char acOP[] = "<";
		static constexpr char acName[] = "isLess";
		return theExpressionEngine::expression<BTHREADED>::template create<
			theExpressionEngine::relational<
				BTHREADED,
				acOP,
				acName,
				[](llvm::IRBuilder<>&_r0, llvm::Value*_p1, llvm::Value*_p2, const llvm::Twine&_r3) -> llvm::Value*
				{	return _r0.CreateFCmpOLT(_p1, _p2, _r3);
				},
				&llvm::IRBuilder<>::CreateICmpSLT,
				&factory<BTHREADED>::less,
				std::less<void>
			>
		>(*this, _p0, _p1);
	}
	virtual exprPtr less_equal(const exprPtr&_p0, const exprPtr&_p1) const override
	{	static constexpr char acOP[] = "<=";
		static constexpr char acName[] = "isLessEqual";
		return theExpressionEngine::expression<BTHREADED>::template create<
			theExpressionEngine::relational<
				BTHREADED,
				acOP,
				acName,
				[](llvm::IRBuilder<>&_r0, llvm::Value*_p1, llvm::Value*_p2, const llvm::Twine&_r3) -> llvm::Value*
				{	return _r0.CreateFCmpOLE(_p1, _p2, _r3);
				},
				&llvm::IRBuilder<>::CreateICmpSLE,
				&factory<BTHREADED>::less_equal,
				std::less_equal<void>
			>
		>(*this, _p0, _p1);
	}
	virtual exprPtr greater_equal(const exprPtr&_p0, const exprPtr&_p1) const override
	{	static constexpr char acOP[] = ">=";
		static constexpr char acName[] = "isGreaterEqual";
		return theExpressionEngine::expression<BTHREADED>::template create<
			theExpressionEngine::relational<
				BTHREADED,
				acOP,
				acName,
				[](llvm::IRBuilder<>&_r0, llvm::Value*_p1, llvm::Value*_p2, const llvm::Twine&_r3) -> llvm::Value*
				{	return _r0.CreateFCmpOGE(_p1, _p2, _r3);
				},
				&llvm::IRBuilder<>::CreateICmpSGE,
				&factory<BTHREADED>::greater_equal,
				std::greater_equal<void>
			>
		>(*this, _p0, _p1);
	}
	virtual exprPtr greater(const exprPtr&_p0, const exprPtr&_p1) const override
	{	static constexpr char acOP[] = ">";
		static constexpr char acName[] = "isGreater";
		return theExpressionEngine::expression<BTHREADED>::template create<
			theExpressionEngine::relational<
				BTHREADED,
				acOP,
				acName,
				[](llvm::IRBuilder<>&_r0, llvm::Value*_p1, llvm::Value*_p2, const llvm::Twine&_r3) -> llvm::Value*
				{	return _r0.CreateFCmpOGT(_p1, _p2, _r3);
				},
				&llvm::IRBuilder<>::CreateICmpSGT,
				&factory<BTHREADED>::greater,
				std::greater<void>
			>
		>(*this, _p0, _p1);
	}
	virtual exprPtr equal_to(const exprPtr&_p0, const exprPtr&_p1) const override
	{	static constexpr char acOP[] = "==";
		static constexpr char acName[] = "isEqual";
		return theExpressionEngine::expression<BTHREADED>::template create<
			theExpressionEngine::relational<
				BTHREADED,
				acOP,
				acName,
				[](llvm::IRBuilder<>&_r0, llvm::Value*_p1, llvm::Value*_p2, const llvm::Twine&_r3) -> llvm::Value*
				{	return _r0.CreateFCmpOEQ(_p1, _p2, _r3);
				},
				&llvm::IRBuilder<>::CreateICmpEQ,
				&factory<BTHREADED>::equal_to,
				std::equal_to<void>
			>
		>(*this, _p0, _p1);
	}
	virtual exprPtr not_equal_to(const exprPtr&_p0, const exprPtr&_p1) const override
	{	static constexpr char acOP[] = "!=";
		static constexpr char acName[] = "isNotEqual";
		return theExpressionEngine::expression<BTHREADED>::template create<
			theExpressionEngine::relational<
				BTHREADED,
				acOP,
				acName,
				[](llvm::IRBuilder<>&_r0, llvm::Value*_p1, llvm::Value*_p2, const llvm::Twine&_r3) -> llvm::Value*
				{	return _r0.CreateFCmpONE(_p1, _p2, _r3);
				},
				&llvm::IRBuilder<>::CreateICmpNE,
				&factory<BTHREADED>::not_equal_to,
				std::not_equal_to<void>
			>
		>(*this, _p0, _p1);
	}
	virtual exprPtr conditional(const exprPtr&_p0, const exprPtr&_p1, const exprPtr&_p2) const override
	{	if (_p0->isNonZero())
			return _p1;
		else
		if (_p0->isZero())
			return _p2;
		else
			return theExpressionEngine::expression<BTHREADED>::template create<theExpressionEngine::conditional<BTHREADED> >(*this, _p0, _p1, _p2);
	}
	virtual exprPtr division(const exprPtr&_p0, const exprPtr&_p1) const override
	{	if (_p0->isZero())
			return _p0;
		else
		if (_p1->isOne())
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
	virtual exprPtr bit_and(const exprPtr&_p0, const exprPtr&_p1) const override
	{	static constexpr char acOP[] = "&";
		static constexpr char acName[] = "bw_and";
		return theExpressionEngine::expression<BTHREADED>::template create<
			theExpressionEngine::bw_logical<
				BTHREADED,
				acOP,
				acName,
				&llvm::IRBuilder<>::CreateAnd,
				&factory<BTHREADED>::bit_and,
				std::bit_and<void>
			>
		>(*this, _p0, _p1);
	}
	virtual exprPtr bit_or(const exprPtr&_p0, const exprPtr&_p1) const override
	{	static constexpr char acOP[] = "|";
		static constexpr char acName[] = "bit_or";
		return theExpressionEngine::expression<BTHREADED>::template create<
			theExpressionEngine::bw_logical<
				BTHREADED,
				acOP,
				acName,
				&llvm::IRBuilder<>::CreateOr,
				&factory<BTHREADED>::bit_or,
				std::bit_or<void>
			>
		>(*this, _p0, _p1);
	}
	virtual exprPtr bit_xor(const exprPtr&_p0, const exprPtr&_p1) const override
	{	static constexpr char acOP[] = "^";
		static constexpr char acName[] = "bit_xor";
		return theExpressionEngine::expression<BTHREADED>::template create<
			theExpressionEngine::bw_logical<
				BTHREADED,
				acOP,
				acName,
				&llvm::IRBuilder<>::CreateXor,
				&factory<BTHREADED>::bit_xor,
				std::bit_xor<void>
			>
		>(*this, _p0, _p1);
	}
	virtual exprPtr bit_not(const exprPtr&_p) const override
	{	return nullptr;
	}
	virtual exprPtr logical_and(const exprPtr&_p0, const exprPtr&_p1) const override
	{	static constexpr char acOP[] = "&&";
		static constexpr char acName[] = "logical_and";
		return theExpressionEngine::expression<BTHREADED>::template create<
			theExpressionEngine::bw_logical<
				BTHREADED,
				acOP,
				acName,
				&llvm::IRBuilder<>::CreateAnd,
				&factory<BTHREADED>::logical_and,
				std::logical_and<void>
			>
		>(*this, _p0, _p1);
	}
	virtual exprPtr logical_or(const exprPtr&_p0, const exprPtr&_p1) const override
	{	static constexpr char acOP[] = "||";
		static constexpr char acName[] = "logical_or";
		return theExpressionEngine::expression<BTHREADED>::template create<
			theExpressionEngine::bw_logical<
				BTHREADED,
				acOP,
				acName,
				&llvm::IRBuilder<>::CreateOr,
				&factory<BTHREADED>::logical_or,
				std::logical_or<void>
			>
		>(*this, _p0, _p1);
	}
	virtual exprPtr logical_not(const exprPtr&_p) const override
	{	return nullptr;
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
