#include "decline_scalping.h"
#include "frame.h"


decline_scalping::decline_scalping(stratid_t id, frame& frame, std::string contract, uint32_t period)
	: strategy(id, frame), _contract(contract), _period(period)
{
	get_contracts().insert(contract);
	frame.register_bar_receiver(contract, _period, this);
}

decline_scalping::~decline_scalping()
{
}

void decline_scalping::on_tick(const MarketData& tick)
{

}

void decline_scalping::on_bar(const Sample& bar)
{

}

void decline_scalping::on_order(const Order& order)
{

}

void decline_scalping::on_trade(const Order& order)
{

}

void decline_scalping::on_cancel(const Order& order)
{

}

void decline_scalping::on_error(const Order& order)
{

}

void decline_scalping::on_update()
{
}
