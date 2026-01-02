#pragma once

#define NB 2

#include "strategy.h"
#include "resample.h"

class decline_scalping : public strategy, public bar_receiver
{
public:
	decline_scalping(stratid_t id, frame& frame, std::string contract, uint32_t period);
	~decline_scalping();
	
	virtual void on_init() {}
	virtual void on_tick(const MarketData& tick) override;
	virtual void on_bar(const Sample& bar) override;
	virtual void on_order(const Order& order) override;
	virtual void on_trade(const Order& order) override;
	virtual void on_cancel(const Order& order) override;
	virtual void on_error(const Order& order) override;
	virtual void on_update() override;

private:
	std::string _contract;
	uint32_t _period;
	orderref_t _buy_orderref = null_orderref;
	orderref_t _sell_orderref = null_orderref;

	bool _is_closing = false;
	
	MarketData _last_tick{};

	Sample _bars[NB]{};
	int _index_b = 0;

	bool _is_decline = false;
	
	int _once_vol = 1;

	int _cancel_count = 0;
};
