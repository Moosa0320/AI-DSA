#include "analysisReport.h"
#include <sstream>
#include <iomanip>

AnalysisReport::AnalysisReport()
{
    totalMissionsAssigned = 0;
    totalMissionsCompleted = 0;
    totalMissionsFailed = 0;

    startingFuel = 0;
    currentFuel = 0;
    totalFuelConsumed = 0;

    missionStartTime = std::time(nullptr);
    currentSequenceCounter = 0;
}

void AnalysisReport::logMissionAssigned(const string& baseName)
{
    MissionLog log;
    log.baseName = baseName;
    log.status = "Pending";
    log.fuelUsed = 0;
    log.timestamp = std::time(nullptr);
    log.sequenceNumber = ++currentSequenceCounter;

    missionHistory.push_back(log);
    totalMissionsAssigned++;
}

void AnalysisReport::logMissionCompleted(const string& baseName, int fuelUsed)
{
    for (auto& log : missionHistory)
    {
        if (log.baseName == baseName && log.status == "Pending")
        {
            log.status = "Completed";
            log.fuelUsed = fuelUsed;
            break;
        }
    }

    totalMissionsCompleted++;
    // REMOVED: totalFuelConsumed += fuelUsed;  
    // We calculate total from startingFuel - currentFuel instead
}

void AnalysisReport::logMissionFailed(const string& baseName)
{
    for (auto& log : missionHistory)
    {
        if (log.baseName == baseName && log.status == "Pending")
        {
            log.status = "Failed";
            break;
        }
    }

    totalMissionsFailed++;
}

void AnalysisReport::updateFuelData(int starting, int current)
{
    startingFuel = starting;
    currentFuel = current;
    // UPDATED: Calculate total fuel consumed from actual fuel difference
    totalFuelConsumed = startingFuel - currentFuel;
}

string AnalysisReport::generateReport() const
{
    std::stringstream report;

    report << "===== ANALYSIS REPORT =====\n";
    report << "Mission Start Time: " << std::ctime(&missionStartTime);
    report << "\n";

    report << "Total Missions Assigned: " << totalMissionsAssigned << "\n";
    report << "Total Missions Completed: " << totalMissionsCompleted << "\n";
    report << "Total Missions Failed: " << totalMissionsFailed << "\n\n";

    report << "Starting Fuel: " << startingFuel << "%\n";
    report << "Current Fuel: " << currentFuel << "%\n";
    report << "Total Fuel Consumed: " << totalFuelConsumed << "%\n\n";

    report << "----- Mission History -----\n";

    for (const auto& log : missionHistory)
    {
        report << "#" << log.sequenceNumber
               << " | Base: " << log.baseName
               << " | Status: " << log.status
               << " | Fuel Used: " << log.fuelUsed
               << "\n";
    }

    report << "===========================\n";
    return report.str();
}

void AnalysisReport::displayReport(sf::RenderWindow& window)
{
    sf::Font font;
    if (!font.openFromFile("arial.ttf"))
    {
        // Font failed to load — do not attempt to draw text
        return;
    }

    sf::Text reportText(font, generateReport(), 14);
    reportText.setFillColor(sf::Color::White);
    reportText.setPosition({20.f, 20.f});

    window.draw(reportText);
}