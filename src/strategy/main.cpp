#include "frame.h"
#include "market_making.h"
#include "statistical_arbitrage.h"
#include "order_flow.h"
#include "order_imbalance.h"
#include "high_low.h"
#include "market_correction.h"
//#include "decline_scalping.h"
//#include "decline_swing.h"

void start_running(const char* filename)
{
	frame run(filename);
	std::vector<std::shared_ptr<strategy>> strategys;
	strategys.emplace_back(std::make_shared<market_making>(1, run, "rb2409", 2, 10, 1));
	//strategys.emplace_back(std::make_shared<market_making>(1, run, "MA505", 2, 5, 1));
	//strategys.emplace_back(std::make_shared<statistical_arbitrage>(2, run, "MA501", "MA505", 30, 1));
	//strategys.emplace_back(std::make_shared<order_flow>(3, run, "MA505", 1, 3, 3, 4, 1));
	//strategys.emplace_back(std::make_shared<order_imbalance>(4, run, "rb2409", 1));
	//strategys.emplace_back(std::make_shared<high_low>(5, run, "MA505"));
	//strategys.emplace_back(std::make_shared<market_correction>(6, run, "RM409", 1));
	//strategys.emplace_back(std::make_shared<decline_scalping>(7, run, "RM505", 1));
	//strategys.emplace_back(std::make_shared<decline_swing>(8, run, "RM505", 1));
	run.run_until_close(strategys);
}


int main()
{
	//start_running("./ini/simnow/1_099008.ini");
	//start_running("./ini/simnow/2_083309.ini");
	//start_running("./ini/simnow/1_120585.ini");
	start_running("./ini/simnow/3_117509.ini");
	//start_running("./ini/simnow/24_120585.ini");
	//start_running("./ini/simnow/24_099008.ini");
	//start_running("./ini/mk_5.ini");
	//start_running("./ini/mk.ini");
	//start_running("./ini/5.ini");
	//start_running("./ini/gt1.ini");
	//start_running("./ini/gt2.ini");

	return 0;
}