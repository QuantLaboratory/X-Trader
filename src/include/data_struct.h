#pragma once

#include "define.h"


///盘口方向
enum class eTapeDir
{
	Up = 1,///向上
	Flat = 0,///平
	Down = -1,///向下
};

///盘口状态
enum class eTapeStatus
{
	DoubleOpen,///双开
	Open,///开仓
	Exchange,///换手
	Close,//平仓
	DoubleClose,///双平
	Unknown,///未知
};

//盘口信息
struct Tape
{
	uint32_t last_volume;//现手
	double last_open_interest;//增仓
	eTapeDir tape_dir;//盘口方向

	Tape() : last_volume(0), last_open_interest(.0F), tape_dir(eTapeDir::Flat) {}

	eTapeStatus get_tape_status()
	{
		if (last_volume == last_open_interest && last_open_interest > 0)
		{
			return eTapeStatus::DoubleOpen;
		}
		else if (last_volume > last_open_interest && last_open_interest > 0)
		{
			return eTapeStatus::Open;
		}
		else if (last_volume > last_open_interest && last_open_interest == 0)
		{
			return eTapeStatus::Exchange;
		}
		else if (last_volume > -last_open_interest && -last_open_interest > 0)
		{
			return eTapeStatus::Close;
		}
		else if (last_volume == -last_open_interest && -last_open_interest > 0)
		{
			return eTapeStatus::DoubleClose;
		}
		else
		{
			return eTapeStatus::Unknown;
		}
	}
};


///K线
struct Sample
{
	std::string id;///合约代码
	uint32_t period;///周期
	std::string time;///时间
	double open;/// 开
	double high;/// 高
	double low;/// 低
	double close;/// 收
	int volume;///成交量
	bool is_new = true;///是否为新
	
	double price_tick; //最小变动价位
	double poc;///订单流中POC
	int32_t delta;///订单流中delta(active_buy_volume - active_sell_volume)
	//订单流中的明细
	std::map<double, uint32_t> active_buy_volume;
	std::map<double, uint32_t> active_sell_volume;

	uint32_t get_buy_volume(double price) const
	{
		auto it = active_buy_volume.find(price);
		if (it == active_buy_volume.end()) { return 0; }
		return it->second;
	}

	uint32_t get_sell_volume(double price) const
	{
		auto it = active_sell_volume.find(price);
		if (it == active_sell_volume.end()) { return 0; }
		return it->second;
	}

	std::vector<std::tuple<double, uint32_t, uint32_t>> get_order_book() const
	{
		std::vector<std::tuple<double, uint32_t, uint32_t>> result;
		for (double price = low; price <= high; price += price_tick)
		{
			result.emplace_back(std::make_tuple(price, get_buy_volume(price), get_sell_volume(price)));
		}
		return result;
	}

	//获取不平衡订单
	std::pair<std::shared_ptr<std::vector<double>>, std::shared_ptr<std::vector<double>>> get_imbalance(uint32_t multiple) const
	{
		//需求失衡(供大于求)
		auto demand_imbalance = std::make_shared<std::vector<double>>();
		//供给失衡(供不应求)
		auto supply_imbalance = std::make_shared<std::vector<double>>();
		//构建订单薄
		auto order_book = get_order_book();

		for (size_t i = 0; i < order_book.size() - 1; i++)
		{
			auto demand_cell = order_book[i];
			auto supply_cell = order_book[i + 1];
			if (std::get<1>(demand_cell) * multiple > std::get<2>(supply_cell))
			{
				demand_imbalance->emplace_back(std::get<0>(demand_cell));
			}
			else if (std::get<2>(supply_cell) * multiple > std::get<1>(demand_cell))
			{
				supply_imbalance->emplace_back(std::get<0>(supply_cell));
			}
		}
		return std::make_pair(demand_imbalance, supply_imbalance);
	}

	Sample() : time(""), period(0U), open(0), close(0), high(0), low(0), volume(0), delta(0), poc(.0), price_tick(1.0) {}

	void clear()
	{
		period = 0U; open = .0; high = .0; low = .0; close = .0; volume = 0; delta = 0; poc = 0;
		active_buy_volume.clear(); active_sell_volume.clear();
	}
};


///深度行情
struct MarketData
{
	//DateType trading_day; ///交易日
	//ExchangeIDType exchange_id; ///交易所代码
	InstrumentIDType instrument_id; ///合约代码
	TimeType update_time; ///最后修改时间
	int update_millisec; ///最后修改毫秒

	double pre_close_price; ///昨收盘
	//double pre_open_interest;  ///昨持仓量
	double pre_settlement_price; ///上次结算价

	double last_price; ///最新价
	int volume; ///数量
	int last_volume; ///成交数量
	//double turnover; ///成交金额
	double open_interest; ///持仓量
	double last_open_interest; ///增仓

	double open_price; ///今开盘
	double highest_price; ///最高价
	double lowest_price; ///最低价
	double upper_limit_price; ///涨停板价
	double lower_limit_price; ///跌停板价

	//double close_price; ///今收盘
	double settlement_price; ///本次结算价

	double bid_price[5]; ///申买价
	int bid_volume[5]; ///申买量
	double ask_price[5]; ///申卖价
	int ask_volume[5]; ///申卖量

	double average_price; ///当日均价
	eTapeDir tape_dir;//盘口方向
};


///合约
struct Instrument
{
	ExchangeIDType exchange_id; ///交易所代码
	InstrumentIDType instrument_id; ///合约代码
	//int delivery_year; ///交割年份
	//int delivery_month; ///交割月
	//int max_market_order_volume; ///市价单最大下单量
	//int min_market_order_volume; ///市价单最小下单量
	//int max_limit_order_volume; ///限价单最大下单量
	//int min_limit_order_volume; ///限价单最小下单量
	int volume_multiple; ///合约数量乘数
	double price_tick; ///最小变动价位

	//DateType create_date; ///创建日
	//DateType open_date; ///上市日
	//DateType expire_date; ///到期日
	//DateType start_deliv_date; ///开始交割日
	//DateType end_deliv_date; ///结束交割日

	bool is_trading; ///当前是否交易
	bool is_use_history;///是否使用历史持仓

	//double long_margin_ratio; ///多头保证金率
	//double short_margin_ratio; ///空头保证金率
};

const Instrument null_instrument{};


///持仓明细
struct PosiDetail
{
	int position;///总仓
	int today_position;///今仓(上期所/上期能源专用)
	int his_position;///昨仓(上期所/上期能源专用)
	
	int closeable;///总可平量
	int today_closeable;///今可平量(上期所/上期能源专用)
	int his_closeable;///昨可平量(上期所/上期能源专用)
	int open_no_trade;///开仓还未成交
	double avg_open_cost;///平均开仓成本
	double avg_posi_cost;///平均持仓成本
};

///持仓
struct Position
{
	std::string id;///合约代码
	PosiDetail long_;///多
	PosiDetail short_;///空
};

const Position null_position{};


///买卖开平类型
enum class eDirOffset : int8_t
{
	BuyOpen,///买开
	SellOpen,///卖开
	BuyClose,///上期所：买平昨    其他所：买平(不区分平今平昨)
	SellClose,///上期所：卖平昨    其他所：卖平(不区分平今平昨)
	BuyCloseToday,///上期所：买平今    其他所：买平(不区分平今平昨)
	SellCloseToday,///上期所：卖平今    其他所：卖平(不区分平今平昨)
	BuyCloseYesterday,///上期所：买平昨    其他所：买平(不区分平今平昨)
	SellCloseYesterday///上期所：卖平昨    其他所：卖平(不区分平今平昨)
};

///报单提交状态类型
enum class eOrderSubmitStatus : int8_t
{
	InsertSubmitted,///已经提交
	CancelSubmitted,///撤单已经提交
	ModifySubmitted,///修改已经提交
	Accepted,///已经接受
	InsertRejected,///报单已经被拒绝
	CancelRejected,///撤单已经被拒绝
	ModifyRejected///改单已经被拒绝
};

///报单状态类型
enum class eOrderStatus : int8_t
{
	AllTraded,///全部成交
	PartTradedQueueing,///部分成交还在队列中
	PartTradedNotQueueing,///部分成交不在队列中
	NoTradeQueueing,///未成交还在队列中
	NoTradeNotQueueing,///未成交不在队列中
	Canceled,///撤单
	Unknown,///未知
	NotTouched,///尚未触发
	Touched///已触发
};

///事件类型
enum class eEventFlag : int8_t
{
	Order,///报单
	Trade,///成交
	Cancel,///撤单
	ErrorInsert,///报单错误
	ErrorCancel///撤单错误
};

///报单标志类型
enum class eOrderFlag : int8_t
{
	Limit,///限价单
	Market,///市价单
	FOK,///Fill or Kill
	FAK///Fill and Kill
};

///报单
struct Order
{
	eEventFlag event_flag;///事件类型
	orderref_t order_ref;///报单引用
	int front_id;///前置编号
	int session_id;///会话编号

	ExchangeIDType exchange_id; ///交易所代码
	InstrumentIDType instrument_id;///合约代码
	eDirOffset dir_offset;///买卖开平标志
	eOrderFlag order_flag;///报单标志
	double limit_price;///价格
	int volume_total_original;///数量
	int volume_traded;///今成交数量
	int volume_total;///剩余数量

	int request_id;///请求编号
	OrderLocalIDType order_local_id;///本地报单编号
	OrderSysIDType order_sys_id;///报单编号

	eOrderSubmitStatus order_submit_status;///报单提交状态
	eOrderStatus order_status;///报单状态
	ErrorMsgType status_msg;///状态信息

	//DateType insert_date;///报单日期
	TimeType insert_time;///委托时间
	//TimeType	active_time;///激活时间
	//TimeType suspend_time;///挂起时间
	//TimeType update_time;///最后修改时间
	TimeType cancel_time;///撤销时间

	int error_id;///错误代码
	ErrorMsgType error_msg;///错误信息

	//bool is_cancelable = true;///是否可以撤销
};

const Order null_order{};

struct tape_receiver
{
	virtual void on_tape(const Tape& tape) = 0;
};

struct bar_receiver
{
	virtual void on_bar(const Sample& bar) = 0;
};

extern "C"
{
	typedef void (PORTER_FLAG* tick_callback)(const MarketData&);
	typedef void (PORTER_FLAG* order_callback)(const Order&);
	typedef void (PORTER_FLAG* trade_callback)(const Order&);
	typedef void (PORTER_FLAG* cancel_callback)(const Order&);
	typedef void (PORTER_FLAG* error_callback)(const Order&);
	typedef void (PORTER_FLAG* update_callback)();
}

struct OrderEvent
{
	order_callback on_order;
	trade_callback on_trade;
	cancel_callback on_cancel;
	error_callback on_error;

	OrderEvent() = default;
	OrderEvent(order_callback order_cb, trade_callback trade_cb, cancel_callback cancel_cb, error_callback error_cb)
		: on_order(order_cb), on_trade(trade_cb), on_cancel(cancel_cb), on_error(error_cb) {}
};

using InstrumentMap = std::unordered_map<std::string, Instrument>;
using OrderMap = std::map<orderref_t, Order>;
using PositionMap = std::map<std::string, Position>;




inline std::map<eDirOffset, std::string> eDirOffsetToString =
{
	{eDirOffset::BuyOpen, "BuyOpen"},{eDirOffset::SellOpen, "SellOpen"},
	{eDirOffset::BuyClose, "BuyClose"},{eDirOffset::SellClose, "SellClose"},
	{eDirOffset::BuyCloseToday, "BuyCloseToday"},{eDirOffset::SellCloseToday, "SellCloseToday"},
	{eDirOffset::BuyCloseYesterday, "BuyCloseYesterday"},{eDirOffset::SellCloseYesterday, "SellCloseYesterday"}
};

inline std::string geteDirOffsetString(eDirOffset t)
{
	return eDirOffsetToString[t];
}

inline std::map<eOrderSubmitStatus, std::string> eOrderSubmitStatusToString =
{
	{eOrderSubmitStatus::InsertSubmitted, "InsertSubmitted"},
	{eOrderSubmitStatus::CancelSubmitted, "CancelSubmitted"},
	{eOrderSubmitStatus::ModifySubmitted, "ModifySubmitted"},
	{eOrderSubmitStatus::Accepted, "Accepted"},
	{eOrderSubmitStatus::InsertRejected, "InsertRejected"},
	{eOrderSubmitStatus::CancelRejected, "CancelRejected"},
	{eOrderSubmitStatus::ModifyRejected, "ModifyRejected"}
};

inline std::string geteOrderSubmitStatusString(eOrderSubmitStatus t)
{
	return eOrderSubmitStatusToString[t];
}

inline std::map<eOrderStatus, std::string> eOrderStatusToString =
{
	{eOrderStatus::AllTraded, "AllTraded"},
	{eOrderStatus::PartTradedQueueing, "PartTradedQueueing"},
	{eOrderStatus::PartTradedNotQueueing, "PartTradedNotQueueing"},
	{eOrderStatus::NoTradeQueueing, "NoTradeQueueing"},
	{eOrderStatus::NoTradeNotQueueing, "NoTradeNotQueueing"},
	{eOrderStatus::Canceled, "Canceled"},
	{eOrderStatus::Unknown, "Unknown"},
	{eOrderStatus::NotTouched, "NotTouched"},
	{eOrderStatus::Touched, "Touched"}
};

inline std::string geteOrderStatusString(eOrderStatus t)
{
	return eOrderStatusToString[t];
}

inline std::map<eOrderFlag, std::string> eOrderFlagToString =
{
	{eOrderFlag::Limit, "Limit"},
	{eOrderFlag::Market, "Market"},
	{eOrderFlag::FOK, "FOK"},
	{eOrderFlag::FAK, "FAK"}
};

inline std::string geteOrderFlagString(eOrderFlag t)
{
	return eOrderFlagToString[t];
}
