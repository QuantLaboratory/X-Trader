#pragma once

#include <functional>
#include "ringbuffer.hpp"


template<typename T, size_t SIZE = 512>
class event_ringbuffer
{
private:
	ringbuffer<T, SIZE> _event_ringbuffer;
	std::function<void(const T&)> _callback;

public:
	void insert_event(const T& event)
	{
		while (!_event_ringbuffer.insert(event));
	}

	void process_event()
	{
		T event;
		while (_event_ringbuffer.remove(event))
		{
			this->_callback(event);
		}
	}

	void bind_callback(std::function<void(const T&)> callback)
	{
		_callback = callback;
	}

	bool is_empty() const
	{
		return _event_ringbuffer.isEmpty();
	}
};
