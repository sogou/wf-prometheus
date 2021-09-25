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

#include "VarFactory.h"

namespace prometheus {

Var *VarFactory::var(const std::string& name)
{
	Var *var;
	Var *new_var;
	VarLocal *local = VarLocal::get_instance();
	auto it = local->vars.find(name);
	if (it != local->vars.end())
		return it->second;

	var = VarGlobal::get_instance()->find(name);

	if (var)
	{
		new_var = var->create();
		local->add(name, new_var);
		return new_var;
	}

	return NULL;
}

std::string VarFactory::expose()
{
	std::string output;
	std::unordered_map<std::string, Var*> tmp;
	std::unordered_map<std::string, Var*>::iterator it;
	std::unordered_map<std::string, Var*>::iterator tmp_it;

	VarGlobal *global_var = VarGlobal::get_instance();

	global_var->mutex.lock();
	for (VarLocal *local : global_var->local_vars)
	{
		local->mutex.lock();
		for (it = local->vars.begin(); it != local->vars.end(); it++)
		{
			tmp_it = tmp.find(it->first);
			if (tmp_it == tmp.end())
				tmp.insert(std::make_pair(it->first, it->second->create()));
			else
				tmp[it->first]->reduce(it->second->get_data(),
									   it->second->get_size());
		}
		local->mutex.unlock();
	}
	global_var->mutex.unlock();

	for (it = tmp.begin(); it != tmp.end(); it++)
	{
		Var *var = it->second;
		output = output +  "# HELP " + var->get_name() + " " + var->get_help() + 
				 "\n# TYPE " + var->get_name() + " " + var->get_type_str() +
				 "\n" + var->collect();
	}

	return output;
}


} // namespace prometheus

