#pragma once
#include <memory>
#include <set>
#include <mutex>
#include <atomic>
#include <optional>
#include <type_traits>
#include <boost/intrusive_ptr.hpp>
#include "onDestroy.h"

namespace theExpressionEngine
{
struct realConstant;
struct factory;
struct expression;
boost::intrusive_ptr<const expression> collapse(const expression&, const factory&);
#if 1
	/// a nullmutex in case of a single threaded environment
class NullMutex
{	public:
	void lock(void)
	{
	}
	void unlock(void)
	{
	}
	bool try_lock(void)
	{	return true;
	}
};
	/// the template class to implement the BASE class
	/// look for example usage below
	/// default is multithreaded using std::recursive_mutex
template<typename T, bool BTHREADED = true>
class unique
{	typedef typename std::conditional<
		BTHREADED,
		std::recursive_mutex,
		NullMutex
	>::type MUTEX;
		/// does not need to be std::atomic as protected by a mutex
	mutable std::size_t m_sRefCount;
	public:
	unique(void)
		:
		m_sRefCount(std::size_t())
	{	//std::cout << typeid(*this).name() << ":" << m_iId << std::endl;
	}
		/// for implementing the static set of registered pointers
	struct compare
	{	bool operator()(const T*const _p0, const T*const _p1) const
		{	return *_p0 < *_p1;
		}
	};
	private:
	typedef std::set<const T*, compare> SET;
	typedef std::pair<SET, MUTEX> setAndMutex;
	static setAndMutex&getSet(void)
	{	static setAndMutex s;
		return s;
	}
	public:
	static MUTEX&getMutex(void)
	{	return getSet().second;
	}
	template<typename DERIVED, typename ...ARGS>
	static boost::intrusive_ptr<const T> _create(ARGS&&..._r)
	{	const auto s = boost::intrusive_ptr<const T>(new onDestroy<DERIVED>(std::forward<ARGS>(_r)...));
		std::lock_guard<MUTEX> sLock(getSet().second);
		return *getSet().first.insert(s.get()).first;
	}
	template<typename DERIVED, typename ...REST>
	static boost::intrusive_ptr<const T> create(const factory&_rF, REST&&..._r)
	{	return collapse(*_create<DERIVED>(std::forward<REST>(_r)...), _rF);
	}
		/// the factory method
	private:
		/// called by boost::intrusive_ptr<const T>
		/// called by boost::intrusive_ptr<const T>
	friend void intrusive_ptr_add_ref(const T* const _p) noexcept
	{	std::lock_guard<MUTEX> sLock(getSet().second);
		++_p->m_sRefCount;
	}
		/// called by boost::intrusive_ptr<const T>
	friend void intrusive_ptr_release(const T* const _p) noexcept
	{	std::optional<std::lock_guard<MUTEX> > sLock(getSet().second);
		if (!--_p->m_sRefCount)
		{	getSet().first.erase(_p);
			sLock.reset();
			delete _p;
		}
	}
};

#else
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
#endif
}
