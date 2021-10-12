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

#ifndef _SUMMARYVAR_H_
#define _SUMMARYVAR_H_

#include <vector>
#include <string> 

#include "Var.h"
#include "TimeWindowQuantiles.h"

#define DEFAULT_MAX_AGE		60
#define DEFAULT_AGE_BUCKET	5

namespace prometheus {

template<typename TYPE>
class SummaryVar : public Var
{
public:
	void observe(const TYPE value);

	bool reduce(const void *ptr, size_t sz) override;
	std::string collect() override;

	size_t get_size() override { return this->quantiles.size(); }
	void *get_data() override { return this; }

	TYPE get_sum() const { return this->sum; }
	size_t get_count() const { return this->count; }
	const TimeWindowQuantiles<TYPE> *get_quantile_values() const
	{
		return &this->quantile_values;
	}

public:
	Var *create() override
	{
		return new SummaryVar<TYPE>(this->name, this->help,
									this->quantiles,
									this->max_age, this->age_buckets);
	}

	SummaryVar(const std::string& name, const std::string& help,
			   const std::vector<struct Quantile>& quantile,
			   const std::chrono::milliseconds max_age, int age_bucket) :
		Var(name, help, VAR_SUMMARY),
		quantiles(quantile),
		quantile_values(quantile, max_age, age_bucket)
	{
		this->max_age = max_age;
		this->age_buckets = age_bucket;
		this->quantile_out.resize(quantile.size(), 0);
		this->available_count.resize(quantile.size(), 0);
		this->init();
	}

	virtual void init()
	{
		this->sum = 0;
		this->count = 0;
	}

private:
	const std::vector<struct Quantile> quantiles;
	TYPE sum;
	size_t count;
	std::chrono::milliseconds max_age;
	int age_buckets;
	TimeWindowQuantiles<TYPE> quantile_values;
	std::vector<TYPE> quantile_out;
	std::vector<size_t> available_count;
};

template<typename TYPE>
void SummaryVar<TYPE>::observe(const TYPE value)
{
	this->quantile_values.insert(value);
	this->sum += value;
	this->count++;
}

template<typename TYPE>
bool SummaryVar<TYPE>::reduce(const void *ptr, size_t sz)
{
	if (sz != this->quantiles.size())
		return false;

	SummaryVar<TYPE> *data = (SummaryVar<TYPE> *)ptr;

	const TimeWindowQuantiles<TYPE> *src = data->get_quantile_values();
	size_t available_count;

	for (size_t i = 0; i< sz; i ++)
	{
		TYPE get_val = src->get(this->quantiles[i].quantile, available_count);
		this->quantile_out[i] += get_val * available_count;
		this->available_count[i] += available_count;
	}

	this->sum += data->get_sum();
	this->count += data->get_count();

	return true;
}

template<typename TYPE>
std::string SummaryVar<TYPE>::collect()
{
	std::string ret;

	for (size_t i = 0; i < this->quantiles.size(); i++)
	{
		ret += this->name + "{quantile=\"" +
			   std::to_string(this->quantiles[i].quantile) + "\"} ";

		if (this->quantile_out[i] == std::numeric_limits<TYPE>::quiet_NaN())
			ret += "NaN";
		else
			ret += std::to_string(this->quantile_out[i] /
								  this->available_count[i]);
		ret += "\n";
	}

	ret += this->name + "_sum " + std::to_string(this->sum) + "\n";
	ret += this->name + "_count " + std::to_string(this->count) + "\n";

	this->quantile_out.clear();
	this->available_count.clear();

	return std::move(ret);
}

} // namespace prometheus

#endif

