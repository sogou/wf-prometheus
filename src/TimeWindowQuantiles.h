#ifndef _TIMEWINDOWQUANTILES_H_
#define _TIMEWINDOWQUANTILES_H_

#include <chrono>
#include <cstddef>
#include <vector>

#include <memory>
#include <ratio>

#include "CKMSQuantiles.h"

namespace prometheus
{

template<typename TYPE>
class TimeWindowQuantiles
{
public:
	using Clock = std::chrono::steady_clock;

	TimeWindowQuantiles(const std::vector<Quantile>& quantiles,
						Clock::duration max_age_seconds,
						int age_buckets);

	size_t get(double quantile, TYPE *value);
	void insert(TYPE value);

private:
	CKMSQuantiles<TYPE>& rotate();

	const std::vector<Quantile>& quantiles;
	std::vector<CKMSQuantiles<TYPE>> ckms_quantiles;
	size_t current_bucket;

	Clock::time_point last_rotation;
	const Clock::duration rotation_interval;
};

/////////////// inl

template<typename TYPE>
TimeWindowQuantiles<TYPE>::TimeWindowQuantiles(const std::vector<Quantile>& q,
											   const Clock::duration max_age,
											   const int age_buckets) :
	quantiles(q),
	ckms_quantiles(age_buckets, CKMSQuantiles<TYPE>(this->quantiles)),
	rotation_interval(max_age / age_buckets)
{
	this->current_bucket = 0;
	this->last_rotation = Clock::now();
}

template<typename TYPE>
size_t TimeWindowQuantiles<TYPE>::get(double quantile, TYPE *value)
{
	CKMSQuantiles<TYPE>& current_bucket = this->rotate();

	*value = current_bucket.get(quantile);
	return current_bucket.get_count();
}

template<typename TYPE>
void TimeWindowQuantiles<TYPE>::insert(TYPE value)
{
	this->rotate();

	for (auto& bucket : this->ckms_quantiles)
		bucket.insert(value);
}

template<typename TYPE>
CKMSQuantiles<TYPE>& TimeWindowQuantiles<TYPE>::rotate()
{
	auto delta = Clock::now() - this->last_rotation;

	while (delta > this->rotation_interval)
	{
		this->ckms_quantiles[this->current_bucket].reset();

		if (++this->current_bucket >= this->ckms_quantiles.size())
			this->current_bucket = 0;

		delta -= this->rotation_interval;
		this->last_rotation += this->rotation_interval;
	}

	return this->ckms_quantiles[this->current_bucket];
}

} // namespace prometheus

#endif

