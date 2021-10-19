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

#ifndef _GAUGEVAR_H_
#define _GAUGEVAR_H_

#include <utility>
#include <string>
#include <vector>

namespace prometheus {

template<typename TYPE>
class GaugeVar : public Var
{
public:
	void increase() { ++this->data; }
	void decrease() { --this->data; }
	size_t get_size() const override { return sizeof(TYPE); }
	const void *get_data() const override { return &this->data; }

	virtual TYPE get() const { return this->data; }
	virtual void set(TYPE var) { this->data = var; }

	bool reduce(const void *ptr, size_t sz) override
	{
		if (sz != sizeof(TYPE))
			return false;

		TYPE *data = (TYPE *)ptr;
		this->data += *data;
//		fprintf(stderr, "reduce data=%d *ptr=%d\n", this->data, *(int *)ptr);
		return true;
	}

public:
	virtual void init() { this->data = 0; }

	GaugeVar(const std::string& name, const std::string& help) :
		Var(name, help, VAR_GAUGE)
	{
		this->init();
	}

	Var *create(bool with_data) override
	{
		GaugeVar *var =  new GaugeVar<TYPE>(this->name, this->help);

		if (with_data)
			var->data = this->data;

		return var;
	}

	std::string collect() override
	{
		return this->name + " " + this->data_str() + "\n";
	}

	std::string data_str()
	{
		return std::to_string(this->data);
	}

	~GaugeVar() {}

	GaugeVar(GaugeVar<TYPE>&& move) = default;
	GaugeVar& operator=(GaugeVar<TYPE>&& move) = default;

private:
	TYPE data;
};

} // namespace prometheus

#endif

