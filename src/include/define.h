#pragma once

#include <stdint.h>
#include <set>
#include <vector>
#include <tuple>
#include <map>
#include<unordered_map>
#include <filesystem>
#include <atomic>
#include <string>
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>


#if defined(WIN32)
#pragma warning(disable:4996)
#endif

#ifndef EXPORT_FLAG
#ifdef _MSC_VER
#define EXPORT_FLAG __declspec(dllexport)
#else
#define EXPORT_FLAG __attribute__((__visibility__("default")))
#endif
#endif

#ifndef PORTER_FLAG
#ifdef _MSC_VER
#define PORTER_FLAG _cdecl
#else
#define PORTER_FLAG 
#endif
#endif

using orderref_t = uint64_t;///报单编号类型
constexpr orderref_t null_orderref = 0x0LLU;

using stratid_t = uint8_t;///策略编号类型


using  DateType = char[9]; ///日期类型
using  TimeType = char[9]; ///时间类型
using  ExchangeIDType = char[9]; ///交易所代码类型
using  InstrumentIDType = char[81]; ///合约代码类型
using  BrokerIDType = char[11]; ///经纪公司代码类型
using  InvestorIDType = char[13]; ///投资者代码类型
using  UserIDType = char[16]; ///用户代码类型
using  OrderRefType = char[13]; ///报单引用类型
using  CombOffsetFlagType = char[5]; ///组合开平标志类型
using  CombHedgeFlagType = char[5]; ///组合投机套保标志类型
using  OrderLocalIDType = char[13]; ///本地报单编号类型
using  OrderSysIDType = char[21]; ///报单编号类型
using  ProductInfoType = char[11]; ///产品信息类型
using  ErrorMsgType = char[81]; ///错误信息类型
using  TradeIDType = char[21]; ///成交编号类型
using  ClientIDType = char[11];///交易编码类型


inline void send2wecom(const char* text)
{
	char command[512];
	std::string curl = "curl ";
	std::string webhook = R"("填入微信群机器人webhook地址")";
	std::string param = R"( -H "Content-Type: application/json" -d "{\"msgtype\": \"text\",\"text\": {\"content\": \"%s\"}}")";
	sprintf(command, (curl + webhook + param).c_str(), text);

	system(command);
}

inline const char* reqRtnReason(int rtn)
{
	switch (rtn)
	{
	case 0:///发送成功
		return "Sent successfully"; break;
	case -1:///因网络原因发送失败
		return "Failed to send"; break;
	case -2:///未处理请求队列总数量超限
		return "Unprocessed requests exceeded the allowed volume"; break;
	case -3:///每秒发送请求数量超限
		return "The number of requests sent per second exceeds the allowed volume"; break;
	default:///未知
		return "Unknown";
	}
}

inline const char* nReason2str(int nReason)
{
	switch (nReason)
	{
	case 0x1001:
		return "网络读失败";
	case 0x1002:
		return "网络写失败";
	case 0x2001:
		return "接收心跳超时";
	case 0x2002:
		return "发送心跳失败";
	case 0x2003:
		return "收到错误报文";
	default:
		return "Unknown";
	}
}
