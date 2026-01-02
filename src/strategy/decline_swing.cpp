#include "decline_swing.h"
#include "frame.h"


decline_swing::decline_swing(stratid_t id, frame& frame, std::string contract, uint32_t period)
	: strategy(id, frame), _contract(contract), _period(period)
{
	get_contracts().insert(contract);
	frame.register_bar_receiver(contract, _period, this);
}

decline_swing::~decline_swing()
{
}

void decline_swing::on_tick(const MarketData& tick)
{

}

void decline_swing::on_bar(const Sample& bar)
{

}

void decline_swing::on_order(const Order& order)
{

}

void decline_swing::on_trade(const Order& order)
{

}

void decline_swing::on_cancel(const Order& order)
{

}

void decline_swing::on_error(const Order& order)
{

}

void decline_swing::on_update()
{
}
