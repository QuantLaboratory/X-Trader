#pragma once

#include "strategy.h"


class market_making : public strategy
{
public:
	market_making(stratid_t id, frame& frame, std::string contract, double price_delta, uint32_t position_limit, int once_vol) :
		strategy(id, frame), _contract(contract),
		_price_delta(price_delta), _position_limit(position_limit), _once_vol(once_vol),
		_buy_orderref(null_orderref), _sell_orderref(null_orderref), _is_closing(false)
	{
		get_contracts().insert(contract);
	}

	~market_making() {}

	virtual void on_tick(const MarketData& tick) override;
	virtual void on_order(const Order& order) override;
	virtual void on_trade(const Order& order) override;
	virtual void on_cancel(const Order& order) override;
	virtual void on_error(const Order& order) override;

private:
	std::string _contract;
	double _price_delta;
	uint32_t _position_limit;
	int _once_vol;
	orderref_t _buy_orderref;
	orderref_t _sell_orderref;
	bool _is_closing;
};
