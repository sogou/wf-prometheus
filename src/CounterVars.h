#ifndef _COUNTERVARS_H_
#define _COUNTERVARS_H_

#include <string>
#include <unordered_map>
#include "Vars.h"
#include "GaugeVars.h"

namespace prometheus {

template<typename TYPE>
class CounterVars : public Vars
{
public:
	BasicVars<TYPE> *add(const std::map<std::string, std::string>& labels);
	bool reduce(const void *ptr, size_t sz) override;
	std::string collect() override;

	size_t get_size() override { return this->data.size(); }
	void *get_data() override { return &(this->data); }
	void increase() override { /* TODO */ }
	void decrease() override { /* TODO */ }

	static bool label_to_str(const std::map<std::string, std::string>& labels,
							 std::string& str);

public:
	Vars *create() override
	{
		return new CounterVars<TYPE>(this->name, this->help);
	}

	CounterVars(const std::string& name, const std::string& help) :
		Vars(name, help, VARS_COUNTER)
	{
		VarsLocal::get_instance()->add(this->name, this);
	}

	~CounterVars()
	{
		for (auto it = this->data.begin();
			 it != this->data.end(); it++)
		{
			delete it->second;
		}
	}


private:
	std::unordered_map<std::string, BasicVars<TYPE> *> data;
};

template<typename TYPE>
BasicVars<TYPE> *CounterVars<TYPE>::add(const std::map<std::string,
													   std::string>& labels)
{
	std::string label_str;
	if (!this->label_to_str(labels, label_str))
		return NULL;

	auto it = this->data.find(label_str);
	BasicVars<TYPE> *var;

	if (it == this->data.end())
	{
		var = new BasicVars<TYPE>(label_str, "");
		this->data.insert(std::make_pair(label_str, var));
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
		auto my_it = this->data.find(it->first);

		if (my_it == this->data.end())
		{
			BasicVars<TYPE> *var = new BasicVars<TYPE>(it->first, "");
			this->data.insert(std::make_pair(it->first, var));
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

	for (auto it = this->data.begin();
		 it != this->data.end(); it++)
	{
		ret += this->name + "{" + it->first + "}" + " " +
			   it->second->data_str() + "\n";
	}

	return std::move(ret);
}

template<typename TYPE>
bool CounterVars<TYPE>::label_to_str(const std::map<std::string, std::string>& labels,
									 std::string& str)
{
	for (auto it = labels.begin(); it != labels.end(); it++)
	{
		fprintf(stderr, "label is : %s %s\n", it->first.c_str(), it->second.c_str());
		if (it != labels.begin())
			str += ",";
		//TODO: check label name regex is "[a-zA-Z_:][a-zA-Z0-9_:]*"
		str += it->first + "=\"" + it->second + "\"";
	}

	return true;
}

} // namespace prometheus

#endif

