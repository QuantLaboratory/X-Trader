#include "resample.h"


void resample::insert_tick(const MarketData& tick)
{
	if (tick.update_time[4] != _min) { ++_count; }
	_min = tick.update_time[4];
	
	if (_count >= _period)
	{
		//printf("time=%s open=%.f high=%.f low=%.f close=%.f volume=%d\n", _bar.time.c_str(), _bar.open, _bar.high, _bar.low, _bar.close, _bar.volume);
		_is_new = true; _bar.is_new = true;

		for (auto it : _bar_callback) { it->on_bar(_bar); }

		_count = 0;
		_bar.clear();
		_bar.price_tick = _price_tick;
	}
	
	if (_is_new)
	{
		_is_new = false; _bar.is_new = false;
		
		_bar.id = tick.instrument_id;
		_bar.period = _period;
		_bar.open = tick.last_price;
		_bar.high = tick.last_price;
		_bar.low = tick.last_price;
		_bar.close = tick.last_price;
		
		_bar.time = tick.update_time;
		_bar.volume = tick.last_volume;

		_poc.clear();
		_poc[tick.last_price] = tick.last_volume;
		_bar.poc = tick.last_price;
	}
	else
	{
		_bar.high = std::max(_bar.high, tick.last_price);
		_bar.low = std::min(_bar.low, tick.last_price);
		_bar.close = tick.last_price;
		_bar.volume += tick.last_volume;
		_poc[tick.last_price] += tick.last_volume;
	}

	for (auto it : _bar_callback) { it->on_bar(_bar); }

	if (_poc[tick.last_price] > _poc[_bar.poc]) { _bar.poc = tick.last_price; }

	if (tick.last_price == tick.bid_price[0])//主动卖出
	{
		_bar.active_sell_volume[tick.last_price] += tick.last_volume;
		_bar.delta -= tick.last_volume;
	}
	if (tick.last_price == tick.ask_price[0])//主动买入
	{
		_bar.active_buy_volume[tick.last_price] += tick.last_volume;
		_bar.delta += tick.last_volume;
	}
}

void resample::add_receiver(bar_receiver* receiver)
{
	auto it = _bar_callback.find(receiver);
	if (it == _bar_callback.end())
	{
		_bar_callback.insert(receiver);
	}
}

void resample::remove_receiver(bar_receiver* receiver)
{
	auto it = _bar_callback.find(receiver);
	if (it != _bar_callback.end())
	{
		_bar_callback.erase(receiver);
	}
}

bool resample::invalid() const
{
	return _bar_callback.empty();
}
