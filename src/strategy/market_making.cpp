#include "market_making.h"


void market_making::on_tick(const MarketData& tick)
{
	if (strncmp(tick.update_time, "14:59", 5) == 0) { _is_closing = true; }
	
	const auto& posi = get_position(_contract);

	if (_is_closing)
	{
		if (posi.long_.closeable)
		{
			sell_close_today(eOrderFlag::Limit, _contract, tick.bid_price[0], posi.long_.closeable);
		}
		if (posi.short_.closeable)
		{
			buy_close_today(eOrderFlag::Limit, _contract, tick.ask_price[0], posi.short_.closeable);
		}
		return;
	}

	
	if (_buy_orderref == null_orderref)
	{
		double buy_price = tick.bid_price[0] - _price_delta;
		
		if (posi.short_.his_closeable > 0)
		{
			uint32_t buy_vol = std::min(posi.short_.his_closeable, _once_vol);
			_buy_orderref = buy_close_yesterday(eOrderFlag::Limit, _contract, buy_price, buy_vol);
		}
		else if (posi.short_.today_closeable > 0)
		{
			uint32_t buy_vol = std::min(posi.short_.today_closeable, _once_vol);
			_buy_orderref = buy_close_today(eOrderFlag::Limit, _contract, buy_price, buy_vol);
		}
		else if (posi.long_.position + posi.long_.open_no_trade < _position_limit)
		{
			_buy_orderref = buy_open(eOrderFlag::Limit, _contract, buy_price, _once_vol);
		}
	}

	if (_sell_orderref == null_orderref)
	{
		double sell_price = tick.ask_price[0] + _price_delta;
		
		if (posi.long_.his_closeable > 0)
		{
			uint32_t sell_vol = std::min(posi.long_.his_closeable, _once_vol);
			_sell_orderref = sell_close_yesterday(eOrderFlag::Limit, _contract, sell_price, sell_vol);
		}
		else if (posi.long_.today_closeable > 0)
		{
			uint32_t sell_vol = std::min(posi.long_.today_closeable, _once_vol);
			_sell_orderref = sell_close_today(eOrderFlag::Limit, _contract, sell_price, sell_vol);
		}
		else if (posi.short_.position + posi.short_.open_no_trade < _position_limit)
		{
			_sell_orderref = sell_open(eOrderFlag::Limit, _contract, sell_price, _once_vol);
		}
	}
}

void market_making::on_order(const Order& order)
{
	if (order.order_ref == _buy_orderref || order.order_ref == _sell_orderref)
	{
		set_cancel_condition(order.order_ref, [this](orderref_t order_id)->bool {
			if (_is_closing) { return true; }
			return false;
			});
	}
}

void market_making::on_trade(const Order& order)
{
	if (_buy_orderref == order.order_ref)
	{
		cancel_order(_sell_orderref);
		_buy_orderref = null_orderref;
	}
	if (_sell_orderref == order.order_ref)
	{
		cancel_order(_buy_orderref);
		_sell_orderref = null_orderref;
	}
}

void market_making::on_cancel(const Order& order)
{
	if (_buy_orderref == order.order_ref) { _buy_orderref = null_orderref; }

	if (_sell_orderref == order.order_ref) { _sell_orderref = null_orderref; }
}

void market_making::on_error(const Order& order)
{
	if (_buy_orderref == order.order_ref) { _buy_orderref = null_orderref; }

	if (_sell_orderref == order.order_ref) { _sell_orderref = null_orderref; }
}
