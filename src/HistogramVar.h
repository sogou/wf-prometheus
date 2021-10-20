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

#ifndef _HISTOGRAMVAR_H_
#define _HISTOGRAMVAR_H_

#include <vector>
#include <string> 

#include "Var.h"

namespace prometheus {

template<typename TYPE>
class HistogramVar : public Var
{
public:
	void observe(const TYPE value);
	bool observe_multi(const std::vector<TYPE>& multi, const TYPE sum);

	bool reduce(const void *ptr, size_t sz) override;
	std::string collect() override;

	size_t get_size() const override { return this->bucket_counts.size(); }
	const void *get_data() const override { return this; }

	TYPE get_sum() const { return this->sum; }
	size_t get_count() const { return this->count; }
	const std::vector<size_t> *get_bucket_counts() const
	{
		return &this->bucket_counts;
	}

public:
	Var *create(bool with_data) override
	{
		HistogramVar *var = new HistogramVar<TYPE>(this->name, this->help,
												   this->bucket_boundaries);
		if (with_data)
		{
			var->bucket_counts = this->bucket_counts;
			var->sum = this->sum;
			var->count = this->count;
		}

		return var;
	}

	HistogramVar(const std::string& name, const std::string& help,
				 const std::vector<TYPE>& bucket) :
		Var(name, help, VAR_HISTOGRAM),
		bucket_boundaries(bucket),
		bucket_counts(bucket.size() + 1)
	{
		this->init();
	}

	virtual void init()
	{
		this->sum = 0;
		this->count = 0;
	}

private:
	std::vector<TYPE> bucket_boundaries;
	std::vector<size_t> bucket_counts;
	TYPE sum;
	size_t count;
};

template<typename TYPE>
void HistogramVar<TYPE>::observe(const TYPE value)
{
	size_t i = 0;

	for (; i < this->bucket_boundaries.size(); i++)
	{
		if (value <= this->bucket_boundaries[i])
			break;
	}

	this->bucket_counts[i]++;
	this->sum += value;
	this->count++;
}

template<typename TYPE>
bool HistogramVar<TYPE>::observe_multi(const std::vector<TYPE>& multi,
									   const TYPE sum)
{
	if (multi.size() != this->bucket_counts.size())
		return false;

	for (size_t i = 0; i < multi.size(); i ++)
	{
		this->bucket_counts[i] += multi[i];
		this->count += multi[i];
	}
	this->sum += sum;

	return true;
}

template<typename TYPE>
bool HistogramVar<TYPE>::reduce(const void *ptr, size_t sz)
{
	if (sz != this->bucket_boundaries.size() + 1)
		return false;

	const HistogramVar<TYPE> *data = (const HistogramVar<TYPE> *)ptr;
	const std::vector<size_t> *src_bucket_counts = data->get_bucket_counts();

	for (size_t i = 0; i < sz; i++)
		this->bucket_counts[i] += (*src_bucket_counts)[i];

	this->bucket_counts[sz] += (*src_bucket_counts)[sz];
	this->sum += data->get_sum();
	this->count += data->get_count();

	return true;
}

template<typename TYPE>
std::string HistogramVar<TYPE>::collect()
{
	std::string ret;
	size_t i = 0;
	size_t current = 0;

	for (; i < this->bucket_boundaries.size(); i++)
	{
		current += this->bucket_counts[i];
		ret += this->name + "_bucket{le=\"" +
			   std::to_string(this->bucket_boundaries[i]) + "\"} " +
			   std::to_string(current) + "\n";
	}

	current += this->bucket_counts[i];
	ret += this->name + "_bucket{le=\"+Inf\"} " +
		   std::to_string(current) + "\n";

	ret += this->name + "_sum " + std::to_string(this->sum) + "\n";
	ret += this->name + "_count " + std::to_string(this->count) + "\n";

	return std::move(ret);
}

} // namespace prometheus
#endif

