#include "GeoSensor.hpp"

#include <curl/curl.h>

#include <chrono>
#include <iostream>
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

static bool hasValidLocationPayload(const std::string& response)
{
    return response.find("\"lat\"") != std::string::npos &&
           response.find("\"lon\"") != std::string::npos;
}

GPSData GeoSensor::getLocation()
{
    GPSData gps;

    while (true)
    {
        CURL* curl = curl_easy_init();
        if (curl == nullptr) {
            std::cout << "Nao consegui iniciar o CURL para geolocalizacao. Vou tentar novamente em 10 segundos.\n";
            std::this_thread::sleep_for(std::chrono::seconds(10));
            continue;
        }

        std::string response;

        curl_easy_setopt(curl, CURLOPT_URL, "http://ip-api.com/json/");
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

        if (res == CURLE_OK && httpCode >= 200 && httpCode < 300 && hasValidLocationPayload(response)) {
            gps.latitude  = extractNumber(response, "\"lat\"", gps.latitude);
            gps.longitude = extractNumber(response, "\"lon\"", gps.longitude);
            gps.city      = extractText(response, "\"city\":\"", gps.city);
            return gps;
        }

        std::cout << "Sem conectividade valida para geolocalizacao. Vou tentar novamente em 10 segundos.\n";
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}