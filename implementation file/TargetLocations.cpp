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
    // City 1: Karachi - 8 bases
    string city1 = "Karachi";
    addCity(city1);
    
    Base k1 = {"Karachi Naval Base", 10.0f, 15.0f, 5, {}};
    Base k2 = {"Karachi Port", 25.0f, 8.0f, 3, {}};
    Base k3 = {"Karachi Airbase", 18.0f, 22.0f, 4, {}};
    Base k4 = {"Karachi Command", 5.0f, 30.0f, 2, {}};
    Base k5 = {"Karachi Industrial", 32.0f, 18.0f, 4, {}};
    Base k6 = {"Karachi Defense HQ", 15.0f, 28.0f, 5, {}};
    Base k7 = {"Karachi Radar Station", 28.0f, 25.0f, 3, {}};
    Base k8 = {"Home Base", 5.0f, 5.0f, 0, {}};
    
    // Setup neighbors (graph edges with fuel weights)
    k8.neighbors = {{0, 8.0f}, {1, 20.0f}, {2, 15.0f}, {3, 25.0f}, {4, 28.0f}, {5, 24.0f}, {6, 30.0f}};
    k1.neighbors = {{7, 8.0f}, {1, 12.0f}, {2, 10.0f}, {3, 18.0f}, {4, 20.0f}, {5, 15.0f}, {6, 22.0f}};
    k2.neighbors = {{7, 20.0f}, {0, 12.0f}, {2, 15.0f}, {3, 25.0f}, {4, 10.0f}, {5, 22.0f}, {6, 14.0f}};
    k3.neighbors = {{7, 15.0f}, {0, 10.0f}, {1, 15.0f}, {3, 12.0f}, {4, 18.0f}, {5, 8.0f}, {6, 16.0f}};
    k4.neighbors = {{7, 25.0f}, {0, 18.0f}, {1, 25.0f}, {2, 12.0f}, {4, 28.0f}, {5, 6.0f}, {6, 20.0f}};
    k5.neighbors = {{7, 28.0f}, {0, 20.0f}, {1, 10.0f}, {2, 18.0f}, {3, 28.0f}, {5, 16.0f}, {6, 8.0f}};
    k6.neighbors = {{7, 24.0f}, {0, 15.0f}, {1, 22.0f}, {2, 8.0f}, {3, 6.0f}, {4, 16.0f}, {6, 12.0f}};
    k7.neighbors = {{7, 30.0f}, {0, 22.0f}, {1, 14.0f}, {2, 16.0f}, {3, 20.0f}, {4, 8.0f}, {5, 12.0f}};
    
    addBase(city1, k1);
    addBase(city1, k2);
    addBase(city1, k3);
    addBase(city1, k4);
    addBase(city1, k5);
    addBase(city1, k6);
    addBase(city1, k7);
    addBase(city1, k8);
    
    // City 2: Lahore - 6 bases
    string city2 = "Lahore";
    addCity(city2);
    
    Base l1 = {"Lahore Garrison", 12.0f, 18.0f, 4, {}};
    Base l2 = {"Lahore Cantonment", 28.0f, 10.0f, 5, {}};
    Base l3 = {"Lahore Arsenal", 20.0f, 25.0f, 3, {}};
    Base l4 = {"Lahore Depot", 8.0f, 28.0f, 2, {}};
    Base l5 = {"Lahore Military Academy", 30.0f, 22.0f, 4, {}};
    Base l6 = {"Home Base", 5.0f, 5.0f, 0, {}};
    
    l6.neighbors = {{0, 10.0f}, {1, 22.0f}, {2, 18.0f}, {3, 24.0f}, {4, 28.0f}};
    l1.neighbors = {{5, 10.0f}, {1, 14.0f}, {2, 12.0f}, {3, 16.0f}, {4, 20.0f}};
    l2.neighbors = {{5, 22.0f}, {0, 14.0f}, {2, 16.0f}, {3, 28.0f}, {4, 8.0f}};
    l3.neighbors = {{5, 18.0f}, {0, 12.0f}, {1, 16.0f}, {3, 10.0f}, {4, 12.0f}};
    l4.neighbors = {{5, 24.0f}, {0, 16.0f}, {1, 28.0f}, {2, 10.0f}, {4, 18.0f}};
    l5.neighbors = {{5, 28.0f}, {0, 20.0f}, {1, 8.0f}, {2, 12.0f}, {3, 18.0f}};
    
    addBase(city2, l1);
    addBase(city2, l2);
    addBase(city2, l3);
    addBase(city2, l4);
    addBase(city2, l5);
    addBase(city2, l6);
    
    // City 3: Islamabad - 7 bases
    string city3 = "Islamabad";
    addCity(city3);
    
    Base i1 = {"ISB Command Center", 15.0f, 12.0f, 5, {}};
    Base i2 = {"ISB Strategic Base", 22.0f, 20.0f, 4, {}};
    Base i3 = {"ISB Defense HQ", 30.0f, 8.0f, 3, {}};
    Base i4 = {"ISB Reserve Base", 10.0f, 25.0f, 2, {}};
    Base i5 = {"ISB Intelligence Center", 26.0f, 28.0f, 5, {}};
    Base i6 = {"ISB Operations Base", 18.0f, 30.0f, 4, {}};
    Base i7 = {"Home Base", 5.0f, 5.0f, 0, {}};
    
    i7.neighbors = {{0, 12.0f}, {1, 20.0f}, {2, 28.0f}, {3, 22.0f}, {4, 30.0f}, {5, 26.0f}};
    i1.neighbors = {{6, 12.0f}, {1, 10.0f}, {2, 18.0f}, {3, 14.0f}, {4, 24.0f}, {5, 16.0f}};
    i2.neighbors = {{6, 20.0f}, {0, 10.0f}, {2, 15.0f}, {3, 12.0f}, {4, 12.0f}, {5, 14.0f}};
    i3.neighbors = {{6, 28.0f}, {0, 18.0f}, {1, 15.0f}, {3, 25.0f}, {4, 22.0f}, {5, 28.0f}};
    i4.neighbors = {{6, 22.0f}, {0, 14.0f}, {1, 12.0f}, {2, 25.0f}, {4, 10.0f}, {5, 8.0f}};
    i5.neighbors = {{6, 30.0f}, {0, 24.0f}, {1, 12.0f}, {2, 22.0f}, {3, 10.0f}, {5, 6.0f}};
    i6.neighbors = {{6, 26.0f}, {0, 16.0f}, {1, 14.0f}, {2, 28.0f}, {3, 8.0f}, {4, 6.0f}};
    
    addBase(city3, i1);
    addBase(city3, i2);
    addBase(city3, i3);
    addBase(city3, i4);
    addBase(city3, i5);
    addBase(city3, i6);
    addBase(city3, i7);
    
    // City 4: Peshawar - 9 bases
    string city4 = "Peshawar";
    addCity(city4);
    
    Base p1 = {"Peshawar Fort", 18.0f, 16.0f, 3, {}};
    Base p2 = {"Peshawar Airfield", 26.0f, 12.0f, 5, {}};
    Base p3 = {"Peshawar Outpost", 14.0f, 28.0f, 4, {}};
    Base p4 = {"Peshawar Supply", 8.0f, 32.0f, 2, {}};
    Base p5 = {"Peshawar Border Post", 32.0f, 20.0f, 4, {}};
    Base p6 = {"Peshawar Training Camp", 22.0f, 26.0f, 3, {}};
    Base p7 = {"Peshawar Communication Hub", 30.0f, 30.0f, 5, {}};
    Base p8 = {"Peshawar Logistics Center", 12.0f, 22.0f, 3, {}};
    Base p9 = {"Home Base", 5.0f, 5.0f, 0, {}};
    
    p9.neighbors = {{0, 15.0f}, {1, 25.0f}, {2, 24.0f}, {3, 30.0f}, {4, 30.0f}, {5, 26.0f}, {6, 32.0f}, {7, 20.0f}};
    p1.neighbors = {{8, 15.0f}, {1, 12.0f}, {2, 14.0f}, {3, 20.0f}, {4, 16.0f}, {5, 12.0f}, {6, 18.0f}, {7, 10.0f}};
    p2.neighbors = {{8, 25.0f}, {0, 12.0f}, {2, 18.0f}, {3, 26.0f}, {4, 10.0f}, {5, 16.0f}, {6, 20.0f}, {7, 18.0f}};
    p3.neighbors = {{8, 24.0f}, {0, 14.0f}, {1, 18.0f}, {3, 8.0f}, {4, 20.0f}, {5, 6.0f}, {6, 12.0f}, {7, 8.0f}};
    p4.neighbors = {{8, 30.0f}, {0, 20.0f}, {1, 26.0f}, {2, 8.0f}, {4, 28.0f}, {5, 10.0f}, {6, 6.0f}, {7, 14.0f}};
    p5.neighbors = {{8, 30.0f}, {0, 16.0f}, {1, 10.0f}, {2, 20.0f}, {3, 28.0f}, {5, 14.0f}, {6, 12.0f}, {7, 22.0f}};
    p6.neighbors = {{8, 26.0f}, {0, 12.0f}, {1, 16.0f}, {2, 6.0f}, {3, 10.0f}, {4, 14.0f}, {6, 10.0f}, {7, 12.0f}};
    p7.neighbors = {{8, 32.0f}, {0, 18.0f}, {1, 20.0f}, {2, 12.0f}, {3, 6.0f}, {4, 12.0f}, {5, 10.0f}, {7, 20.0f}};
    p8.neighbors = {{8, 20.0f}, {0, 10.0f}, {1, 18.0f}, {2, 8.0f}, {3, 14.0f}, {4, 22.0f}, {5, 12.0f}, {6, 20.0f}};
    
    addBase(city4, p1);
    addBase(city4, p2);
    addBase(city4, p3);
    addBase(city4, p4);
    addBase(city4, p5);
    addBase(city4, p6);
    addBase(city4, p7);
    addBase(city4, p8);
    addBase(city4, p9);
    
    // City 5: Quetta - 5 bases
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