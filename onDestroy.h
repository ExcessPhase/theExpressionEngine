#pragma once
#include <functional>


namespace theExpressionEngine
{
template<typename T>
struct onDestroy:T
{	template<typename ...REST>
	onDestroy(REST&&..._r)
		:T(std::forward<REST>(_r)...)
	{
	}
	~onDestroy(void)
	{	T::onDestroy();
	}
};
}
