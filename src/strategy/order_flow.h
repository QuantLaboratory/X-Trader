#pragma once

#include "strategy.h"
#include "frame.h"
#include "resample.h"

class order_flow : public strategy, public bar_receiver
{
public:
	order_flow(stratid_t id, frame& frame, std::string contract, uint32_t period, uint32_t multiples, uint32_t threshold, uint32_t position_limit, uint32_t once_vol)
		: strategy(id, frame), _contract(contract), _period(period),
		_multiples(multiples), _threshold(threshold), _position_limit(position_limit), _once_vol(once_vol),
		_buy_orderref(null_orderref), _sell_orderref(null_orderref), _is_closing(false)
	{
		get_contracts().insert(contract);
	}

	~order_flow() {}
	
	virtual void on_init() { get_frame().register_bar_receiver(_contract, _period, this); }

	virtual void on_tick(const MarketData& tick) override;
	virtual void on_bar(const Sample& bar) override;
	virtual void on_order(const Order& order) override;
	virtual void on_trade(const Order& order) override;
	virtual void on_cancel(const Order& order) override;
	virtual void on_error(const Order& order) override;

private:
	std::string _contract;
	uint32_t _period;
	uint32_t _multiples;//±¶Êý
	uint32_t _threshold;//ãÐÖµ
	uint32_t _position_limit;
	uint32_t _once_vol;
	orderref_t _buy_orderref;
	orderref_t _sell_orderref;
	bool _is_closing;
	MarketData _tick;

private:
	orderref_t try_buy();
	orderref_t try_sell();
};
