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
// FUNÇÃO AUXILIAR SEGURA (COLE AQUI)
// =======================================

static double extractValue(const std::string& json,
                           const std::string& key)
{
    auto pos = json.find(key);
    if (pos == std::string::npos)
        return 0.0;

    auto start = json.find(":", pos);
    if (start == std::string::npos)
        return 0.0;

    start++;

    auto end = json.find(",", start);
    if (end == std::string::npos)
        end = json.find("}", start);

    std::string value =
        json.substr(start, end - start);

    try {
        return std::stod(value);
    }
    catch (...) {
        return 0.0;
    }
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
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                     WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,
                     &response);

    curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    WeatherImpact impact;
    impact.cloudFactor = 1.0;
    impact.rainFactor = 1.0;
    impact.tempFactor = 1.0;
    impact.windCoolingFactor = 1.0;

    double cloud = extractValue(response, "cloudcover");
    double temp  = extractValue(response, "temperature_2m");
    double rain  = extractValue(response, "rain");
    double wind  = extractValue(response, "windspeed_10m");

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
