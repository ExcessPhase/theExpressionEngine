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
#include <iostream>

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
	virtual llvm::Value* generateCode(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module *const M) const override
	{	return builder.CreateFMul(
			m_sChildren.at(0)->generateCodeW(context, builder, M),
			m_sChildren.at(1)->generateCodeW(context, builder, M),
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
	virtual llvm::Value* generateCode(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module *const M) const override
	{	return builder.CreateFDiv(
			m_sChildren.at(0)->generateCodeW(context, builder, M),
			m_sChildren.at(1)->generateCodeW(context, builder, M),
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
	virtual llvm::Value* generateCode(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module *const M) const override
	{	return builder.CreateFAdd(
			m_sChildren.at(0)->generateCodeW(context, builder, M),
			m_sChildren.at(1)->generateCodeW(context, builder, M),
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
	virtual llvm::Value* generateCode(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module *const M) const override
	{	return builder.CreateFSub(
			m_sChildren.at(0)->generateCodeW(context, builder, M),
			m_sChildren.at(1)->generateCodeW(context, builder, M),
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
	virtual llvm::Value* generateCode(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module *const M) const override
	{	    // Create the function prototype for std::sqrt
		using namespace llvm;
	    	return builder.CreateCall(
			Intrinsic::getDeclaration(M, EID, builder.getDoubleTy()),
			m_sChildren.at(0)->generateCodeW(context, builder, M)
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
	virtual llvm::Value* generateCode(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module *const M) const override
	{	    // Create the function prototype for std::sqrt
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
			{	m_sChildren.at(0)->generateCodeW(context, builder, M)
			}
		);
	}
};
struct factoryImpl:factory
{	virtual exprPtr realConstant(const double _d) const override
	{	return unique<onDestroy<theExpessionEngine::realConstant> >::create(_d);
	}
#define __COMMA__
#define __COMMA2__
#define __MAKE_ENTRY2__(a)\
	static constexpr const char s_ac_##a[] = #a;\
	virtual exprPtr a(const exprPtr&_p) const override\
	{	return unique<onDestroy<theExpessionEngine::unaryF<std::a, s_ac_##a> > >::create(_p);\
	}
#define __MAKE_ENTRY__(a)\
	virtual exprPtr a(const exprPtr&_p) const override\
	{	return unique<onDestroy<theExpessionEngine::unary<std::a, llvm::Intrinsic::a> > >::create(_p);\
	}
#include "unary.h"
	virtual exprPtr addition(const exprPtr&_p0, const exprPtr&_p1) const override
	{	return unique<onDestroy<theExpessionEngine::addition> >::create(_p0, _p1);
	}
	virtual exprPtr subtraction(const exprPtr&_p0, const exprPtr&_p1) const override
	{	return unique<onDestroy<theExpessionEngine::subtraction> >::create(_p0, _p1);
	}
	virtual exprPtr multiplication(const exprPtr&_p0, const exprPtr&_p1) const override
	{	return unique<onDestroy<theExpessionEngine::multiplication> >::create(_p0, _p1);
	}
	virtual exprPtr division(const exprPtr&_p0, const exprPtr&_p1) const override
	{	return unique<onDestroy<theExpessionEngine::division> >::create(_p0, _p1);
	}
	exprPtr parseParenthesis(std::string::const_iterator &_p, const std::string::const_iterator &_pEnd) const
	{	if (_p == _pEnd)
			return nullptr;
		const auto pIt = _p;
		if (*_p == '(')
		{	++_p;	
			if (const auto p = parseAddSub(_p, _pEnd))
				if (*_p == ')')
				{	++_p;
					return p;
				}
				else
				{	_p = pIt;
					return nullptr;
				}
			else
			{	_p = pIt;
				return nullptr;
			}
		}
		else
		{	_p = pIt;
			return nullptr;
		}
	}
	enum enumUnary
	{
#define __COMMA__ ,
#define __MAKE_ENTRY__(a) e_##a
#define __COMMA2__ ,
#define __MAKE_ENTRY2__(a) __MAKE_ENTRY__(a)
#include "unary.h"
	};
	typedef std::map<std::string, enumUnary> Name2Enum;
	static std::string getRegex(const Name2Enum&_r)
	{	std::string s;
		for (const auto &r : _r)
			if (s.empty())
				s = r.first;
			else
				s += "|" + r.first;
		return "^(\\b(" + s + ")\\b)[ \\t]*\\(";
	}
	exprPtr parseSqrt(std::string::const_iterator &_p, const std::string::const_iterator &_pEnd) const
	{	static const Name2Enum s_sName2Create = {
			//{"sqrt", e_sqrt},
#define __COMMA__ ,
#define __MAKE_ENTRY__(a) {#a, e_##a}
#define __COMMA2__ ,
#define __MAKE_ENTRY2__(a) __MAKE_ENTRY__(a)
#include "unary.h"
		};
		//static const std::regex sRegex(R"(^(\bsqrt\b)[ \t]*\()");
		static const std::regex sRegex(getRegex(s_sName2Create));
		if (_p == _pEnd)
			return nullptr;
		const auto pIt = _p;
		std::smatch sMatch;

		if (std::regex_search(_p, _pEnd, sMatch, sRegex))
		{	_p = sMatch[0].second - 1;
			if (const auto p = parseParenthesis(_p, _pEnd))
				switch (s_sName2Create.at(sMatch[1]))
				{	default:
						throw std::logic_error("Invalid enum!");
					//case e_sqrt:
						//return sqrt(p);
#define __COMMA__
#define __MAKE_ENTRY__(a) case e_##a:\
				return a(p);
#define __COMMA2__
#define __MAKE_ENTRY2__(a) __MAKE_ENTRY__(a)
#include "unary.h"
				}
			else
			{	_p = pIt;
				return nullptr;
			}
		}
		else
			return nullptr;
	}
	exprPtr parseReal(std::string::const_iterator &_p, const std::string::const_iterator &_pEnd) const
	{	if (_p == _pEnd)
			return nullptr;
		const std::string s(_p, _pEnd);
		std::size_t i = 0;
		try
		{	const auto d = std::stod(s, &i);
			_p += i;
			return realConstant(d);
		} catch (...)
		{	return nullptr;
		}
	}
	exprPtr parseSingle(std::string::const_iterator &_p, const std::string::const_iterator &_pEnd) const
	{	if (_p == _pEnd)
			return nullptr;
		if (const auto p = parseSqrt(_p, _pEnd))
			return p;
		else
			if (const auto p = parseReal(_p, _pEnd))
				return p;
			else
				return parseParenthesis(_p, _pEnd);
	}
	typedef exprPtr (factoryImpl::*parseMethod)(std::string::const_iterator &, const std::string::const_iterator &) const;
	exprPtr eatSpaceAndTry(
		std::string::const_iterator &_p,
		const std::string::const_iterator &_pEnd,
		const parseMethod _pParse
	) const
	{	static const std::regex sRegex(R"([ \t]*)");
		const auto pIt = _p;
		std::smatch sMatch;

		if (std::regex_match(_p, _pEnd, sMatch, sRegex))
		{	_p = sMatch[0].second;
			if (const auto p = (this->*_pParse)(_p, _pEnd))
				return p;
			_p = pIt;
			return nullptr;
		}
		else
			return (this->*_pParse)(_p, _pEnd);
	}
	exprPtr parseMultDiv(std::string::const_iterator &_p, const std::string::const_iterator &_pEnd) const
	{	if (_p == _pEnd)
			return nullptr;
		const auto pIt = _p;
		if (const auto pL = parseSingle(_p, _pEnd))
			switch (*_p)
			{	default:
					return pL;
				case '*':
					++_p;
					if (const auto p = eatSpaceAndTry(_p, _pEnd, &factoryImpl::parseSingle))
						return multiplication(pL, p);
					_p = pIt;
					return nullptr;
				case '/':
					++_p;
					if (const auto p = eatSpaceAndTry(_p, _pEnd, &factoryImpl::parseSingle))
						return division(pL, p);
					_p = pIt;
					return nullptr;
			}
		else
			return nullptr;
	}
	exprPtr parseAddSub(std::string::const_iterator &_p, const std::string::const_iterator &_pEnd) const
	{	if (_p == _pEnd)
			return nullptr;
		const auto pIt = _p;
		if (const auto pL = parseMultDiv(_p, _pEnd))
			switch (*_p)
			{	default:
					return pL;
				case '+':
					++_p;
					if (const auto p = eatSpaceAndTry(_p, _pEnd, &factoryImpl::parseMultDiv))
						return addition(pL, p);
					_p = pIt;
					return nullptr;
				case '-':
					++_p;
					if (const auto p = eatSpaceAndTry(_p, _pEnd, &factoryImpl::parseMultDiv))
						return subtraction(pL, p);
					_p = pIt;
					return nullptr;
			}
		else
			return nullptr;
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
