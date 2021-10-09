#include "TimeWindowQuantiles.h"

#include <memory>
#include <ratio>

namespace prometheus {

TimeWindowQuantiles::TimeWindowQuantiles(const std::vector<Quantile>& q,
										 const Clock::duration max_age,
										 const int age_buckets) :
	quantiles(q),
	ckms_quantiles(age_buckets, CKMSQuantiles(this->quantiles)),
	rotation_interval(max_age / age_buckets)
{
	this->current_bucket = 0;
	this->last_rotation = Clock::now();
}

double TimeWindowQuantiles::get(double q) const
{
	CKMSQuantiles& current_bucket = rotate();
	return current_bucket.get(q);
}

void TimeWindowQuantiles::insert(double value)
{
	this->rotate();

	for (auto& bucket : this->ckms_quantiles)
		bucket.insert(value);
}

CKMSQuantiles& TimeWindowQuantiles::rotate() const
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

}	// namespace prometheus

