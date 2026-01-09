// WeatherAPI.cpp - SIMPLIFIED VERSION
#include "weatherAPI.h"
#include <cmath>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <ctime>

using namespace std;

WeatherAPI::WeatherAPI(string& key) : apiKey(key), lastUpdateTime(0) {
    // Initialize with default values
    currentWeather.temperature = 20.0f;
    currentWeather.windspeed = 10.0f;
    currentWeather.alerts = "No alerts";
    
    // Seed random number generator for simulation
    srand(static_cast<unsigned int>(time(nullptr)));
}

void WeatherAPI::setLocation(const string& city) {
    cityName = city;
    // Reset last update time to force a fresh fetch
    lastUpdateTime = 0;
}

Weather WeatherAPI::getCurrentWeather() {
    return currentWeather;
}

void WeatherAPI::update() {
    time_t currentTime = time(nullptr);
    
    // Update every 30 seconds
    if (currentTime - lastUpdateTime >= 30) {
        simulateWeather();
        lastUpdateTime = currentTime;
    }
}

bool WeatherAPI::fetchFromAPI() {
    return false; // Use simulation
}

void WeatherAPI::parseResponse(string& jsonResponse) {
    // Not implemented in simplified version
}

void WeatherAPI::simulateWeather() {
    // Generate weather based on city name for consistent simulation
    
    // Create a hash from city name for deterministic "random" weather
    unsigned int cityHash = 0;
    for (char c : cityName) {
        cityHash = cityHash * 31 + static_cast<unsigned int>(c);
    }
    
    // Use the hash to generate consistent weather for each city
    float baseTemp = 15.0f + static_cast<float>(cityHash % 20); // 15-35°C
    float baseWind = 5.0f + static_cast<float>((cityHash / 100) % 25); // 5-30 km/h
    
    // Add some variation over time
    time_t now = time(nullptr);
    float timeVariation = sin(static_cast<float>(now) * 0.01f);
    
    // Calculate current weather
    currentWeather.temperature = baseTemp + 5.0f * timeVariation;
    currentWeather.windspeed = baseWind + 5.0f * sin(static_cast<float>(now) * 0.02f);
    
    // Ensure reasonable limits
    if (currentWeather.temperature < -10.0f) currentWeather.temperature = -10.0f;
    if (currentWeather.temperature > 40.0f) currentWeather.temperature = 40.0f;
    if (currentWeather.windspeed < 0.0f) currentWeather.windspeed = 0.0f;
    if (currentWeather.windspeed > 50.0f) currentWeather.windspeed = 50.0f;
    
    // Determine alerts based on conditions
    if (currentWeather.windspeed > 25.0f) {
        currentWeather.alerts = "High wind warning";
    } else if (currentWeather.temperature < 5.0f) {
        currentWeather.alerts = "Low temperature alert";
    } else if (currentWeather.temperature > 35.0f) {
        currentWeather.alerts = "Heat wave warning";
    } else {
        currentWeather.alerts = "No alerts";
    }
}