#include "SimulationController.hpp"
#include "SimGridJobRunner.hpp"
#include "sensors/GeoSensor.hpp"
#include "sensors/MetarSensor.hpp"
#include "sensors/SolarModel.hpp"

#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

SimulationController::SimulationController()
    : model(100.0)
{
}

double SimulationController::parseJobInput(const std::string& input)
{
    // Se eu apertar enter sem digitar nada, uso um valor padrao para nao travar o teste.
    if (input.empty())
        return 5e10;

    return std::stod(input);
}

double SimulationController::askJobFlops()
{
    std::string input;

    std::cout << "Digite a carga do job em FLOPs (ex: 5e10).\n";
    std::cout << "Se quiser usar o valor padrao, e so apertar Enter: ";
    std::getline(std::cin, input);

    return parseJobInput(input);
}

void SimulationController::run()
{
    std::cout << "\n============================================================\n";
    std::cout << "SIMULACAO PV-FIRST COM JOB DO SIMGRID\n";
    std::cout << "============================================================\n\n";

    std::cout << "Fluxo da simulacao:\n";
    std::cout << "1) o SimGrid executa o job e mede a energia exigida pelo host\n";
    std::cout << "2) eu calculo a energia solar disponivel com base no local e no clima\n";
    std::cout << "3) a politica PV-First tenta atender primeiro o job com a placa\n\n";

    double jobFlops = askJobFlops();

    // ============================== LOCALIZACAO ==============================
    GeoSensor geo;
    GPSData gps = geo.getLocation();

    // ========================== HORA LOCAL E DIA =============================
    std::time_t now = std::time(nullptr);
    std::tm localTime = *std::localtime(&now);

    int dayOfYear = localTime.tm_yday + 1;
    int hourInt   = localTime.tm_hour;
    int minuteInt = localTime.tm_min;
    double hour   = hourInt + minuteInt / 60.0;

    // ========================== CLIMA E IRRADIANCIA ==========================
    SolarModel solar;
    double G = solar.computeIrradiance(gps.latitude, dayOfYear, hour);

    MetarSensor metar;
    WeatherImpact impact = metar.getWeatherImpact(gps.latitude, gps.longitude);

    // Aqui eu junto as perdas do clima em cima da irradiancia teorica.
    // Primeiro reduzo a irradiancia com nuvem e chuva.
    // Depois ajusto a eficiencia da placa com temperatura e vento.
    double G_adjusted = G * impact.cloudFactor * impact.rainFactor;

    double baseEfficiency = 0.20;
    double efficiency = baseEfficiency * impact.tempFactor * impact.windCoolingFactor;

    double area = 10.0;

    double P_pv = efficiency * area * G_adjusted / 1000.0;
    if (P_pv < 0.0)
        P_pv = 0.0;

    std::cout << "\n-------------------- DADOS DO LOCAL --------------------\n";
    std::cout << "Cidade detectada : " << gps.city << "\n";
    std::cout << "Latitude         : " << gps.latitude << "\n";
    std::cout << "Longitude        : " << gps.longitude << "\n";
    std::cout << "Hora local       : "
              << std::setfill('0') << std::setw(2) << hourInt << ":"
              << std::setfill('0') << std::setw(2) << minuteInt << "\n";
    std::cout << "Dia do ano       : " << dayOfYear << "\n";

    std::cout << "\n------------------ CONDICOES DO CLIMA ------------------\n";
    std::cout << "Cobertura nuvens : " << impact.cloudCover << " %\n";
    std::cout << "Chuva            : " << impact.rainAmount << " mm\n";
    std::cout << "Temperatura      : " << impact.temperature << " C\n";
    std::cout << "Vento            : " << impact.windSpeed << " km/h\n";

    std::cout << "\n----------------- MODELO FOTOVOLTAICO ------------------\n";
    std::cout << "Irradiancia teorica     : " << G << " W/m2\n";
    std::cout << "Irradiancia ajustada    : " << G_adjusted << " W/m2\n";
    std::cout << "Eficiencia final arranjo: " << efficiency << "\n";
    std::cout << "Potencia PV disponivel  : " << P_pv << " kW\n";

    if (G <= 0.0) {
        std::cout << "Observacao: neste horario a irradiancia teorica ficou zerada.\n";
        std::cout << "Isso normalmente acontece porque o calculo caiu em periodo noturno ou muito proximo disso.\n";
    }

    // ============================= JOB DO SIMGRID ============================
    SimGridJobRunner jobRunner;
    SimGridJobConfig jobConfig;
    jobConfig.jobFlops = jobFlops;

    SimGridJobResult job = jobRunner.run(jobConfig);

    std::cout << "\n--------------------- JOB DO SIMGRID -------------------\n";
    std::cout << "Host usado          : " << job.hostName << "\n";
    std::cout << "Carga do job        : " << job.jobFlops << " FLOPs\n";
    std::cout << "Velocidade do host  : " << job.hostSpeedFlops << " flop/s\n";
    std::cout << "Duracao do job      : " << job.durationSeconds << " s\n";
    std::cout << "Energia do job      : " << job.energyJoules << " J\n";
    std::cout << "Energia do job      : " << job.energyKWh << " kWh\n";
    std::cout << "Potencia media job  : " << job.averagePowerKW << " kW\n";

    // ============================= TRIAGEM PV-FIRST ==========================
    // A leitura aqui e direta:
    // - o SimGrid me diz quanta potencia media o job exigiu
    // - o modelo solar me diz quanta potencia PV eu tenho nesse instante
    // - a politica PV-First usa primeiro a placa e manda o restante para a rede
    model.update(job.averagePowerKW, P_pv, job.durationSeconds);
    EnergyStats stats = model.getStats();

    double pvPossible = P_pv * (job.durationSeconds / 3600.0);

    std::cout << "\n-------------------- RESULTADO PV-FIRST ----------------\n";
    std::cout << "Energia total do job : " << stats.E_total << " kWh\n";
    std::cout << "Energia vinda da PV  : " << stats.E_pv << " kWh\n";
    std::cout << "Energia vinda da rede: " << stats.E_grid << " kWh\n";
    std::cout << "CO2 da parte da rede : " << stats.CO2 << " gCO2\n";

    std::cout << "\nLeitura rapida do experimento:\n";
    std::cout << "- o job do SimGrid pediu " << job.energyKWh << " kWh no total\n";
    std::cout << "- a placa poderia entregar ate " << pvPossible << " kWh nesse mesmo intervalo\n";
    std::cout << "- a politica PV-First usou primeiro a energia solar e mandou o resto para a rede\n";

    // ================================ CSV ====================================
    bool fileExists = std::filesystem::exists("results.csv");
    std::ofstream file("results.csv", std::ios::app);

    if (!file.is_open()) {
        throw std::runtime_error("Nao consegui abrir ou criar o arquivo results.csv dentro da pasta build.");
    }

    if (!fileExists) {
        file << "Cidade;Latitude;Longitude;Hora_local;Dia_ano;"
                "Temperatura;Nuvens;Chuva;Vento;"
                "Irradiancia_teorica;Irradiancia_ajustada;Potencia_pv;"
                "Job_flops;Job_duracao_s;Job_energia_j;Job_energia_kwh;Job_potencia_media_kw;"
                "Energia_total_kwh;Energia_pv_kwh;Energia_grid_kwh;CO2_g\n";
    }

    std::ostringstream hourText;
    hourText << std::setfill('0') << std::setw(2) << hourInt << ":"
             << std::setfill('0') << std::setw(2) << minuteInt;

    file << gps.city << ";"
         << gps.latitude << ";"
         << gps.longitude << ";"
         << hourText.str() << ";"
         << dayOfYear << ";"
         << impact.temperature << ";"
         << impact.cloudCover << ";"
         << impact.rainAmount << ";"
         << impact.windSpeed << ";"
         << G << ";"
         << G_adjusted << ";"
         << P_pv << ";"
         << job.jobFlops << ";"
         << job.durationSeconds << ";"
         << job.energyJoules << ";"
         << job.energyKWh << ";"
         << job.averagePowerKW << ";"
         << stats.E_total << ";"
         << stats.E_pv << ";"
         << stats.E_grid << ";"
         << stats.CO2 << "\n";

    file.close();

    std::cout << "\nresults.csv atualizado com sucesso.\n";
    std::cout << "\n============================================================\n";
    std::cout << "SIMULACAO FINALIZADA\n";
    std::cout << "============================================================\n";
}