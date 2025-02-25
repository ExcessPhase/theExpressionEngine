#pragma once
#include <typeinfo>


namespace theExpessionEngine
{
template<typename T>
struct dynamic_cast_interface
{	virtual ~dynamic_cast_interface(void) = default;
	virtual const T*getPtr(void) const
	{	return nullptr;
	}
	virtual T*getPtr(void)
	{	return nullptr;
	}
	virtual const T&getReference(void) const
	{	throw std::bad_cast();
	}
	virtual T&getReference(void)
	{	throw std::bad_cast();
	}
};


template<typename T>
struct dynamic_cast_implementation:T
{	virtual ~dynamic_cast_implementation(void) = default;
	template<typename ...ARGS>
	dynamic_cast_implementation(ARGS&&..._r)
		:T(std::forward<ARGS>(_r)...)
	{
	}
	virtual const T*getPtr(void) const override
	{	return this;
	}
	virtual T*getPtr(void) override
	{	return this;
	}
	virtual const T&getReference(void) const override
	{	return *this;
	}
	virtual T&getReference(void) override
	{	return *this;
	}
};
}