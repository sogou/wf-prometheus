#include "CKMSQuantiles.h"

namespace prometheus {

Quantile::Quantile(double q, double err)
{
	quantile = q;
	error = err;
	u = 2.0 * err / (1.0 - q);
 	v = 2.0 * err / q;
}

} // namespace prometheus

