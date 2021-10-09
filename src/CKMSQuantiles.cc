#include "CKMSQuantiles.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>

namespace prometheus {

Quantile::Quantile(double q, double err)
{
	quantile = q;
	error = err;
	u = 2.0 * err / (1.0 - q);
 	v = 2.0 * err / q;
}

CKMSQuantiles::Item::Item(double val, int lower_del, int del)
{
	value = val;
	g = lower_del;
	delta = del;
}

CKMSQuantiles::CKMSQuantiles(const std::vector<Quantile>& q) :
	quantiles(q)
{
	this->count = 0;
	//this->buffer;
	this->buffer_count = 0;
}

void CKMSQuantiles::insert(double value)
{
	this->buffer[this->buffer_count] = value;
	++this->buffer_count;

 	if (this->buffer_count == this->buffer.size())
	{
		this->insertBatch();
		this->compress();
	}
}

double CKMSQuantiles::get(double q)
{
	this->insertBatch();
	this->compress();

	if (this->sample.empty())
		return std::numeric_limits<double>::quiet_NaN();

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

void CKMSQuantiles::reset()
{
	this->count = 0;
	this->sample.clear();
	this->buffer_count = 0;
}

double CKMSQuantiles::allowableError(int rank)
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

bool CKMSQuantiles::insertBatch()
{
	if (this->buffer_count == 0)
		return false;

	std::sort(this->buffer.begin(), this->buffer.begin() + this->buffer_count);

	size_t start = 0;
	size_t idx = 0;
	size_t item = idx++;

	if (this->sample.empty())
	{
		this->sample.emplace_back(this->buffer[0], 1, 0);
		++start;
		++this->count;
	}

	for (size_t i = start; i < this->buffer_count; ++i)
	{
		double v = this->buffer[i];
		int delta;

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

void CKMSQuantiles::compress()
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

