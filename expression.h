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
#include "unique.h"
//#include "environment.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <boost/intrusive_ptr.hpp>
//#include <llvm/IR/Module.h>
//#include <llvm/IR/Constants.h>


namespace theExpressionEngine
{
template<bool BTHREADED>
struct realConstant;
template<bool BTHREADED>
struct factory;
//struct type;
//struct environment;
//struct realConstant;
/// the base class of all expression types
/// immutable
/// no two with the same content are guaranteed to exist
#if 0
template<typename>
struct hasId
{
	static std::size_t s_iNextId;
	const std::size_t m_iId = s_iNextId++;
};
template<typename T>
std::size_t hasId<T>::s_iNextId;
#endif
template<bool BTHREADED>
struct expression:dynamic_cast_interface<realConstant<BTHREADED> >, unique<expression<BTHREADED>, BTHREADED>//, hasId<expression>
{	using typename unique<expression<BTHREADED>, BTHREADED>::MUTEX;
	using unique<expression<BTHREADED>, BTHREADED>::getMutex;
	typedef boost::intrusive_ptr<const expression<BTHREADED> > ptr;
	typedef std::vector<ptr> children;
	enum enumAttachedData
	{	eLLVMValuePtr,
		eLLVMdata,
		__NUMBER_OF_DATA__
	};
	typedef std::array<std::any, __NUMBER_OF_DATA__> ARRAY;
	typedef std::map<const expression<BTHREADED> *, ARRAY> MAP;
	const children m_sChildren;
	expression(
		children&&_rChildren = {}
	);
	virtual ~expression(void) = default;
		/// this compares everything contained in expression
		/// and calls isSmaller() if there is no difference
	bool operator<(const expression<BTHREADED> &_r) const;
		/// this is to be implemented for all derived classes containing data
		/// the type of the LHS and RHS is guaranteed to be identical
	virtual bool isSmaller(const expression<BTHREADED> &) const;
	virtual double evaluate(const double *const) const = 0;
	double evaluateLLVM(const double *const, const expression<BTHREADED> *const) const;
	llvm::Value *generateCodeW(
		const expression<BTHREADED>  *const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP
	) const;
	virtual void onDestroy(void) const;
	ptr collapse(const factory<BTHREADED>&_rF) const;
	typedef std::map<ptr, ptr> ptr2ptr;
	virtual ptr recreateFromChildren(children, const factory<BTHREADED>&) const = 0;
	ptr replace(const ptr2ptr&, const factory<BTHREADED>&) const;
	private:
	mutable MAP m_sAttachedData;
	virtual llvm::Value* generateCode(
		const expression<BTHREADED>  *const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP
	) const = 0;
	typedef std::function<void(void)> onDestroyFunctor;
	mutable std::list<onDestroyFunctor> m_sOnDestroyList;
	virtual void addOnDestroy(onDestroyFunctor _s) const;
	virtual std::size_t getWeight(void) const = 0;
	std::size_t getWeightW(void) const;
	//mutable llvm::Value *m_pValue = nullptr;
};
template<bool BTHREADED>
boost::intrusive_ptr<const expression<BTHREADED> > collapse(const expression<BTHREADED> &, const factory<BTHREADED>&);
}
#include "expression.cpp"
