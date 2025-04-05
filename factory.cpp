#include "factory.h"
#include "expression.h"
#include "unique.h"
#include "onDestroy.h"
#include "flex.cpp"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <stdexcept>
#include <regex>
#include <iostream>

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
};
namespace
{
template<const char ac[], double(*PFCT)(double, double)>
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
};
template<double(*PMATH)(double), llvm::Intrinsic::ID EID>
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
};
template<double(*PMATH)(double), const char AC[]>
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
};
struct factoryImpl:factory
{	virtual exprPtr realConstant(const double _d) const override
	{	return unique<onDestroy<dynamic_cast_implementation<theExpressionEngine::realConstant> > >::create(_d);
	}
	virtual exprPtr parameter(const std::size_t _i) const override
	{	return unique<onDestroy<theExpressionEngine::parameter> >::create(_i);
	}
#define __COMMA__
#define __COMMA2__
#define __MAKE_ENTRY2__(a)\
	static constexpr const char s_ac_##a[] = #a;\
	virtual exprPtr a(const exprPtr&_p) const override\
	{	return unique<onDestroy<theExpressionEngine::unaryF<std::a, s_ac_##a> > >::create(_p);\
	}
#define __MAKE_ENTRY__(a)\
	virtual exprPtr a(const exprPtr&_p) const override\
	{	return unique<onDestroy<theExpressionEngine::unary<std::a, llvm::Intrinsic::a> > >::create(_p);\
	}
#include "unary.h"
	virtual exprPtr pow(const exprPtr&_p0, const exprPtr&_p1) const override
	{	static const char ac[] = "pow";
		return unique<onDestroy<theExpressionEngine::binary<ac, &std::pow> > >::create(_p0, _p1);
	}
	virtual exprPtr fmod(const exprPtr&_p0, const exprPtr&_p1) const override
	{	static const char ac[] = "fmod";
		return unique<onDestroy<theExpressionEngine::binary<ac, &std::fmod> > >::create(_p0, _p1);
	}
	virtual exprPtr hypot(const exprPtr&_p0, const exprPtr&_p1) const override
	{	static const char ac[] = "hypot";
		return unique<onDestroy<theExpressionEngine::binary<ac, &std::hypot> > >::create(_p0, _p1);
	}
	virtual exprPtr atan2(const exprPtr&_p0, const exprPtr&_p1) const override
	{	static const char ac[] = "atan2";
		return unique<onDestroy<theExpressionEngine::binary<ac, &std::atan2> > >::create(_p0, _p1);
	}
	virtual exprPtr max(const exprPtr&_p0, const exprPtr&_p1) const override
	{	return unique<onDestroy<theExpressionEngine::max> >::create(_p0, _p1);
	}
	virtual exprPtr min(const exprPtr&_p0, const exprPtr&_p1) const override
	{	return unique<onDestroy<theExpressionEngine::min> >::create(_p0, _p1);
	}
	virtual exprPtr addition(const exprPtr&_p0, const exprPtr&_p1) const override
	{	return unique<onDestroy<theExpressionEngine::addition> >::create(_p0, _p1);
	}
	virtual exprPtr subtraction(const exprPtr&_p0, const exprPtr&_p1) const override
	{	return unique<onDestroy<theExpressionEngine::subtraction> >::create(_p0, _p1);
	}
	virtual exprPtr multiplication(const exprPtr&_p0, const exprPtr&_p1) const override
	{	return unique<onDestroy<theExpressionEngine::multiplication> >::create(_p0, _p1);
	}
	virtual exprPtr division(const exprPtr&_p0, const exprPtr&_p1) const override
	{	return unique<onDestroy<theExpressionEngine::division> >::create(_p0, _p1);
	}
	virtual exprPtr negation(const exprPtr&_r) const override
	{	return unique<onDestroy<theExpressionEngine::negation> >::create(_r);
	}
	virtual exprPtr parse(const char *const _r) const override
	{	yyscan_t scanner;
		yylex_init(&scanner);
		auto pBuffer = yy_scan_string(_r, scanner);
		exprPtr p;
		yyparse(scanner, &p, shared_from_this());
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
