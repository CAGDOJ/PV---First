#include "MetarSensor.hpp"
#include <curl/curl.h>
#include <string>
#include <sstream>
#include <iostream>
#include <cmath>

// =====================================================
// CALLBACK DO CURL
// =====================================================

static size_t WriteCallback(void* contents,
                            size_t size,
                            size_t nmemb,
                            std::string* output)
{
    size_t total = size * nmemb;
    output->append((char*)contents, total);
    return total;
}

// =====================================================
// EXTRAÇÃO SEGURA DENTRO DO BLOCO "current"
// =====================================================

static double extractCurrentValue(const std::string& json,
                                  const std::string& key)
{
    auto currentPos = json.find("\"current\"");
    if (currentPos == std::string::npos)
        return 0.0;

    auto keyPos = json.find(key, currentPos);
    if (keyPos == std::string::npos)
        return 0.0;

    auto start = json.find(":", keyPos);
    if (start == std::string::npos)
        return 0.0;

    start++;

    auto end = json.find_first_of(",}", start);
    std::string value = json.substr(start, end - start);

    try {
        return std::stod(value);
    }
    catch (...) {
        return 0.0;
    }
}

// =====================================================
// FUNÇÃO PRINCIPAL
// =====================================================

WeatherImpact MetarSensor::getWeatherImpact(double lat,
                                            double lon)
{
    CURL* curl = curl_easy_init();
    std::string response;

    std::stringstream url;
    url << "https://api.open-meteo.com/v1/forecast?"
        << "latitude=" << lat
        << "&longitude=" << lon
        << "&current=temperature_2m,cloudcover,precipitation,windspeed_10m";

    curl_easy_setopt(curl, CURLOPT_URL, url.str().c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    WeatherImpact impact;

    // =====================================================
    // VALORES REAIS DA API
    // =====================================================

    impact.temperature = extractCurrentValue(response, "temperature_2m");
    impact.cloudCover  = extractCurrentValue(response, "cloudcover");
    impact.rainAmount  = extractCurrentValue(response, "precipitation");
    impact.windSpeed   = extractCurrentValue(response, "windspeed_10m");

    // =====================================================
    // MODELO DE NUVEM (REALISTA)
    // =====================================================

    impact.cloudFactor =
        1.0 - 0.75 * (impact.cloudCover / 100.0);

    if (impact.cloudFactor < 0.25)
        impact.cloudFactor = 0.25;

    // =====================================================
    // MODELO DE CHUVA (AJUSTADO E PROPORCIONAL)
    // =====================================================

    impact.rainFactor = 1.0;

    if (impact.rainAmount > 0.0 && impact.rainAmount <= 1.0)
        impact.rainFactor = 0.95;      // 5% perda
    else if (impact.rainAmount > 1.0 && impact.rainAmount <= 5.0)
        impact.rainFactor = 0.85;      // 15% perda
    else if (impact.rainAmount > 5.0)
        impact.rainFactor = 0.70;      // 30% perda

    // =====================================================
    // MODELO TÉRMICO REAL
    // -0.45% por °C acima de 25°C
    // =====================================================

    impact.tempFactor = 1.0;

    if (impact.temperature > 25.0)
    {
        double coef = -0.0045;
        double deltaT = impact.temperature - 25.0;

        impact.tempFactor = 1.0 + coef * deltaT;

        if (impact.tempFactor < 0.85)
            impact.tempFactor = 0.85;
    }

    // =====================================================
    // RESFRIAMENTO POR VENTO
    // =====================================================

    impact.windCoolingFactor =
        1.0 + (impact.windSpeed * 0.0008);

    return impact;
}