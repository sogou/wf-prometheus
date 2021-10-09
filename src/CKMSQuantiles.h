#ifndef _CKMSQUANTILE_H_
#define _CKMSQUANTILE_H_

#include <array>
#include <cstddef>
#include <functional>
#include <vector>

namespace prometheus {

struct Quantile
{
	Quantile(double quantile, double error);

	double quantile;
	double error;
	double u;
	double v;
};

class CKMSQuantiles
{
private:
	struct Item
	{
		Item(double value, int lower_delta, int delta);

		double value;
		int g;
		int delta;
	};

 public:
	CKMSQuantiles(const std::vector<Quantile>& quantiles);

	void insert(double value);
	double get(double q);
	void reset();

 private:
	double allowableError(int rank);
	bool insertBatch();
	void compress();

 private:
	const std::vector<Quantile>& quantiles;

	size_t count;
	std::vector<Item> sample;
	std::array<double, 500> buffer;
	size_t buffer_count;
};

} // namespace prometheus

#endif

