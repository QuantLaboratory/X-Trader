#pragma once

#include "trader_api.h"
#include "ThostFtdcTraderApi.h"

#include <iostream>
#include <chrono>
#include <iomanip>
#include <thread>


class ctp_trader : public trader_api, public CThostFtdcTraderSpi
{
public:
	ctp_trader(std::map<std::string, std::string>& config, std::set<std::string>& contracts);
	virtual ~ctp_trader();

public:
	virtual void release() override;
	virtual orderref_t insert_order(eOrderFlag order_flag, const std::string& contract, eDirOffset dir_offset, double price, int volume) override;
	virtual bool cancel_order(const orderref_t order_ref) override;
	virtual std::string get_trading_day() const override;
	virtual void get_trader_data(InstrumentMap& i_map, PositionMap& p_map, OrderMap& o_map) override;
	virtual void get_account() override;

private:
	void req_auth();
	void req_user_login();
	void req_user_logout();
	void req_settlement_confirm();
	void req_qry_instrument();
	void req_qry_position();
	void req_qry_order();
	inline uint32_t generate_reqid() { return _req_id.fetch_add(1); }

public:
	///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
	virtual void OnFrontConnected() override;
	///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
	///@param nReason 错误原因
	///        0x1001 网络读失败
	///        0x1002 网络写失败
	///        0x2001 接收心跳超时
	///        0x2002 发送心跳失败
	///        0x2003 收到错误报文
	virtual void OnFrontDisconnected(int nReason) override;
	///客户端认证响应
	virtual void OnRspAuthenticate(CThostFtdcRspAuthenticateField* pRspAuthenticateField, CThostFtdcRspInfoField* pRspInfo, int nRequorder_id, bool bIsLast) override;
	///登录请求响应
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequorder_id, bool bIsLast) override;
	///登出请求响应
	virtual void OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequorder_id, bool bIsLast) override;
	///投资者结算结果确认响应
	virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequorder_id, bool bIsLast) override;
	///请求查询合约响应
	virtual void OnRspQryInstrument(CThostFtdcInstrumentField* pInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequorder_id, bool bIsLast) override;
	///请求查询投资者持仓响应
	virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField* pInvestorPosition, CThostFtdcRspInfoField* pRspInfo, int nRequorder_id, bool bIsLast) override;
	///请求查询资金账户响应
	virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField* pTradingAccount, CThostFtdcRspInfoField* pRspInfo, int nRequorder_id, bool bIsLast) override;
	///请求查询报单响应
	virtual void OnRspQryOrder(CThostFtdcOrderField* pOrder, CThostFtdcRspInfoField* pRspInfo, int nRequorder_id, bool bIsLast) override;
	///报单录入请求响应
	virtual void OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo, int nRequorder_id, bool bIsLast) override;
	///报单操作请求响应
	virtual void OnRspOrderAction(CThostFtdcInputOrderActionField* pInputOrderAction, CThostFtdcRspInfoField* pRspInfo, int nRequorder_id, bool bIsLast) override;
	///报单通知
	virtual void OnRtnOrder(CThostFtdcOrderField* pOrder) override;
	///成交通知
	virtual void OnRtnTrade(CThostFtdcTradeField* pTrade) override;
	///报单录入错误回报
	virtual void OnErrRtnOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo) override;
	///报单操作错误回报
	virtual void OnErrRtnOrderAction(CThostFtdcOrderActionField* pOrderAction, CThostFtdcRspInfoField* pRspInfo) override;


private:
	inline void to_ctp_dirOffset(const eDirOffset& local, CThostFtdcInputOrderField& ctp)
	{
		switch (local)
		{
		case eDirOffset::BuyOpen:
			ctp.Direction = THOST_FTDC_D_Buy;
			ctp.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
			break;
		case eDirOffset::SellOpen:
			ctp.Direction = THOST_FTDC_D_Sell;
			ctp.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
			break;
		case eDirOffset::BuyClose:
			ctp.Direction = THOST_FTDC_D_Buy;
			ctp.CombOffsetFlag[0] = THOST_FTDC_OF_Close;
			break;
		case eDirOffset::SellClose:
			ctp.Direction = THOST_FTDC_D_Sell;
			ctp.CombOffsetFlag[0] = THOST_FTDC_OF_Close;
			break;
		case eDirOffset::BuyCloseToday:
			ctp.Direction = THOST_FTDC_D_Buy;
			ctp.CombOffsetFlag[0] = THOST_FTDC_OF_CloseToday;
			break;
		case eDirOffset::SellCloseToday:
			ctp.Direction = THOST_FTDC_D_Sell;
			ctp.CombOffsetFlag[0] = THOST_FTDC_OF_CloseToday;
			break;
		case eDirOffset::BuyCloseYesterday:
			ctp.Direction = THOST_FTDC_D_Buy;
			ctp.CombOffsetFlag[0] = THOST_FTDC_OF_CloseYesterday;
			break;
		case eDirOffset::SellCloseYesterday:
			ctp.Direction = THOST_FTDC_D_Sell;
			ctp.CombOffsetFlag[0] = THOST_FTDC_OF_CloseYesterday;
			break;
		}
	}

	inline void to_local_dirOffset(const CThostFtdcOrderField& ctp, eDirOffset& local)
	{
		auto Direction = ctp.Direction;
		auto OffsetFlag = ctp.CombOffsetFlag[0];
		switch (Direction)
		{
		case THOST_FTDC_D_Buy:
			if (OffsetFlag == THOST_FTDC_OF_Open)
			{
				local = eDirOffset::BuyOpen;
			}
			else if (OffsetFlag == THOST_FTDC_OF_Close)
			{
				local = eDirOffset::BuyClose;
			}
			else if (OffsetFlag == THOST_FTDC_OF_CloseToday)
			{
				local = eDirOffset::BuyCloseToday;
			}
			else if (OffsetFlag == THOST_FTDC_OF_CloseYesterday)
			{
				local = eDirOffset::BuyCloseYesterday;
			}
			break;
		case THOST_FTDC_D_Sell:
			if (OffsetFlag == THOST_FTDC_OF_Open)
			{
				local = eDirOffset::SellOpen;
			}
			else if (OffsetFlag == THOST_FTDC_OF_Close)
			{
				local = eDirOffset::SellClose;
			}
			else if (OffsetFlag == THOST_FTDC_OF_CloseToday)
			{
				local = eDirOffset::SellCloseToday;
			}
			else if (OffsetFlag == THOST_FTDC_OF_CloseYesterday)
			{
				local = eDirOffset::SellCloseYesterday;
			}
			break;
		}
	}

	inline void to_local_orderSubmitStatus(const char& ctp, eOrderSubmitStatus& local)
	{
		switch (ctp)
		{
		case THOST_FTDC_OSS_InsertSubmitted:///已经提交
			local = eOrderSubmitStatus::InsertSubmitted; break;
		case THOST_FTDC_OSS_CancelSubmitted:///撤单已经提交
			local = eOrderSubmitStatus::CancelSubmitted; break;
		case THOST_FTDC_OSS_ModifySubmitted:///修改已经提交
			local = eOrderSubmitStatus::ModifySubmitted; break;
		case THOST_FTDC_OSS_Accepted:///已经接受
			local = eOrderSubmitStatus::Accepted; break;
		case THOST_FTDC_OSS_InsertRejected:///报单已经被拒绝
			local = eOrderSubmitStatus::InsertRejected; break;
		case THOST_FTDC_OSS_CancelRejected:///撤单已经被拒绝
			local = eOrderSubmitStatus::CancelRejected; break;
		case THOST_FTDC_OSS_ModifyRejected:///改单已经被拒绝
			local = eOrderSubmitStatus::ModifyRejected; break;
		}
	}

	inline void to_local_orderStatus(const char& ctp, eOrderStatus& local)
	{
		switch (ctp)
		{
		case THOST_FTDC_OST_AllTraded:///全部成交
			local = eOrderStatus::AllTraded; break;
		case THOST_FTDC_OST_PartTradedQueueing:///部分成交还在队列中
			local = eOrderStatus::PartTradedQueueing; break;
		case THOST_FTDC_OST_PartTradedNotQueueing:///部分成交不在队列中
			local = eOrderStatus::PartTradedNotQueueing; break;
		case THOST_FTDC_OST_NoTradeQueueing:///未成交还在队列中
			local = eOrderStatus::NoTradeQueueing; break;
		case THOST_FTDC_OST_NoTradeNotQueueing:///未成交不在队列中
			local = eOrderStatus::NoTradeNotQueueing; break;
		case THOST_FTDC_OST_Canceled:///撤单
			local = eOrderStatus::Canceled; break;
		case THOST_FTDC_OST_Unknown:///未知
			local = eOrderStatus::Unknown; break;
		case THOST_FTDC_OST_NotTouched:///尚未触发
			local = eOrderStatus::NotTouched; break;
		case THOST_FTDC_OST_Touched:///已触发
			local = eOrderStatus::Touched; break;
		}
	}


private:
	CThostFtdcTraderApi* _td_api;
	std::atomic<uint32_t> _req_id;

	std::string _broker_id;
	std::string _user_id;
	std::string _password;
	std::string _app_id;
	std::string _auth_code;
	std::string _user_product_Info;

	uint32_t _front_id;
	uint32_t _session_id;
	std::atomic<uint32_t> _order_ref;

	InstrumentMap _instrument_map{};
	OrderMap _order_map{};
	PositionMap _position_map{};
	std::set<std::string> _contracts{};


private:
	inline void print(CThostFtdcRspInfoField* p)
	{
		printf("\n");
		printf("ErrorID = %d\n", p->ErrorID);///错误代码
		printf("ErrorMsg = %s\n", p->ErrorMsg);///错误信息
		printf("\n");
	}

	inline void print(CThostFtdcInvestorPositionField* p)
	{
		printf("\n");
		printf("PosiDirection = %s\n", getPosiDirectionString(p->PosiDirection).c_str());///持仓多空方向
		printf("PositionDate = %s\n", getPositionDateString(p->PositionDate).c_str());///持仓日期
		printf("YdPosition = %d\n", p->YdPosition);///上日持仓
		printf("Position = %d\n", p->Position);///今日持仓
		printf("LongFrozen = %d\n", p->LongFrozen);///多头冻结
		printf("ShortFrozen = %d\n", p->ShortFrozen);///空头冻结
		//printf("LongFrozenAmount = %.2f\n", p->LongFrozenAmount);///开仓冻结金额
		//printf("ShortFrozenAmount = %.2f\n", p->ShortFrozenAmount);///开仓冻结金额
		////printf("OpenVolume = %d\n", p->OpenVolume);///开仓量
		//printf("CloseVolume = %d\n", p->CloseVolume);///平仓量
		//printf("OpenAmount = %.2f\n", p->OpenAmount);///开仓金额
		//printf("CloseAmount = %.2f\n", p->CloseAmount);///平仓金额
		printf("PositionCost = %.2f\n", p->PositionCost);///持仓成本
		//printf("PreMargin = %.2f\n", p->PreMargin);///上次占用的保证金
		//printf("UseMargin = %.2f\n", p->UseMargin);///占用的保证金
		//printf("FrozenMargin = %.2f\n", p->FrozenMargin);///冻结的保证金
		//printf("FrozenCash = %.2f\n", p->FrozenCash);///冻结的资金
		//printf("FrozenCommission = %.2f\n", p->FrozenCommission);///冻结的手续费
		//printf("CashIn = %.2f\n", p->CashIn);///资金差额
		printf("Commission = %.2f\n", p->Commission);///手续费
		printf("CloseProfit = %.2f\n", p->CloseProfit);///平仓盈亏
		printf("PositionProfit = %.2f\n", p->PositionProfit);///持仓盈亏
		printf("PreSettlementPrice = %.2f\n", p->PreSettlementPrice);///上次结算价
		printf("SettlementPrice = %.2f\n", p->SettlementPrice);///本次结算价
		//printf("TradingDay = %s\n", p->TradingDay);///交易日
		printf("SettlementID = %d\n", p->SettlementID);///结算编号
		printf("OpenCost = %.2f\n", p->OpenCost);///开仓成本
		//printf("ExchangeMargin = %.2f\n", p->ExchangeMargin);///交易所保证金
		//printf("CloseProfitByDate = %.2f\n", p->CloseProfitByDate);///逐日盯市平仓盈亏
		//printf("CloseProfitByTrade = %.2f\n", p->CloseProfitByTrade);///逐笔对冲平仓盈亏
		printf("TodayPosition = %d\n", p->TodayPosition);///今日持仓
		//printf("MarginRateByMoney = %.2f\n", p->MarginRateByMoney);///保证金率
		//printf("MarginRateByVolume = %.2f\n", p->MarginRateByVolume);///保证金率(按手数)
		printf("ExchangeID = %s\n", p->ExchangeID);///交易所代码
		printf("InstrumentID = %s\n", p->InstrumentID);///合约代码
		printf("\n");
	}

	inline void print(CThostFtdcOrderField* p)
	{
		print_local_time(); printf("OnRtnOrder\n");
		//printf("BrokerID = %s\n", p->BrokerID);///经纪公司代码
		//printf("InvestorID = %s\n", p->InvestorID);///投资者代码
		printf("OrderRef = %s\n", p->OrderRef);///报单引用
		//printf("UserID = %s\n", p->UserID);///用户代码
		//printf("OrderPriceType = %s\n", getOrderPriceTypeString(p->OrderPriceType).c_str());///报单价格条件
		printf("Direction = %s\n", getDirectionString(p->Direction).c_str());///买卖方向
		printf("CombOffsetFlag[0] = %s\n", getOffsetFlagString(p->CombOffsetFlag[0]).c_str());///组合开平标志
		printf("LimitPrice = %.2f\n", p->LimitPrice);///价格
		
		//printf("TimeCondition = %s\n", getTimeConditionString(p->TimeCondition).c_str());///有效期类型
		//printf("GTDDate = %s\n", p->GTDDate);///GTD日期
		//printf("VolumeCondition = %s\n", getVolumeConditionString(p->VolumeCondition).c_str());///成交量类型
		//printf("MinVolume = %d\n", p->MinVolume);///最小成交量
		//printf("ContingentCondition = %s\n", getContingentConditionString(p->ContingentCondition).c_str());///触发条件
		//printf("StopPrice = %.2f\n", p->StopPrice);///止损价
		//printf("ForceCloseReason = %s\n", getForceCloseReasonString(p->ForceCloseReason).c_str());///强平原因
		//printf("	IsAutoSuspend = %d\n", p->IsAutoSuspend);///自动挂起标志
		printf("RequestID = %d\n", p->RequestID);///请求编号
		printf("OrderLocalID = %s\n", p->OrderLocalID);///本地报单编号
		printf("OrderSysID = %s\n", p->OrderSysID);///报单编号
		printf("OrderSubmitStatus = %s\n", getOrderSubmitStatusString(p->OrderSubmitStatus).c_str());///报单提交状态
		//printf("TradingDay = %s\n", p->TradingDay);///交易日
		//printf("SettlementID = %d\n", p->SettlementID);///结算编号
		printf("OrderStatus = %s\n", getOrderStatusString(p->OrderStatus).c_str());///报单状态
		printf("StatusMsg = %s\n", p->StatusMsg);///状态信息
		printf("VolumeTotalOriginal = %d\n", p->VolumeTotalOriginal);///数量
		printf("VolumeTraded = %d\n", p->VolumeTraded);///今成交数量
		printf("VolumeTotal = %d\n", p->VolumeTotal);///剩余数量
		//printf("	InsertDate = %s\n", p->InsertDate);///报单日期
		printf("InsertTime = %s\n", p->InsertTime);///委托时间
		//printf("ActiveTime = %s\n", p->ActiveTime);///激活时间
		//printf("SuspendTime = %s\n", p->SuspendTime);///挂起时间
		//printf("UpdateTime = %s\n", p->UpdateTime);///最后修改时间
		printf("CancelTime = %s\n", p->CancelTime);///撤销时间
		printf("FrontID = %d\n", p->FrontID);///前置编号
		printf("SessionID = %d\n", p->SessionID);///会话编号
		printf("UserProductInfo = %s\n", p->UserProductInfo);///用户端产品信息		
		//printf("UserForceClose = %d\n", p->UserForceClose);///用户强评标志
		printf("InstrumentID = %s\n", p->InstrumentID);///合约代码
		printf("ExchangeID = %s\n", p->ExchangeID);///交易所代码
		//printf("ExchangeInstID = %s\n", p->ExchangeInstID);///合约在交易所的代码
		printf("\n");
	}

	inline void print(CThostFtdcTradeField* p)
	{
		print_local_time();
		printf("OnRtnTrade\n");
		//printf("BrokerID = %s\n", p->BrokerID);///经纪公司代码
		//printf("InvestorID = %s\n", p->InvestorID);///投资者代码
		printf("OrderRef = %s\n", p->OrderRef);///报单引用
		//printf("UserID = %s\n", p->UserID);///用户代码
		printf("TradeID = %s\n", p->TradeID);///成交编号
		printf("Direction = %s\n", getDirectionString(p->Direction).c_str());///买卖方向
		printf("OrderSysID = %s\n", p->OrderSysID);///报单编号
		printf("OffsetFlag = %s\n", getOffsetFlagString(p->OffsetFlag).c_str());///开平标志
		printf("Price = %.2f\n", p->Price);///价格
		printf("Volume = %d\n", p->Volume);///数量
		//printf("TradeDate = %s\n", p->TradeDate);///成交日期
		printf("TradeTime = %s\n", p->TradeTime);///成交时间
		printf("PriceSource = %s\n", getPriceSourceString(p->PriceSource).c_str());///成交价来源
		printf("OrderLocalID = %s\n", p->OrderLocalID);///本地报单编号
		//printf("TradingDay = %s\n", p->TradingDay);///交易日
		printf("SettlementID = %d\n", p->SettlementID);///结算编号
		printf("InstrumentID = %s\n", p->InstrumentID);///合约代码
		//printf("ExchangeInstID = %s\n", p->ExchangeInstID);///合约在交易所的代码
		printf("\n");
	}

	void print_local_time()
	{
		// 获取当前时间点  
		auto now = std::chrono::system_clock::now();

		// 转换为time_t以获取tm结构体  
		std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
		std::tm* now_tm = std::localtime(&now_time_t);

		// 获取小时、分钟和秒  
		int hours = now_tm->tm_hour;
		int minutes = now_tm->tm_min;
		int seconds = now_tm->tm_sec;

		// 获取毫秒  
		auto duration = now.time_since_epoch();
		auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000;

		// 打印时间  
		std::cout << std::setfill('0')
			<< std::setw(2) << hours << ":"
			<< std::setw(2) << minutes << ":"
			<< std::setw(2) << seconds << ":"
			<< std::setw(3) << millis << " ";
	}

	std::map<char, std::string> PosiDirectionToString =
	{
		{THOST_FTDC_PD_Net, "Net"},
		{THOST_FTDC_PD_Long, "Long"},
		{THOST_FTDC_PD_Short, "Short"}
	};

	inline std::string getPosiDirectionString(char t)
	{
		return PosiDirectionToString[t];
	}

	std::map<char, std::string> PositionDateToString =
	{
		{THOST_FTDC_PSD_Today, "Today"},
		{THOST_FTDC_PSD_History, "History"}
	};

	inline std::string getPositionDateString(char t)
	{
		return PositionDateToString[t];
	}

	std::map<char, std::string> OrderPriceTypeToString =
	{
		{THOST_FTDC_OPT_AnyPrice, "AnyPrice"},
		{THOST_FTDC_OPT_LimitPrice, "LimitPrice"},
		{THOST_FTDC_OPT_BestPrice, "BestPrice"},
		{THOST_FTDC_OPT_LastPrice, "LastPrice"}
	};

	inline std::string getOrderPriceTypeString(char t)
	{
		return OrderPriceTypeToString[t];
	}

	std::map<char, std::string> DirectionToString =
	{
		{THOST_FTDC_D_Buy, "Buy"},
		{THOST_FTDC_D_Sell, "Sell"}
	};

	inline std::string getDirectionString(char t)
	{
		return DirectionToString[t];
	}

	std::map<char, std::string> OffsetFlagToString =
	{
		{THOST_FTDC_OF_Open, "Open"},
		{THOST_FTDC_OF_Close, "Close"},
		{THOST_FTDC_OF_ForceClose, "ForceClose"},
		{THOST_FTDC_OF_CloseToday, "CloseToday"},
		{THOST_FTDC_OF_CloseYesterday, "CloseYesterday"},
		{THOST_FTDC_OF_ForceOff, "ForceOff"},
		{THOST_FTDC_OF_LocalForceClose, "LocalForceClose"}
	};

	inline std::string getOffsetFlagString(char t)
	{
		return OffsetFlagToString[t];
	}

	std::map<char, std::string> TimeConditionToString =
	{
		{THOST_FTDC_TC_IOC, "IOC"},
		{THOST_FTDC_TC_GFS, "GFS"},
		{THOST_FTDC_TC_GFD, "GFD"},
		{THOST_FTDC_TC_GFA, "GFA"}
	};

	inline std::string getTimeConditionString(char t)
	{
		return TimeConditionToString[t];
	}

	std::map<char, std::string> VolumeConditionToString =
	{
		{THOST_FTDC_VC_AV, "AV"},
		{THOST_FTDC_VC_MV, "MV"},
		{THOST_FTDC_VC_CV, "CV"}
	};

	inline std::string getVolumeConditionString(char t)
	{
		return VolumeConditionToString[t];
	}

	std::map<char, std::string> ContingentConditionToString =
	{
		{THOST_FTDC_CC_Immediately, "Immediately"},
		{THOST_FTDC_CC_Touch, "Touch"},
		{THOST_FTDC_CC_TouchProfit, "TouchProfit"}
	};

	inline std::string getContingentConditionString(char t)
	{
		return ContingentConditionToString[t];
	}

	std::map<char, std::string> ForceCloseReasonToString =
	{
		{THOST_FTDC_FCC_NotForceClose, "NotForceClose"},
		{THOST_FTDC_FCC_LackDeposit, "LackDeposit"},
		{THOST_FTDC_FCC_Violation, "Violation"},
		{THOST_FTDC_FCC_Other, "Other"},
		{THOST_FTDC_FCC_PersonDeliv, "PersonDeliv"},
		{THOST_FTDC_FCC_Notverifycapital, "Notverifycapital"}
	};

	inline std::string getForceCloseReasonString(char t)
	{
		return ForceCloseReasonToString[t];
	}

	std::map<char, std::string> OrderSubmitStatusToString =
	{
		{THOST_FTDC_OSS_InsertSubmitted, "InsertSubmitted"},
		{THOST_FTDC_OSS_CancelSubmitted, "CancelSubmitted"},
		{THOST_FTDC_OSS_ModifySubmitted, "ModifySubmitted"},
		{THOST_FTDC_OSS_Accepted, "Accepted"},
		{THOST_FTDC_OSS_InsertRejected, "InsertRejected"},
		{THOST_FTDC_OSS_CancelRejected, "CancelRejected"},
		{THOST_FTDC_OSS_ModifyRejected, "ModifyRejected"}
	};

	inline std::string getOrderSubmitStatusString(char t)
	{
		return OrderSubmitStatusToString[t];
	}

	std::map<char, std::string> OrderStatusToString =
	{
		{THOST_FTDC_OST_AllTraded, "AllTraded"},
		{THOST_FTDC_OST_PartTradedQueueing, "PartTradedQueueing"},
		{THOST_FTDC_OST_PartTradedNotQueueing, "PartTradedNotQueueing"},
		{THOST_FTDC_OST_NoTradeQueueing, "NoTradeQueueing"},
		{THOST_FTDC_OST_NoTradeNotQueueing, "NoTradeNotQueueing"},
		{THOST_FTDC_OST_Canceled, "Canceled"},
		{THOST_FTDC_OST_Unknown, "Unknown"},
		{THOST_FTDC_OST_NotTouched, "NotTouched"},
		{THOST_FTDC_OST_Touched, "Touched"}
	};

	inline std::string getOrderStatusString(char t)
	{
		return OrderStatusToString[t];
	}

	std::map<char, std::string> PriceSourceToString =
	{
		{THOST_FTDC_PSRC_LastPrice, "LastPrice"},
		{THOST_FTDC_PSRC_Buy, "Buy"},
		{THOST_FTDC_PSRC_Sell, "Sell"},
		{THOST_FTDC_PSRC_OTC, "OTC"}
	};

	inline std::string getPriceSourceString(char t)
	{
		return PriceSourceToString[t];
	}

	inline void print_position()
	{
		printf("\n");
		for (auto& it : _position_map)
		{
			if (it.second.long_.position)
			{
				printf("instrument_id = %s\n", it.first.c_str());
				printf("long_.position = %d\n", it.second.long_.position);
				printf("long_.today_position = %d\n", it.second.long_.today_position);
				printf("long_.his_position = %d\n", it.second.long_.his_position);
				printf("long_.closeable = %d\n", it.second.long_.closeable);
				printf("long_.his_closeable = %d\n", it.second.long_.his_closeable);
				printf("long_.today_closeable = %d\n", it.second.long_.today_closeable);
				printf("long_.avg_open_cost = %.2f\n", it.second.long_.avg_open_cost);
				printf("long_.avg_posi_cost = %.2f\n", it.second.long_.avg_posi_cost);
				printf("long_.open_no_trade = %d\n", it.second.long_.open_no_trade);
				printf("\n");
			}
			
			if (it.second.short_.position)
			{
				printf("instrument_id = %s\n", it.first.c_str());
				printf("short_.position = %d\n", it.second.short_.position);
				printf("short_.today_position = %d\n", it.second.short_.today_position);
				printf("short_.his_position = %d\n", it.second.short_.his_position);
				printf("short_.closeable = %d\n", it.second.short_.closeable);
				printf("short_.his_closeable = %d\n", it.second.short_.his_closeable);
				printf("short_.today_closeable = %d\n", it.second.short_.today_closeable);
				printf("short_.avg_open_cost = %.2f\n", it.second.short_.avg_open_cost);
				printf("short_.avg_posi_cost = %.2f\n", it.second.short_.avg_posi_cost);
				printf("short_.open_no_trade = %d\n", it.second.short_.open_no_trade);
				printf("\n");
			}
		}
		printf("\n");
	}
};
