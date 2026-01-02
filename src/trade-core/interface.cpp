#include "interface.h"
#include "market/ctp_market.h"
#include "trader/ctp_trader.h"


market_api* create_market(std::map<std::string, std::string>& config, std::set<std::string> contracts)
{
	return new ctp_market(config, contracts);
}

void destory_market(market_api*& api)
{
	if (nullptr != api)
	{
		delete api; api = nullptr;
	}
}

trader_api* create_trader(std::map<std::string, std::string>& config, std::set<std::string> contracts)
{
	return new ctp_trader(config, contracts);
}

void destory_trader(trader_api*& api)
{
	if (nullptr != api)
	{
		delete api; api = nullptr;
	}
}
