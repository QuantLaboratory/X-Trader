#include "frame.h"
#include <sstream>


frame::frame(const char* filename) : _filename(filename)
{
	_realtime = new realtime();
	frame::_self = this;
}

frame::~frame()
{
	if (_realtime)
	{
		delete _realtime; _realtime = nullptr;
	}
}

orderref_t frame::insert_order(const stratid_t sid, eOrderFlag order_flag, const std::string& contract, eDirOffset dir_offset, double price, int volume)
{
	auto id = _realtime->insert_order(order_flag, contract, dir_offset, price, volume);
	if (id != null_orderref) { _order_to_strategy_map[id] = sid; }
	return id;
}

bool frame::cancel_order(const orderref_t order_ref)
{
	return _realtime->cancel_order(order_ref);
}

void frame::set_cancel_condition(orderref_t id, std::function<bool(orderref_t)> callback)
{
	_cancel_condition_map[id] = callback;
}

void frame::remove_cancel_condition(orderref_t id)
{
	auto it = _cancel_condition_map.find(id);
	if (it != _cancel_condition_map.end())
	{
		_cancel_condition_map.erase(it);
	}
}

void frame::register_tape_receiver(const std::string& contract, tape_receiver* receiver)
{
	_tape_receiver[contract].insert(receiver);
}

void frame::register_bar_receiver(const std::string& contract, uint32_t period, bar_receiver* receiver)
{
	auto iter = _resample[contract].find(period);
	if (iter == _resample[contract].end())
	{
		_resample[contract][period] = std::make_shared<resample>(period, _realtime->get_instrument(contract).price_tick);
	}
	_resample[contract][period]->add_receiver(receiver);
}

void frame::register_strategy(const std::vector<std::shared_ptr<strategy>>& strategys)
{
	for (auto iter : strategys)
	{
		_strategy_map[iter->get_id()] = iter;
		for (auto it : iter->get_contracts())
		{
			_contracts.insert(it);
			_contract_to_strategy_map[it].push_back(iter);
		}
	}
	_realtime->bind_tick_event(_tick_callback);
	_realtime->bind_update_callback(_update_callback);
	_realtime->bind_order_event(OrderEvent{ _order_callback, _trade_callback, _cancel_callback, _error_callback });
}

void frame::clear_strategy()
{
	_strategy_map.clear();
	_contracts.clear();
	_contract_to_strategy_map.clear();
	_order_to_strategy_map.clear();
	_cancel_condition_map.clear();
	_resample.clear(); _tape_receiver.clear();
}

void frame::check_cancel_condition()
{
	for (auto it = _cancel_condition_map.begin(); it != _cancel_condition_map.end();)
	{
		if (it->second(it->first))
		{
			if (_realtime->cancel_order(it->first))
			{
				it = _cancel_condition_map.erase(it);
			}
			else { ++it; }
		}
		else { ++it; }
	}
}

void frame::unregister_orderref_strategy(orderref_t order_ref)
{
	auto it = _order_to_strategy_map.find(order_ref);
	if (it != _order_to_strategy_map.end())
	{
		_order_to_strategy_map.erase(it);
	}
}

time_t frame::make_close_time(const std::string day, std::string close)
{
	std::string time_str = day + close;
	std::tm tm = {};
	std::stringstream ss(time_str);
	ss >> std::get_time(&tm, "%Y%m%d%H:%M:%S");
	std::time_t time = std::mktime(&tm);
	return time;
}

void frame::run_until_close(const std::vector<std::shared_ptr<strategy>>& strategys)
{
	register_strategy(strategys);
	if (!_realtime->init(_filename, _contracts)) { return; }
	_realtime->start_service();
	_init_callback();
	time_t close_time = make_close_time(_realtime->get_trading_day(), "15:00:00");
	time_t now_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	time_t delta_seconds = close_time - now_time;
	if (delta_seconds > 0)
	{
		std::this_thread::sleep_for(std::chrono::seconds(delta_seconds));
	}
	//std::this_thread::sleep_for(std::chrono::seconds(3600));
	//send2wecom("close");
	_realtime->get_account();
	std::this_thread::sleep_for(std::chrono::seconds(60));
	_realtime->stop_service();
	clear_strategy();
	_realtime->release();
}
