#pragma once

#include "market_api.h"
#include "ThostFtdcMdApi.h"


class ctp_market : public market_api, public CThostFtdcMdSpi
{
public:
	ctp_market(std::map<std::string, std::string>& config, std::set<std::string>& contracts);
	virtual ~ctp_market();	
	virtual void release() override;

private:
	void req_user_login();
	void subscribe();
	void unsubscribe();

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
	///登录请求响应
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) override;
	///登出请求响应
	virtual void OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) override;
	///订阅行情应答
	virtual void OnRspSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) override;
	///取消订阅行情应答
	virtual void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) override;
	///深度行情通知
	virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData) override;

private:
	CThostFtdcMdApi* _md_api;
	std::string _broker_id;
	std::string _user_id;
	std::string _password;

	int _req_id;
	std::set<std::string> _contracts;
	std::unordered_map<std::string, CThostFtdcDepthMarketDataField> _previous_tick_map{};
};
