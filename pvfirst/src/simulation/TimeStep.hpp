#pragma once

struct TimeStep {
    double P_job;          // kW
    double P_pv;           // kW
    double delta_seconds;  // duração da janela em segundos
};
