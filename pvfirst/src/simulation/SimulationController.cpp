#include "SimulationController.hpp"
#include "sensors/GeoSensor.hpp"
#include "sensors/SolarModel.hpp"
#include "sensors/MetarSensor.hpp"

#include <iostream>
#include <string>
#include <cctype>
#include <fstream>
#include <filesystem>
#include <ctime>
#include <sstream>
#include <cmath>



SimulationController::SimulationController()
    : model(100.0)
{}



double SimulationController::parseTimeInput(const std::string& input)
{
    double value = std::stod(input.substr(0, input.size() - 1));
    char unit = std::toupper(input.back());

    switch(unit)
    {
        case 'S': return value;
        case 'M': return value * 60.0;
        case 'H': return value * 3600.0;
        default:
            std::cout << "Unidade invalida. Usando segundos.\n";
            return value;
    }
}



void SimulationController::run()
{
    std::string input;

    std::cout << "\nDigite o tempo de simulacao (ex: 10S, 5M, 2H): ";
    std::cin >> input;

    double total_seconds = parseTimeInput(input);

    std::cout << "\n============= INICIANDO SIMULACAO =============\n";
    std::cout << "Simulando " << total_seconds << " segundos...\n";



    // =========================================================
    // GPS
    // =========================================================

    GeoSensor geo;
    GPSData gps = geo.getLocation();

    std::cout << "\nLocal detectado: "
              << gps.city
              << " (Lat: " << gps.latitude
              << ", Lon: " << gps.longitude
              << ")\n";



    // =========================================================
    // DATA E HORA
    // =========================================================

    std::time_t now = std::time(nullptr);
    std::tm* local = std::localtime(&now);

    int dayOfYear = local->tm_yday + 1;
    int hourInt   = local->tm_hour;
    int minuteInt = local->tm_min;

    double hour = hourInt + minuteInt / 60.0;

    std::cout << "Hora local: "
              << (hourInt < 10 ? "0" : "") << hourInt << ":"
              << (minuteInt < 10 ? "0" : "") << minuteInt << "\n";

    std::cout << "Dia do ano: " << dayOfYear << "\n";



    // =========================================================
    // MODELO SOLAR
    // =========================================================

    SolarModel solar;

    double G = solar.computeIrradiance(
                   gps.latitude,
                   dayOfYear,
                   hour);

    std::cout << "Irradiancia teorica: "
              << G << " W/m2\n";



    // =========================================================
    // CLIMA (API)
    // =========================================================

    MetarSensor metar;

    WeatherImpact impact =
        metar.getWeatherImpact(
            gps.latitude,
            gps.longitude);

    double temp  = impact.temperature;
    double cloud = impact.cloudCover;
    double rain  = impact.rainAmount;
    double wind  = impact.windSpeed;

    std::cout << "\n============ CONDICOES METEOROLOGICAS ============\n";

    std::cout << "Cobertura de nuvens: " << cloud << " % ";
    if (impact.cloudFactor == 1.0)
        std::cout << "(Ceu limpo)\n";
    else
        std::cout << "(Reducao de "
                  << (1.0 - impact.cloudFactor) * 100
                  << "%)\n";

    std::cout << "Chuva: " << rain << " mm ";
    if (impact.rainFactor == 1.0)
        std::cout << "(Sem precipitacao)\n";
    else
        std::cout << "(Reducao significativa)\n";

    std::cout << "Temperatura: " << temp << " °C ";
    if (impact.tempFactor == 1.0)
        std::cout << "(Sem perda termica significativa)\n";
    else
        std::cout << "(Perda de "
                  << (1.0 - impact.tempFactor) * 100
                  << "%)\n";

    std::cout << "Velocidade do vento: " << wind << " km/h ";
    if (impact.windCoolingFactor == 1.0)
        std::cout << "(Sem impacto significativo)\n";
    else
        std::cout << "(Ganho de "
                  << (impact.windCoolingFactor - 1.0) * 100
                  << "%)\n";



    // =========================================================
    // APLICAR IMPACTOS
    // =========================================================

    G *= impact.cloudFactor;
    G *= impact.rainFactor;

    std::cout << "\nIrradiancia ajustada: "
              << G << " W/m2\n";



    // =========================================================
    // EFICIENCIA + POTENCIA
    // =========================================================

    double baseEfficiency = 0.20;

    double efficiency =
        baseEfficiency *
        impact.tempFactor *
        impact.windCoolingFactor;

    double area = 10.0;

    double P_pv =
        efficiency * area * G / 1000.0;

    std::cout << "Eficiencia final: "
              << efficiency << "\n";

    std::cout << "Potencia PV estimada: "
              << P_pv << " kW\n";



    // =========================================================
    // ENERGY MODEL
    // =========================================================

    double P_job = 2.0;

    model.update(P_job,
                 P_pv,
                 total_seconds);

    EnergyStats stats = model.getStats();

    std::cout << "\n============= RESULTADO =============\n";
    std::cout << "Energia Total: " << stats.E_total << " kWh\n";
    std::cout << "Energia PV: "    << stats.E_pv    << " kWh\n";
    std::cout << "Energia Grid: "  << stats.E_grid  << " kWh\n";
    std::cout << "Emissoes CO2: "  << stats.CO2     << " gCO2\n";



    // =========================================================
    // SALVAR CSV
    // =========================================================

    auto fmt = [](double v)
    {
        if (std::abs(v) < 1e-9)
            return std::string("0");

        std::ostringstream oss;
        oss << v;
        return oss.str();
    };

    bool fileExists =
        std::filesystem::exists("results.csv");

    std::ofstream file("results.csv",
                       std::ios::app);

    if (!fileExists)
    {
        file << "Hora_local;Dia_ano;Irradiancia;Temperatura;"
                "P_pv_inst;E_total;E_pv;E_grid;CO2\n";
    }

    file << (hourInt < 10 ? "0" : "") << hourInt
         << ":" 
         << (minuteInt < 10 ? "0" : "") << minuteInt
         << ";"
         << dayOfYear << ";"
         << fmt(G) << ";"
         << fmt(temp) << ";"
         << fmt(P_pv) << ";"
         << fmt(stats.E_total) << ";"
         << fmt(stats.E_pv) << ";"
         << fmt(stats.E_grid) << ";"
         << fmt(stats.CO2)
         << "\n";

    file.close();

    std::cout << "\nDados salvos em results.csv\n";
}
