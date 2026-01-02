#include "order_flow.h"


void order_flow::on_tick(const MarketData& tick)
{

}

void order_flow::on_bar(const Sample& bar)
{

}

void order_flow::on_order(const Order& order)
{

}

void order_flow::on_trade(const Order& order)
{

}

void order_flow::on_cancel(const Order& order)
{

}

void order_flow::on_error(const Order& order)
{

}

orderref_t order_flow::try_buy()
{
	return null_orderref;
}

orderref_t order_flow::try_sell()
{
	return null_orderref;
}
