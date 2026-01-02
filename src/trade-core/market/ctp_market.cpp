#include "ctp_market.h"


ctp_market::ctp_market(std::map<std::string, std::string>& config, std::set<std::string>& contracts)
	: _req_id(0), _contracts(contracts)
{
	_broker_id = config["broker_id"];
	_user_id = config["user_id"];
	_password = config["password"];

	char flow_path[64]{};
	sprintf(flow_path, "flow/md/%s/%s/", _broker_id.c_str(), _user_id.c_str());
	if (!std::filesystem::exists(flow_path))
	{
		std::filesystem::create_directories(flow_path);
	}
	_md_api = CThostFtdcMdApi::CreateFtdcMdApi(flow_path, true, true);
	_md_api->RegisterSpi(this);
	_md_api->RegisterFront((char*)config["market_front"].c_str());
	_md_api->Init();
}

ctp_market::~ctp_market()
{
	_req_id = 0;
	_contracts.clear();
	_previous_tick_map.clear();
	_is_ready.store(false);
	if (_md_api)
	{
		_md_api->RegisterSpi(nullptr);
		_md_api->Release();
		_md_api = nullptr;
	}
}

void ctp_market::release()
{
	_req_id = 0;
	_contracts.clear();
	_previous_tick_map.clear();
	_is_ready.store(false);
	if (_md_api)
	{
		_md_api->RegisterSpi(nullptr);
		_md_api->Release();
		_md_api = nullptr;
	}
}


void ctp_market::req_user_login()
{
	CThostFtdcReqUserLoginField t{};
	strcpy(t.BrokerID, _broker_id.c_str());
	strcpy(t.UserID, _user_id.c_str());
	strcpy(t.Password, _password.c_str());

	int rtn = _md_api->ReqUserLogin(&t, _req_id++);
	if (rtn != 0) {}
}

void ctp_market::subscribe()
{
	for (const auto& iter : _contracts)
	{
		char* ppInsts[] = { const_cast<char*>(iter.c_str()) };
		_md_api->SubscribeMarketData(ppInsts, 1);
	}
}

void ctp_market::unsubscribe()
{
	for (const auto& iter : _contracts)
	{
		char* ppInsts[] = { const_cast<char*>(iter.c_str()) };
		_md_api->UnSubscribeMarketData(ppInsts, 1);
	}
}


///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
void ctp_market::OnFrontConnected()
{
	req_user_login();
}

///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
	///@param nReason 错误原因
	///        0x1001 网络读失败
	///        0x1002 网络写失败
	///        0x2001 接收心跳超时
	///        0x2002 发送心跳失败
	///        0x2003 收到错误报文
void ctp_market::OnFrontDisconnected(int nReason)
{
	printf("nReason=%d\n", nReason);
	char text[128]; sprintf(text, "%s Market Front Disconnected", _user_id.c_str());
	//send2wecom(text);
}

///登录请求响应
void ctp_market::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo && pRspInfo->ErrorID) { return; }
	if (bIsLast) { subscribe(); }
}

///登出请求响应
void ctp_market::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo && pRspInfo->ErrorID) { return; }
	if (bIsLast) {}
}

///订阅行情应答
void ctp_market::OnRspSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo && pRspInfo->ErrorID) { return; }
	if (bIsLast && !_is_ready.load())
	{
		_is_ready.store(true, std::memory_order_release);
		char text[128]; sprintf(text, "md@%s is ready!", _user_id.c_str());
		//send2wecom(text);
		
		printf("%s\n", text);
	}
}

///取消订阅行情应答
void ctp_market::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo && pRspInfo->ErrorID) { return; }
	if (bIsLast) {}
}

///深度行情通知
void ctp_market::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* ptr)
{
	auto it = _previous_tick_map.find(ptr->InstrumentID);
	if (it == _previous_tick_map.end())
	{
		_previous_tick_map[ptr->InstrumentID] = *ptr;
		return;
	}
	static MarketData _tick{};

	//strcpy(_tick.exchange_id, ptr->ExchangeID);
	strcpy(_tick.instrument_id, ptr->InstrumentID);
	strcpy(_tick.update_time, ptr->UpdateTime);
	_tick.update_millisec = ptr->UpdateMillisec;
	_tick.pre_close_price = ptr->PreClosePrice;
	//_tick.pre_open_interest = ptr->PreOpenInterest;
	_tick.pre_settlement_price = ptr->PreSettlementPrice;
	_tick.last_price = ptr->LastPrice;
	_tick.volume = ptr->Volume;
	_tick.last_volume = ptr->Volume - it->second.Volume;
	_tick.open_interest = ptr->OpenInterest;
	_tick.last_open_interest = ptr->OpenInterest - it->second.OpenInterest;
	_tick.open_price = ptr->OpenPrice;
	_tick.highest_price = ptr->HighestPrice;
	_tick.lowest_price = ptr->LowestPrice;
	_tick.upper_limit_price = ptr->UpperLimitPrice;
	_tick.lower_limit_price = ptr->LowerLimitPrice;

	_tick.bid_price[0] = ptr->BidPrice1; _tick.bid_volume[0] = ptr->BidVolume1;
	_tick.bid_price[1] = ptr->BidPrice2; _tick.bid_volume[1] = ptr->BidVolume2;
	_tick.bid_price[2] = ptr->BidPrice3; _tick.bid_volume[2] = ptr->BidVolume3;
	_tick.bid_price[3] = ptr->BidPrice4; _tick.bid_volume[3] = ptr->BidVolume4;
	_tick.bid_price[4] = ptr->BidPrice5; _tick.bid_volume[4] = ptr->BidVolume5;

	_tick.ask_price[0] = ptr->AskPrice1; _tick.ask_volume[0] = ptr->AskVolume1;
	_tick.ask_price[1] = ptr->AskPrice2; _tick.ask_volume[1] = ptr->AskVolume2;
	_tick.ask_price[2] = ptr->AskPrice3; _tick.ask_volume[2] = ptr->AskVolume3;
	_tick.ask_price[3] = ptr->AskPrice4; _tick.ask_volume[3] = ptr->AskVolume4;
	_tick.ask_price[4] = ptr->AskPrice5; _tick.ask_volume[4] = ptr->AskVolume5;

	if (ptr->LastPrice >= it->second.AskPrice1 || ptr->LastPrice >= ptr->AskPrice1)
	{
		_tick.tape_dir = eTapeDir::Up;
	}
	else if (ptr->LastPrice <= it->second.BidPrice1 || ptr->LastPrice <= ptr->BidPrice1)
	{
		_tick.tape_dir = eTapeDir::Down;
	}
	else
	{
		_tick.tape_dir = eTapeDir::Flat;
	}	

	this->insert_event(_tick);

	it->second = *ptr;

	//printf("LastPrice : %.2f\n", ptr->LastPrice);
	//printf("AskPrice1 : %.2f	AskVolume1 : %d\t", ptr->AskPrice1, ptr->AskVolume1);
	//printf("BidPrice1 : %.2f	BidVolume1 : %d\n", ptr->BidPrice1, ptr->BidVolume1);
	//printf("AskPrice2 : %.2f	AskVolume2 : %d\t", ptr->AskPrice2, ptr->AskVolume2);
	//printf("BidPrice2 : %.2f	BidVolume2 : %d\n", ptr->BidPrice2, ptr->BidVolume2);
	//printf("AskPrice3 : %.2f	AskVolume3 : %d\t", ptr->AskPrice3, ptr->AskVolume3);
	//printf("BidPrice3 : %.2f	BidVolume3 : %d\n", ptr->BidPrice3, ptr->BidVolume3);
	//printf("AskPrice4 : %.2f	AskVolume4 : %d\t", ptr->AskPrice4, ptr->AskVolume4);
	//printf("BidPrice4 : %.2f	BidVolume4 : %d\n", ptr->BidPrice4, ptr->BidVolume4);
	//printf("AskPrice5 : %.2f	AskVolume5 : %d\t", ptr->AskPrice5, ptr->AskVolume5);
	//printf("BidPrice5 : %.2f	BidVolume5 : %d\n\n", ptr->BidPrice5, ptr->BidVolume5);
}
