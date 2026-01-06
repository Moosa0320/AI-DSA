#include "drone s & r.h"
#include <sstream>
#include <iomanip>

// MissionLogNode Constructor
MissionLogNode::MissionLogNode(std::string name, float fuel, time_t time)
    : baseName(name), fuelConsumed(fuel), timestamp(time), next(nullptr) {}

// DroneState Constructor
DroneState::DroneState()
    : currentFuel(MAX_FUEL_CAPACITY),
      positionX(5),
      positionY(5),
      missionsCompleted(0),
      historyHead(nullptr),
      historySize(0) {}

// DroneState Destructor
DroneState::~DroneState() {
    clearHistory();
}

// Private helper function
float DroneState::calculateFuelConsumptionRate(float windSpeed, bool hasAlert) {
    float rate = FUEL_CONSUMPTION_BASE_RATE;
    rate += windSpeed * WIND_SPEED_FUEL_MULTIPLIER;
    if (hasAlert) {
        rate *= 1.2f; // 20% extra fuel for alert missions
    }
    return rate;
}

void DroneState::validateFuelLevel() {
    if (currentFuel > MAX_FUEL_CAPACITY) {
        currentFuel = MAX_FUEL_CAPACITY;
    }
    if (currentFuel < 0.0f) {
        currentFuel = 0.0f;
    }
}

// Public methods
bool DroneState::consumeFuel(float amount, float windSpeed, bool hasAlert) {
    float adjustedAmount = amount;
    if (windSpeed > 0.0f || hasAlert) {
        float rate = calculateFuelConsumptionRate(windSpeed, hasAlert);
        adjustedAmount = rate;
    }
    
    if (currentFuel >= adjustedAmount) {
        currentFuel -= adjustedAmount;
        validateFuelLevel();
        return true;
    }
    return false;
}

float DroneState::getFuelPercentage() const {
    return (currentFuel / MAX_FUEL_CAPACITY) * 100.0f;
}

float DroneState::estimateFuelForMission(float distance, float windSpeed) const {
    float baseFuel = distance * FUEL_CONSUMPTION_BASE_RATE;
    baseFuel += windSpeed * WIND_SPEED_FUEL_MULTIPLIER * distance;
    return baseFuel;
}

bool DroneState::isDroneOperational() const {
    return currentFuel > MIN_OPERATIONAL_FUEL;
}

DroneStatus DroneState::getDroneStatus() const {
    if (currentFuel <= 0.0f) {
        return OUT_OF_FUEL;
    } else if (currentFuel <= MIN_OPERATIONAL_FUEL) {
        return LOW_FUEL;
    }
    return OPERATIONAL;
}

void DroneState::pushMission(std::string baseName, float fuelConsumed) {
    time_t now = time(nullptr);
    MissionLogNode* newNode = new MissionLogNode(baseName, fuelConsumed, now);
    
    newNode->next = historyHead;
    historyHead = newNode;
    historySize++;
    missionsCompleted++;
}

MissionLogNode* DroneState::peekLastMission() const {
    return historyHead;
}

void DroneState::clearHistory() {
    while (historyHead != nullptr) {
        MissionLogNode* temp = historyHead;
        historyHead = historyHead->next;
        delete temp;
    }
    historySize = 0;
}

int DroneState::getHistorySize() const {
    return historySize;
}

MissionLogNode* DroneState::getAllMissions() const {
    return historyHead;
}

void DroneState::updatePosition(int x, int y) {
    positionX = x;
    positionY = y;
}

void DroneState::getPosition(int& x, int& y) const {
    x = positionX;
    y = positionY;
}

void DroneState::resetDrone() {
    currentFuel = MAX_FUEL_CAPACITY;
    positionX = 5;
    positionY = 5;
    missionsCompleted = 0;
    clearHistory();
}

std::string DroneState::getStateSnapshot(){
    std::ostringstream oss;
    oss << "=== DRONE STATE SNAPSHOT ===" << std::endl;
    oss << "Position: (" << positionX << ", " << positionY << ")" << std::endl;
    oss << "Fuel: " << std::fixed << std::setprecision(1) << currentFuel << "%" << std::endl;
    oss << "Status: ";
    
    DroneStatus status = getDroneStatus();
    switch (status) {
        case OPERATIONAL:
            oss << "OPERATIONAL";
            break;
        case LOW_FUEL:
            oss << "LOW FUEL";
            break;
        case OUT_OF_FUEL:
            oss << "OUT OF FUEL";
            break;
    }
    oss << std::endl;
    
    oss << "Missions Completed: " << missionsCompleted << std::endl;
    oss << "History Size: " << historySize << std::endl;
    
    return oss.str();
}

int DroneState::getMissionsCompleted() const {
    return missionsCompleted;
}

float DroneState::getRemainingFuel() const {
    return currentFuel;
}