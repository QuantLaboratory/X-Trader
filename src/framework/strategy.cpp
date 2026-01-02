#include "strategy.h"
#include "frame.h"


strategy::strategy(stratid_t id, frame& frame) : _id(id), _frame(frame) {}

strategy::~strategy() {}


orderref_t strategy::buy_open(eOrderFlag order_flag, const std::string& contract, double price, int volume)
{
	return _frame.insert_order(_id, order_flag, contract, eDirOffset::BuyOpen, price, volume);
}

orderref_t strategy::sell_open(eOrderFlag order_flag, const std::string& contract, double price, int volume)
{
	return _frame.insert_order(_id, order_flag, contract, eDirOffset::SellOpen, price, volume);
}

orderref_t strategy::buy_close(eOrderFlag order_flag, const std::string& contract, double price, int volume)
{
	return _frame.insert_order(_id, order_flag, contract, eDirOffset::BuyClose, price, volume);
}

orderref_t strategy::sell_close(eOrderFlag order_flag, const std::string& contract, double price, int volume)
{
	return _frame.insert_order(_id, order_flag, contract, eDirOffset::SellClose, price, volume);
}

orderref_t strategy::buy_close_today(eOrderFlag order_flag, const std::string& contract, double price, int volume)
{
	return _frame.insert_order(_id, order_flag, contract, eDirOffset::BuyCloseToday, price, volume);
}

orderref_t strategy::sell_close_today(eOrderFlag order_flag, const std::string& contract, double price, int volume)
{
	return _frame.insert_order(_id, order_flag, contract, eDirOffset::SellCloseToday, price, volume);
}

orderref_t strategy::buy_close_yesterday(eOrderFlag order_flag, const std::string& contract, double price, int volume)
{
	return _frame.insert_order(_id, order_flag, contract, eDirOffset::BuyCloseYesterday, price, volume);
}

orderref_t strategy::sell_close_yesterday(eOrderFlag order_flag, const std::string& contract, double price, int volume)
{
	return _frame.insert_order(_id, order_flag, contract, eDirOffset::SellCloseYesterday, price, volume);
}

bool strategy::cancel_order(const orderref_t order_ref)
{
	return _frame.cancel_order(order_ref);
}

const Position& strategy::get_position(const std::string& contract) const
{
	return _frame._realtime->get_position(contract);
}

const Order& strategy::get_order(const orderref_t orderref) const
{
	return _frame._realtime->get_order(orderref);
}

void strategy::set_cancel_condition(orderref_t id, std::function<bool(orderref_t)> callback)
{
	return _frame.set_cancel_condition(id, callback);
}
