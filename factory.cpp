#include "factory.h"
#include "expression.h"
#include "unique.h"
#include "onDestroy.h"
#include "parser.h"
#include "flex.cpp"
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
	virtual llvm::Value* generateCode(
		const expression*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP
	) const override
	{	return llvm::ConstantFP::get(context, llvm::APFloat(m_d));
	}
	virtual ptr recreateFromChildren(children, const factory&) const override
	{	return this;
	}
	virtual std::size_t getWeight(void) const override
	{	return 1;
	}
};
expression::ptr expression::collapse(const factory&_rF) const
{	if (!getPtr(dummy<realConstant>()) && !m_sChildren.empty() && std::all_of(
		m_sChildren.begin(),
		m_sChildren.end(),
		[](const ptr&_p)
		{	return _p->getPtr(dummy<realConstant>()) != nullptr;
		}
	))
		return _rF.realConstant(evaluate(nullptr));
	else
		return this;
}
namespace
{
template<const char ac[], double(*PFCT)(double, double), factory::exprPtr (factory::*CREATE)(const factory::exprPtr&, const factory::exprPtr&) const>
struct binary:expression
{	binary(const ptr&_p0, const ptr&_p1)
		:expression(
			children({_p0, _p1})
		)
	{
	}
	virtual double evaluate(const double *const _p) const override
	{	return PFCT(m_sChildren.at(0)->evaluate(_p), m_sChildren.at(1)->evaluate(_p));
	}
	virtual llvm::Value* generateCode(
		const expression*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP
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
			{	m_sChildren.at(0)->generateCodeW(_pRoot, context, builder, M, _pP),
				m_sChildren.at(1)->generateCodeW(_pRoot, context, builder, M, _pP)
			}
		);
	}
	virtual ptr recreateFromChildren(children _s, const factory&_rF) const override
	{	return (_rF.*CREATE)(_s.at(0), _s.at(1));
	}
	virtual std::size_t getWeight(void) const override
	{	return 10;
	}
};
struct max:expression
{	max(const ptr&_p0, const ptr&_p1)
		:expression(
			children({_p0, _p1})
		)
	{
	}
	virtual double evaluate(const double *const _p) const override
	{	return std::max(m_sChildren.at(0)->evaluate(_p), m_sChildren.at(1)->evaluate(_p));
	}
	virtual llvm::Value* generateCode(
		const expression*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP
	) const override
	{	// Create the function prototype for std::sqrt
		using namespace llvm;
			// Call the tan function
		Type* const doubleType = Type::getDoubleTy(context);
		return builder.CreateCall(
			Intrinsic::getDeclaration(M, Intrinsic::maxnum, {doubleType}),
			{	m_sChildren.at(0)->generateCodeW(_pRoot, context, builder, M, _pP),
				m_sChildren.at(1)->generateCodeW(_pRoot, context, builder, M, _pP)
			},
			"maxnum"
		);
	}
	virtual ptr recreateFromChildren(children _s, const factory&_rF) const override
	{	return _rF.max(_s.at(0), _s.at(1));
	}
	virtual std::size_t getWeight(void) const override
	{	return 2;
	}
};
struct min:expression
{	min(const ptr&_p0, const ptr&_p1)
		:expression(
			children({_p0, _p1})
		)
	{
	}
	virtual double evaluate(const double *const _p) const override
	{	return std::min(m_sChildren.at(0)->evaluate(_p), m_sChildren.at(1)->evaluate(_p));
	}
	virtual llvm::Value* generateCode(
		const expression*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP
	) const override
	{	// Create the function prototype for std::sqrt
		using namespace llvm;
		//Value* const Input = ConstantFP::get(llvm::Type::getDoubleTy(context), m_sChildren.at(0)->generateCodeW(context, builder, M));
		// Call the tan function
		Type* const doubleType = Type::getDoubleTy(context);
		return builder.CreateCall(
			Intrinsic::getDeclaration(M, Intrinsic::minnum, {doubleType}),
			{	m_sChildren.at(0)->generateCodeW(_pRoot, context, builder, M, _pP),
				m_sChildren.at(1)->generateCodeW(_pRoot, context, builder, M, _pP),
			},
			"minnum"
		);
	}
	virtual ptr recreateFromChildren(children _s, const factory&_rF) const override
	{	return _rF.min(_s.at(0), _s.at(1));
	}
	virtual std::size_t getWeight(void) const override
	{	return 2;
	}
};
struct multiplication:expression
{	multiplication(const ptr&_p0, const ptr&_p1)
		:expression(
			children({_p0, _p1})
		)
	{
	}
	virtual double evaluate(const double *const _p) const override
	{	return m_sChildren.at(0)->evaluate(_p) * m_sChildren.at(1)->evaluate(_p);
	}
	virtual llvm::Value* generateCode(
		const expression*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP
	) const override
	{	return builder.CreateFMul(
			m_sChildren.at(0)->generateCodeW(_pRoot, context, builder, M, _pP),
			m_sChildren.at(1)->generateCodeW(_pRoot, context, builder, M, _pP),
			"plus"
		);
	}
	virtual ptr recreateFromChildren(children _s, const factory&_rF) const override
	{	return _rF.multiplication(_s.at(0), _s.at(1));
	}
	virtual std::size_t getWeight(void) const override
	{	return 4;
	}
};
struct division:expression
{	division(const ptr&_p0, const ptr&_p1)
		:expression(
			children({_p0, _p1})
		)
	{
	}
	virtual double evaluate(const double *const _p) const override
	{	return m_sChildren.at(0)->evaluate(_p) / m_sChildren.at(1)->evaluate(_p);
	}
	virtual llvm::Value* generateCode(
		const expression*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP
	) const override
	{	return builder.CreateFDiv(
			m_sChildren.at(0)->generateCodeW(_pRoot, context, builder, M, _pP),
			m_sChildren.at(1)->generateCodeW(_pRoot, context, builder, M, _pP),
			"plus"
		);
	}
	virtual ptr recreateFromChildren(children _s, const factory&_rF) const override
	{	return _rF.division(_s.at(0), _s.at(1));
	}
	virtual std::size_t getWeight(void) const override
	{	return 10;
	}
};
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
	virtual llvm::Value* generateCode(
		const expression*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP
	) const override
	{	return builder.CreateFAdd(
			m_sChildren.at(0)->generateCodeW(_pRoot, context, builder, M, _pP),
			m_sChildren.at(1)->generateCodeW(_pRoot, context, builder, M, _pP),
			"plus"
		);
	}
	virtual ptr recreateFromChildren(children _s, const factory&_rF) const override
	{	return _rF.addition(_s.at(0), _s.at(1));
	}
	virtual std::size_t getWeight(void) const override
	{	return 2;
	}
};
struct subtraction:expression
{	subtraction(const ptr&_p0, const ptr&_p1)
		:expression(
			children({_p0, _p1})
		)
	{
	}
	virtual double evaluate(const double *const _p) const override
	{	return m_sChildren.at(0)->evaluate(_p) - m_sChildren.at(1)->evaluate(_p);
	}
	virtual llvm::Value* generateCode(
		const expression*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP
	) const override
	{	return builder.CreateFSub(
			m_sChildren.at(0)->generateCodeW(_pRoot, context, builder, M, _pP),
			m_sChildren.at(1)->generateCodeW(_pRoot, context, builder, M, _pP),
			"plus"
		);
	}
	virtual ptr recreateFromChildren(children _s, const factory&_rF) const override
	{	return _rF.subtraction(_s.at(0), _s.at(1));
	}
	virtual std::size_t getWeight(void) const override
	{	return 2;
	}
};
template<double(*PMATH)(double), llvm::Intrinsic::ID EID, factory::exprPtr (factory::*CREATE)(const factory::exprPtr&) const>
struct unary:expression
{	unary(const ptr&_p)
		:expression(
			children({_p})
		)
	{
	}
	virtual double evaluate(const double *const _p) const override
	{	return PMATH(m_sChildren.at(0)->evaluate(_p));
	}
	virtual llvm::Value* generateCode(
		const expression*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP
	) const override
	{	// Create the function prototype for std::sqrt
		using namespace llvm;
		return builder.CreateCall(
			Intrinsic::getDeclaration(M, EID, builder.getDoubleTy()),
			m_sChildren.at(0)->generateCodeW(_pRoot, context, builder, M, _pP)
		);
	}
	virtual ptr recreateFromChildren(children _s, const factory&_rF) const override
	{	return (_rF.*CREATE)(_s.at(0));
	}
	virtual std::size_t getWeight(void) const override
	{	return 10;
	}
};
template<double(*PMATH)(double), const char AC[], factory::exprPtr (factory::*CREATE)(const factory::exprPtr&) const>
struct unaryF:expression
{	unaryF(const ptr&_p)
		:expression(
			children({_p})
		)
	{
	}
	virtual double evaluate(const double *const _p) const override
	{	return PMATH(m_sChildren.at(0)->evaluate(_p));
	}
	virtual llvm::Value* generateCode(
		const expression*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP
	) const override
	{		// Create the function prototype for std::sqrt
		using namespace llvm;
		//Value* const Input = ConstantFP::get(llvm::Type::getDoubleTy(context), m_sChildren.at(0)->generateCodeW(context, builder, M));
		// Call the tan function
		return builder.CreateCall(
			M->getOrInsertFunction(
				AC,
				FunctionType::get(
					llvm::Type::getDoubleTy(context), // Return type: double
					{llvm::Type::getDoubleTy(context)}, // Argument type: double
					false // Not variadic
				)
			),
			{	m_sChildren.at(0)->generateCodeW(_pRoot, context, builder, M, _pP)
			}
		);
	}
	virtual ptr recreateFromChildren(children _s, const factory&_rF) const override
	{	return (_rF.*CREATE)(_s.at(0));
	}
	virtual std::size_t getWeight(void) const override
	{	return 10;
	}
};
struct negation:expression
{	negation(const ptr&_p)
		:expression(
			children({_p})
		)
	{
	}
	virtual double evaluate(const double *const _p) const override
	{	return -m_sChildren.at(0)->evaluate(_p);
	}
	virtual llvm::Value* generateCode(
		const expression*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP
	) const override
	{	// Create the function prototype for std::sqrt
		using namespace llvm;
		llvm::Type* doubleType = Type::getDoubleTy(M->getContext());
		return builder.CreateFSub(
			ConstantFP::get(doubleType, 0.0),
			m_sChildren.at(0)->generateCodeW(_pRoot, context, builder, M, _pP)
		);
	}
	virtual ptr recreateFromChildren(children _s, const factory&_rF) const override
	{	return _rF.negation(_s.at(0));
	}
	virtual std::size_t getWeight(void) const override
	{	return 1;
	}
};
struct parameter:expression
{	const std::size_t m_i;
	parameter(const std::size_t _i)
		:
		m_i(_i)	
	{
	}
	virtual double evaluate(const double *const _p) const override
	{	return _p[m_i];
	}
	virtual bool isSmaller(const expression&_r) const override
	{	const auto &r = dynamic_cast<const parameter&>(_r);
		if (m_i < r.m_i)
			return true;
		else
			return false;
	}
	virtual llvm::Value* generateCode(
		const expression*const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
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
	virtual ptr recreateFromChildren(children, const factory&) const override
	{	return this;
	}
	virtual std::size_t getWeight(void) const override
	{	return 1;
	}
};
struct factoryImpl:factory
{	virtual exprPtr realConstant(const double _d) const override
	{	return theExpressionEngine::expression::create<dynamic_cast_implementation<theExpressionEngine::realConstant> >(*this, _d);
	}
	virtual exprPtr parameter(const std::size_t _i) const override
	{	return theExpressionEngine::expression::create<theExpressionEngine::parameter>(*this, _i);
	}
#define __COMMA__
#define __COMMA2__
#define __MAKE_ENTRY2__(a)\
	static constexpr const char s_ac_##a[] = #a;\
	virtual exprPtr a(const exprPtr&_p) const override\
	{	return theExpressionEngine::expression::create<theExpressionEngine::unaryF<std::a, s_ac_##a, &factory::a> >(*this, _p);\
	}
#define __MAKE_ENTRY__(a)\
	virtual exprPtr a(const exprPtr&_p) const override\
	{	return theExpressionEngine::expression::create<theExpressionEngine::unary<std::a, llvm::Intrinsic::a, &factory::a> >(*this, _p);\
	}
#include "unary.h"
	virtual exprPtr pow(const exprPtr&_p0, const exprPtr&_p1) const override
	{	static const char ac[] = "pow";
		return theExpressionEngine::expression::create<theExpressionEngine::binary<ac, &std::pow, &factory::pow> >(*this, _p0, _p1);
	}
	virtual exprPtr fmod(const exprPtr&_p0, const exprPtr&_p1) const override
	{	static const char ac[] = "fmod";
		return theExpressionEngine::expression::create<theExpressionEngine::binary<ac, &std::fmod, &factory::fmod> >(*this, _p0, _p1);
	}
	virtual exprPtr hypot(const exprPtr&_p0, const exprPtr&_p1) const override
	{	static const char ac[] = "hypot";
		return theExpressionEngine::expression::create<theExpressionEngine::binary<ac, &std::hypot, &factory::hypot> >(*this, _p0, _p1);
	}
	virtual exprPtr atan2(const exprPtr&_p0, const exprPtr&_p1) const override
	{	static const char ac[] = "atan2";
		return theExpressionEngine::expression::create<theExpressionEngine::binary<ac, &std::atan2, &factory::atan2> >(*this, _p0, _p1);
	}
	virtual exprPtr max(const exprPtr&_p0, const exprPtr&_p1) const override
	{	return theExpressionEngine::expression::create<theExpressionEngine::max>(*this, _p0, _p1);
	}
	virtual exprPtr min(const exprPtr&_p0, const exprPtr&_p1) const override
	{	return theExpressionEngine::expression::create<theExpressionEngine::min>(*this, _p0, _p1);
	}
	virtual exprPtr addition(const exprPtr&_p0, const exprPtr&_p1) const override
	{	if (const auto p = _p0->getPtr(dummy<theExpressionEngine::realConstant>()); p && p->m_d == 0)
			return _p1;
		else
		if (const auto p = _p1->getPtr(dummy<theExpressionEngine::realConstant>()); p && p->m_d == 0)
			return _p0;
		else
			return theExpressionEngine::expression::create<theExpressionEngine::addition>(*this, _p0, _p1);
	}
	virtual exprPtr subtraction(const exprPtr&_p0, const exprPtr&_p1) const override
	{	if (const auto p = _p0->getPtr(dummy<theExpressionEngine::realConstant>()); p && p->m_d == 0)
			return negation(_p1);
		else
		if (const auto p = _p1->getPtr(dummy<theExpressionEngine::realConstant>()); p && p->m_d == 0)
			return _p0;
		else
		return theExpressionEngine::expression::create<theExpressionEngine::subtraction>(*this, _p0, _p1);
	}
	virtual exprPtr multiplication(const exprPtr&_p0, const exprPtr&_p1) const override
	{	if (const auto p = _p0->getPtr(dummy<theExpressionEngine::realConstant>()))
			if (p->m_d == 1)
				return _p1;
			else
			if (p->m_d == 0)
				return _p0;
		if (const auto p = _p1->getPtr(dummy<theExpressionEngine::realConstant>()))
			if (p->m_d == 1)
				return _p0;
			else
			if (p->m_d == 0)
				return _p1;
		return theExpressionEngine::expression::create<theExpressionEngine::multiplication>(*this, _p0, _p1);
	}
	virtual exprPtr division(const exprPtr&_p0, const exprPtr&_p1) const override
	{	if (const auto p = _p0->getPtr(dummy<theExpressionEngine::realConstant>()); p && p->m_d == 0)
			return _p0;
		else
		if (const auto p = _p1->getPtr(dummy<theExpressionEngine::realConstant>()); p && p->m_d == 1)
			return _p0;
		else
		return theExpressionEngine::expression::create<theExpressionEngine::division>(*this, _p0, _p1);
	}
	virtual exprPtr negation(const exprPtr&_r) const override
	{	return theExpressionEngine::expression::create<theExpressionEngine::negation>(*this, _r);
	}
	virtual exprPtr parse(const char *const _r, const name2int&_rP) const override
	{	yyscan_t scanner;
		yylex_init(&scanner);
		auto pBuffer = yy_scan_string(_r, scanner);
		exprPtr p;
		yy::parser my_parser(scanner, &p, shared_from_this(), _rP);
		my_parser.parse();
		//yyparse(scanner, &p, this, _rP);
		yy_delete_buffer(pBuffer, scanner);
		yylex_destroy(scanner);
		return p;
	}
};
}
factory::ptr factory::getFactory(void)
{	static const factory::ptr s(std::make_shared<const factoryImpl>());
	return s;
}
}
