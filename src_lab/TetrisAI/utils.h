#pragma once
#include <memory>
#include <cassert>
#include <ctype.h>
namespace moenew
{
	template <typename T, int O, int N>
	class Oary
	{
	private:
		T data[O + N];
	public:
		Oary()
		{
			assert(O >= 0);
			for (int i = 0; i < O + N; ++i) {
				data[i] = T();
			}
		}
		T& operator[](int i)
		{
			return data[O + i];
		}
	};
}