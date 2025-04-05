#pragma once
#include <iostream>
namespace theExpressionEngine
{
template<typename>
struct dummy
{
};
template<typename T>
struct dynamic_cast_interface
{	virtual ~dynamic_cast_interface(void) = default;
	virtual const T*getPtr(const dummy<T>&) const
	{	return nullptr;
	}
	virtual T*getPtr(const dummy<T>&)
	{	return nullptr;
	}
	virtual const T&getReference(const dummy<T>&) const
	{	throw std::bad_cast();
	}
	virtual T&getReference(const dummy<T>&)
	{	throw std::bad_cast();
	}
};


template<typename T>
inline T *dynCast(dynamic_cast_interface<T> *const _p)
{	return _p->getPtr(dummy<T>());
}
template<typename T>
inline const T *dynCast(const dynamic_cast_interface<T> *const _p)
{	return _p->getPtr(dummy<T>());
}
template<typename T>
inline T &dynCast(dynamic_cast_interface<T> &_r)
{	return _r.getReference(dummy<T>());
}
template<typename T>
inline const T &dynCast(const dynamic_cast_interface<T> &_r)
{	return _r.getReference(dummy<T>());
}


template<typename T>
struct dynamic_cast_implementation:T
{	virtual ~dynamic_cast_implementation(void) = default;
	template<typename ...ARGS>
	dynamic_cast_implementation(ARGS&&..._r)
		:T(std::forward<ARGS>(_r)...)
	{
	}
	virtual const T*getPtr(const dummy<T>&) const override
	{	return this;
	}
	virtual T*getPtr(const dummy<T>&) override
	{	return this;
	}
	virtual const T&getReference(const dummy<T>&) const override
	{	return *this;
	}
	virtual T&getReference(const dummy<T>&) override
	{	return *this;
	}
};
}
