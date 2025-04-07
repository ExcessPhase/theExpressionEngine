#pragma once
#include <memory>
#include <set>
#include <mutex>

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
	{	//std::cout << typeid(*this).name() << ":" << m_iId << std::endl;
	}
	unique(void) = delete;
	unique(const unique&) = delete;
	unique(unique&&) = delete;
	unique&operator=(const unique&) = delete;
	unique&operator=(unique&&) = delete;
	virtual ~unique(void)
	{	//std::cout << "start of ~" << typeid(*this).name() << ":" << m_iId << std::endl;
		std::lock_guard<std::recursive_mutex> sLock(getMutex());
		auto &rSet = getSet();
		const auto pFind = rSet.find(this);
		if (pFind != rSet.end() && *pFind == this)
			rSet.erase(pFind);
		//std::cout << "end of ~" << typeid(*this).name() << ":" << m_iId << std::endl;
	}
	struct compare
	{	bool operator()(const unique<T>*const _p0, const unique<T>*const _p1) const
		{	return *_p0 < *_p1;
		}
	};
	static auto &getMutex(void)
	{	static std::recursive_mutex s;
		return s;
	}
	static auto &getSet(void)
	{	static std::set<const unique<T>*, compare> s_sSet;
		return s_sSet;
	}
	template<typename ...REST>
	static typename T::ptr _create(const factory&_rF, REST&&..._r)
	{	const auto s = std::make_shared<const unique<T> >(std::forward<REST>(_r)...);
		std::lock_guard<std::recursive_mutex> sLock(getMutex());
		return (*getSet().insert(s.get()).first)->shared_from_this();
	}
	template<typename ...REST>
	static typename T::ptr create(const factory&_rF, REST&&..._r)
	{	return collapse(*_create(_rF, std::forward<REST>(_r)...), _rF);
	}
};
}
