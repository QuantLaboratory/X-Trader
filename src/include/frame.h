#pragma once

#include <ctime>
#include <iomanip>
#include "realtime.h"
#include "strategy.h"
#include "resample.h"


class frame
{
	friend strategy;

public:
	frame(const char* filename);
	~frame();

private:
	static inline void _tick_callback(const MarketData& tick)
	{
		auto it_c = _self->_contract_to_strategy_map.find(tick.instrument_id);
		if (it_c != _self->_contract_to_strategy_map.end())
		{
			for (auto strat : it_c->second)
			{
				if (strat) { strat->on_tick(tick); }
			}
		}
		
		auto it_r = _self->_resample.find(tick.instrument_id);
		if (it_r != _self->_resample.end())
		{
			for (auto iter : it_r->second)
			{
				iter.second->insert_tick(tick);
			}
		}

		auto it_t = _self->_tape_receiver.find(tick.instrument_id);
		if (it_t != _self->_tape_receiver.end())
		{
			for (auto iter : it_t->second)
			{
				if (iter)
				{
					Tape tape;
					tape.last_volume = tick.last_volume;
					tape.last_open_interest = tick.last_open_interest;
					tape.tape_dir = tick.tape_dir;
					iter->on_tape(tape);
				}
			}
		}
	}

	static inline void _order_callback(const Order& order)
	{
		auto it = _self->_order_to_strategy_map.find(order.order_ref);
		if (it != _self->_order_to_strategy_map.end())
		{
			auto strat = _self->get_strategy(it->second);
			if (strat) { strat->on_order(order); }
		}
	}

	static inline void _trade_callback(const Order& order)
	{
		auto it = _self->_order_to_strategy_map.find(order.order_ref);
		if (it != _self->_order_to_strategy_map.end())
		{
			auto strat = _self->get_strategy(it->second);
			if (strat) { strat->on_trade(order); }
		}
		_self->remove_cancel_condition(order.order_ref);
		_self->unregister_orderref_strategy(order.order_ref);
	}

	static inline void _cancel_callback(const Order& order)
	{
		auto it = _self->_order_to_strategy_map.find(order.order_ref);
		if (it != _self->_order_to_strategy_map.end())
		{
			auto strat = _self->get_strategy(it->second);
			if (strat) { strat->on_cancel(order); }
		}
		_self->remove_cancel_condition(order.order_ref);
		_self->unregister_orderref_strategy(order.order_ref);
	}

	static inline void _error_callback(const Order& order)
	{
		auto it = _self->_order_to_strategy_map.find(order.order_ref);
		if (it != _self->_order_to_strategy_map.end())
		{
			auto strat = _self->get_strategy(it->second);
			if (strat) { strat->on_error(order); }
		}
		
		if (order.event_flag == eEventFlag::ErrorInsert)
		{
			_self->remove_cancel_condition(order.order_ref);
			_self->unregister_orderref_strategy(order.order_ref);
		}
		else if (order.event_flag == eEventFlag::ErrorCancel)
		{
			_self->set_cancel_condition(order.order_ref, [](orderref_t order_ref)->bool {
				return false;
				});
		}
	}

	static inline void _update_callback()
	{
		if (_self)
		{
			for (auto& it : _self->_strategy_map)
			{
				it.second->on_update();
			}
			_self->check_cancel_condition();
		}
	};

	static inline void _init_callback()
	{
		if (_self)
		{
			for (auto& it : _self->_strategy_map)
			{
				it.second->on_init();
			}
		}
	}

	inline std::shared_ptr<strategy> get_strategy(stratid_t id) const
	{
		auto it = _strategy_map.find(id);
		if (it == _strategy_map.end()) { return nullptr; }
		return it->second;
	}


public:
	orderref_t insert_order(const stratid_t sid, eOrderFlag order_flag, const std::string& contract, eDirOffset dir_offset, double price, int volume);
	bool cancel_order(const orderref_t order_ref);
	void set_cancel_condition(orderref_t id, std::function<bool(orderref_t)> callback);
	void remove_cancel_condition(orderref_t id);
	void register_tape_receiver(const std::string& contract, tape_receiver* receiver);
	void register_bar_receiver(const std::string& contract, uint32_t period, bar_receiver* receiver);

private:
	void register_strategy(const std::vector<std::shared_ptr<strategy>>& strategys);
	void clear_strategy();
	void check_cancel_condition();
	void unregister_orderref_strategy(orderref_t order_ref);

public:
	time_t make_close_time(const std::string day, std::string close = "15:00:00");
	void run_until_close(const std::vector<std::shared_ptr<strategy>>& strategys);

private:
	static inline frame* _self;
	realtime* _realtime;
	std::map<stratid_t, std::shared_ptr<strategy>> _strategy_map;
	std::set<std::string> _contracts;
	std::string _filename;
	std::unordered_map<std::string, std::vector<std::shared_ptr<strategy>>> _contract_to_strategy_map;
	std::map<orderref_t, stratid_t> _order_to_strategy_map;
	std::map<orderref_t, std::function<bool(orderref_t)>> _cancel_condition_map;
	std::map<std::string, std::map<uint32_t, std::shared_ptr<resample>>> _resample;
	std::map<std::string, std::set<tape_receiver*>> _tape_receiver;
};
