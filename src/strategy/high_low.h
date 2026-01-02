#pragma once

#include <float.h>
#include "strategy.h"

class high_low : public strategy
{
public:
	high_low(stratid_t id, frame& frame, std::string contract);
	~high_low();
	
	virtual void on_tick(const MarketData& tick) override;
	virtual void on_order(const Order& order) override;
	virtual void on_trade(const Order& order) override;
	virtual void on_cancel(const Order& order) override;
	virtual void on_error(const Order& order) override;

private:
	std::string _contract;
	orderref_t _buy_orderref = null_orderref;
	orderref_t _sell_orderref = null_orderref;
	
	double tmp_high = 0;
	double tmp_low = DBL_MAX;

	int open_point = 10;
	int close_point = 5;
	int stop_loss = 3;

	int max_loss_count = 4;
	int buy_loss_count = 0;
	int sell_loss_count = 0;

	int _once_vol = 1;

	bool _is_closing = false;
};