#pragma once
#include <memory>
#include <set>

namespace theExpressionEngine
{
struct realConstant;
struct factory;
struct expression;
std::shared_ptr<const expression> collapse(const expression&, const factory&);
template<typename T>
struct unique:T
{	template<typename ...REST>
	unique(REST&&..._r)
		:T(std::forward<REST>(_r)...)
	{
	}
	~unique(void)
	{	getSet().erase(this);
	}
	struct compare
	{	bool operator()(const unique*const _p0, const unique*const _p1) const
		{	return *_p0 < *_p1;
		}
	};
	static std::set<const unique<T>*, compare> &getSet(void)
	{	static std::set<const unique<T>*, compare> s_sSet;
		return s_sSet;
	}
	template<typename ...REST>
	static typename T::ptr create(const factory&_rF, REST&&..._r)
	{	return (*getSet().insert(
			static_cast<const unique<T>*>(collapse(*std::make_shared<const unique<T> >(std::forward<REST>(_r)...), _rF).get())
		).first)->shared_from_this();
	}
};
}
