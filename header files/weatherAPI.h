// WeatherAPI.h
#ifndef WEATHERAPI_H
#define WEATHERAPI_H

#include<string>
#include<ctime>

using namespace std;

// Weather data structure for sidebar display
struct Weather{
    float temperature;  // in Celsius (will be displayed directly)
    float windspeed;    // in km/h (used for drone health calculation)
    string alerts; // stored but not displayed
};

class WeatherAPI{
private:
    string apiKey;
    float latitude;
    float longitude;
    time_t lastUpdateTime;
    Weather currentWeather;
    
    // Helper functions
    bool fetchFromAPI();
    void parseResponse(string& jsonResponse);
    void simulateWeather();  // fallback for testing
    
public:
    // Constructor
    WeatherAPI(string& key);
    
    // Set drone location
    void setLocation(float lat, float lon);
    
    // Get current weather data (for sidebar display)
    Weather getCurrentWeather();
    
    // Update weather (call every frame, internally checks 5 sec interval)
    void update();
};

#endif