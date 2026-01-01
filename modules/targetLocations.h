// TargetLocations.h
#ifndef TARGETLOCATIONS_H
#define TARGETLOCATIONS_H

#include <string>
#include <vector>

using namespace std;

// Base structure - represents individual target locations
struct Base
{
    string name;           // Base name (e.g., "Base Alpha")
    float x, y;                 // Coordinates on map
    int priority;   // Critical points (higher = more important)

    vector <pair<int, float>> neighbors;

    void BaseDetails ();
};

// City structure - contains multiple bases
struct City
{
    string name;           // City name (e.g., "Karachi")
    vector<Base> bases;    // List of all bases in this city

    void DisplayBases();
};

class TargetLocations
{
private:
    vector<City> cities;   // All cities with their bases
    
public:
    // Constructor
    TargetLocations();
    
    // Add new city
    void addCity(string& cityName);
    
    // Add base to a specific city
    void addBase(string& cityName, Base& base);
    
    // Get all cities
    vector<City> getAllCities();
    
    // Get bases from specific city
    vector<Base> getBasesFromCity(string& cityName);
    
    // Get highest priority base from a city
    Base getHighestPriorityBase(string& cityName);
    
    // Load predefined targets (for initialization)
    void loadDefaultTargets();

    //Dijkstra's Algorithm
    vector<int> diskstra (string& cityName, int HomeBase, int TarBase);
};

#endif