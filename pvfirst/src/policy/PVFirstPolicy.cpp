#include "PVFirstPolicy.hpp"
#include <algorithm>

PowerSplit PVFirstPolicy::apply(double P_job, double P_pv) const {
    PowerSplit split;
    split.pv = std::min(P_job, P_pv);
    split.grid = P_job - split.pv;
    return split;
}
