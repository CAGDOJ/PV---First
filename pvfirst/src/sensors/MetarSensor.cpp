#include "MetarSensor.hpp"
#include <curl/curl.h>
#include <string>
#include <iostream>
#include <sstream>

// =======================================
// CALLBACK DO CURL
// =======================================

static size_t WriteCallback(void* contents,
                            size_t size,
                            size_t nmemb,
                            std::string* output)
{
    size_t total = size * nmemb;
    output->append((char*)contents, total);
    return total;
}

// =======================================
// FUNÇÃO PRINCIPAL
// =======================================

WeatherImpact MetarSensor::getWeatherImpact(double lat,
                                            double lon)
{
    CURL* curl = curl_easy_init();
    std::string response;

    std::stringstream url;
    url << "https://api.open-meteo.com/v1/forecast?"
        << "latitude=" << lat
        << "&longitude=" << lon
        << "&current_weather=true"
        << "&hourly=cloudcover,temperature_2m,rain,windspeed_10m";

    curl_easy_setopt(curl, CURLOPT_URL, url.str().c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    WeatherImpact impact;

    // Inicialização padrão
    impact.cloudFactor = 1.0;
    impact.rainFactor = 1.0;
    impact.tempFactor = 1.0;
    impact.windCoolingFactor = 1.0;

    impact.temperature = 0.0;
    impact.cloudCover = 0.0;
    impact.rainAmount = 0.0;
    impact.windSpeed = 0.0;

    // =======================================
    // 🔹 EXTRAÇÃO SEGURA DO current_weather
    // =======================================

    auto cw_pos = response.find("\"current_weather\"");
    double temp = 0.0;
    double wind = 0.0;

    if (cw_pos != std::string::npos)
    {
        auto temp_pos = response.find("\"temperature\":", cw_pos);
        if (temp_pos != std::string::npos)
        {
            temp_pos += 14;
            auto end = response.find(",", temp_pos);
            temp = std::stod(response.substr(temp_pos, end - temp_pos));
        }

        auto wind_pos = response.find("\"windspeed\":", cw_pos);
        if (wind_pos != std::string::npos)
        {
            wind_pos += 12;
            auto end = response.find(",", wind_pos);
            wind = std::stod(response.substr(wind_pos, end - wind_pos));
        }
    }

    // =======================================
    // 🔹 EXTRAÇÃO DO HOURLY (primeiro valor)
    // =======================================

    double cloud = 0.0;
    double rain = 0.0;

    auto cloud_pos = response.find("\"cloudcover\":[");
    if (cloud_pos != std::string::npos)
    {
        cloud_pos += 14;
        auto end = response.find(",", cloud_pos);
        cloud = std::stod(response.substr(cloud_pos, end - cloud_pos));
    }

    auto rain_pos = response.find("\"rain\":[");
    if (rain_pos != std::string::npos)
    {
        rain_pos += 8;
        auto end = response.find(",", rain_pos);
        rain = std::stod(response.substr(rain_pos, end - rain_pos));
    }

    // =======================================
    // 🔹 SALVAR VALORES REAIS
    // =======================================

    impact.temperature = temp;
    impact.cloudCover = cloud;
    impact.rainAmount = rain;
    impact.windSpeed = wind;

    // =======================================
    // 🔹 CÁLCULO DOS FATORES
    // =======================================

    impact.cloudFactor = 1.0 - (cloud / 100.0);

    if (rain > 0.0)
        impact.rainFactor = 0.3;

    if (temp > 25.0)
    {
        double loss = (temp - 25.0) * 0.004;
        impact.tempFactor = 1.0 - loss;
    }

    impact.windCoolingFactor = 1.0 + (wind * 0.001);

    return impact;
}
