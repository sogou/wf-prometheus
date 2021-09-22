#include <mutex>
#include <string>
#include <vector>
#include <unordered_map>
#include "Vars.h"

namespace prometheus {

void Vars::format_name()
{
	//TODO: change aaa.bbb AAA.BBB to aaa_bbb
}

VarsLocal::~VarsLocal()
{
//	for (auto it = this->vars.begin(); it != this->vars.end(); it++)
//		delete it->second;
}

Vars *VarsLocal::var(const std::string& name)
{
	VarsLocal *local = VarsLocal::get_instance();
	auto it = local->vars.find(name);
	if (it != local->vars.end())
		return it->second;

	Vars *var = VarsGlobal::get_instance()->find(name);
	if (var)
	{
		Vars *new_var = var->create();
		VarsLocal::get_instance()->add(name, new_var);
		return new_var;
	}

	return NULL;
}

Vars *VarsGlobal::find(const std::string& name)
{
	std::unordered_map<std::string, Vars*>::iterator it;
	VarsGlobal *global_var = VarsGlobal::get_instance();
	Vars *ret = NULL;
	VarsLocal *local;

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

std::string VarsGlobal::expose()
{
	std::string output;
	std::unordered_map<std::string, Vars*> tmp;
	std::unordered_map<std::string, Vars*>::iterator it;
	std::unordered_map<std::string, Vars*>::iterator tmp_it;

	VarsGlobal *global_var = VarsGlobal::get_instance();

	global_var->mutex.lock();
	for (VarsLocal *local : global_var->local_vars)
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
		Vars *var = it->second;
		output = output +  "# HELP " + var->get_name() + " " + var->get_help() + 
				 "\n# TYPE " + var->get_name() + " " + var->get_type_str() +
				 "\n" + var->collect();
	}

	return output;
}

} // namespace prometheus

