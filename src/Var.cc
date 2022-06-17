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

#include <mutex>
#include <string>
#include <vector>
#include <unordered_map>
#include "Var.h"

namespace prometheus {

void Var::format_name()
{
	//TODO: change aaa.bbb AAA.BBB to aaa_bbb
}

VarLocal::~VarLocal()
{
	VarGlobal *global_var = VarGlobal::get_instance();

	global_var->dup(this->vars);

	for (auto it = this->vars.begin(); it != this->vars.end(); it++)
		delete it->second;

	global_var->del(this);
}

void VarGlobal::dup(const std::unordered_map<std::string, Var *>& vars)
{
	if (this->local_vars.empty())
		new VarLocal();

	VarLocal *local = this->local_vars[0];
	local->mutex.lock();

	std::unordered_map<std::string, Var*>& local_var = local->vars;

	for (auto it = vars.begin(); it != vars.end(); it++)
	{
		if (local_var.find(it->first) == local_var.end())
		{
			local_var.insert(std::make_pair(it->first,
											it->second->create(true)));
		}
		else
		{
			local_var[it->first]->reduce(it->second->get_data(),
										 it->second->get_size());
		}
	}

	local->mutex.unlock();
}

void VarGlobal::del(const VarLocal *var)
{
	this->mutex.lock();
	for (size_t i = 0; i < this->local_vars.size(); i++)
	{
		if (this->local_vars[i] == var)
		{
			for (size_t j = i; j < this->local_vars.size() - 1; j++)
				this->local_vars[j] = this->local_vars[j + 1];

			break;
		}
	}

	this->local_vars.resize(this->local_vars.size() - 1);
	this->mutex.unlock();
}

Var *VarGlobal::find(const std::string& name)
{
	std::unordered_map<std::string, Var*>::iterator it;
	VarGlobal *global_var = VarGlobal::get_instance();
	Var *ret = NULL;
	VarLocal *local;

	global_var->mutex.lock();
	for (size_t i = 0; i < global_var->local_vars.size() && !ret; i++)
	{
		local = global_var->local_vars[i];
		for (it = local->vars.begin(); it != local->vars.end(); it++)
		{
			if (!name.compare(it->second->get_name()))
			{
				ret = it->second;
				break;
			}
		}
	}

	global_var->mutex.unlock();
	return ret;
}

} // namespace prometheus

