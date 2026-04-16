#include "MetarSensor.hpp"

#include <curl/curl.h>

#include <cmath>
#include <iostream>
#include <sstream>
#include <string>

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

WeatherImpact MetarSensor::getWeatherImpact(double lat,
                                            double lon)
{
    // O nome da classe continuou o mesmo para nao mudar a cara do projeto,
    // mas aqui a consulta esta vindo da Open-Meteo.

    WeatherImpact impact;

    CURL* curl = curl_easy_init();
    if (curl == nullptr) {
        std::cout << "Nao consegui iniciar o CURL para consultar o clima. Vou seguir com clima padrao.\n";
        return impact;
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

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK || response.empty()) {
        std::cout << "Nao consegui consultar o clima atual. Vou seguir com valores padrao.\n";
        return impact;
    }

    impact.temperature = extractCurrentValue(response, "temperature_2m", impact.temperature);
    impact.cloudCover  = extractCurrentValue(response, "cloudcover", impact.cloudCover);
    impact.rainAmount  = extractCurrentValue(response, "precipitation", impact.rainAmount);
    impact.windSpeed   = extractCurrentValue(response, "windspeed_10m", impact.windSpeed);

    // Ajuste de nuvem: quanto mais nuvem, menos irradiancia util chega na placa.
    impact.cloudFactor = 1.0 - 0.75 * (impact.cloudCover / 100.0);
    if (impact.cloudFactor < 0.25)
        impact.cloudFactor = 0.25;

    // Ajuste de chuva: aqui eu mantive um modelo simples em degraus para ficar facil de ler.
    impact.rainFactor = 1.0;
    if (impact.rainAmount > 0.0 && impact.rainAmount <= 1.0)
        impact.rainFactor = 0.95;
    else if (impact.rainAmount > 1.0 && impact.rainAmount <= 5.0)
        impact.rainFactor = 0.85;
    else if (impact.rainAmount > 5.0)
        impact.rainFactor = 0.70;

    // Temperatura alta derruba a eficiencia do modulo.
    impact.tempFactor = 1.0;
    if (impact.temperature > 25.0)
    {
        double coef   = -0.0045;
        double deltaT = impact.temperature - 25.0;
        impact.tempFactor = 1.0 + coef * deltaT;

        if (impact.tempFactor < 0.85)
            impact.tempFactor = 0.85;
    }

    // Vento ajuda a resfriar a placa. Aqui o efeito foi deixado moderado.
    impact.windCoolingFactor = 1.0 + (impact.windSpeed * 0.0008);

    return impact;
}
