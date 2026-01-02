#pragma once

#include "data_struct.h"


class resample
{
public:
	resample(uint32_t period, double price_tick) :_period(period), _price_tick(price_tick) {}
	~resample(){}

private:
	uint32_t _count = 0;
	uint32_t _period;
	double _price_tick;
	Sample _bar{};
	char _min = '9';
	bool _is_new = true;
	std::map<double, uint32_t> _poc;

	std::set<bar_receiver*> _bar_callback;

public:
	void insert_tick(const MarketData& tick);
	void add_receiver(bar_receiver* receiver);
	void remove_receiver(bar_receiver* receiver);
	bool invalid() const;
};
