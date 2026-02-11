#pragma once

struct PowerSplit {
    double pv;    // kW
    double grid;  // kW
};

class PVFirstPolicy {
public:
    PowerSplit apply(double P_job, double P_pv) const;
};
