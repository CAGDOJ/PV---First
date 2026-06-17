#include "MetarSensor.hpp"

#include <curl/curl.h>

#include <chrono>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

static size_t WriteCallback(void* contents,
                            size_t size,
                            size_t nmemb,
                            std::string* output)
{
    size_t total = size * nmemb;
    output->append(static_cast<char*>(contents), total);
    return total;
}

static double extractCurrentValue(const std::string& json,
                                  const std::string& key,
                                  double defaultValue)
{
    auto currentPos = json.find("\"current\"");
    if (currentPos == std::string::npos)
        return defaultValue;

    auto keyPos = json.find(key, currentPos);
    if (keyPos == std::string::npos)
        return defaultValue;

    auto start = json.find(":", keyPos);
    if (start == std::string::npos)
        return defaultValue;

    start++;
    auto end = json.find_first_of(",}", start);

    try {
        return std::stod(json.substr(start, end - start));
    }
    catch (...) {
        return defaultValue;
    }
}

static bool hasValidWeatherPayload(const std::string& response)
{
    return response.find("\"current\"") != std::string::npos &&
           response.find("temperature_2m") != std::string::npos;
}

WeatherImpact MetarSensor::getWeatherImpact(double lat,
                                            double lon)
{
    WeatherImpact impact;

    while (true)
    {
        CURL* curl = curl_easy_init();
        if (curl == nullptr) {
            std::cout << "Nao consegui iniciar o CURL para consultar o clima. Vou tentar novamente em 10 segundos.\n";
            std::this_thread::sleep_for(std::chrono::seconds(10));
            continue;
        }

        std::string response;
        std::stringstream url;
        url << "https://api.open-meteo.com/v1/forecast?"
            << "latitude=" << lat
            << "&longitude=" << lon
            << "&current=temperature_2m,cloudcover,precipitation,windspeed_10m";

        curl_easy_setopt(curl, CURLOPT_URL, url.str().c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

        CURLcode res = curl_easy_perform(curl);

        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

        curl_easy_cleanup(curl);

        if (res == CURLE_OK && httpCode >= 200 && httpCode < 300 && hasValidWeatherPayload(response)) {
            impact.temperature = extractCurrentValue(response, "temperature_2m", impact.temperature);
            impact.cloudCover  = extractCurrentValue(response, "cloudcover", impact.cloudCover);
            impact.rainAmount  = extractCurrentValue(response, "precipitation", impact.rainAmount);
            impact.windSpeed   = extractCurrentValue(response, "windspeed_10m", impact.windSpeed);

            impact.cloudFactor = 1.0 - 0.75 * (impact.cloudCover / 100.0);
            if (impact.cloudFactor < 0.25)
                impact.cloudFactor = 0.25;

            impact.rainFactor = 1.0;
            if (impact.rainAmount > 0.0 && impact.rainAmount <= 1.0)
                impact.rainFactor = 0.95;
            else if (impact.rainAmount > 1.0 && impact.rainAmount <= 5.0)
                impact.rainFactor = 0.85;
            else if (impact.rainAmount > 5.0)
                impact.rainFactor = 0.70;

            impact.tempFactor = 1.0;
            if (impact.temperature > 25.0)
            {
                double coef   = -0.0045;
                double deltaT = impact.temperature - 25.0;
                impact.tempFactor = 1.0 + coef * deltaT;

                if (impact.tempFactor < 0.85)
                    impact.tempFactor = 0.85;
            }

            impact.windCoolingFactor = 1.0 + (impact.windSpeed * 0.0008);

            return impact;
        }

        std::cout << "Sem conectividade valida para consulta meteorologica. Vou tentar novamente em 10 segundos.\n";
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}