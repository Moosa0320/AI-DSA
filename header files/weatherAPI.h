// weatherAPI.h
#ifndef WEATHERAPI_H
#define WEATHERAPI_H

#include <string>
#include <ctime>

using namespace std;

// Weather data structure for sidebar display
struct Weather {
    float temperature;  // in Celsius (will be displayed directly)
    float windspeed;    // in km/h (used for drone health calculation)
    string alerts; // stored but not displayed
};

class WeatherAPI {
private:
    string apiKey;
    string cityName;
    time_t lastUpdateTime;
    Weather currentWeather;
    
    // Helper functions
    bool fetchFromAPI();
    void parseResponse(string& jsonResponse);
    void simulateWeather();  // fallback for testing
    
public:
    // Constructor
    WeatherAPI(string& key);
    
    // Set drone location using city name
    void setLocation(const string& city);
    
    // Get current weather data (for sidebar display)
    Weather getCurrentWeather();
    
    // Update weather (call every frame, internally checks 30 sec interval)
    void update();
};

#endif