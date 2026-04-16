#include "PVFirstPolicy.hpp"
#include <algorithm>

PowerSplit PVFirstPolicy::apply(double P_job, double P_pv) const
{
    // A regra aqui e a mais importante do projeto:
    // eu tento atender primeiro o job com a potencia fotovoltaica disponivel.
    // O que faltar, eu completo com a rede.

    PowerSplit split;
    split.pv   = std::min(P_job, P_pv);
    split.grid = P_job - split.pv;
    return split;
}
