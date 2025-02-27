#pragma once
#include <memory>
#include <vector>
//#include <string>
//#include <variant>
//#include <map>
//#include "dynamic_cast.h"
//#include "environment.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
//#include <llvm/IR/Module.h>
//#include <llvm/IR/Constants.h>


namespace theExpessionEngine
{
//struct type;
//struct environment;
//struct realConstant;
/// the base class of all expression types
/// immutable
/// no two with the same content are guaranteed to exist
struct expression:std::enable_shared_from_this<const expression>//, dynamic_cast_interface<realConstant>
{	typedef std::shared_ptr<const expression> ptr;
	typedef std::vector<ptr> children;
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
	llvm::Value *generateCodeW(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module *const M) const
	{	if (!m_pValue)
			m_pValue = generateCode(context, builder, M);
		return m_pValue;
	}
	virtual void onDestroy(void) const;
	private:
	virtual llvm::Value* generateCode(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module *const M) const = 0;
	mutable llvm::Value *m_pValue = nullptr;
};
}
