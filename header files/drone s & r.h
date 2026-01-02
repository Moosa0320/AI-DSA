#ifndef DRONE_STATE_H
#define DRONE_STATE_H

#include <string>
#include <ctime>

// Forward declarations (circular dependency avoid karne ke liye)
struct Base;

const float MAX_FUEL_CAPACITY = 100.0f;              // Maximum fuel percentage
const float MIN_OPERATIONAL_FUEL = 10.0f;            // Critical fuel level
const float FUEL_CONSUMPTION_BASE_RATE = 5.0f;       // Base fuel consumption per mission
const float WIND_SPEED_FUEL_MULTIPLIER = 0.5f;       // Additional fuel per unit wind speed

enum DroneStatus {
    OPERATIONAL,      // Fuel > 10%
    LOW_FUEL,         // Fuel <= 10%
    OUT_OF_FUEL       // Fuel = 0%
};

struct MissionLogNode {
    std::string baseName;           // Target base name/ID
    float fuelConsumed;             // Fuel consumed for this mission
    time_t timestamp;               // Mission completion time
    MissionLogNode* next;           // Pointer to next node in stack
    
    // Constructor
    MissionLogNode(std::string name, float fuel, time_t time);
};

class DroneState {
private:
    float currentFuel;              // Current fuel level (0-100%)
    int positionX;                  // Current X coordinate on map
    int positionY;                  // Current Y coordinate on map
    int missionsCompleted;          // Total missions completed counter

    MissionLogNode* historyHead;    // Head pointer for stack
    int historySize;                // Number of missions logged

    float calculateFuelConsumptionRate(float windSpeed, bool hasAlert);
    void validateFuelLevel();
    
public:
   
    DroneState();                   // Initialize with default values
    ~DroneState();                  // Cleanup stack memory
  
    // Consume fuel during mission
    // Parameters: amount - fuel to deduct, windSpeed - current wind speed
    // Returns: true if successful, false if insufficient fuel
    bool consumeFuel(float amount, float windSpeed = 0.0f, bool hasAlert = false);
    
    // Get current fuel percentage
    // Returns: fuel level (0-100%)
    float getFuelPercentage() const;
    
    // Estimate fuel required for next mission
    // Parameters: distance - mission distance, windSpeed - expected wind speed
    // Returns: estimated fuel consumption
    float estimateFuelForMission(float distance, float windSpeed = 0.0f) const;
    
    // Check if drone can fly
    // Returns: true if fuel > MIN_OPERATIONAL_FUEL
    bool isDroneOperational() const;
    
    // Get current drone status
    // Returns: DroneStatus enum value
    DroneStatus getDroneStatus() const;
    
    // Push new mission log entry to stack
    // Parameters: baseName - target base name, fuelConsumed - fuel used
    void pushMission(std::string baseName, float fuelConsumed);
    
    // Peek at last mission without removing
    // Returns: pointer to last mission node (nullptr if empty)
    MissionLogNode* peekLastMission() const;
    
    // Clear entire mission history
    void clearHistory();
    
    // Get number of missions in history
    // Returns: stack size
    int getHistorySize() const;
    
    // Get all missions for Analysis Module (Module 6)
    // Returns: pointer to stack head for traversal
    MissionLogNode* getAllMissions() const;
 
    // Update drone position on map
    // Parameters: x, y - new coordinates
    void updatePosition(int x, int y);
    
    // Get current position
    // Parameters: x, y - reference variables to store position
    void getPosition(int& x, int& y) const;
    
    // Reset drone to initial state
    // Resets: fuel to 100%, clears history, resets mission counter
    void resetDrone();
    
    // Get complete state snapshot for visualization (Module 2)
    // Returns: formatted string with all state info
    std::string getStateSnapshot() const;
    
    // Get total missions completed
    // Returns: mission counter
    int getMissionsCompleted() const;
    
    // Get remaining fuel in absolute value
    // Returns: current fuel amount
    float getRemainingFuel() const;
};

#endif // DRONE_STATE_H