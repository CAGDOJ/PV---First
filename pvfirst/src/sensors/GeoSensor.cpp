#include "GeoSensor.hpp"

#include <curl/curl.h>

#include <iostream>
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

static double extractNumber(const std::string& json, const std::string& key, double defaultValue)
{
    auto keyPos = json.find(key);
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

static std::string extractText(const std::string& json, const std::string& keyPrefix, const std::string& defaultValue)
{
    auto keyPos = json.find(keyPrefix);
    if (keyPos == std::string::npos)
        return defaultValue;

    auto start = keyPos + keyPrefix.size();
    auto end = json.find('"', start);
    if (end == std::string::npos)
        return defaultValue;

    return json.substr(start, end - start);
}

GPSData GeoSensor::getLocation()
{
    // Se a API falhar, eu prefiro continuar a simulacao com um fallback conhecido.
    // Para nao parar o experimento inteiro, o default ficou em Belem.

    GPSData gps;

    CURL* curl = curl_easy_init();
    if (curl == nullptr) {
        std::cout << "Nao consegui iniciar o CURL para geolocalizacao. Vou usar Belem como fallback.\n";
        return gps;
    }

    std::string response;

    curl_easy_setopt(curl, CURLOPT_URL, "http://ip-api.com/json/");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK || response.empty()) {
        std::cout << "Nao consegui detectar a localizacao automaticamente. Vou usar Belem como fallback.\n";
        return gps;
    }

    gps.latitude  = extractNumber(response, "\"lat\"", gps.latitude);
    gps.longitude = extractNumber(response, "\"lon\"", gps.longitude);
    gps.city      = extractText(response, "\"city\":\"", gps.city);

    return gps;
}
