#include "statistical_arbitrage.h"


void statistical_arbitrage::on_tick(const MarketData& tick)
{

}

void statistical_arbitrage::on_order(const Order& order)
{

}

void statistical_arbitrage::on_trade(const Order& order)
{

}

void statistical_arbitrage::on_cancel(const Order& order)
{

}

void statistical_arbitrage::on_error(const Order& order)
{

}

void statistical_arbitrage::on_update()
{

}

orderref_t statistical_arbitrage::try_buy(const std::string& contract)
{
	return null_orderref;
}

orderref_t statistical_arbitrage::try_sell(const std::string& contract)
{
	return null_orderref;
}
