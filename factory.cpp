#include "factory.h"
#include "environment.h"
#include "expression.h"
#include "type.h"
#include "unique.h"
#include "onDestroy.h"
#include <stdexcept>
#include "dynamic_cast.h"

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
	virtual value evaluateThis(environment&) const override
	{	return m_d;
	}
	virtual llvm::Value* generateCode(llvm::LLVMContext& context, llvm::IRBuilder<>& builder) const override
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
	virtual value evaluateThis(environment&_r) const override
	{	switch (m_sType->m_eType)
		{	default:
				throw std::logic_error("Invalid type for plus operator!");
			case type::eReal:
				return std::get<double>(m_sChildren.at(0)->evaluate(_r)) + std::get<double>(m_sChildren.at(1)->evaluate(_r));
		}
	}
	virtual llvm::Value* generateCode(llvm::LLVMContext& context, llvm::IRBuilder<>& builder) const override
	{	return builder.CreateFAdd(
			m_sChildren.at(0)->generateCodeW(context, builder),
			m_sChildren.at(1)->generateCodeW(context, builder),
			"plus"
		);
	}
};
struct factoryImpl:factory
{	virtual exprPtr realConstant(const double _d) const override
	{	return unique<onDestroy<dynamic_cast_implementation<theExpessionEngine::realConstant> > >::create(_d);
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
