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

#ifndef _VAR_H_
#define _VAR_H_

#include <utility>
#include <mutex>
#include <string>
#include <vector>
#include <unordered_map>

namespace prometheus {

class Var;
class VarLocal;

enum VarType
{
	VAR_GAUGE		=	0,
	VAR_COUNTER		=	1,
	VAR_HISTOGRAM	=	2,
	VAR_SUMMARY		=	3
};

static std::string type_string(VarType type)
{
	switch (type)
	{
	case VAR_GAUGE:
		return "gauge";
	case VAR_COUNTER:
		return "counter";
	case VAR_HISTOGRAM:
		return "histogram";
	case VAR_SUMMARY:
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

	void del(const VarLocal *var);
	Var *find(const std::string& name);
	void dup(const std::unordered_map<std::string, Var *>& vars);

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

	virtual Var *create(bool with_data) = 0;
	virtual std::string collect() = 0;
	virtual bool reduce(const void *ptr, size_t sz) = 0;
	virtual size_t get_size() const = 0;
	virtual const void *get_data() const = 0;

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

