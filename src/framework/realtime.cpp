#include "realtime.h"
#include "cpu_utils.hpp"
#include "INIReader.h"


realtime::realtime() :
	_market(nullptr), _trader(nullptr), _realtime_thread(nullptr),
	_tick_callback(nullptr), _update_callback(nullptr), _order_event(),
	_bind_cpu_core(-1), _loop_interval(1) {}

realtime::~realtime() {}


orderref_t realtime::insert_order(eOrderFlag order_flag, const std::string& contract, eDirOffset dir_offset, double price, int volume)
{
	return get_trader().insert_order(order_flag, contract, dir_offset, price, volume);
}

bool realtime::cancel_order(const orderref_t order_ref)
{
	return get_trader().cancel_order(order_ref);
}

const Position& realtime::get_position(const std::string& contract) const
{
	const auto& it = _position_map.find(contract);
	if (it != _position_map.end())
	{
		return (it->second);
	}
	return null_position;
}

const Order& realtime::get_order(const orderref_t orderref) const
{
	const auto& it = _order_map.find(orderref);
	if (it != _order_map.end())
	{
		return (it->second);
	}
	return null_order;
}

const Instrument& realtime::get_instrument(const std::string& contract) const
{
	const auto& it = _instrument_map.find(contract);
	if (it != _instrument_map.end())
	{
		return (it->second);
	}
	return null_instrument;
}

bool realtime::init(const std::string& filename, const std::set<std::string>& contracts)
{
	if (!std::filesystem::exists(filename.c_str()))
	{
		printf("%s does not exist\n", filename.c_str());
		return false;
	}
	INIReader reader(filename);
	if (reader.ParseError() < 0)
	{
		printf("can't load %s\n", filename.c_str());
		return false;
	}
	///market
	std::map<std::string, std::string> md_config;
	md_config["market_front"] = reader.Get("market", "market_front", "");
	md_config["broker_id"] = reader.Get("market", "broker_id", "");
	md_config["user_id"] = reader.Get("market", "user_id", "");
	md_config["password"] = reader.Get("market", "password", "");
	_market = create_market(md_config, contracts);
	if (_market == nullptr) { return false; }
	///trader
	std::map<std::string, std::string> td_config;
	td_config["trade_front"] = reader.Get("trader", "trade_front", "");
	td_config["broker_id"] = reader.Get("trader", "broker_id", "");
	td_config["user_id"] = reader.Get("trader", "user_id", "");
	td_config["password"] = reader.Get("trader", "password", "");
	td_config["app_id"] = reader.Get("trader", "app_id", "");
	td_config["auth_code"] = reader.Get("trader", "auth_code", "");
	td_config["user_product_Info"] = reader.Get("trader", "user_product_Info", "");
	_trader = create_trader(td_config, contracts);
	if (_trader == nullptr) { return false; }

	bind_callback();
	
	_bind_cpu_core = reader.GetInteger("control", "bind_cpu_core", 0);
	_loop_interval = reader.GetInteger("control", "loop_interval", 1);
	return true;
}

void realtime::start_service()
{
	while (!(get_market()._is_ready.load(std::memory_order_acquire))) {}
	while (!(get_trader()._is_ready.load(std::memory_order_acquire))) {}
	
	load_trader_data();
	_realtime_thread = new std::thread([this]()->void {
		if (0 <= _bind_cpu_core && _bind_cpu_core < cpu_utils::get_cpu_cores())
		{
			if (!cpu_utils::bind_core(_bind_cpu_core)) {}
		}
	
		while (true)
		{
			this->update();
			if (_loop_interval) { std::this_thread::sleep_for(std::chrono::microseconds(_loop_interval)); }
		}
		});
}

void realtime::update()
{
	get_market().process_event();
	get_trader().process_event();
	if (this->_update_callback) { this->_update_callback(); }
}

void realtime::stop_service()
{
	if (_realtime_thread)
	{
		_realtime_thread->join();
		delete _realtime_thread;
		_realtime_thread = nullptr;
	}
}

void realtime::release()
{
	if (_trader) { _trader->release(); }
	if (_market) { _market->release(); }
}

void realtime::bind_callback()
{
	get_market().bind_callback([this](const MarketData& tick)->void {handle_tick(tick); });

	get_trader().bind_callback([this](const Order& order)->void {
		switch (order.event_flag)
		{
		case eEventFlag::Order:
			handle_order(order); break;
		case eEventFlag::Trade:
			handle_trade(order); break;
		case eEventFlag::Cancel:
			handle_cancel(order); break;
		case eEventFlag::ErrorInsert:
		case eEventFlag::ErrorCancel:
			handle_error(order); break;
		}
		});
}

void realtime::handle_tick(const MarketData& tick)
{
	if (this->_tick_callback) { this->_tick_callback(tick); }
}

void realtime::handle_order(const Order& order)
{
	Order& o = _order_map[order.order_ref];
	
	if (o.volume_total != order.volume_total)
	{
		//更新持仓
		const int& volume_total = order.volume_total;
		Position& p = _position_map[order.instrument_id];
		p.id = order.instrument_id;

		if (_instrument_map[order.instrument_id].is_use_history)
		{
			switch (order.dir_offset)
			{
			case eDirOffset::BuyOpen:
				p.long_.open_no_trade += volume_total; break;
			case eDirOffset::SellOpen:
				p.short_.open_no_trade += volume_total; break;
			case eDirOffset::BuyCloseYesterday:
			case eDirOffset::BuyClose:
				p.short_.closeable -= volume_total;
				p.short_.his_closeable -= volume_total;
				break;
			case eDirOffset::BuyCloseToday:
				p.short_.closeable -= volume_total;
				p.short_.today_closeable -= volume_total;
				break;
			case eDirOffset::SellCloseYesterday:
			case eDirOffset::SellClose:
				p.long_.closeable -= volume_total;
				p.long_.his_closeable -= volume_total;
				break;
			case eDirOffset::SellCloseToday:
				p.long_.closeable -= volume_total;
				p.long_.today_closeable -= volume_total;
				break;
			}
		}
		else
		{
			switch (order.dir_offset)
			{
			case eDirOffset::BuyOpen:
				p.long_.open_no_trade += volume_total; break;
			case eDirOffset::SellOpen:
				p.short_.open_no_trade += volume_total; break;
			case eDirOffset::BuyClose:
			case eDirOffset::BuyCloseToday:
			case eDirOffset::BuyCloseYesterday:
				p.short_.closeable -= volume_total; break;
			case eDirOffset::SellClose:
			case eDirOffset::SellCloseToday:
			case eDirOffset::SellCloseYesterday:
				p.long_.closeable -= volume_total; break;
			}
		}
		print_position(p, "handle_order");
	}

	//更新报单
	memcpy(&o, &order, sizeof(struct Order));
	
	if (_order_event.on_order) { _order_event.on_order(order); }

	//print_order(order);
}

void realtime::handle_trade(const Order& order)
{
	//更新报单
	Order& o = _order_map[order.order_ref];
	const int& vol_traded_once = order.volume_traded - o.volume_traded;
	memcpy(&o, &order, sizeof(struct Order));
	
	//更新持仓
	if (vol_traded_once > 0)
	{
		Position& p = _position_map[order.instrument_id];
		p.id = order.instrument_id;

		if (_instrument_map[order.instrument_id].is_use_history)
		{
			switch (order.dir_offset)
			{
			case eDirOffset::BuyOpen:
				p.long_.position += vol_traded_once;
				p.long_.today_position += vol_traded_once;
				p.long_.closeable += vol_traded_once;
				p.long_.today_closeable += vol_traded_once;
				p.long_.open_no_trade -= vol_traded_once;
				p.long_.avg_open_cost = (p.long_.avg_open_cost * (p.long_.position - vol_traded_once) + order.limit_price * vol_traded_once) / p.long_.position;
				p.long_.avg_posi_cost = (p.long_.avg_posi_cost * (p.long_.position - vol_traded_once) + order.limit_price * vol_traded_once) / p.long_.position;
				break;
			case eDirOffset::SellOpen:
				p.short_.position += vol_traded_once;
				p.short_.today_position += vol_traded_once;
				p.short_.closeable += vol_traded_once;
				p.short_.today_closeable += vol_traded_once;
				p.short_.open_no_trade -= vol_traded_once;
				p.short_.avg_open_cost = (p.short_.avg_open_cost * (p.short_.position - vol_traded_once) + order.limit_price * vol_traded_once) / p.short_.position;
				p.short_.avg_posi_cost = (p.short_.avg_posi_cost * (p.short_.position - vol_traded_once) + order.limit_price * vol_traded_once) / p.short_.position;
				break;
			case eDirOffset::BuyCloseYesterday:
			case eDirOffset::BuyClose:
				p.short_.position -= vol_traded_once;
				p.short_.his_position -= vol_traded_once;
				break;
			case eDirOffset::BuyCloseToday:
				p.short_.position -= vol_traded_once;
				p.short_.today_position -= vol_traded_once;
				break;
			case eDirOffset::SellCloseYesterday:
			case eDirOffset::SellClose:
				p.long_.position -= vol_traded_once;
				p.long_.his_position -= vol_traded_once;
				break;
			case eDirOffset::SellCloseToday:
				p.long_.position -= vol_traded_once;
				p.long_.today_position -= vol_traded_once;
				break;
			}
		}
		else
		{
			switch (order.dir_offset)
			{
			case eDirOffset::BuyOpen:
				p.long_.position += vol_traded_once;
				p.long_.closeable += vol_traded_once;
				p.long_.open_no_trade -= vol_traded_once;
				p.long_.avg_open_cost = (p.long_.avg_open_cost * (p.long_.position - vol_traded_once) + order.limit_price * vol_traded_once) / p.long_.position;
				p.long_.avg_posi_cost = (p.long_.avg_posi_cost * (p.long_.position - vol_traded_once) + order.limit_price * vol_traded_once) / p.long_.position;
				break;
			case eDirOffset::SellOpen:
				p.short_.position += vol_traded_once;
				p.short_.closeable += vol_traded_once;
				p.short_.open_no_trade -= vol_traded_once;
				p.short_.avg_open_cost = (p.short_.avg_open_cost * (p.short_.position - vol_traded_once) + order.limit_price * vol_traded_once) / p.short_.position;
				p.short_.avg_posi_cost = (p.short_.avg_posi_cost * (p.short_.position - vol_traded_once) + order.limit_price * vol_traded_once) / p.short_.position;
				break;
			case eDirOffset::BuyClose:
			case eDirOffset::BuyCloseToday:
			case eDirOffset::BuyCloseYesterday:
				p.short_.position -= vol_traded_once; break;
			case eDirOffset::SellClose:
			case eDirOffset::SellCloseToday:
			case eDirOffset::SellCloseYesterday:
				p.long_.position -= vol_traded_once; break;
			}
		}
		
		print_position(p, "handle_trade");
	}
	
	//删除报单及持仓
	if (order.order_status == eOrderStatus::AllTraded)
	{
		auto it_o = _order_map.find(order.order_ref);
		if (it_o != _order_map.end()) {_order_map.erase(it_o);}

		Position& p = _position_map[order.instrument_id];
		if (p.long_.position == 0)
		{
			p.long_.avg_open_cost = 0;
			p.long_.avg_posi_cost = 0;
		}
		if (p.short_.position == 0)
		{
			p.short_.avg_open_cost = 0;
			p.short_.avg_posi_cost = 0;
		}
		if (p.long_.position + p.long_.open_no_trade == 0 &&
			p.short_.position + p.short_.open_no_trade == 0)
		{
			_position_map.erase(order.instrument_id);
		}
	}

	if (_order_event.on_trade) { _order_event.on_trade(order); }
}

void realtime::handle_cancel(const Order& order)
{
	//更新持仓
	Position& p = _position_map[order.instrument_id];
	const int& volume_total = order.volume_total;

	if (_instrument_map[order.instrument_id].is_use_history)
	{
		switch (order.dir_offset)
		{
		case eDirOffset::BuyOpen:
			p.long_.open_no_trade -= volume_total; break;
		case eDirOffset::SellOpen:
			p.short_.open_no_trade -= volume_total; break;
		case eDirOffset::BuyCloseYesterday:
		case eDirOffset::BuyClose:
			p.short_.closeable += volume_total;
			p.short_.his_closeable += volume_total;
			break;
		case eDirOffset::BuyCloseToday:
			p.short_.closeable += volume_total;
			p.short_.today_closeable += volume_total;
			break;
		case eDirOffset::SellCloseYesterday:
		case eDirOffset::SellClose:
			p.long_.closeable += volume_total;
			p.long_.his_closeable += volume_total;
			break;
		case eDirOffset::SellCloseToday:
			p.long_.closeable += volume_total;
			p.long_.today_closeable += volume_total;
			break;
		}
	}
	else
	{
		switch (order.dir_offset)
		{
		case eDirOffset::BuyOpen:
			p.long_.open_no_trade -= volume_total; break;
		case eDirOffset::SellOpen:
			p.short_.open_no_trade -= volume_total; break;
		case eDirOffset::BuyClose:
		case eDirOffset::BuyCloseToday:
		case eDirOffset::BuyCloseYesterday:
			p.short_.closeable += volume_total; break;
		case eDirOffset::SellClose:
		case eDirOffset::SellCloseToday:
		case eDirOffset::SellCloseYesterday:
			p.long_.closeable += volume_total; break;
		}
	}

	//删除报单
	auto iter = _order_map.find(order.order_ref);
	if (iter != _order_map.end()) { _order_map.erase(iter); }
	
	if (_order_event.on_cancel) { _order_event.on_cancel(order); }

	print_position(p, "handle_cancel");
}

void realtime::handle_error(const Order& order)
{
	auto iter = _order_map.find(order.order_ref);
	if (iter != _order_map.end()) { _order_map.erase(iter); }

	if (_order_event.on_error) { _order_event.on_error(order); }
}

void realtime::load_trader_data()
{
	get_trader().get_trader_data(_instrument_map, _position_map, _order_map);
}



void realtime::print_order(const Order& order)
{
	printf("\n");
	printf("order_ref = %llu\n", order.order_ref);
	printf("exchange_id = %s\n", order.exchange_id);
	printf("instrument_id = %s\n", order.instrument_id);
	printf("dir_offset = %s\n", geteDirOffsetString(order.dir_offset).c_str());
	printf("order_flag = %s\n", geteOrderFlagString(order.order_flag).c_str());
	printf("limit_price = %.2f\n", order.limit_price);
	printf("volume_total_original = %d\n", order.volume_total_original);
	printf("volume_traded = %d\n", order.volume_traded);
	printf("volume_total = %d\n", order.volume_total);
	printf("order_submit_status = %s\n", geteOrderSubmitStatusString(order.order_submit_status).c_str());
	printf("order_status = %s\n", geteOrderStatusString(order.order_status).c_str());
	printf("status_msg = %s\n", order.status_msg);
	printf("insert_time = %s\n", order.insert_time);
	//printf("active_time = %s\n", order.active_time);
	//printf("suspend_time = %s\n", order.suspend_time);
	//printf("update_time = %s\n", order.update_time);
	printf("cancel_time = %s\n", order.cancel_time);
	printf("error_id = %d\n", order.error_id);
	printf("error_msg = %s\n", order.error_msg);
	//printf("is_cancelable = %d\n", order.is_cancelable);
	printf("\n");
}

void realtime::print_position(const Position& posi, const char* msg)
{
	printf("\n");
	printf("Update %s in %s\n", posi.id.c_str(), msg);
	if (posi.long_.position || posi.long_.open_no_trade)
	{
		printf("long_.position = %d\n", posi.long_.position);
		printf("long_.today_position = %d\n", posi.long_.today_position);
		printf("long_.his_position = %d\n", posi.long_.his_position);
		printf("long_.closeable = %d\n", posi.long_.closeable);
		printf("long_.his_closeable = %d\n", posi.long_.his_closeable);
		printf("long_.today_closeable = %d\n", posi.long_.today_closeable);
		printf("long_.avg_open_cost = %.2f\n", posi.long_.avg_open_cost);
		printf("long_.avg_posi_cost = %.2f\n", posi.long_.avg_posi_cost);
		printf("long_.open_no_trade = %d\n", posi.long_.open_no_trade);
		printf("\n");
	}

	if (posi.short_.position || posi.short_.open_no_trade)
	{
		printf("short_.position = %d\n", posi.short_.position);
		printf("short_.today_position = %d\n", posi.short_.today_position);
		printf("short_.his_position = %d\n", posi.short_.his_position);
		printf("short_.closeable = %d\n", posi.short_.closeable);
		printf("short_.his_closeable = %d\n", posi.short_.his_closeable);
		printf("short_.today_closeable = %d\n", posi.short_.today_closeable);
		printf("short_.avg_open_cost = %.2f\n", posi.short_.avg_open_cost);
		printf("short_.avg_posi_cost = %.2f\n", posi.short_.avg_posi_cost);
		printf("short_.open_no_trade = %d\n", posi.short_.open_no_trade);
		printf("\n");
	}
}
