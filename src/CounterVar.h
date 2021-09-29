/*
  Copyright (c) 2021 Sogou, Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Author: Li Yingxin (liyingxin@sogou-inc.com)
*/

#ifndef _COUNTERVARS_H_
#define _COUNTERVARS_H_

#include <map>
#include <string>
#include <unordered_map>
#include "Var.h"
#include "GaugeVar.h"

namespace prometheus {

template<typename TYPE>
class CounterVar : public Var
{
public:
	using LABEL_MAP = std::map<std::string, std::string>;
	GaugeVar<TYPE> *add(const LABEL_MAP& labels);

	bool reduce(const void *ptr, size_t sz) override;
	std::string collect() override;

	size_t get_size() override { return this->data.size(); }
	void *get_data() override { return &(this->data); }

	static bool label_to_str(const LABEL_MAP& labels, std::string& str);

public:
	Var *create() override
	{
		return new CounterVar<TYPE>(this->name, this->help);
	}

	CounterVar(const std::string& name, const std::string& help) :
		Var(name, help, VARS_COUNTER)
	{
	}

	~CounterVar()
	{
		for (auto it = this->data.begin();
			 it != this->data.end(); it++)
		{
			delete it->second;
		}
	}

private:
	std::unordered_map<std::string, GaugeVar<TYPE> *> data;
};

template<typename TYPE>
GaugeVar<TYPE> *CounterVar<TYPE>::add(const LABEL_MAP& labels)
{
	std::string label_str;
	GaugeVar<TYPE> *var;

	if (!this->label_to_str(labels, label_str))
		return NULL;

	auto it = this->data.find(label_str);

	if (it == this->data.end())
	{
		var = new GaugeVar<TYPE>(label_str, "");
		this->data.insert(std::make_pair(label_str, var));
	}
	else
		var = it->second;

	return var;
}

template<typename TYPE>
bool CounterVar<TYPE>::reduce(const void *ptr, size_t)
{
	std::unordered_map<std::string, GaugeVar<TYPE> *> *data;
	data = (std::unordered_map<std::string, GaugeVar<TYPE> *> *)ptr;

	for (auto it = data->begin(); it != data->end(); it++)
	{
		auto my_it = this->data.find(it->first);

		if (my_it == this->data.end())
		{
			GaugeVar<TYPE> *var = new GaugeVar<TYPE>(it->first, "");
			this->data.insert(std::make_pair(it->first, var));
		}
		else
			my_it->second->reduce(it->second->get_data(),
								  it->second->get_size());
	}

	return true;
}

template<typename TYPE>
std::string CounterVar<TYPE>::collect()
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
bool CounterVar<TYPE>::label_to_str(const LABEL_MAP& labels, std::string& str)
{
	for (auto it = labels.begin(); it != labels.end(); it++)
	{
		if (it != labels.begin())
			str += ",";
		//TODO: check label name regex is "[a-zA-Z_:][a-zA-Z0-9_:]*"
		str += it->first + "=\"" + it->second + "\"";
	}

	return true;
}

} // namespace prometheus

#endif

