#include "market_correction.h"
#include "frame.h"


market_correction::market_correction(stratid_t id, frame& frame, std::string contract, uint32_t period)
	: strategy(id, frame), _contract(contract), _period(period)
{
	get_contracts().insert(contract);
	frame.register_bar_receiver(contract, _period, this);
	_last_bar.high = 0; _last_bar.low = DBL_MAX;
}

market_correction::~market_correction()
{
}

void market_correction::on_tick(const MarketData& tick)
{

}

void market_correction::on_bar(const Sample& bar)
{

}

void market_correction::on_order(const Order& order)
{

}

void market_correction::on_trade(const Order& order)
{

}

void market_correction::on_cancel(const Order& order)
{

}

void market_correction::on_error(const Order& order)
{

}

void market_correction::on_update()
{
}
