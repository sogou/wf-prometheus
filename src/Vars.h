#ifndef _VARS_H__
#define _VARS_H__

#include <utility>
#include <mutex>
#include <string>
#include <vector>
#include <unordered_map>

namespace prometheus {

class Vars;
class VarsLocal;

template<typename TYPE>
class GaugeVars;

template<typename TYPE>
class CounterVars;

enum VarsType
{
	RPC_VARS_GAUGE		=	0,
	RPC_VARS_COUNTER	=	1,
	RPC_VARS_HISTOGRAM	=	2,
	RPC_VARS_SUMMARY	=	3
};

static std::string type_string(VarsType type)
{
	switch (type)
	{
	case RPC_VARS_GAUGE:
		return "gauge";
	case RPC_VARS_COUNTER:
		return "counter";
	case RPC_VARS_HISTOGRAM:
		return "histogram";
	case RPC_VARS_SUMMARY:
		return "summary";
	default:
		break;
	}
	return NULL;
}

class VarsGlobal
{
public:
	static VarsGlobal *get_instance()
	{
		static VarsGlobal kInstance;
		return &kInstance;
	}

	void add(VarsLocal *var)
	{
		this->mutex.lock();
		this->local_vars.push_back(var);
		this->mutex.unlock();
	}

	Vars *find(const std::string& name);

	//	void expose(std::string& name) { }
	static std::string expose();

private:
	VarsGlobal() { }

private:
	std::mutex mutex;
	std::vector<VarsLocal *> local_vars;
};

class VarsLocal
{
public:
	static VarsLocal *get_instance()
	{
		static thread_local VarsLocal kInstance;
		return &kInstance;
	}

	void add(std::string name, Vars *var)
	{
		this->mutex.lock();
		const auto it = this->vars.find(name);

		if (it == this->vars.end())
			this->vars.insert(std::make_pair(std::move(name), var));

		this->mutex.unlock();
	}

	static Vars *var(const std::string& name);

	template<typename TYPE>
	static GaugeVars<TYPE> *gauge(const std::string& name)
	{
		return static_cast<GaugeVars<TYPE> *>(VarsLocal::var(name));
	}

	template<typename TYPE>
	static CounterVars<TYPE> *counter(const std::string& name)
	{
		return static_cast<CounterVars<TYPE> *>(VarsLocal::var(name));
	}

	~VarsLocal();

private:
	VarsLocal()
	{
		VarsGlobal::get_instance()->add(this);
	}

private:
	std::mutex mutex;
	std::unordered_map<std::string, Vars *> vars;
	friend class VarsGlobal;
};

class Vars
{
public:
	const std::string& get_name() const { return this->name; }
	const std::string& get_help() const { return this->help; }
	VarsType get_type() const { return this->type; }
	const std::string get_type_str() const { return type_string(this->type); }

	virtual Vars *create() = 0;
	virtual std::string collect() = 0;
	virtual void increase() = 0;
	virtual void decrease() = 0;
	virtual bool reduce(const void *ptr, size_t sz) = 0;
	virtual size_t get_size() = 0;
	virtual void *get_data() = 0;

public:
	Vars(std::string name, std::string help, VarsType type) :
		name(std::move(name)),
		help(std::move(help)),
		type(type)
	{
		this->format_name();
	}

	virtual ~Vars() {}

private:
	void format_name();

protected:
	std::string name;
	std::string help;
	VarsType type;
};

} // namespace prometheus

#endif

