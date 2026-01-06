#include "targetLocations.h"
#include <iostream>
#include <algorithm>
#include <limits>
#include <queue>

using namespace std;

// ================= Base =================
void Base::BaseDetails()
{
    cout << "Base: " << name << endl;
    cout << "Location: (" << x << ", " << y << ")" << endl;
    cout << "Priority: " << priority << endl;
    cout << "Neighbors: ";
    for (auto& n : neighbors)
        cout << n.first << "(fuel:" << n.second << ") ";
    cout << endl;
}

// ================= City =================
void City::DisplayBases()
{
    cout << "\n=== " << name << " Bases ===" << endl;
    for (size_t i = 0; i < bases.size(); i++)
        cout << i << ". " << bases[i].name
             << " - Priority: " << bases[i].priority
             << " - Pos: (" << bases[i].x << ", " << bases[i].y << ")" << endl;
}

// ================= TargetLocations =================
TargetLocations::TargetLocations()
{
    loadDefaultTargets();
}

void TargetLocations::addCity(string& cityName)
{
    City c;
    c.name = cityName;
    cities.push_back(c);
}

void TargetLocations::addBase(string& cityName, Base& base)
{
    for (auto& c : cities)
        if (c.name == cityName)
            c.bases.push_back(base);
}

vector<City> TargetLocations::getAllCities() { return cities; }

vector<Base> TargetLocations::getBasesFromCity(string& cityName)
{
    for (auto& c : cities)
        if (c.name == cityName)
            return c.bases;
    return {};
}

Base TargetLocations::getHighestPriorityBase(string& cityName)
{
    auto b = getBasesFromCity(cityName);
    Base h = b[0];
    for (auto& x : b)
        if (x.priority > h.priority) h = x;
    return h;
}

// ================= Dijkstra =================
vector<int> TargetLocations::diskstra(string& cityName, int HomeBase, int TarBase)
{
    auto bases = getBasesFromCity(cityName);
    int n = bases.size();

    vector<float> dist(n, numeric_limits<float>::max());
    vector<int> parent(n, -1);
    vector<bool> visited(n, false);

    priority_queue<pair<float,int>, vector<pair<float,int>>, greater<pair<float,int>>> pq;
    dist[HomeBase] = 0;
    pq.push({0, HomeBase});

    while (!pq.empty()) {
        int u = pq.top().second; pq.pop();
        if (visited[u]) continue;
        visited[u] = true;

        for (auto& nb : bases[u].neighbors) {
            if (dist[u] + nb.second < dist[nb.first]) {
                dist[nb.first] = dist[u] + nb.second;
                parent[nb.first] = u;
                pq.push({dist[nb.first], nb.first});
            }
        }
    }

    vector<int> path;
    for (int v = TarBase; v != -1; v = parent[v])
        path.push_back(v);
    reverse(path.begin(), path.end());
    return path;
}

// ================= LOAD TARGETS =================
void TargetLocations::loadDefaultTargets()
{
    // ================= Karachi =================
    string c1 = "Karachi"; addCity(c1);
    Base k1={"Karachi Naval Base",7,8,5,{}};
    Base k2={"Karachi Port",9,6,3,{}};
    Base k3={"Karachi Airbase",14,12,4,{}};
    Base k4={"Karachi Command",26,24,2,{}};
    Base k5={"Karachi Industrial",30,20,4,{}};
    Base k6={"Karachi Defense HQ",18,16,5,{}};
    Base k7={"Karachi Radar Station",28,26,3,{}};
    Base k8={"Home Base",5,5,0,{}};

    k8.neighbors={{0,4},{1,5},{2,8},{3,22},{4,25},{5,12},{6,28}};
    k1.neighbors={{7,4},{2,6}};
    k2.neighbors={{7,5},{0,6}};
    k3.neighbors={{7,8},{5,6}};
    k4.neighbors={{7,22},{6,8}};
    k5.neighbors={{7,25},{6,6}};
    k6.neighbors={{7,12}};
    k7.neighbors={{7,28}};

    addBase(c1,k1);addBase(c1,k2);addBase(c1,k3);addBase(c1,k4);
    addBase(c1,k5);addBase(c1,k6);addBase(c1,k7);addBase(c1,k8);

    // ================= Lahore =================
    string c2="Lahore"; addCity(c2);
    Base l1={"Lahore Garrison",7,9,4,{}};
    Base l2={"Lahore Cantonment",10,8,5,{}};
    Base l3={"Lahore Arsenal",15,14,3,{}};
    Base l4={"Lahore Depot",26,24,2,{}};
    Base l5={"Lahore Military Academy",30,22,4,{}};
    Base l6={"Home Base",5,5,0,{}};

    l6.neighbors={{0,4},{1,6},{2,10},{3,22},{4,26}};
    l1.neighbors={{5,4}};
    l2.neighbors={{5,6}};
    l3.neighbors={{5,10}};
    l4.neighbors={{5,22}};
    l5.neighbors={{5,26}};

    addBase(c2,l1);addBase(c2,l2);addBase(c2,l3);
    addBase(c2,l4);addBase(c2,l5);addBase(c2,l6);

    // ================= Islamabad =================
    string c3="Islamabad"; addCity(c3);
    Base i1={"ISB Command Center",7,7,5,{}};
    Base i2={"ISB Strategic Base",9,10,4,{}};
    Base i3={"ISB Defense HQ",14,12,3,{}};
    Base i4={"ISB Reserve Base",22,24,2,{}};
    Base i5={"ISB Intelligence Center",26,28,5,{}};
    Base i6={"ISB Operations Base",30,30,4,{}};
    Base i7={"Home Base",5,5,0,{}};

    i7.neighbors={{0,3},{1,5},{2,8},{3,18},{4,24},{5,28}};
    i1.neighbors={{6,3}};
    i2.neighbors={{6,5}};
    i3.neighbors={{6,8}};
    i4.neighbors={{6,18}};
    i5.neighbors={{6,24}};
    i6.neighbors={{6,28}};

    addBase(c3,i1);addBase(c3,i2);addBase(c3,i3);
    addBase(c3,i4);addBase(c3,i5);addBase(c3,i6);addBase(c3,i7);

    // ================= Peshawar =================
    string c4="Peshawar"; addCity(c4);
    Base p1={"Peshawar Fort",7,8,3,{}};
    Base p2={"Peshawar Airfield",10,9,5,{}};
    Base p3={"Peshawar Outpost",14,12,4,{}};
    Base p4={"Peshawar Supply",26,26,2,{}};
    Base p5={"Peshawar Border Post",30,20,4,{}};
    Base p6={"Peshawar Training Camp",18,16,3,{}};
    Base p7={"Peshawar Communication Hub",28,28,5,{}};
    Base p8={"Peshawar Logistics Center",12,14,3,{}};
    Base p9={"Home Base",5,5,0,{}};

    p9.neighbors={{0,4},{1,6},{2,8},{3,22},{4,26},{5,12},{6,28},{7,10}};
    p1.neighbors={{8,4}};
    p2.neighbors={{8,6}};
    p3.neighbors={{8,8}};
    p4.neighbors={{8,22}};
    p5.neighbors={{8,26}};
    p6.neighbors={{8,12}};
    p7.neighbors={{8,28}};
    p8.neighbors={{8,10}};

    addBase(c4,p1);addBase(c4,p2);addBase(c4,p3);addBase(c4,p4);
    addBase(c4,p5);addBase(c4,p6);addBase(c4,p7);addBase(c4,p8);addBase(c4,p9);

    // ================= Quetta =================
    string c5="Quetta"; addCity(c5);
    Base q1={"Quetta Base Alpha",8,7,4,{}};
    Base q2={"Quetta Base Beta",10,9,3,{}};
    Base q3={"Quetta Base Gamma",14,13,5,{}};
    Base q4={"Quetta Base Delta",26,24,2,{}};
    Base q5={"Home Base",5,5,0,{}};

    q5.neighbors={{0,4},{1,6},{2,8},{3,22}};
    q1.neighbors={{4,4}};
    q2.neighbors={{4,6}};
    q3.neighbors={{4,8}};
    q4.neighbors={{4,22}};

    addBase(c5,q1);addBase(c5,q2);addBase(c5,q3);addBase(c5,q4);addBase(c5,q5);
}