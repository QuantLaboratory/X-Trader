#pragma once

#include "data_struct.h"
#include "event_center.hpp"


class trader_api : public event_ringbuffer<Order>
{
public:
	trader_api() {}
	virtual ~trader_api() {}

public:
	virtual void release() = 0;
	virtual std::string get_trading_day() const = 0;
	virtual void get_account() = 0;
	virtual void get_trader_data(InstrumentMap& i_map, PositionMap& p_map, OrderMap& o_map) = 0;
	virtual orderref_t insert_order(eOrderFlag order_flag, const std::string& contract, eDirOffset dir_offset, double price, int volume) = 0;
	virtual bool cancel_order(const orderref_t order_ref) = 0;

public:
	std::atomic<bool> _is_ready{ false };
};
