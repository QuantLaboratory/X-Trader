#include "high_low.h"
#include "frame.h"


high_low::high_low(stratid_t id, frame& frame, std::string contract)
	: strategy(id, frame), _contract(contract)
{
	get_contracts().insert(contract);
}

high_low::~high_low()
{
}

void high_low::on_tick(const MarketData& tick)
{

}

void high_low::on_order(const Order& order)
{

}

void high_low::on_trade(const Order& order)
{

}

void high_low::on_cancel(const Order& order)
{

}

void high_low::on_error(const Order& order)
{

}
