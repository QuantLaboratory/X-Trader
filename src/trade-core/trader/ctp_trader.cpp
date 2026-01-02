#include "ctp_trader.h"


ctp_trader::ctp_trader(std::map<std::string, std::string>& config, std::set<std::string>& contracts)
	: _req_id(0), _front_id(0), _session_id(0), _order_ref(0), _contracts(contracts)
{
	_broker_id = config["broker_id"];
	_user_id = config["user_id"];
	_password = config["password"];
	_app_id = config["app_id"];
	_auth_code = config["auth_code"];
	_user_product_Info = config["user_product_Info"];

	char flow_path[64]{};
	sprintf(flow_path, "flow/td/%s/%s/", _broker_id.c_str(), _user_id.c_str());
	if (!std::filesystem::exists(flow_path))
	{
		std::filesystem::create_directories(flow_path);
	}
	_td_api = CThostFtdcTraderApi::CreateFtdcTraderApi(flow_path);
	_td_api->RegisterSpi(this);
	_td_api->RegisterFront(const_cast<char*>(config["trade_front"].c_str()));
	_td_api->SubscribePrivateTopic(THOST_TERT_QUICK);
	_td_api->SubscribePublicTopic(THOST_TERT_QUICK);
	_td_api->Init();
}

ctp_trader::~ctp_trader()
{
	_instrument_map.clear();
	_position_map.clear();
	_order_map.clear();
	if (_td_api)
	{
		_td_api->RegisterSpi(nullptr);
		_td_api->Release();
		_td_api = nullptr;
	}
}

void ctp_trader::release()
{
	_instrument_map.clear();
	_position_map.clear();
	_order_map.clear();
	if (_td_api)
	{
		_td_api->RegisterSpi(nullptr);
		_td_api->Release();
		_td_api = nullptr;
	}
}

orderref_t ctp_trader::insert_order(eOrderFlag order_flag, const std::string& contract, eDirOffset dir_offset, double price, int volume)
{
	CThostFtdcInputOrderField t{};

	strcpy(t.BrokerID, _broker_id.c_str());
	strcpy(t.InvestorID, _user_id.c_str());
	strcpy(t.InstrumentID, contract.c_str());
	strcpy(t.ExchangeID, _instrument_map[contract].exchange_id);

	sprintf(t.OrderRef, "%u", _order_ref.load());

	t.LimitPrice = price;
	t.VolumeTotalOriginal = volume;
	to_ctp_dirOffset(dir_offset, t);

	if (price != .0F)
	{
		t.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
	}
	else
	{
		t.OrderPriceType = THOST_FTDC_OPT_AnyPrice;
	}
	switch (order_flag)
	{
	case eOrderFlag::Limit:
		t.TimeCondition = THOST_FTDC_TC_GFD;
		t.VolumeCondition = THOST_FTDC_VC_AV;
		t.MinVolume = 1;
		break;
	case eOrderFlag::Market:
		t.TimeCondition = THOST_FTDC_TC_IOC;
		t.VolumeCondition = THOST_FTDC_VC_AV;
		t.MinVolume = 1;
		break;
	case eOrderFlag::FOK:
		t.TimeCondition = THOST_FTDC_TC_IOC;
		t.VolumeCondition = THOST_FTDC_VC_CV;
		t.MinVolume = volume;
		break;
	case eOrderFlag::FAK:
		t.TimeCondition = THOST_FTDC_TC_IOC;
		t.VolumeCondition = THOST_FTDC_VC_AV;
		t.MinVolume = 1;
		break;
	}

	t.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
	t.ContingentCondition = THOST_FTDC_CC_Immediately;
	t.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
	t.IsAutoSuspend = 0;
	t.UserForceClose = 0;
	int rtn = _td_api->ReqOrderInsert(&t, generate_reqid());
	if (rtn != 0) { return null_orderref; }
	return _order_ref++;
}

bool ctp_trader::cancel_order(const orderref_t order_ref)
{
	const auto it = _order_map.find(order_ref);
	if (it == _order_map.end()) { return false; }
	auto& order = it->second;

	CThostFtdcInputOrderActionField t{};

	strcpy(t.BrokerID, _broker_id.c_str());
	strcpy(t.InvestorID, _user_id.c_str());
	strcpy(t.UserID, _user_id.c_str());
	
	sprintf(t.OrderRef, "%llu", order_ref);
	t.FrontID = order.front_id;
	t.SessionID = order.session_id;
	t.ActionFlag = THOST_FTDC_AF_Delete;

	strcpy(t.InstrumentID, order.instrument_id);
	strcpy(t.ExchangeID, order.exchange_id);
	int rtn = _td_api->ReqOrderAction(&t, generate_reqid());
	if (rtn != 0) { return false; }
	return true;
}

std::string ctp_trader::get_trading_day() const
{
	return _td_api->GetTradingDay();
}

void ctp_trader::get_trader_data(InstrumentMap& i_map, PositionMap& p_map, OrderMap& o_map)
{
	i_map = _instrument_map;
	p_map = _position_map;
	o_map = _order_map;
}

void ctp_trader::get_account()
{
	CThostFtdcQryTradingAccountField t{};
	strcpy(t.BrokerID, _broker_id.c_str());
	strcpy(t.InvestorID, _user_id.c_str());

	int rtn = _td_api->ReqQryTradingAccount(&t, generate_reqid());
}

void ctp_trader::req_auth()
{
	CThostFtdcReqAuthenticateField t{};
	strcpy(t.BrokerID, _broker_id.c_str());
	strcpy(t.UserID, _user_id.c_str());
	strcpy(t.AuthCode, _auth_code.c_str());
	strcpy(t.AppID, _app_id.c_str());
	strcpy(t.UserProductInfo, _user_product_Info.c_str());

	int rtn = _td_api->ReqAuthenticate(&t, generate_reqid());
	if (rtn != 0) {}
}

void ctp_trader::req_user_login()
{
	CThostFtdcReqUserLoginField t{};
	strcpy(t.BrokerID, _broker_id.c_str());
	strcpy(t.UserID, _user_id.c_str());
	strcpy(t.Password, _password.c_str());

	int rtn = _td_api->ReqUserLogin(&t, generate_reqid());
	if (rtn != 0) {}
}

void ctp_trader::req_user_logout()
{
	CThostFtdcUserLogoutField t{};
	strcpy(t.BrokerID, _broker_id.c_str());
	strcpy(t.UserID, _user_id.c_str());

	int rtn = _td_api->ReqUserLogout(&t, generate_reqid());
	if (rtn != 0) {}
}

void ctp_trader::req_settlement_confirm()
{
	CThostFtdcSettlementInfoConfirmField t{};
	strcpy(t.BrokerID, _broker_id.c_str());
	strcpy(t.InvestorID, _user_id.c_str());

	int rtn = _td_api->ReqSettlementInfoConfirm(&t, generate_reqid());
	if (rtn != 0) {}
}

void ctp_trader::req_qry_instrument()
{
	CThostFtdcQryInstrumentField t{};

	int rtn = _td_api->ReqQryInstrument(&t, generate_reqid());
	if (rtn != 0) {}
}

void ctp_trader::req_qry_position()
{
	CThostFtdcQryInvestorPositionField t{};
	strcpy(t.BrokerID, _broker_id.c_str());
	strcpy(t.InvestorID, _user_id.c_str());

	auto rtn = _td_api->ReqQryInvestorPosition(&t, generate_reqid());
	if (rtn != 0) {}
}

void ctp_trader::req_qry_order()
{
	CThostFtdcQryOrderField t{};
	strcpy(t.BrokerID, _broker_id.c_str());
	strcpy(t.InvestorID, _user_id.c_str());

	auto rtn = _td_api->ReqQryOrder(&t, generate_reqid());
	if (rtn != 0) {}
}


///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
void ctp_trader::OnFrontConnected()
{
	req_auth();
}

///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
	///@param nReason 错误原因
	///        0x1001 网络读失败
	///        0x1002 网络写失败
	///        0x2001 接收心跳超时
	///        0x2002 发送心跳失败
	///        0x2003 收到错误报文
void ctp_trader::OnFrontDisconnected(int nReason)
{
	char text[128]; sprintf(text, "%s Trade Front Disconnected", _user_id.c_str());
	//send2wecom(text);
}

///客户端认证响应
void ctp_trader::OnRspAuthenticate(CThostFtdcRspAuthenticateField* pRspAuthenticateField, CThostFtdcRspInfoField* pRspInfo, int nRequorder_id, bool bIsLast)
{
	if (pRspInfo && pRspInfo->ErrorID) { return; }
	if (bIsLast) { req_user_login(); }
}

///登录请求响应
void ctp_trader::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequorder_id, bool bIsLast)
{
	if (pRspInfo && pRspInfo->ErrorID){ return; }

	if (pRspUserLogin)
	{
		_front_id = pRspUserLogin->FrontID;
		_session_id = pRspUserLogin->SessionID;
		_order_ref = atoi(pRspUserLogin->MaxOrderRef);
	}

	if (bIsLast) { req_settlement_confirm(); }
}

///登出请求响应
void ctp_trader::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequorder_id, bool bIsLast)
{
	if (pRspInfo && pRspInfo->ErrorID) { return; }
	if (bIsLast) {}
}

///投资者结算结果确认响应
void ctp_trader::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequorder_id, bool bIsLast)
{
	if (pRspInfo && pRspInfo->ErrorID) { return; }

	if (bIsLast) { req_qry_instrument(); }
}

///请求查询合约响应
void ctp_trader::OnRspQryInstrument(CThostFtdcInstrumentField* ptr, CThostFtdcRspInfoField* pRspInfo, int nRequorder_id, bool bIsLast)
{
	if (pRspInfo && pRspInfo->ErrorID) { return; }

	if (ptr && ptr->ProductClass == THOST_FTDC_PC_Futures)
	{
		//if (_contracts.find(ptr->InstrumentID) == _contracts.end()) { return; }

		Instrument t{};
		strcpy(t.exchange_id, ptr->ExchangeID);
		strcpy(t.instrument_id, ptr->InstrumentID);
		//t.delivery_year = ptr->DeliveryYear;
		//t.delivery_month = ptr->DeliveryMonth;
		//t.max_market_order_volume = ptr->MaxMarketOrderVolume;
		//t.min_market_order_volume = ptr->MinMarketOrderVolume;
		//t.max_limit_order_volume = ptr->MaxLimitOrderVolume;
		//t.min_limit_order_volume = ptr->MinLimitOrderVolume;
		t.volume_multiple = ptr->VolumeMultiple;
		t.price_tick = ptr->PriceTick;
		//strcpy(t.create_date, ptr->CreateDate);
		//strcpy(t.open_date, ptr->OpenDate);
		//strcpy(t.expire_date, ptr->ExpireDate);
		//strcpy(t.start_deliv_date, ptr->StartDelivDate);
		//strcpy(t.end_deliv_date, ptr->EndDelivDate);
		if (ptr->PositionDateType == THOST_FTDC_PDT_UseHistory)
		{
			t.is_use_history = true;
		}
		else if (ptr->PositionDateType == THOST_FTDC_PDT_NoUseHistory)
		{
			t.is_use_history = false;
		}
		t.is_trading = ptr->IsTrading;
		//t.long_margin_ratio = ptr->LongMarginRatio;
		//t.short_margin_ratio = ptr->ShortMarginRatio;

		_instrument_map[ptr->InstrumentID] = t;
	}

	if (bIsLast) { req_qry_position(); }
}

///请求查询投资者持仓响应
void ctp_trader::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField* ptr, CThostFtdcRspInfoField* pRspInfo, int nRequorder_id, bool bIsLast)
{
	if (pRspInfo && pRspInfo->ErrorID) { return; }
	//print(ptr);
	if (ptr)
	{
		const bool& is_use_history = _instrument_map[ptr->InstrumentID].is_use_history;
		const int& volume_multiple = _instrument_map[ptr->InstrumentID].volume_multiple;
		auto& p = _position_map[ptr->InstrumentID];
		p.id = ptr->InstrumentID;
		
		switch (ptr->PosiDirection)
		{
		case THOST_FTDC_PD_Long:
			p.long_.closeable += ptr->Position - ptr->ShortFrozen;
			switch (ptr->PositionDate)
			{
			case THOST_FTDC_PSD_Today:
				p.long_.open_no_trade = ptr->LongFrozen;
				if (is_use_history)
				{
					p.long_.today_position = ptr->TodayPosition;
					p.long_.today_closeable = ptr->Position - ptr->ShortFrozen;
				}
				break;
			case THOST_FTDC_PSD_History:
				p.long_.his_position = ptr->Position;
				p.long_.his_closeable = ptr->Position - ptr->ShortFrozen;
				break;
			}
			break;
		case THOST_FTDC_PD_Short:
			p.short_.closeable += ptr->Position - ptr->LongFrozen;
			switch (ptr->PositionDate)
			{
			case THOST_FTDC_PSD_Today:
				p.short_.open_no_trade = ptr->ShortFrozen;
				if (is_use_history)
				{
					p.short_.today_position = ptr->TodayPosition;
					p.short_.today_closeable = ptr->Position - ptr->LongFrozen;
				}
				break;
			case THOST_FTDC_PSD_History:
				p.short_.his_position = ptr->Position;
				p.short_.his_closeable = ptr->Position - ptr->LongFrozen;
				break;
			}
			break;
		}

		if (ptr->Position)
		{
			auto& posi = ptr->PosiDirection == THOST_FTDC_PD_Long ? p.long_ : p.short_;
			posi.position += ptr->Position;

			posi.avg_open_cost = (posi.avg_open_cost * (posi.position - ptr->Position) * volume_multiple + ptr->OpenCost)
				/ (posi.position * volume_multiple);

			posi.avg_posi_cost = (posi.avg_posi_cost * (posi.position - ptr->Position) * volume_multiple + ptr->PositionCost)
				/ (posi.position * volume_multiple);
		}
	}
	
	if (bIsLast)
	{
		//print_position();
		req_qry_order();
	}
}

///请求查询资金账户响应
void ctp_trader::OnRspQryTradingAccount(CThostFtdcTradingAccountField* pAcc, CThostFtdcRspInfoField* pRspInfo, int nRequorder_id, bool bIsLast)
{
	if (pRspInfo && pRspInfo->ErrorID) { return; }
	if (pAcc)
	{
		char text[128];
		sprintf(text, "AccountID=%s, CloseProfit=%.2f, PositionProfit=%.2f, Commission=%.2f, NetProfit=%.2f",
			pAcc->AccountID, pAcc->CloseProfit, pAcc->PositionProfit, pAcc->Commission,
			pAcc->CloseProfit + pAcc->PositionProfit - pAcc->Commission);

		//send2wecom(text);
	}
}

///请求查询报单响应
void ctp_trader::OnRspQryOrder(CThostFtdcOrderField* pOrder, CThostFtdcRspInfoField* pRspInfo, int nRequorder_id, bool bIsLast)
{
	//if (pOrder) { print(pOrder); }

	if (pOrder && pOrder->VolumeTotal > 0 &&
		pOrder->OrderStatus != THOST_FTDC_OST_Canceled &&
		pOrder->OrderStatus != THOST_FTDC_OST_AllTraded)
	{
		//print(pOrder);

		orderref_t order_ref = strtoul(pOrder->OrderRef, NULL, 10);
		Order& order = _order_map[order_ref];

		order.order_ref = order_ref;
		order.front_id = pOrder->FrontID;
		order.session_id = pOrder->SessionID;
		strcpy(order.instrument_id, pOrder->InstrumentID);
		strcpy(order.exchange_id, pOrder->ExchangeID);
		to_local_dirOffset(*pOrder, order.dir_offset);
		order.volume_traded = pOrder->VolumeTraded;
		order.volume_total = pOrder->VolumeTotal;
		order.volume_total_original = pOrder->VolumeTotalOriginal;
		order.limit_price = pOrder->LimitPrice;

		to_local_orderSubmitStatus(pOrder->OrderSubmitStatus, order.order_submit_status);
		to_local_orderStatus(pOrder->OrderStatus, order.order_status);
		strcpy(order.status_msg, pOrder->StatusMsg);

		strcpy(order.insert_time, pOrder->InsertTime);
		//strcpy(order.active_time, pOrder->ActiveTime);
		//strcpy(order.update_time, pOrder->UpdateTime);
	}

	if (bIsLast && !_is_ready.load())
	{
		int max_index = 0;
		for (const auto& pair : _order_map)
		{
			if (pair.first > max_index) { max_index = pair.first; }
		}
		_order_ref.store(max_index + 1);
		_is_ready.store(true, std::memory_order_release);

		char text[128]; sprintf(text, "td@%s is ready!", _user_id.c_str());
		//send2wecom(text);

		printf("%s\n", text);
	}
}

///报单录入请求响应
void ctp_trader::OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo, int nRequorder_id, bool bIsLast)
{
	if (pInputOrder && pRspInfo)
	{
		print_local_time(); printf("OnRspOrderInsert"); print(pRspInfo);

		Order order{};
		order.order_ref = strtol(pInputOrder->OrderRef, NULL, 10);
		order.event_flag = eEventFlag::ErrorInsert;
		order.error_id = pRspInfo->ErrorID;
		strcpy(order.error_msg, pRspInfo->ErrorMsg);

		this->insert_event(order);
	}
}

///报单操作请求响应
void ctp_trader::OnRspOrderAction(CThostFtdcInputOrderActionField* pInputOrderAction, CThostFtdcRspInfoField* pRspInfo, int nRequorder_id, bool bIsLast)
{
	if (pInputOrderAction && pRspInfo)
	{
		print_local_time(); printf("OnRspOrderAction"); print(pRspInfo);
		
		Order order{};
		order.order_ref = strtol(pInputOrderAction->OrderRef, NULL, 10);
		order.event_flag = eEventFlag::ErrorCancel;
		order.error_id = pRspInfo->ErrorID;
		strcpy(order.error_msg, pRspInfo->ErrorMsg);

		this->insert_event(order);
	}
}

///报单通知
void ctp_trader::OnRtnOrder(CThostFtdcOrderField* pOrder)
{
	if (pOrder == nullptr) { return; }

	//print(pOrder);
	
	orderref_t order_ref = strtol(pOrder->OrderRef, NULL, 10);
	eOrderSubmitStatus order_submit_status; to_local_orderSubmitStatus(pOrder->OrderSubmitStatus, order_submit_status);
	eOrderStatus order_status; to_local_orderStatus(pOrder->OrderStatus, order_status);

	Order& order = _order_map[order_ref];
	if (order.session_id)
	{
		if (order.order_status == order_status && order.volume_total == pOrder->VolumeTotal) { return; }
	}
	else
	{
		order.order_ref = order_ref;
		order.front_id = pOrder->FrontID;
		order.session_id = pOrder->SessionID;
		strcpy(order.instrument_id, pOrder->InstrumentID);
		strcpy(order.exchange_id, pOrder->ExchangeID);
		to_local_dirOffset(*pOrder, order.dir_offset);
		order.limit_price = pOrder->LimitPrice;
		order.volume_total_original = pOrder->VolumeTotalOriginal;
		strcpy(order.status_msg, pOrder->StatusMsg);
		strcpy(order.insert_time, pOrder->InsertTime);
	}

	//printf("After "); print(pOrder);
	
	order.volume_traded = pOrder->VolumeTraded;
	order.volume_total = pOrder->VolumeTotal;
	order.order_status = order_status;
	order.order_submit_status = order_submit_status;

	if (pOrder->OrderStatus == THOST_FTDC_OST_Canceled)
	{
		strcpy(order.cancel_time, pOrder->CancelTime);
		order.event_flag = eEventFlag::Cancel;
		this->insert_event(order);
	}
	else if (pOrder->VolumeTraded == 0)
	{
		order.event_flag = eEventFlag::Order;
		this->insert_event(order);
	}
	else if (pOrder->VolumeTraded > 0)
	{
		order.event_flag = eEventFlag::Trade;
		this->insert_event(order);
	}

	//删除报单
	switch (pOrder->OrderStatus)
	{
	case THOST_FTDC_OST_AllTraded:
	case THOST_FTDC_OST_Canceled:
		auto it = _order_map.find(order.order_ref);
		if (it != _order_map.end()) { _order_map.erase(it); }
		break;
	}
}

///成交通知
void ctp_trader::OnRtnTrade(CThostFtdcTradeField* pTrade)
{
	//print(pTrade);
}

///报单录入错误回报
void ctp_trader::OnErrRtnOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo)
{
	if (pRspInfo)
	{
		print_local_time(); printf("OnErrRtnOrderInsert"); print(pRspInfo);
	}
}

///报单操作错误回报
void ctp_trader::OnErrRtnOrderAction(CThostFtdcOrderActionField* pOrderAction, CThostFtdcRspInfoField* pRspInfo)
{
	if (pRspInfo)
	{
		print_local_time(); printf("OnErrRtnOrderAction"); print(pRspInfo);
	}
}
