#ifndef ANALYSISREPORT_H
#define ANALYSISREPORT_H

#include <string>
#include <vector>
#include <ctime>
#include <SFML/Graphics.hpp>

using namespace std;

class AnalysisReport {
private: .
    struct MissionLog {
        string baseName;        // Target base name
        string status;          // "Pending", "Completed", "Failed"
        int fuelUsed;           // Fuel consumed for this mission
        time_t timestamp;       // Time of assignment/completion
        int sequenceNumber;     // Execution order
    };

    vector<MissionLog> missionHistory;

    int totalMissionsAssigned;
    int totalMissionsCompleted;
    int totalMissionsFailed;

    int startingFuel;
    int currentFuel;
    int totalFuelConsumed;

    time_t missionStartTime;
    int currentSequenceCounter;

public:
    AnalysisReport();

    void logMissionAssigned(const string& baseName);
    void logMissionCompleted(const string& baseName, int fuelUsed);
    void logMissionFailed(const string& baseName);

    void updateFuelData(int starting, int current); // Fuel & Resource Tracking

    string generateReport() const; // Report Generation

    void displayReport(sf::RenderWindow& window); // Visual Display (SFML)
};

#endif // ANALYSISREPORT_H
