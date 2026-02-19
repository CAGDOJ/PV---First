#include "GeoSensor.hpp"
#include <curl/curl.h>
#include <string>
#include <iostream>

static size_t WriteCallback(void* contents,
                            size_t size,
                            size_t nmemb,
                            std::string* output)
{
    size_t total = size * nmemb;
    output->append((char*)contents, total);
    return total;
}

GPSData GeoSensor::getLocation()
{
    CURL* curl = curl_easy_init();
    std::string response;

    curl_easy_setopt(curl, CURLOPT_URL,
                     "http://ip-api.com/json/");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                     WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,
                     &response);

    curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    GPSData gps;

    auto latPos = response.find("\"lat\":");
    auto lonPos = response.find("\"lon\":");
    auto cityPos = response.find("\"city\":");

    gps.latitude =
        std::stod(response.substr(latPos + 6));

    gps.longitude =
        std::stod(response.substr(lonPos + 6));

    auto cityStart =
        response.find("\"", cityPos + 7) + 1;
    auto cityEnd =
        response.find("\"", cityStart);

    gps.city =
        response.substr(cityStart,
                        cityEnd - cityStart);

    return gps;
}
