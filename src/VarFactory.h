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

#include "Var.h"
#include "GaugeVar.h"
#include "CounterVar.h"

#ifndef _VARFACTORY_H_
#define _VARFACTORY_H_

namespace prometheus {

class VarFactory
{
public:
	template<typename TYPE>
	static GaugeVar<TYPE> *create_gauge(const std::string& name,
										const std::string& help);

	template<typename TYPE>
	static CounterVar<TYPE> *create_counter(const std::string& name,
											const std::string& help);

	// thread local api
	template<typename TYPE>
	static GaugeVar<TYPE> *gauge(const std::string& name);
	template<typename TYPE>
	static CounterVar<TYPE> *counter(const std::string& name);

	static Var *var(const std::string& name);

	// global api
	static std::string expose();
	//void expose(std::string& name) { }
};

template<typename TYPE>
GaugeVar<TYPE> *VarFactory::create_gauge(const std::string& name,
										 const std::string& help)
{
	GaugeVar<TYPE> *gauge = new GaugeVar<TYPE>(name, help);
	VarLocal::get_instance()->add(name, gauge);
	return gauge;
}

template<typename TYPE>
CounterVar<TYPE> *VarFactory::create_counter(const std::string& name,
											 const std::string& help)
{
	CounterVar<TYPE> *counter = new CounterVar<TYPE>(name, help);
	VarLocal::get_instance()->add(name, counter);
	return counter;
}

template<typename TYPE>
GaugeVar<TYPE> *VarFactory::gauge(const std::string& name)
{
	return static_cast<GaugeVar<TYPE> *>(VarFactory::var(name));
}

template<typename TYPE>
CounterVar<TYPE> *VarFactory::counter(const std::string& name)
{
	return static_cast<CounterVar<TYPE> *>(VarFactory::var(name));
}

} // namespace prometheus

#endif

