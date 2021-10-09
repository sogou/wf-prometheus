#ifndef _TIMEWINDOWQUANTILES_H_
#define _TIMEWINDOWQUANTILES_H_

#include <chrono>
#include <cstddef>
#include <vector>

#include "CKMSQuantiles.h"

namespace prometheus
{

class TimeWindowQuantiles
{
public:
	using Clock = std::chrono::steady_clock;

	TimeWindowQuantiles(const std::vector<Quantile>& quantiles,
						Clock::duration max_age_seconds, int age_buckets);

	double get(double q) const;
	void insert(double value);

private:
	CKMSQuantiles& rotate() const;

	const std::vector<Quantile>& quantiles;
	mutable std::vector<CKMSQuantiles> ckms_quantiles;
	mutable size_t current_bucket;

	mutable Clock::time_point last_rotation;
	const Clock::duration rotation_interval;
};

} // namespace prometheus

#endif

