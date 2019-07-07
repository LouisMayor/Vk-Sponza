#pragma once

#include <functional>
#include <algorithm>
#include <vector>

/* Wanting to create an extremely simple event system. Design based on Unity Actions */

template<typename _paramT>
class Event
{
public:
	Event() = default;

	~Event()
	{
		_funcs.clear();
	}

	Event(const Event& other) = delete;
	Event operator=(const Event& other) = delete;

	template<typename _funcPointer>
	void operator=(_funcPointer func)
	{
		_funcs.clear();
		_funcs.push_back(std::function<void(_paramT)>(func));
	}

	template<typename _funcPointer>
	void operator+=(_funcPointer func)
	{
		_funcs.push_back(std::function<void(_paramT)>(func));
	}

	template<typename _funcPointer>
	void operator-=(_funcPointer func)
	{
		std::function<void(_paramT)> to_find(func);
		size_t addr_to_find = get_addr(to_find);

		std::vector <std::vector<std::function < void(_paramT)> >::iterator> to_remove;

		for (auto it = _funcs.begin(); it != _funcs.end(); ++it)
		{
			if (addr_to_find == get_addr(*it))
			{
				to_remove.push_back(it);
			}
		}

		for (auto& i : to_remove)
		{
			_funcs.erase(i);
		}
	}

	void invoke_safe(_paramT value)
	{
		for (auto& func : _funcs)
		{
			func(value);
		}
	}
private:
	std::vector<std::function<void(_paramT)>> _funcs;

	// src: https://stackoverflow.com/a/18039824
	size_t get_addr(std::function < void(_paramT) > f)
	{
		typedef void(fnType)(_paramT);
		fnType** fnPointer = f.template target<fnType*>();
		return reinterpret_cast<size_t> (*fnPointer);
	}
};
