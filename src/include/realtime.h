#pragma once

#include <thread>
#include "interface.h"


class realtime
{
public:
	realtime();
	virtual ~realtime();

private:
	class market_api* _market;
	class trader_api* _trader;
	std::thread* _realtime_thread;
	uint32_t _bind_cpu_core;
	uint32_t _loop_interval;

	InstrumentMap _instrument_map;
	PositionMap _position_map;
	OrderMap _order_map;

	tick_callback _tick_callback;
	update_callback _update_callback;
	OrderEvent _order_event;

public:
	virtual trader_api& get_trader() { return *_trader; }
	virtual market_api& get_market() { return *_market; }

	inline void bind_tick_event(const tick_callback& tick_cb)
	{
		_tick_callback = tick_cb;
	}

	inline void bind_update_callback(const update_callback& update_cb)
	{
		_update_callback = update_cb;
	}

	inline void bind_order_event(const OrderEvent& order_event)
	{
		_order_event = order_event;
	}

	orderref_t insert_order(eOrderFlag order_flag, const std::string& contract, eDirOffset dir_offset, double price, int volume);
	bool cancel_order(const orderref_t order_ref);
	std::string get_trading_day() { return get_trader().get_trading_day(); }
	const Position& get_position(const std::string& contract) const;
	const Order& get_order(const orderref_t orderref) const;
	const Instrument& get_instrument(const std::string& contract) const;
	void get_account() { get_trader().get_account(); }

public:
	bool init(const std::string& filename, const std::set<std::string>& contracts);
	void start_service();
	void update();
	void stop_service();
	void release();

private:
	void bind_callback();
	void handle_tick(const MarketData& tick);
	void handle_order(const Order& order);
	void handle_trade(const Order& order);
	void handle_cancel(const Order& order);
	void handle_error(const Order& order);
	void load_trader_data();

public:
	void print_order(const Order& order);
	void print_position(const Position& posi, const char* msg);
};
