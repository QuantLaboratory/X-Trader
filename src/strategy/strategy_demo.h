#pragma once

#include "strategy.h"

class strategy_demo : public strategy
{
public:
	strategy_demo(stratid_t id, frame& frame, std::string contract);
	~strategy_demo();
	
	virtual void on_tick(const MarketData& tick) override;
	virtual void on_order(const Order& order) override;
	virtual void on_trade(const Order& order) override;
	virtual void on_cancel(const Order& order) override;
	virtual void on_error(const Order& order) override;

private:
	std::string _contract;
	orderref_t _buy_orderref = null_orderref;
	orderref_t _sell_orderref = null_orderref;

	bool _is_closing = false;
};