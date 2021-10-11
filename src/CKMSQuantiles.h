#ifndef _CKMSQUANTILE_H_
#define _CKMSQUANTILE_H_

#include <array>
#include <cstddef>
#include <functional>
#include <vector>

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>

namespace prometheus {

struct Quantile
{
	Quantile(double quantile, double error);

	double quantile;
	double error;
	double u;
	double v;
};

template<typename TYPE>
class CKMSQuantiles
{
private:
	struct Item
	{
		Item(TYPE value, int lower_delta, int delta);

		TYPE value;
		int g;
		int delta;
	};

 public:
	CKMSQuantiles(const std::vector<Quantile>& quantiles);

	void insert(TYPE value);
	TYPE get(double q);
	void reset();

 private:
	double allowableError(int rank);
	bool insertBatch();
	void compress();

 private:
	const std::vector<Quantile>& quantiles;

	size_t count;
	std::vector<Item> sample;
	std::array<TYPE, 500> buffer;
	size_t buffer_count;
};

//////////// inl

template<typename TYPE>
CKMSQuantiles<TYPE>::Item::Item(TYPE val, int lower_del, int del)
{
	value = val;
	g = lower_del;
	delta = del;
}

template<typename TYPE>
CKMSQuantiles<TYPE>::CKMSQuantiles(const std::vector<Quantile>& q) :
	quantiles(q)
{
	this->count = 0;
	//this->buffer;
	this->buffer_count = 0;
}

template<typename TYPE>
void CKMSQuantiles<TYPE>::insert(TYPE value)
{
	this->buffer[this->buffer_count] = value;
	++this->buffer_count;

 	if (this->buffer_count == this->buffer.size())
	{
		this->insertBatch();
		this->compress();
	}
}

template<typename TYPE>
TYPE CKMSQuantiles<TYPE>::get(double q)
{
	this->insertBatch();
	this->compress();

	if (this->sample.empty())
		return std::numeric_limits<TYPE>::quiet_NaN();

	int rankMin = 0;
	const auto desired = static_cast<int>(q * this->count);
	const auto bound = desired + (allowableError(desired) / 2);

	auto it = this->sample.begin();
	decltype(it) prev;
	auto cur = it++;

	while (it != this->sample.end())
	{
		prev = cur;
		cur = it++;

		rankMin += prev->g;

		if (rankMin + cur->g + cur->delta > bound)
			return prev->value;
	}

	return this->sample.back().value;
}

template<typename TYPE>
void CKMSQuantiles<TYPE>::reset()
{
	this->count = 0;
	this->sample.clear();
	this->buffer_count = 0;
}

template<typename TYPE>
double CKMSQuantiles<TYPE>::allowableError(int rank)
{
	auto size = this->sample.size();
	double minError = size + 1;

	for (const auto& q : this->quantiles)
	{
		double error;
		if (rank <= q.quantile * size)
			error = q.u * (size - rank);
		else
			error = q.v * rank;

		if (error < minError)
			minError = error;
	}

	return minError;
}

template<typename TYPE>
bool CKMSQuantiles<TYPE>::insertBatch()
{
	if (this->buffer_count == 0)
		return false;

	std::sort(this->buffer.begin(), this->buffer.begin() + this->buffer_count);

	size_t start = 0;
	size_t idx = 0;
	size_t item = idx++;
	TYPE v;
	int delta;

	if (this->sample.empty())
	{
		this->sample.emplace_back(this->buffer[0], 1, 0);
		++start;
		++this->count;
	}

	for (size_t i = start; i < this->buffer_count; ++i)
	{
		v = this->buffer[i];

		while (idx < this->sample.size() && this->sample[item].value < v)
			item = idx++;

		if (this->sample[item].value > v)
			--idx;

		if (idx - 1 == 0 || idx + 1 == this->sample.size())
			delta = 0;
		else
			delta = static_cast<int>(std::floor(allowableError(idx + 1))) + 1;

		this->sample.emplace(this->sample.begin() + idx, v, 1, delta);
		this->count++;
		item = idx++;
	}

	this->buffer_count = 0;
	return true;
}

template<typename TYPE>
void CKMSQuantiles<TYPE>::compress()
{
	if (this->sample.size() < 2)
		return;

	size_t idx = 0;
	size_t prev;
	size_t next = idx++;

	while (idx < this->sample.size())
	{
		prev = next;
		next = idx++;

		if (this->sample[prev].g + this->sample[next].g +
			this->sample[next].delta <= allowableError(idx - 1))
		{
			this->sample[next].g += this->sample[prev].g;
			this->sample.erase(this->sample.begin() + prev);
		}
	}
}

} // namespace prometheus

#endif

