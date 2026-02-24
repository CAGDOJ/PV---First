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

    // =====================================================
    // GPS
    // =====================================================

    GeoSensor geo;
    GPSData gps = geo.getLocation();

    std::cout << "Local detectado: "
              << gps.city
              << " (Lat: " << gps.latitude
              << ", Lon: " << gps.longitude
              << ")\n";

    // =====================================================
    // DATA
    // =====================================================

    std::time_t now = std::time(nullptr);
    std::tm* local = std::localtime(&now);

    int dayOfYear = local->tm_yday + 1;
    int hourInt   = local->tm_hour;
    int minuteInt = local->tm_min;

    double hour = hourInt + minuteInt / 60.0;

    std::cout << "Hora local: "
              << hourInt << ":"
              << minuteInt << "\n";

    std::cout << "Dia do ano: "
              << dayOfYear << "\n";

    // =====================================================
    // SOLAR MODEL
    // =====================================================

    SolarModel solar;

    double G = solar.computeIrradiance(
        gps.latitude,
        dayOfYear,
        hour);

    std::cout << "Irradiancia teorica: "
              << G << " W/m2\n";

    // =====================================================
    // WEATHER
    // =====================================================

    MetarSensor metar;
    WeatherImpact impact =
        metar.getWeatherImpact(
            gps.latitude,
            gps.longitude);

    std::cout << "\n============ CONDICOES METEOROLOGICAS ============\n";

    // ☁️ NUVENS
    std::cout << "Cobertura de nuvens: "
            << impact.cloudCover << " % ";

    if (impact.cloudCover < 20)
        std::cout << "(Ceu limpo - impacto minimo)\n";
    else if (impact.cloudCover < 60)
        std::cout << "(Reducao moderada na irradiancia)\n";
    else
        std::cout << "(Alta reducao na irradiancia)\n";

    // 🌧 CHUVA
    std::cout << "Chuva: "
            << impact.rainAmount << " mm ";

    if (impact.rainAmount == 0)
        std::cout << "(Sem precipitacao)\n";
    else if (impact.rainAmount < 2)
        std::cout << "(Chuva leve - pequeno impacto)\n";
    else
        std::cout << "(Precipitacao significativa - forte impacto)\n";

    // 🌡 TEMPERATURA
    std::cout << "Temperatura: "
            << impact.temperature << " °C ";

    if (impact.temperature <= 25)
        std::cout << "(Sem perda termica significativa)\n";
    else
    {
        double loss = (1.0 - impact.tempFactor) * 100.0;
        std::cout << "(Perda de " << loss << "% na eficiencia)\n";
    }

    // 💨 VENTO
    std::cout << "Velocidade do vento: "
            << impact.windSpeed << " km/h ";

    if (impact.windSpeed < 1)
        std::cout << "(Sem impacto significativo)\n";
    else
    {
        double gain = (impact.windCoolingFactor - 1.0) * 100.0;
        std::cout << "(Ganho de " << gain << "% por resfriamento)\n";
    }

    // =====================================================
    // APLICAR IMPACTOS
    // =====================================================

    double G_adjusted = G * impact.cloudFactor * impact.rainFactor;

    std::cout << "Irradiancia ajustada: "
              << G_adjusted << " W/m2\n";

    double baseEfficiency = 0.20;

    double efficiency =
        baseEfficiency *
        impact.tempFactor *
        impact.windCoolingFactor;

    std::cout << "Eficiencia final: "
              << efficiency << "\n";

    double area = 10.0;

    double P_pv =
        efficiency * area * G_adjusted / 1000.0;

    if (P_pv < 0) P_pv = 0;

    std::cout << "Potencia PV estimada: "
              << P_pv << " kW\n";

    // =====================================================
    // ENERGY MODEL
    // =====================================================

    double P_job = 2.0;

    model.update(P_job,
                 P_pv,
                 total_seconds);

    EnergyStats stats = model.getStats();

    std::cout << "\n============= RESULTADO =============\n";
    std::cout << "Energia Total: "
              << stats.E_total << " kWh\n";
    std::cout << "Energia PV: "
              << stats.E_pv << " kWh\n";
    std::cout << "Energia Grid: "
              << stats.E_grid << " kWh\n";
    std::cout << "Emissoes CO2: "
              << stats.CO2 << " gCO2\n";

    // =====================================================
    // CSV
    // =====================================================

    bool fileExists = std::filesystem::exists("results.csv");

    std::ofstream file("results.csv", std::ios::app);

   if (!fileExists)
    {
        file << "Hora_local;Dia_ano;"
                "Irradiancia_teorica;"
                "Irradiancia_ajustada;"
                "Temperatura;"
                "P_pv_inst;"
                "E_total;"
                "E_pv;"
                "E_grid;"
                "CO2\n";
    }

   file << hourInt << ":" << minuteInt << ";"
     << dayOfYear << ";"
     << G << ";"               
     << G_adjusted << ";"      
     << impact.temperature << ";"
     << P_pv << ";"
     << stats.E_total << ";"
     << stats.E_pv << ";"
     << stats.E_grid << ";"
     << stats.CO2 << "\n";
    std::cout << "\nDados salvos em results.csv\n";
}