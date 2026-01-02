#pragma once

#include "market_api.h"
#include "trader_api.h"


EXPORT_FLAG market_api* create_market(std::map<std::string, std::string>& config, std::set<std::string> contracts);
EXPORT_FLAG void destory_market(market_api*& api);

EXPORT_FLAG trader_api* create_trader(std::map<std::string, std::string>& config, std::set<std::string> contracts);
EXPORT_FLAG void destory_trader(trader_api*& api);
