#pragma once
#include <mutex>
#include <deque>

template<typename T>
class tsdeque
{
public:
	tsdeque() = default;
	tsdeque(const tsdeque<T>&) = delete;

	~tsdeque() { clear(); }

public:
	const T& front()
	{
		std::scoped_lock lock(mtx);
		return deque.front();
	}

	const T& back()
	{
		std::scoped_lock lock(mtx);
		return deque.back();
	}

	const T& pop_front()
	{
		std::scoped_lock lock(mtx);
		T& item = std::move(deque.front());
		deque.pop_front();
		return item;
	}

	const T& pop_back()
	{
		std::scoped_lock lock(mtx);
		T& item = std::move(deque.back());
		deque.pop_back();
		return item;
	}

	void push_back(const T& item)
	{
		std::scoped_lock lock(mtx);
		deque.push_back(std::move(item));
	}

	void push_front(const T& item)
	{
		std::scoped_lock lock(mtx);
		deque.push_front(std::move(item));
	}

	bool empty()
	{
		std::scoped_lock lock(mtx);
		return deque.empty();
	}

	const size_t size()
	{
		std::scoped_lock lock(mtx);
		return deque.size();
	}

	void clear()
	{
		std::scoped_lock lock(mtx);
		deque.clear();
	}

protected:
	std::mutex mtx;
	std::deque<T> deque;
};