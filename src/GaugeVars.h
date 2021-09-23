#ifndef _GAUGEVARS_H_
#define _GAUGEVARS_H_

#include <utility>
#include <string>
#include <vector>

namespace prometheus {

template<typename TYPE>
class BasicVars : public Vars
{
public:
	void increase() override { ++this->data; }
	void decrease() override { --this->data; }
	size_t get_size() override { return sizeof(TYPE); }
	void *get_data() override { return &(this->data); }

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

	BasicVars(const std::string& name, const std::string& help) :
		Vars(name, help, VARS_GAUGE)
	{
		this->init();
	}

	Vars *create() override
	{
//		fprintf(stderr, "create() with name %s\n", this->name.c_str());
		return new BasicVars<TYPE>(this->name, this->help);
	}

	std::string collect() override
	{
		return this->name + " " + this->data_str() + "\n";
	}

	std::string data_str()
	{
		return std::to_string(this->data);
	}

	~BasicVars() {}

	BasicVars(BasicVars<TYPE>&& move) = default;
	BasicVars& operator=(BasicVars<TYPE>&& move) = default;

private:
	TYPE data;
};

template<typename TYPE>
class GaugeVars : public BasicVars<TYPE>
{
public:
	GaugeVars(const std::string& name, const std::string& help) :
		BasicVars<TYPE>(name, help)
	{
		VarsLocal::get_instance()->add(this->name, this);
	}
};

} // namespace prometheus

#endif

