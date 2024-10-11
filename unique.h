#pragma once
#include <memory>
#include <set>

namespace theExpessionEngine
{
template<typename T>
struct unique:T
{	template<typename ...REST>
	unique(REST&&..._r)
		:T(std::forward<REST>(_r)...)
	{
	}
	struct compare
	{	bool operator()(const unique*const _p0, const unique*const _p1) const
		{	return *_p0 < *_p1;
		}
	};
	template<typename ...REST>
	static typename T::ptr create(REST&&..._r)
	{	static std::set<const unique<T>*, compare> s_sSet;
		return (*s_sSet.insert(
			std::make_shared<const unique>(std::forward<REST>(_r)...).get()
		).first)->shared_from_this();
	}
};
}
