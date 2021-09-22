#ifndef __COUNTVARS_H__
#define __COUNTVARS_H__

#include <string>
#include <unordered_map>
#include "Vars.h"
#include "GaugeVars.h"

namespace prometheus {

template<typename TYPE>
class CounterVars : public Vars
{
public:
	BasicVars<TYPE> *add(const std::string &name);
	bool reduce(const void *ptr, size_t sz) override;
	std::string collect() override;

	size_t get_size() override { return this->gauge_data.size(); }
	void *get_data() override { return &(this->gauge_data); }
	void increase() override { /* TODO */ }
	void decrease() override { /* TODO */ }

public:
	Vars *create() override
	{
		return new CounterVars<TYPE>(this->name, this->help);
	}

	CounterVars(const std::string& name, const std::string& help) :
		Vars(name, help, RPC_VARS_COUNTER)
	{
		VarsLocal::get_instance()->add(this->name, this);
	}

	~CounterVars()
	{
		for (auto it = this->gauge_data.begin();
			 it != this->gauge_data.end(); it++)
		{
			delete it->second;
		}
	}
/*
	CounterVars(CounterVars<TYPE>&& move) = default;
	CounterVars(const CounterVars<TYPE>&) = delete;
	CounterVars& operator=(CounterVars<TYPE>&& move) = default;
	CounterVars& operator=(const CounterVars<TYPE>&) = delete;
*/
private:
	std::unordered_map<std::string, BasicVars<TYPE> *> gauge_data;
};

template<typename TYPE>
BasicVars<TYPE> *CounterVars<TYPE>::add(const std::string &name)
{
	
	auto it = this->gauge_data.find(name);
	BasicVars<TYPE> *var;

	if (it == this->gauge_data.end())
	{
		var = new BasicVars<TYPE>(name, "");
		this->gauge_data.insert(std::make_pair(name, var));
	}
	else
		var = it->second;

	return var;
}

template<typename TYPE>
bool CounterVars<TYPE>::reduce(const void *ptr, size_t)
{
	std::unordered_map<std::string, BasicVars<TYPE> *> *data;
	data = (std::unordered_map<std::string, BasicVars<TYPE> *> *)ptr;

	for (auto it = data->begin(); it != data->end(); it++)
	{
		auto my_it = this->gauge_data.find(it->first);

		if (my_it == this->gauge_data.end())
		{
			BasicVars<TYPE> *var = new BasicVars<TYPE>(it->first, "");
			this->gauge_data.insert(std::make_pair(it->first, var));
		}
		else
			my_it->second->reduce(it->second->get_data(),
								  it->second->get_size());
	}

	return true;
}

template<typename TYPE>
std::string CounterVars<TYPE>::collect()
{
	std::string ret;

	for (auto it = this->gauge_data.begin();
		 it != this->gauge_data.end(); it++)
	{
		ret += this->name + "{" + it->first + "}" + " " +
			   it->second->data_str() + "\n";
	}

	return std::move(ret);
}

} // namespace prometheus

#endif

