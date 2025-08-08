#pragma once
#include <memory>
#include <vector>
#include <map>
#include <unordered_map>
#include <any>
#include <array>
#include <list>
#include <stack>
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
struct intConstant;
template<bool BTHREADED>
struct factory;
//struct type;
//struct environment;
//struct realConstant;
template<bool BTHREADED>
struct llvmData;
template<bool BTHREADED>
struct expression;
/// the base class of all expression types
/// immutable
/// no two with the same content are guaranteed to exist
template<bool BTHREADED>
struct expression:dynamic_cast_interface<realConstant<BTHREADED> >, dynamic_cast_interface<intConstant<BTHREADED> >, unique<expression<BTHREADED>, BTHREADED>//, hasId<expression>
{	using typename unique<expression<BTHREADED>, BTHREADED>::MUTEX;
	using dynamic_cast_interface<realConstant<BTHREADED> >::getPtr;
	using dynamic_cast_interface<intConstant<BTHREADED> >::getPtr;
	//using unique<expression<BTHREADED>, BTHREADED>::getMutex;
	typedef boost::intrusive_ptr<const expression<BTHREADED> > ptr;
	typedef factory<BTHREADED> FACTORY;
	using is_expression_ptr_tag = std::true_type;
	typedef std::vector<ptr> children;
	enum enumAttachedData
	{	eLLVMValuePtr,
		eLLVMdata,
		__NUMBER_OF_DATA__
	};
	enum enumType
	{	eFloatingPoint,
		eInteger
	};
	const enumType m_eType;
	typedef std::array<std::any, __NUMBER_OF_DATA__> ARRAY;
	typedef std::unordered_map<const expression<BTHREADED> *, ARRAY> MAP;
	const children m_sChildren;
	expression(
		children&&_rChildren,
		const enumType _eType
	);
	virtual ~expression(void) = default;
	expression(const expression&) = delete;
	expression(expression&&) = delete;
	expression &operator=(const expression&) = delete;
	expression &operator=(expression&&) = delete;
		/// this compares everything contained in expression
		/// and calls isSmaller() if there is no difference
	bool operator<(const expression<BTHREADED> &_r) const;
		/// this is to be implemented for all derived classes containing data
		/// the type of the LHS and RHS is guaranteed to be identical
	virtual bool isSmaller(const expression<BTHREADED> &) const;
	double evaluateW(const double *const _p0, const int*const _p1, const double*const _p2, const int*const _p3) const
	{	if (m_eType == eInteger)
			return evaluateInt(_p0, _p1, _p2, _p3);
		else
			return evaluate(_p0, _p1, _p2, _p3);
	}
	virtual int evaluateInt(const double *const, const int*const, const double*const, const int*const) const;
	virtual double evaluate(const double *const, const int*const, const double*const, const int*const) const;
	double evaluateLLVM(const double *const, const int*const, const double *const, const int*const) const;
	int evaluateIntLLVM(const double *const, const int*const, const double *const, const int*const) const;
	llvm::Value *generateCodeWF(
		const expression<BTHREADED>  *const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP,
		llvm::Value*const _pIP,
		llvm::Value*const _pT,
		llvm::Value*const _pIT
	) const;
	llvm::Value *generateCodeW(
		const expression<BTHREADED>  *const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP,
		llvm::Value*const _pIP,
		llvm::Value*const _pT,
		llvm::Value*const _pIT
	) const;
	virtual void onDestroy(void) const;
	ptr collapse(const factory<BTHREADED>&_rF) const;
	typedef std::map<ptr, ptr> ptr2ptr;
	virtual ptr recreateFromChildren(children, const factory<BTHREADED>&) const = 0;
	ptr replace(const ptr2ptr&, const factory<BTHREADED>&) const;
	typedef std::function<bool(const expression<BTHREADED>*)> BEFORE;
	void DFS(const BEFORE&_r) const
	{	std::stack<const expression<BTHREADED>*> s;
		s.push(this);
		while (!s.empty())
		{	const auto p = s.top();
			s.pop();
			if (_r(p))
				for (const auto &pC : p->m_sChildren)
					s.push(pC.get());
		}
	}
	virtual std::ostream &print(std::ostream&) const = 0;
	void initializeLLVM(void) const;
	virtual bool isZero(void) const
	{	return false;
	}
	virtual bool isOne(void) const
	{	return false;
	}
	virtual bool isNonZero(void) const
	{	return false;
	}
	private:
	mutable MUTEX m_sMutex;
	mutable MAP m_sAttachedData;
	virtual llvm::Value* generateCode(
		const expression<BTHREADED>  *const _pRoot,
		llvm::LLVMContext& context,
		llvm::IRBuilder<>& builder,
		llvm::Module *const M,
		llvm::Value*const _pP,
		llvm::Value*const _pIP,
		llvm::Value*const _pT,
		llvm::Value*const _pIT
	) const = 0;
	typedef std::function<void(void)> onDestroyFunctor;
	mutable std::list<onDestroyFunctor> m_sOnDestroyList;
	virtual void addOnDestroy(onDestroyFunctor _s) const;
	virtual std::size_t getWeight(void) const = 0;
	std::size_t getWeightW(void) const;
	//mutable llvm::Value *m_pValue = nullptr;
};
template<bool BTHREADED>
struct expressionSet:unique<expressionSet<BTHREADED>, BTHREADED>
{	typedef std::vector<boost::intrusive_ptr<const expression<BTHREADED> > > children;
	//virtual const children&getChildren(void) const = 0;
	virtual ~expressionSet(void) = default;
	virtual bool operator<(const expressionSet<BTHREADED> &_r) const = 0;
	virtual void evaluate(
		std::vector<double>&_rChildren,
		std::vector<int>&_rIntChildren,
		const double *const _pParams,
		const int *const _pIntParams
	) const = 0;
	virtual void evaluateLLVM(
		std::vector<double>&_rChildren,
		std::vector<int>&_rIntChildren,
		const double *const _pParams,
		const int *const _pIntParams
	) const = 0;
	virtual const typename expression<BTHREADED>::children &getChildren(void) const = 0;
	//virtual const typename expression<BTHREADED>::children &getTemps(void) const = 0;
	//virtual std::size_t getTempSize(void) const = 0;
	virtual const std::vector<std::size_t> &getOrder(void) const = 0;
};
template<bool BTHREADED>
boost::intrusive_ptr<const expression<BTHREADED> > collapse(const expression<BTHREADED> &, const factory<BTHREADED>&);
template<bool BTHREADED>
boost::intrusive_ptr<const expressionSet<BTHREADED> > collapse(const expressionSet<BTHREADED> &_r, const factory<BTHREADED>&)
{	return &_r;
}

template<typename T, typename = void>
struct is_expression_ptr : std::false_type {};
template<typename T>
struct is_expression_ptr<T, std::void_t<typename T::element_type::is_expression_ptr_tag>> : std::true_type {};

}
#include "expression.ipp"
