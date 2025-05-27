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
template<bool BTHREADED>
struct realConstant;
template<bool BTHREADED>
struct factory;
template<bool BTHREADED>
struct expression;
template<bool BTHREADED>
boost::intrusive_ptr<const expression<BTHREADED> > collapse(const expression<BTHREADED> &, const factory<BTHREADED>&);
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
{	public:
	typedef typename std::conditional<
		BTHREADED,
		std::recursive_mutex,
		NullMutex
	>::type MUTEX;
	private:
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
	static boost::intrusive_ptr<const T> create(const factory<BTHREADED>&_rF, REST&&..._r)
	{	return collapse<BTHREADED>(*_create<DERIVED>(std::forward<REST>(_r)...), _rF);
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

}
