#include "targetLocations.h"
#include <iostream>
#include <algorithm>
#include <limits>
#include <queue>

using namespace std;

// Base member functions
void Base::BaseDetails()
{
    cout << "Base: " << name << endl;
    cout << "Location: (" << x << ", " << y << ")" << endl;
    cout << "Priority: " << priority << endl;
    cout << "Neighbors: ";
    for (auto& n : neighbors) {
        cout << n.first << "(fuel:" << n.second << ") ";
    }
    cout << endl;
}

// City member functions
void City::DisplayBases()
{
    cout << "\n=== " << name << " Bases ===" << endl;
    for (size_t i = 0; i < bases.size(); i++) {
        cout << i << ". " << bases[i].name 
             << " - Priority: " << bases[i].priority 
             << " - Pos: (" << bases[i].x << ", " << bases[i].y << ")" << endl;
    }
}

// TargetLocations constructor
TargetLocations::TargetLocations()
{
    loadDefaultTargets();
}

// Add new city
void TargetLocations::addCity(string& cityName)
{
    City newCity;
    newCity.name = cityName;
    cities.push_back(newCity);
}

// Add base to a specific city
void TargetLocations::addBase(string& cityName, Base& base)
{
    for (size_t i = 0; i < cities.size(); i++) {
        if (cities[i].name == cityName) {
            cities[i].bases.push_back(base);
            return;
        }
    }
}

// Get all cities
vector<City> TargetLocations::getAllCities()
{
    return cities;
}

// Get bases from specific city
vector<Base> TargetLocations::getBasesFromCity(string& cityName)
{
    for (size_t i = 0; i < cities.size(); i++) {
        if (cities[i].name == cityName) {
            return cities[i].bases;
        }
    }
    return vector<Base>();
}

// Get highest priority base from a city
Base TargetLocations::getHighestPriorityBase(string& cityName)
{
    vector<Base> bases = getBasesFromCity(cityName);
    if (bases.empty()) {
        return Base();
    }
    
    Base highest = bases[0];
    for (size_t i = 1; i < bases.size(); i++) {
        if (bases[i].priority > highest.priority) {
            highest = bases[i];
        }
    }
    return highest;
}

// Dijkstra's Algorithm
vector<int> TargetLocations::diskstra(string& cityName, int HomeBase, int TarBase)
{
    // Get bases from city
    vector<Base> bases = getBasesFromCity(cityName);
    if (bases.empty() || HomeBase >= (int)bases.size() || TarBase >= (int)bases.size()) {
        return vector<int>();
    }
    
    int n = bases.size();
    vector<float> dist(n, numeric_limits<float>::max());
    vector<int> parent(n, -1);
    vector<bool> visited(n, false);
    
    // Priority queue: pair<distance, node>
    priority_queue<pair<float, int>, vector<pair<float, int>>, greater<pair<float, int>>> pq;
    
    dist[HomeBase] = 0;
    pq.push({0, HomeBase});
    
    while (!pq.empty()) {
        int u = pq.top().second;
        pq.pop();
        
        if (visited[u]) continue;
        visited[u] = true;
        
        // Check all neighbors
        for (auto& neighbor : bases[u].neighbors) {
            int v = neighbor.first;
            float weight = neighbor.second;
            
            if (dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight;
                parent[v] = u;
                pq.push({dist[v], v});
            }
        }
    }
    
    // Reconstruct path
    vector<int> path;
    if (dist[TarBase] == numeric_limits<float>::max()) {
        return path; // No path found
    }
    
    int current = TarBase;
    while (current != -1) {
        path.push_back(current);
        current = parent[current];
    }
    
    reverse(path.begin(), path.end());
    return path;
}

// Load predefined targets
void TargetLocations::loadDefaultTargets()
{
    // City 1: Karachi
    string city1 = "Karachi";
    addCity(city1);
    
    Base k1 = {"Karachi Naval Base", 10.0f, 15.0f, 5, {}};
    Base k2 = {"Karachi Port", 25.0f, 8.0f, 3, {}};
    Base k3 = {"Karachi Airbase", 18.0f, 22.0f, 4, {}};
    Base k4 = {"Karachi Command", 5.0f, 30.0f, 2, {}};
    Base k5 = {"Home Base", 5.0f, 5.0f, 0, {}};
    
    // Setup neighbors (graph edges with fuel weights)
    k5.neighbors = {{0, 8.0f}, {1, 20.0f}, {2, 15.0f}, {3, 25.0f}};  // Home to all
    k1.neighbors = {{4, 8.0f}, {1, 12.0f}, {2, 10.0f}, {3, 18.0f}};
    k2.neighbors = {{4, 20.0f}, {0, 12.0f}, {2, 15.0f}, {3, 25.0f}};
    k3.neighbors = {{4, 15.0f}, {0, 10.0f}, {1, 15.0f}, {3, 12.0f}};
    k4.neighbors = {{4, 25.0f}, {0, 18.0f}, {1, 25.0f}, {2, 12.0f}};
    
    addBase(city1, k1);
    addBase(city1, k2);
    addBase(city1, k3);
    addBase(city1, k4);
    addBase(city1, k5);
    
    // City 2: Lahore
    string city2 = "Lahore";
    addCity(city2);
    
    Base l1 = {"Lahore Garrison", 12.0f, 18.0f, 4, {}};
    Base l2 = {"Lahore Cantonment", 28.0f, 10.0f, 5, {}};
    Base l3 = {"Lahore Arsenal", 20.0f, 25.0f, 3, {}};
    Base l4 = {"Lahore Depot", 8.0f, 28.0f, 2, {}};
    Base l5 = {"Home Base", 5.0f, 5.0f, 0, {}};
    
    l5.neighbors = {{0, 10.0f}, {1, 22.0f}, {2, 18.0f}, {3, 24.0f}};
    l1.neighbors = {{4, 10.0f}, {1, 14.0f}, {2, 12.0f}, {3, 16.0f}};
    l2.neighbors = {{4, 22.0f}, {0, 14.0f}, {2, 16.0f}, {3, 28.0f}};
    l3.neighbors = {{4, 18.0f}, {0, 12.0f}, {1, 16.0f}, {3, 10.0f}};
    l4.neighbors = {{4, 24.0f}, {0, 16.0f}, {1, 28.0f}, {2, 10.0f}};
    
    addBase(city2, l1);
    addBase(city2, l2);
    addBase(city2, l3);
    addBase(city2, l4);
    addBase(city2, l5);
    
    // City 3: Islamabad
    string city3 = "Islamabad";
    addCity(city3);
    
    Base i1 = {"ISB Command Center", 15.0f, 12.0f, 5, {}};
    Base i2 = {"ISB Strategic Base", 22.0f, 20.0f, 4, {}};
    Base i3 = {"ISB Defense HQ", 30.0f, 8.0f, 3, {}};
    Base i4 = {"ISB Reserve Base", 10.0f, 25.0f, 2, {}};
    Base i5 = {"Home Base", 5.0f, 5.0f, 0, {}};
    
    i5.neighbors = {{0, 12.0f}, {1, 20.0f}, {2, 28.0f}, {3, 22.0f}};
    i1.neighbors = {{4, 12.0f}, {1, 10.0f}, {2, 18.0f}, {3, 14.0f}};
    i2.neighbors = {{4, 20.0f}, {0, 10.0f}, {2, 15.0f}, {3, 12.0f}};
    i3.neighbors = {{4, 28.0f}, {0, 18.0f}, {1, 15.0f}, {3, 25.0f}};
    i4.neighbors = {{4, 22.0f}, {0, 14.0f}, {1, 12.0f}, {2, 25.0f}};
    
    addBase(city3, i1);
    addBase(city3, i2);
    addBase(city3, i3);
    addBase(city3, i4);
    addBase(city3, i5);
    
    // City 4: Peshawar
    string city4 = "Peshawar";
    addCity(city4);
    
    Base p1 = {"Peshawar Fort", 18.0f, 16.0f, 3, {}};
    Base p2 = {"Peshawar Airfield", 26.0f, 12.0f, 5, {}};
    Base p3 = {"Peshawar Outpost", 14.0f, 28.0f, 4, {}};
    Base p4 = {"Peshawar Supply", 8.0f, 32.0f, 2, {}};
    Base p5 = {"Home Base", 5.0f, 5.0f, 0, {}};
    
    p5.neighbors = {{0, 15.0f}, {1, 25.0f}, {2, 24.0f}, {3, 30.0f}};
    p1.neighbors = {{4, 15.0f}, {1, 12.0f}, {2, 14.0f}, {3, 20.0f}};
    p2.neighbors = {{4, 25.0f}, {0, 12.0f}, {2, 18.0f}, {3, 26.0f}};
    p3.neighbors = {{4, 24.0f}, {0, 14.0f}, {1, 18.0f}, {3, 8.0f}};
    p4.neighbors = {{4, 30.0f}, {0, 20.0f}, {1, 26.0f}, {2, 8.0f}};
    
    addBase(city4, p1);
    addBase(city4, p2);
    addBase(city4, p3);
    addBase(city4, p4);
    addBase(city4, p5);
    
    // City 5: Quetta
    string city5 = "Quetta";
    addCity(city5);
    
    Base q1 = {"Quetta Base Alpha", 20.0f, 14.0f, 4, {}};
    Base q2 = {"Quetta Base Beta", 28.0f, 18.0f, 3, {}};
    Base q3 = {"Quetta Base Gamma", 16.0f, 26.0f, 5, {}};
    Base q4 = {"Quetta Base Delta", 12.0f, 30.0f, 2, {}};
    Base q5 = {"Home Base", 5.0f, 5.0f, 0, {}};
    
    q5.neighbors = {{0, 18.0f}, {1, 26.0f}, {2, 22.0f}, {3, 28.0f}};
    q1.neighbors = {{4, 18.0f}, {1, 14.0f}, {2, 12.0f}, {3, 16.0f}};
    q2.neighbors = {{4, 26.0f}, {0, 14.0f}, {2, 16.0f}, {3, 20.0f}};
    q3.neighbors = {{4, 22.0f}, {0, 12.0f}, {1, 16.0f}, {3, 10.0f}};
    q4.neighbors = {{4, 28.0f}, {0, 16.0f}, {1, 20.0f}, {2, 10.0f}};
    
    addBase(city5, q1);
    addBase(city5, q2);
    addBase(city5, q3);
    addBase(city5, q4);
    addBase(city5, q5);
}