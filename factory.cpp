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
	virtual double evaluate(const double *const, const double*const) const override
	{	return m_d;
	}
	virtual llvm::Value* generateCode(
		const expression<BTHREADED>*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
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
		return _rF.realConstant(evaluate(nullptr, nullptr));
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
		this->m_sChildren.at(0)->print(_r) << ",";
		return this->m_sChildren.at(1)->print(_r) << ")";
	}
	virtual double evaluate(const double *const _p, const double*const _pT) const override
	{	return PFCT(this->m_sChildren.at(0)->evaluate(_p, _pT), this->m_sChildren.at(1)->evaluate(_p, _pT));
	}
	virtual llvm::Value* generateCode(
		const expression<BTHREADED>*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP,
		llvm::Value*const _pT
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
			{	this->m_sChildren.at(0)->generateCodeW(_pRoot, context, builder, M, _pP, _pT),
				this->m_sChildren.at(1)->generateCodeW(_pRoot, context, builder, M, _pP, _pT)
			}
		);
	}
	virtual typename expression<BTHREADED>::ptr recreateFromChildren(typename expression<BTHREADED>::children _s, const factory<BTHREADED>&_rF) const override
	{	return (_rF.*CREATE)(_s.at(0), _s.at(1));
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
		this->m_sChildren.at(0)->print(_r) << ",";
		return this->m_sChildren.at(1)->print(_r) << ")";
	}
	virtual double evaluate(const double *const _p, const double*const _pT) const override
	{	return std::max(this->m_sChildren.at(0)->evaluate(_p, _pT), this->m_sChildren.at(1)->evaluate(_p, _pT));
	}
	virtual llvm::Value* generateCode(
		const expression<BTHREADED>*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP,
		llvm::Value*const _pT
	) const override
	{	// Create the function prototype for std::sqrt
		using namespace llvm;
			// Call the tan function
		Type* const doubleType = Type::getDoubleTy(context);
		return builder.CreateCall(
			Intrinsic::getDeclaration(M, Intrinsic::maxnum, {doubleType}),
			{	this->m_sChildren.at(0)->generateCodeW(_pRoot, context, builder, M, _pP, _pT),
				this->m_sChildren.at(1)->generateCodeW(_pRoot, context, builder, M, _pP, _pT)
			},
			"maxnum"
		);
	}
	virtual typename expression<BTHREADED>::ptr recreateFromChildren(typename expression<BTHREADED>::children _s, const factory<BTHREADED>&_rF) const override
	{	return _rF.max(_s.at(0), _s.at(1));
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
		this->m_sChildren.at(0)->print(_r) << ",";
		return this->m_sChildren.at(1)->print(_r) << ")";
	}
	virtual double evaluate(const double *const _p, const double*const _pT) const override
	{	return std::min(this->m_sChildren.at(0)->evaluate(_p, _pT), this->m_sChildren.at(1)->evaluate(_p, _pT));
	}
	virtual llvm::Value* generateCode(
		const expression<BTHREADED>*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP,
		llvm::Value*const _pT
	) const override
	{	// Create the function prototype for std::sqrt
		using namespace llvm;
		//Value* const Input = ConstantFP::get(llvm::Type::getDoubleTy(context), m_sChildren.at(0)->generateCodeW(context, builder, M));
		// Call the tan function
		Type* const doubleType = Type::getDoubleTy(context);
		return builder.CreateCall(
			Intrinsic::getDeclaration(M, Intrinsic::minnum, {doubleType}),
			{	this->m_sChildren.at(0)->generateCodeW(_pRoot, context, builder, M, _pP, _pT),
				this->m_sChildren.at(1)->generateCodeW(_pRoot, context, builder, M, _pP, _pT),
			},
			"minnum"
		);
	}
	virtual typename expression<BTHREADED>::ptr recreateFromChildren(typename expression<BTHREADED>::children _s, const factory<BTHREADED>&_rF) const override
	{	return _rF.min(_s.at(0), _s.at(1));
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
		this->m_sChildren.at(0)->print(_r) << "*";
		return this->m_sChildren.at(1)->print(_r) << ")";
	}
	virtual double evaluate(const double *const _p, const double*const _pT) const override
	{	return this->m_sChildren.at(0)->evaluate(_p, _pT) * this->m_sChildren.at(1)->evaluate(_p, _pT);
	}
	virtual llvm::Value* generateCode(
		const expression<BTHREADED>*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP,
		llvm::Value*const _pT
	) const override
	{	return builder.CreateFMul(
			this->m_sChildren.at(0)->generateCodeW(_pRoot, context, builder, M, _pP, _pT),
			this->m_sChildren.at(1)->generateCodeW(_pRoot, context, builder, M, _pP, _pT),
			"plus"
		);
	}
	virtual typename expression<BTHREADED>::ptr recreateFromChildren(typename expression<BTHREADED>::children _s, const factory<BTHREADED>&_rF) const override
	{	return _rF.multiplication(_s.at(0), _s.at(1));
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
		this->m_sChildren.at(0)->print(_r) << "/";
		return this->m_sChildren.at(1)->print(_r) << ")";
	}
	virtual double evaluate(const double *const _p, const double*const _pT) const override
	{	return this->m_sChildren.at(0)->evaluate(_p, _pT) / this->m_sChildren.at(1)->evaluate(_p, _pT);
	}
	virtual llvm::Value* generateCode(
		const expression<BTHREADED>*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP,
		llvm::Value*const _pT
	) const override
	{	return builder.CreateFDiv(
			this->m_sChildren.at(0)->generateCodeW(_pRoot, context, builder, M, _pP, _pT),
			this->m_sChildren.at(1)->generateCodeW(_pRoot, context, builder, M, _pP, _pT),
			"plus"
		);
	}
	virtual typename expression<BTHREADED>::ptr recreateFromChildren(typename expression<BTHREADED>::children _s, const factory<BTHREADED>&_rF) const override
	{	return _rF.division(_s.at(0), _s.at(1));
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
		this->m_sChildren.at(0)->print(_r) << "+";
		return this->m_sChildren.at(1)->print(_r) << ")";
	}
	virtual double evaluate(const double *const _p, const double*const _pT) const override
	{	return this->m_sChildren.at(0)->evaluate(_p, _pT) + this->m_sChildren.at(1)->evaluate(_p, _pT);
	}
	virtual llvm::Value* generateCode(
		const expression<BTHREADED>*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP,
		llvm::Value*const _pT
	) const override
	{	return builder.CreateFAdd(
			this->m_sChildren.at(0)->generateCodeW(_pRoot, context, builder, M, _pP, _pT),
			this->m_sChildren.at(1)->generateCodeW(_pRoot, context, builder, M, _pP, _pT),
			"plus"
		);
	}
	virtual typename expression<BTHREADED>::ptr recreateFromChildren(typename expression<BTHREADED>::children _s, const factory<BTHREADED>&_rF) const override
	{	return _rF.addition(_s.at(0), _s.at(1));
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
		this->m_sChildren.at(0)->print(_r) << "-";
		return this->m_sChildren.at(1)->print(_r) << ")";
	}
	virtual double evaluate(const double *const _p, const double*const _pT) const override
	{	return this->m_sChildren.at(0)->evaluate(_p, _pT) - this->m_sChildren.at(1)->evaluate(_p, _pT);
	}
	virtual llvm::Value* generateCode(
		const expression<BTHREADED>*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP,
		llvm::Value*const _pT
	) const override
	{	return builder.CreateFSub(
			this->m_sChildren.at(0)->generateCodeW(_pRoot, context, builder, M, _pP, _pT),
			this->m_sChildren.at(1)->generateCodeW(_pRoot, context, builder, M, _pP, _pT),
			"plus"
		);
	}
	virtual typename expression<BTHREADED>::ptr recreateFromChildren(typename expression<BTHREADED>::children _s, const factory<BTHREADED>&_rF) const override
	{	return _rF.subtraction(_s.at(0), _s.at(1));
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
		return this->m_sChildren.at(0)->print(_r) << ")";
	}
	virtual double evaluate(const double *const _p, const double*const _pT) const override
	{	return PMATH(this->m_sChildren.at(0)->evaluate(_p, _pT));
	}
	virtual llvm::Value* generateCode(
		const expression<BTHREADED>*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP,
		llvm::Value*const _pT
	) const override
	{	// Create the function prototype for std::sqrt
		using namespace llvm;
		return builder.CreateCall(
			Intrinsic::getDeclaration(M, EID, builder.getDoubleTy()),
			this->m_sChildren.at(0)->generateCodeW(_pRoot, context, builder, M, _pP, _pT)
		);
	}
	virtual typename expression<BTHREADED>::ptr recreateFromChildren(typename expression<BTHREADED>::children _s, const factory<BTHREADED>&_rF) const override
	{	return (_rF.*CREATE)(_s.at(0));
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
		return this->m_sChildren.at(0)->print(_r) << ")";
	}
	virtual double evaluate(const double *const _p, const double*const _pT) const override
	{	return PMATH(this->m_sChildren.at(0)->evaluate(_p, _pT));
	}
	virtual llvm::Value* generateCode(
		const expression<BTHREADED>*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP,
		llvm::Value*const _pT
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
					{	llvm::Type::getDoubleTy(context),
						llvm::Type::getDoubleTy(context)
					}, // Argument type: double
					false // Not variadic
				)
			),
			{	this->m_sChildren.at(0)->generateCodeW(_pRoot, context, builder, M, _pP, _pT)
			}
		);
	}
	virtual typename expression<BTHREADED>::ptr recreateFromChildren(typename expression<BTHREADED>::children _s, const factory<BTHREADED>&_rF) const override
	{	return (_rF.*CREATE)(_s.at(0));
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
		return this->m_sChildren.at(0)->print(_r) << ")";
	}
	virtual double evaluate(const double *const _p, const double*const _pT) const override
	{	return -this->m_sChildren.at(0)->evaluate(_p, _pT);
	}
	virtual llvm::Value* generateCode(
		const expression<BTHREADED>*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP,
		llvm::Value*const _pT
	) const override
	{	// Create the function prototype for std::sqrt
		using namespace llvm;
		llvm::Type* doubleType = Type::getDoubleTy(M->getContext());
		return builder.CreateFSub(
			ConstantFP::get(doubleType, 0.0),
			this->m_sChildren.at(0)->generateCodeW(_pRoot, context, builder, M, _pP, _pT)
		);
	}
	virtual typename expression<BTHREADED>::ptr recreateFromChildren(typename expression<BTHREADED>::children _s, const factory<BTHREADED>&_rF) const override
	{	return _rF.negation(_s.at(0));
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
	virtual double evaluate(const double *const, const double*const _p) const override
	{	return _p[m_i];
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
		llvm::Value*const _pP
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
	virtual double evaluate(const double *const _p, const double*const) const override
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
	const std::pair<children, children> m_sChildren;
	static auto get(
		const children& _rChildren,
		const factory<BTHREADED>&_rF
	)
	{	std::set<
			const expression<BTHREADED>*
		> sSet;
		typename expression<BTHREADED>::ptr2ptr sMap;
		for (const auto &p : _rChildren)
			p->DFS(
				[&](const expression<BTHREADED>*const _p)
				{	if (_p->m_sChildren.empty())
						return false;
					const auto sInsert = sSet.emplace(_p);
					if (sInsert.second)
						return true;
					if (auto s = sMap.emplace(_p, nullptr); s.second)
						s.first->second = _rF.variable(sMap.size() - 1);
					return false;
				}
			);
		children sC;
		sC.reserve(_rChildren.size());
		for (const auto &p : _rChildren)
			sC.push_back(p->replace(sMap, _rF));
		children sTemp;
		sTemp.reserve(sMap.size());
		auto sM = sMap;
		for (const auto &r : sMap)
		{	sM.erase(r.first);
			sTemp.push_back(r.first->replace(sM, _rF));
			sM.insert(r);
		}
		return std::make_pair(sC, sTemp);
	}
	expressionSetImpl(
		const children& _rChildren,
		const factory<BTHREADED>&_rF
	)
		:m_sChildren(get(_rChildren, _rF))
	{
	}
	virtual void calculate(
		std::vector<double>&_rChildren,
		std::vector<double>&_rTemp,
		const double *const _pParams
	) const override
	{	_rTemp.reserve(m_sChildren.second.size());
		std::transform(
			m_sChildren.second.begin(),
			m_sChildren.second.end(),
			_rTemp.begin(),
			[_pParams, &_rTemp](const typename expression<BTHREADED>::ptr&_p)
			{	return _p->evaluate(_pParams, _rTemp.data());
			}
		);
		_rChildren.resize(m_sChildren.first.size());
		std::transform(
			m_sChildren.first.begin(),
			m_sChildren.first.end(),
			_rChildren.begin(),
			[_pParams, &_rTemp](const typename expression<BTHREADED>::ptr&_p)
			{	return _p->evaluate(_pParams, _rTemp.data());
			}
		);
	}
	virtual const typename expression<BTHREADED>::children &getChildren(void) const override
	{	return m_sChildren.first;
	}
	virtual const typename expression<BTHREADED>::children &getTemps(void) const override
	{	return m_sChildren.second;
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
	virtual exprPtr pow(const exprPtr&_p0, const exprPtr&_p1) const override
	{	static const char ac[] = "pow";
		return theExpressionEngine::expression<BTHREADED>::template create<theExpressionEngine::binary<BTHREADED, ac, &std::pow, &factory<BTHREADED>::pow> >(*this, _p0, _p1);
	}
	virtual exprPtr fmod(const exprPtr&_p0, const exprPtr&_p1) const override
	{	static const char ac[] = "fmod";
		return theExpressionEngine::expression<BTHREADED>::template create<theExpressionEngine::binary<BTHREADED, ac, &std::fmod, &factory<BTHREADED>::fmod> >(*this, _p0, _p1);
	}
	virtual exprPtr hypot(const exprPtr&_p0, const exprPtr&_p1) const override
	{	static const char ac[] = "hypot";
		return theExpressionEngine::expression<BTHREADED>::template create<theExpressionEngine::binary<BTHREADED, ac, &std::hypot, &factory<BTHREADED>::hypot> >(*this, _p0, _p1);
	}
	virtual exprPtr atan2(const exprPtr&_p0, const exprPtr&_p1) const override
	{	static const char ac[] = "atan2";
		return theExpressionEngine::expression<BTHREADED>::template create<theExpressionEngine::binary<BTHREADED, ac, &std::atan2, &factory<BTHREADED>::atan2> >(*this, _p0, _p1);
	}
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
		return theExpressionEngine::expression<BTHREADED>::template create<theExpressionEngine::division<BTHREADED> >(*this, _p0, _p1);
	}
	virtual exprPtr negation(const exprPtr&_r) const override
	{	return theExpressionEngine::expression<BTHREADED>::template create<theExpressionEngine::negation<BTHREADED> >(*this, _r);
	}
	virtual exprPtr parse(const char *const _r, const typename factory<BTHREADED>::name2int&_rP) const override;
	virtual std::shared_ptr<const expressionSet<BTHREADED> > createExpressionSet(const std::vector<exprPtr>&_r) const override
	{	return std::make_shared<const expressionSetImpl<BTHREADED> >(_r, *this);
	}
};
template<>
typename factoryImpl<true>::exprPtr factoryImpl<true>::parse(const char *const _r, const name2int&_rP) const
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
typename factoryImpl<false>::exprPtr factoryImpl<false>::parse(const char *const _r, const name2int&_rP) const
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
}
