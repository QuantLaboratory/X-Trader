#include "strategy_demo.h"
#include "frame.h"


strategy_demo::strategy_demo(stratid_t id, frame& frame, std::string contract)
	: strategy(id, frame), _contract(contract)
{
	get_contracts().insert(contract);
}

strategy_demo::~strategy_demo()
{
}

void strategy_demo::on_tick(const MarketData& tick)
{

}

void strategy_demo::on_order(const Order& order)
{

}

void strategy_demo::on_trade(const Order& order)
{
}

void strategy_demo::on_cancel(const Order& order)
{
}

void strategy_demo::on_error(const Order& order)
{
}
