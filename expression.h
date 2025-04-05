#pragma once
#include <memory>
#include <vector>
#include <map>
#include <any>
#include <array>
#include <list>
//#include <string>
//#include <variant>
//#include <map>
#include "dynamic_cast.h"
//#include "environment.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
//#include <llvm/IR/Module.h>
//#include <llvm/IR/Constants.h>


namespace theExpressionEngine
{
struct realConstant;
struct factory;
//struct type;
//struct environment;
//struct realConstant;
/// the base class of all expression types
/// immutable
/// no two with the same content are guaranteed to exist
struct expression:std::enable_shared_from_this<const expression>, dynamic_cast_interface<realConstant>
{	typedef std::shared_ptr<const expression> ptr;
	typedef std::vector<ptr> children;
	enum enumAttachedData
	{	eLLVMValuePtr,
		eLLVMdata,
		__NUMBER_OF_DATA__
	};
	typedef std::array<std::any, __NUMBER_OF_DATA__> ARRAY;
	typedef std::map<const expression*, ARRAY> MAP;
	const children m_sChildren;
	expression(
		children&&_rChildren = {}
	);
	virtual ~expression(void) = default;
		/// this compares everything contained in expression
		/// and calls isSmaller() if there is no difference
	bool operator<(const expression&_r) const;
		/// this is to be implemented for all derived classes containing data
		/// the type of the LHS and RHS is guaranteed to be identical
	virtual bool isSmaller(const expression&) const;
	virtual double evaluate(const double *const) const = 0;
	double evaluateLLVM(const double *const) const;
	llvm::Value *generateCodeW(
		const expression *const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP
	) const
	{	const auto sInsert = m_sAttachedData.emplace(_pRoot, ARRAY());
		if (sInsert.second)
			addOnDestroy(
				[_pRoot, this](void)
				{	m_sAttachedData.erase(_pRoot);
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
	virtual void onDestroy(void) const;
	ptr collapse(const factory&_rF) const;
	private:
	mutable MAP m_sAttachedData;
	virtual llvm::Value* generateCode(
		const expression *const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP
	) const = 0;
	typedef std::function<void(void)> onDestroyFunctor;
	mutable std::list<onDestroyFunctor> m_sOnDestroyList;
	virtual void addOnDestroy(onDestroyFunctor _s) const;
	//mutable llvm::Value *m_pValue = nullptr;
};
expression::ptr collapse(const expression&, const factory&);
}
