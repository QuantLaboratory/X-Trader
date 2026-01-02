#pragma once

#include "event_center.hpp"
#include "data_struct.h"

class market_api : public event_ringbuffer<MarketData>
{
public:
	market_api() {}
	virtual ~market_api() {}

public:
	virtual void release() = 0;

public:
	std::atomic<bool> _is_ready{ false };
};
