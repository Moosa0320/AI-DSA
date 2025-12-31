#ifndef WEATHERAPI_H_INCLUDED
#define WEATHERAPI_H_INCLUDED

#include<ctime>
#include<string>
#include<cstlib>

using namespace std;

// Weather Data Model
struct Weather{
    float temperature, windspeed;
    string alerts;

};

// Weather Service Class
class WeatherAPI{
public:
    WeatherAPI(string& apiKey); // Constructor

    // Set target location dynamically
    Weather setLocation(float latitude, float longitude);

    // Returns the latest weather data
    void getCurrent Weather();

    // Call periodically to refresh weather
    void update();
private:

    Weather currentWeather;  // Current weather snapshot
    time_t lastUpdateTime;  // Last update timestamp

    float longitude, latitude;  // Location coordinates
    string apiKey;

    bool fetchFromAPI();  // Fetch weather from real API

    void simulateWeather(); // Fallback simulation if API fails

};

#endif // WEATHERAPI_H_INCLUDED
