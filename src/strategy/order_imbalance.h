#pragma once

#include "strategy.h"
#include "resample.h"

class order_imbalance : public strategy, public bar_receiver, public tape_receiver
{
public:
	order_imbalance(stratid_t id, frame& frame, std::string contract, uint32_t period);
	~order_imbalance();
	
	virtual void on_init() {}
	virtual void on_tick(const MarketData& tick) override;
	virtual void on_bar(const Sample& bar) override;
	virtual void on_tape(const Tape& tape) override;
	virtual void on_order(const Order& order) override;
	virtual void on_trade(const Order& order) override;
	virtual void on_cancel(const Order& order) override;
	virtual void on_error(const Order& order) override;

private:
	std::string _contract;
	uint32_t _period;
	orderref_t _buy_orderref = null_orderref;
	orderref_t _sell_orderref = null_orderref;

	bool _is_closing = false;
};