#include "order_imbalance.h"
#include "frame.h"


order_imbalance::order_imbalance(stratid_t id, frame& frame, std::string contract, uint32_t period)
	: strategy(id, frame), _contract(contract), _period(period)
{
	get_contracts().insert(contract);
	frame.register_bar_receiver(contract, _period, this);
	frame.register_tape_receiver(contract, this);
}

order_imbalance::~order_imbalance()
{
}

void order_imbalance::on_tick(const MarketData& tick)
{

}

void order_imbalance::on_bar(const Sample& bar)
{
	//printf("%s O:%.f H:%.f L:%.f C:%.f V:%d\n", bar.time.c_str(), bar.open, bar.high, bar.low, bar.close, bar.volume);
}

void order_imbalance::on_tape(const Tape& tape)
{
	//printf("%d %.2f %d\n", tape.last_volume, tape.last_open_interest, tape.tape_dir);
}

void order_imbalance::on_order(const Order& order)
{

}

void order_imbalance::on_trade(const Order& order)
{
}

void order_imbalance::on_cancel(const Order& order)
{
}

void order_imbalance::on_error(const Order& order)
{
}
