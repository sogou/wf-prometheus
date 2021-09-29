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

#ifndef _VARS_H_
#define _VARS_H_

#include <utility>
#include <mutex>
#include <string>
#include <vector>
#include <unordered_map>

namespace prometheus {

class Var;
class VarLocal;

template<typename TYPE>
class GaugeVar;

template<typename TYPE>
class CounterVar;

enum VarType
{
	VARS_GAUGE		=	0,
	VARS_COUNTER	=	1,
	VARS_HISTOGRAM	=	2,
	VARS_SUMMARY	=	3
};

static std::string type_string(VarType type)
{
	switch (type)
	{
	case VARS_GAUGE:
		return "gauge";
	case VARS_COUNTER:
		return "counter";
	case VARS_HISTOGRAM:
		return "histogram";
	case VARS_SUMMARY:
		return "summary";
	default:
		break;
	}
	return "";
}

class VarGlobal
{
public:
	static VarGlobal *get_instance()
	{
		static VarGlobal kInstance;
		return &kInstance;
	}

	void add(VarLocal *var)
	{
		this->mutex.lock();
		this->local_vars.push_back(var);
		this->mutex.unlock();
	}

	Var *find(const std::string& name);

private:
	VarGlobal() { }

private:
	std::mutex mutex;
	std::vector<VarLocal *> local_vars;
	friend class VarFactory;
};

class VarLocal
{
public:
	static VarLocal *get_instance()
	{
		static thread_local VarLocal kInstance;
		return &kInstance;
	}

	void add(std::string name, Var *var)
	{
		this->mutex.lock();
		const auto it = this->vars.find(name);

		if (it == this->vars.end())
			this->vars.insert(std::make_pair(std::move(name), var));

		this->mutex.unlock();
	}

	virtual ~VarLocal();

private:
	VarLocal()
	{
		VarGlobal::get_instance()->add(this);
	}

private:
	std::mutex mutex;
	std::unordered_map<std::string, Var *> vars;
	friend class VarGlobal;
	friend class VarFactory;
};

class Var
{
public:
	const std::string& get_name() const { return this->name; }
	const std::string& get_help() const { return this->help; }
	VarType get_type() const { return this->type; }
	const std::string get_type_str() const { return type_string(this->type); }

	virtual Var *create() = 0;
	virtual std::string collect() = 0;
	virtual bool reduce(const void *ptr, size_t sz) = 0;
	virtual size_t get_size() = 0;
	virtual void *get_data() = 0;

public:
	Var(std::string name, std::string help, VarType type) :
		name(std::move(name)),
		help(std::move(help)),
		type(type)
	{
		this->format_name();
	}

	virtual ~Var() {}

private:
	void format_name();

protected:
	std::string name;
	std::string help;
	VarType type;
};

} // namespace prometheus

#endif

