#pragma once

#include "data_struct.h"
#include <functional>


class frame;

class strategy
{
public:
	strategy(stratid_t id, frame& frame);
	virtual ~strategy();

private:
	stratid_t _id;
	frame& _frame;
	std::set<std::string> _contracts;

public:
	virtual void on_init() {}
	virtual void on_tick(const MarketData& tick) {}
	virtual void on_order(const Order& order) {}
	virtual void on_trade(const Order& order) {}
	virtual void on_cancel(const Order& order) {}
	virtual void on_error(const Order& order) {}
	virtual void on_update() {};

public:
	inline stratid_t get_id() const { return _id; }
	inline frame& get_frame() { return _frame; }
	inline std::set<std::string>& get_contracts() { return _contracts; }

protected:
	orderref_t buy_open(eOrderFlag order_flag, const std::string& contract, double price, int volume);
	orderref_t sell_open(eOrderFlag order_flag, const std::string& contract, double price, int volume);
	orderref_t buy_close(eOrderFlag order_flag, const std::string& contract, double price, int volume);
	orderref_t sell_close(eOrderFlag order_flag, const std::string& contract, double price, int volume);

	orderref_t buy_close_today(eOrderFlag order_flag, const std::string& contract, double price, int volume);
	orderref_t sell_close_today(eOrderFlag order_flag, const std::string& contract, double price, int volume);
	orderref_t buy_close_yesterday(eOrderFlag order_flag, const std::string& contract, double price, int volume);
	orderref_t sell_close_yesterday(eOrderFlag order_flag, const std::string& contract, double price, int volume);

	bool cancel_order(const orderref_t order_ref);
	const Position& get_position(const std::string& contract) const;
	const Order& get_order(const orderref_t orderref) const;
	void set_cancel_condition(orderref_t id, std::function<bool(orderref_t)> callback);
};
