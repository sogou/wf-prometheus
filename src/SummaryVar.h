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

	size_t get_size() const override { return this->quantiles.size(); }
	const void *get_data() const override { return this; }

	TYPE get_sum() const { return this->sum; }
	size_t get_count() const { return this->count; }
	TimeWindowQuantiles<TYPE> *get_quantile_values()
	{
		return &this->quantile_values;
	}

public:
	Var *create(bool with_data) override
	{
		SummaryVar<TYPE> *var = new SummaryVar<TYPE>(this->name,
													 this->help,
													 this->quantiles,
													 this->max_age,
													 this->age_buckets);
		if (with_data)
		{
			var->sum = this->sum;
			var->count = this->count;
			var->quantile_values = this->quantile_values;
			var->quantile_size = this->quantile_size;
			var->quantile_out = this->quantile_out;
		}

		return var;
	}

	SummaryVar(const std::string& name, const std::string& help,
			   const std::vector<struct Quantile>& quantile,
			   const std::chrono::milliseconds max_age, int age_bucket) :
		Var(name, help, VAR_SUMMARY),
		quantiles(quantile),
		quantile_values(&this->quantiles, max_age, age_bucket)
	{
		this->max_age = max_age;
		this->age_buckets = age_bucket;
		this->quantile_size = this->quantiles.size(); // for output
		if (this->quantiles[this->quantile_size - 1].quantile != 1.0)
		{
			struct Quantile q(1.0, 0.1);
			this->quantiles.push_back(q);
		}
		this->quantile_out.resize(this->quantiles.size(), 0);
		this->init();
	}

	virtual void init()
	{
		this->sum = 0;
		this->count = 0;
	}

private:
	std::vector<struct Quantile> quantiles;
	TYPE sum;
	size_t count;
	size_t quantile_size;
	std::chrono::milliseconds max_age;
	int age_buckets;
	TimeWindowQuantiles<TYPE> quantile_values;
	std::vector<TYPE> quantile_out;
};

template<typename TYPE>
void SummaryVar<TYPE>::observe(const TYPE value)
{
	this->quantile_values.insert(value);
}

template<typename TYPE>
bool SummaryVar<TYPE>::reduce(const void *ptr, size_t sz)
{
	if (sz != this->quantiles.size())
		return false;

	SummaryVar<TYPE> *data = (SummaryVar<TYPE> *)ptr;

	TimeWindowQuantiles<TYPE> *src = data->get_quantile_values();
	TYPE get_val;
	size_t src_count = 0;
	TYPE *src_value = new TYPE[sz]();
	TYPE src_sum = src->get_sum();

	for (size_t i = 0; i < sz; i++)
	{
		src_count = src->get(this->quantiles[i].quantile, &get_val);
		src_value[i] = get_val;
	}

	TYPE pilot;
	size_t cnt;
	size_t idx;
	TYPE range;
	TYPE count = 0;
	TYPE value = 0;
	size_t src_idx = 0;
	size_t dst_idx = 0;
	size_t total = this->count + src_count;
	TYPE *out = new TYPE[sz]();

	for (size_t i = 0; i < sz; i++)
	{
		pilot = this->quantiles[i].quantile * total;

		while (count < pilot && src_idx < sz && dst_idx < sz)
		{
			if (this->quantile_out[dst_idx] <= src_value[src_idx])
			{
				value = this->quantile_out[dst_idx];
				idx = dst_idx;
				cnt = this->count;
				dst_idx++;
			}
			else
			{
				value = src_value[src_idx];
				idx = src_idx;
				cnt = src_count;
				src_idx++;
			}

			if (idx == 0)
				range = this->quantiles[0].quantile;
			else
				range = this->quantiles[idx].quantile -
						this->quantiles[idx - 1].quantile;

			count += cnt * range;
		}

		if (count >= pilot)
			out[i] = value;
		else if (src_idx < sz)
			out[i] = src_value[i];
		else
			out[i] = this->quantile_out[i];
	}

	for (size_t i = 0; i < sz; i++)
		this->quantile_out[i] = out[i];

	this->count = total;
	this->sum += src_sum;

	delete[] out;
	delete[] src_value;

	return true;
}

template<typename TYPE>
std::string SummaryVar<TYPE>::collect()
{
	std::string ret;

	for (size_t i = 0; i < this->quantile_size; i++)
	{
		ret += this->name + "{quantile=\"" +
			   std::to_string(this->quantiles[i].quantile) + "\"} ";

		if (this->quantile_out[i] == std::numeric_limits<TYPE>::quiet_NaN())
			ret += "NaN";
		else
			ret += std::to_string(this->quantile_out[i]);
		ret += "\n";
	}

	ret += this->name + "_sum " + std::to_string(this->sum) + "\n";
	ret += this->name + "_count " + std::to_string(this->count) + "\n";

	this->quantile_out.clear();

	return std::move(ret);
}

} // namespace prometheus

#endif

