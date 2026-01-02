#include <SFML/Graphics.hpp>
#include "targetLocations.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <queue>
#include <sstream>
#include <set>

using namespace std;

// Constants
const unsigned int WINDOW_WIDTH = 1200;
const unsigned int WINDOW_HEIGHT = 800;
const unsigned int GRID_SIZE = 700;
const unsigned int CELL_SIZE = 20;
const unsigned int SIDEBAR_WIDTH = 500;
const float FUEL_PER_GRID_UNIT = 2.0f;

// Colors
const sf::Color GRID_COLOR(50, 50, 50, 100);
const sf::Color BACKGROUND_COLOR(15, 20, 30);
const sf::Color SIDEBAR_COLOR(25, 30, 40);
const sf::Color DRONE_BODY_COLOR(100, 200, 255);
const sf::Color DRONE_DOME_COLOR(80, 180, 240);
const sf::Color DRONE_LIGHT_COLOR(255, 255, 150);
const sf::Color TARGET_COLOR(255, 50, 50);
const sf::Color DESTROYED_TARGET_COLOR(100, 50, 50);
const sf::Color SKIPPED_TARGET_COLOR(150, 150, 50);
const sf::Color HOME_BASE_COLOR(0, 255, 100);
const sf::Color PATH_COLOR(0, 255, 100, 150);
const sf::Color TEXT_COLOR(220, 220, 220);
const sf::Color BOX_COLOR(40, 45, 60);

// Game States
enum GameState {
    CITY_SELECTION,
    BASE_SELECTION,
    PRIORITY_CONFIRMATION,
    MISSION_EXECUTION,
    MISSION_COMPLETE,
    MISSION_LOG_VIEW
};

// Target Status
enum TargetStatus {
    PENDING,
    DESTROYED,
    SKIPPED
};

// Global variables
TargetLocations targetSystem;
GameState currentState = CITY_SELECTION;
string selectedCity = "";
vector<int> selectedBaseIndices;
vector<Base> selectedBases;
vector<Base> allCityBases;
vector<TargetStatus> targetStatuses;

sf::Vector2f homeBasePosition(5.0f, 5.0f);
sf::Vector2f dronePosition = homeBasePosition;
bool missionStarted = false;
bool returningHome = false;
float droneLightPulse = 0.0f;
float totalFuel = 100.0f;
float currentFuel = 100.0f;
int currentTargetIndex = -1;
vector<int> currentPath;
size_t pathStep = 0;
bool movingToTarget = false;
string missionLog = "";
float destroyAnimationTimer = 0.0f;
bool showDestroyAnimation = false;
sf::Vector2f destroyAnimationPos;

// Priority queue for mission execution
struct PriorityTarget {
    int index;
    Base base;
    
    bool operator<(const PriorityTarget& other) const {
        return base.priority < other.base.priority; // Max heap (higher priority first)
    }
};

priority_queue<PriorityTarget> missionQueue;

// Function declarations
float calculateDistance(sf::Vector2f pos1, sf::Vector2f pos2);
float calculateFuelRequired(sf::Vector2f from, sf::Vector2f to);
int getDroneGridIndex();

// Calculate distance
float calculateDistance(sf::Vector2f pos1, sf::Vector2f pos2) {
    float dx = pos2.x - pos1.x;
    float dy = pos2.y - pos1.y;
    return std::sqrt(dx * dx + dy * dy);
}

// Calculate fuel required
float calculateFuelRequired(sf::Vector2f from, sf::Vector2f to) {
    return calculateDistance(from, to) * FUEL_PER_GRID_UNIT;
}

int getDroneGridIndex() {
    for (size_t i = 0; i < allCityBases.size(); i++) {
        float diffX = dronePosition.x - allCityBases[i].x;
        float diffY = dronePosition.y - allCityBases[i].y;
        if (diffX * diffX + diffY * diffY < 0.5f) {
            return static_cast<int>(i);
        }
    }
    return static_cast<int>(allCityBases.size()) - 1;
}

void drawGrid(sf::RenderWindow& window) {
    for (unsigned int x = 0; x <= GRID_SIZE; x += CELL_SIZE) {
        sf::VertexArray line(sf::PrimitiveType::Lines, 2);
        line[0].position = sf::Vector2f(static_cast<float>(x), 0.0f);
        line[0].color = GRID_COLOR;
        line[1].position = sf::Vector2f(static_cast<float>(x), static_cast<float>(GRID_SIZE));
        line[1].color = GRID_COLOR;
        window.draw(line);
    }
    
    for (unsigned int y = 0; y <= GRID_SIZE; y += CELL_SIZE) {
        sf::VertexArray line(sf::PrimitiveType::Lines, 2);
        line[0].position = sf::Vector2f(0.0f, static_cast<float>(y));
        line[0].color = GRID_COLOR;
        line[1].position = sf::Vector2f(static_cast<float>(GRID_SIZE), static_cast<float>(y));
        line[1].color = GRID_COLOR;
        window.draw(line);
    }
}

void drawHomeBase(sf::RenderWindow& window, sf::Font& font) {
    float centerX = homeBasePosition.x * CELL_SIZE;
    float centerY = homeBasePosition.y * CELL_SIZE;
    
    sf::RectangleShape baseStructure(sf::Vector2f(CELL_SIZE * 2.0f, CELL_SIZE * 2.0f));
    baseStructure.setPosition(sf::Vector2f(centerX - CELL_SIZE, centerY - CELL_SIZE));
    baseStructure.setFillColor(HOME_BASE_COLOR);
    baseStructure.setOutlineThickness(3);
    baseStructure.setOutlineColor(sf::Color::White);
    window.draw(baseStructure);
    
    sf::Text hSymbol(font, "H", 30);
    hSymbol.setFillColor(sf::Color::Black);
    hSymbol.setStyle(sf::Text::Bold);
    hSymbol.setPosition(sf::Vector2f(centerX - 10.0f, centerY - 15.0f));
    window.draw(hSymbol);
    
    sf::Text baseLabel(font, "HOME", 11);
    baseLabel.setFillColor(sf::Color::White);
    baseLabel.setStyle(sf::Text::Bold);
    baseLabel.setPosition(sf::Vector2f(centerX - 18.0f, centerY + CELL_SIZE + 5.0f));
    window.draw(baseLabel);
}

void drawDottedPath(sf::RenderWindow& window) {
    if (currentPath.empty() || pathStep >= currentPath.size()) return;
    
    for (size_t i = pathStep; i < currentPath.size(); i++) {
        sf::Vector2f startPos;
        if (i == pathStep) {
            startPos = sf::Vector2f(dronePosition.x * CELL_SIZE, dronePosition.y * CELL_SIZE);
        } else {
            Base& prevBase = allCityBases[currentPath[i - 1]];
            startPos = sf::Vector2f(prevBase.x * CELL_SIZE, prevBase.y * CELL_SIZE);
        }
        
        Base& nextBase = allCityBases[currentPath[i]];
        sf::Vector2f endPos(nextBase.x * CELL_SIZE, nextBase.y * CELL_SIZE);
        
        float length = std::sqrt((endPos.x - startPos.x) * (endPos.x - startPos.x) + 
                                 (endPos.y - startPos.y) * (endPos.y - startPos.y));
        float dashLength = 10.0f;
        float gapLength = 5.0f;
        float totalSegment = dashLength + gapLength;
        int numSegments = static_cast<int>(length / totalSegment);
        
        sf::Vector2f direction = endPos - startPos;
        float dirLength = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (dirLength > 0) {
            direction.x /= dirLength;
            direction.y /= dirLength;
        }
        
        for (int j = 0; j < numSegments; j++) {
            sf::Vector2f dashStart = startPos + direction * (j * totalSegment);
            sf::Vector2f dashEnd = dashStart + direction * dashLength;
            
            sf::VertexArray dash(sf::PrimitiveType::Lines, 2);
            dash[0].position = dashStart;
            dash[0].color = PATH_COLOR;
            dash[1].position = dashEnd;
            dash[1].color = PATH_COLOR;
            window.draw(dash);
        }
    }
}

void drawDestroyAnimation(sf::RenderWindow& window) {
    if (!showDestroyAnimation) return;
    
    float size = 30.0f + destroyAnimationTimer * 10.0f;
    float alpha = 255.0f * (1.0f - destroyAnimationTimer / 2.0f);
    
    sf::RectangleShape line1(sf::Vector2f(size * 1.4f, 4.0f));
    line1.setPosition(destroyAnimationPos);
    line1.setRotation(sf::degrees(45.0f));
    line1.setFillColor(sf::Color(255, 0, 0, static_cast<unsigned char>(alpha)));
    line1.setOrigin(sf::Vector2f(size * 0.7f, 2.0f));
    window.draw(line1);
    
    sf::RectangleShape line2(sf::Vector2f(size * 1.4f, 4.0f));
    line2.setPosition(destroyAnimationPos);
    line2.setRotation(sf::degrees(-45.0f));
    line2.setFillColor(sf::Color(255, 0, 0, static_cast<unsigned char>(alpha)));
    line2.setOrigin(sf::Vector2f(size * 0.7f, 2.0f));
    window.draw(line2);
}

void drawDrone(sf::RenderWindow& window) {
    float centerX = dronePosition.x * CELL_SIZE;
    float centerY = dronePosition.y * CELL_SIZE;
    
    sf::CircleShape droneBody(CELL_SIZE * 0.8f);
    droneBody.setPosition(sf::Vector2f(centerX - CELL_SIZE * 0.8f, centerY - CELL_SIZE * 0.8f));
    droneBody.setFillColor(DRONE_BODY_COLOR);
    droneBody.setOutlineThickness(2);
    droneBody.setOutlineColor(sf::Color::White);
    window.draw(droneBody);
    
    sf::CircleShape droneDome(CELL_SIZE * 0.5f);
    droneDome.setPosition(sf::Vector2f(centerX - CELL_SIZE * 0.5f, centerY - CELL_SIZE * 0.5f - 3.0f));
    droneDome.setFillColor(DRONE_DOME_COLOR);
    droneDome.setOutlineThickness(1);
    droneDome.setOutlineColor(sf::Color(200, 230, 255));
    window.draw(droneDome);
    
    float pulseSize = 5.0f + 3.0f * std::sin(droneLightPulse);
    sf::CircleShape centerLight(pulseSize);
    centerLight.setPosition(sf::Vector2f(centerX - pulseSize, centerY - pulseSize));
    centerLight.setFillColor(DRONE_LIGHT_COLOR);
    window.draw(centerLight);
}

void drawBases(sf::RenderWindow& window, sf::Font& font) {
    if (currentState >= PRIORITY_CONFIRMATION) {
        for (size_t i = 0; i < selectedBases.size(); i++) {
            const Base& base = selectedBases[i];
            TargetStatus status = targetStatuses[i];
            
            sf::Color baseColor = TARGET_COLOR;
            if (status == DESTROYED) baseColor = DESTROYED_TARGET_COLOR;
            else if (status == SKIPPED) baseColor = SKIPPED_TARGET_COLOR;
            
            sf::RectangleShape targetBase(sf::Vector2f(CELL_SIZE * 1.2f, CELL_SIZE * 0.8f));
            targetBase.setPosition(sf::Vector2f(
                base.x * CELL_SIZE - CELL_SIZE * 0.1f, 
                base.y * CELL_SIZE + CELL_SIZE * 0.2f));
            targetBase.setFillColor(baseColor);
            targetBase.setOutlineThickness(2);
            targetBase.setOutlineColor(sf::Color::White);
            window.draw(targetBase);
            
            if (status == DESTROYED) {
                float centerX = base.x * CELL_SIZE + CELL_SIZE * 0.5f;
                float centerY = base.y * CELL_SIZE + CELL_SIZE * 0.6f;
                
                sf::RectangleShape cross1(sf::Vector2f(CELL_SIZE * 1.2f, 3.0f));
                cross1.setPosition(sf::Vector2f(centerX, centerY));
                cross1.setRotation(sf::degrees(45.0f));
                cross1.setFillColor(sf::Color::Red);
                cross1.setOrigin(sf::Vector2f(CELL_SIZE * 0.6f, 1.5f));
                window.draw(cross1);
                
                sf::RectangleShape cross2(sf::Vector2f(CELL_SIZE * 1.2f, 3.0f));
                cross2.setPosition(sf::Vector2f(centerX, centerY));
                cross2.setRotation(sf::degrees(-45.0f));
                cross2.setFillColor(sf::Color::Red);
                cross2.setOrigin(sf::Vector2f(CELL_SIZE * 0.6f, 1.5f));
                window.draw(cross2);
            } else if (status == SKIPPED) {
                float centerX = base.x * CELL_SIZE + CELL_SIZE * 0.5f;
                float centerY = base.y * CELL_SIZE + CELL_SIZE * 0.6f;
                
                sf::Text skipText(font, "SKIP", 10);
                skipText.setFillColor(sf::Color::Black);
                skipText.setStyle(sf::Text::Bold);
                skipText.setPosition(sf::Vector2f(centerX - 15.0f, centerY - 5.0f));
                window.draw(skipText);
            }
            
            sf::CircleShape priorityCircle(10.0f);
            priorityCircle.setPosition(sf::Vector2f(
                base.x * CELL_SIZE + CELL_SIZE * 0.4f, 
                base.y * CELL_SIZE - 10.0f));
            priorityCircle.setFillColor(sf::Color::Black);
            priorityCircle.setOutlineThickness(2);
            
            sf::Color outlineColor = sf::Color::Yellow;
            if (status == DESTROYED) outlineColor = sf::Color(150, 150, 150);
            else if (status == SKIPPED) outlineColor = sf::Color(200, 200, 100);
            
            priorityCircle.setOutlineColor(outlineColor);
            window.draw(priorityCircle);
            
            sf::Text priorityText(font, std::to_string(base.priority), 12);
            priorityText.setFillColor(outlineColor);
            priorityText.setStyle(sf::Text::Bold);
            priorityText.setPosition(sf::Vector2f(
                base.x * CELL_SIZE + CELL_SIZE * 0.4f + 5.0f, 
                base.y * CELL_SIZE - 8.0f));
            window.draw(priorityText);
        }
    }
}

void drawCitySelectionUI(sf::RenderWindow& window, sf::Font& font) {
    sf::RectangleShape sidebar(sf::Vector2f(static_cast<float>(SIDEBAR_WIDTH), static_cast<float>(WINDOW_HEIGHT)));
    sidebar.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE), 0.0f));
    sidebar.setFillColor(SIDEBAR_COLOR);
    window.draw(sidebar);
    
    sf::Text title(font, "DRONE ATTACK SYSTEM", 24);
    title.setFillColor(sf::Color(0, 200, 255));
    title.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 70.0f, 30.0f));
    window.draw(title);
    
    sf::Text subtitle(font, "SELECT TARGET CITY", 20);
    subtitle.setFillColor(sf::Color(255, 200, 100));
    subtitle.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 90.0f, 70.0f));
    window.draw(subtitle);
    
    sf::Text instruction(font, "Press number key (1-5)", 16);
    instruction.setFillColor(TEXT_COLOR);
    instruction.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 100.0f, 110.0f));
    window.draw(instruction);
    
    vector<City> cities = targetSystem.getAllCities();
    int yPos = 160;
    
    for (size_t i = 0; i < cities.size(); i++) {
        sf::RectangleShape cityBox(sf::Vector2f(440.0f, 70.0f));
        cityBox.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 30.0f, static_cast<float>(yPos)));
        cityBox.setFillColor(BOX_COLOR);
        cityBox.setOutlineThickness(2);
        cityBox.setOutlineColor(sf::Color(100, 150, 200));
        window.draw(cityBox);
        
        sf::Text cityNum(font, std::to_string(i + 1), 28);
        cityNum.setFillColor(sf::Color(0, 255, 100));
        cityNum.setStyle(sf::Text::Bold);
        cityNum.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 50.0f, static_cast<float>(yPos) + 18.0f));
        window.draw(cityNum);
        
        sf::Text cityText(font, cities[i].name, 24);
        cityText.setFillColor(sf::Color::White);
        cityText.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 100.0f, static_cast<float>(yPos) + 20.0f));
        window.draw(cityText);
        
        yPos += 90;
    }
    
    sf::Text footer(font, "Drone Attack System v2.0", 12);
    footer.setFillColor(sf::Color(100, 100, 100));
    footer.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 150.0f, static_cast<float>(WINDOW_HEIGHT) - 30.0f));
    window.draw(footer);
}

void drawBaseSelectionUI(sf::RenderWindow& window, sf::Font& font) {
    sf::RectangleShape sidebar(sf::Vector2f(static_cast<float>(SIDEBAR_WIDTH), static_cast<float>(WINDOW_HEIGHT)));
    sidebar.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE), 0.0f));
    sidebar.setFillColor(SIDEBAR_COLOR);
    window.draw(sidebar);
    
    sf::Text title(font, "SELECT TARGET BASES", 22);
    title.setFillColor(sf::Color(255, 100, 100));
    title.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 80.0f, 20.0f));
    window.draw(title);
    
    sf::Text cityName(font, "City: " + selectedCity, 18);
    cityName.setFillColor(sf::Color(150, 200, 150));
    cityName.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 80.0f, 60.0f));
    window.draw(cityName);
    
    sf::Text instruction(font, "Press 0-9 to toggle selection", 14);
    instruction.setFillColor(TEXT_COLOR);
    instruction.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 80.0f, 90.0f));
    window.draw(instruction);
    
    sf::Text instruction2(font, "Max 4 bases | Press ENTER to confirm", 13);
    instruction2.setFillColor(sf::Color(255, 255, 100));
    instruction2.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 60.0f, 110.0f));
    window.draw(instruction2);
    
    int yPos = 150;
    for (size_t i = 0; i < allCityBases.size() - 1; i++) {
        bool isSelected = std::find(selectedBaseIndices.begin(), selectedBaseIndices.end(), static_cast<int>(i)) != selectedBaseIndices.end();
        
        sf::RectangleShape baseBox(sf::Vector2f(440.0f, 65.0f));
        baseBox.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 30.0f, static_cast<float>(yPos)));
        baseBox.setFillColor(isSelected ? sf::Color(50, 80, 50) : BOX_COLOR);
        baseBox.setOutlineThickness(2);
        baseBox.setOutlineColor(isSelected ? sf::Color(0, 255, 100) : sf::Color(100, 100, 100));
        window.draw(baseBox);
        
        if (isSelected) {
            sf::Text checkMark(font, "[X]", 20);
            checkMark.setFillColor(sf::Color(0, 255, 100));
            checkMark.setStyle(sf::Text::Bold);
            checkMark.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 45.0f, static_cast<float>(yPos) + 20.0f));
            window.draw(checkMark);
        }
        
        stringstream ss;
        ss << i << ". " << allCityBases[i].name;
        
        sf::Text baseText(font, ss.str(), 16);
        baseText.setFillColor(sf::Color::White);
        baseText.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 85.0f, static_cast<float>(yPos) + 10.0f));
        window.draw(baseText);
        
        sf::Text priorityText(font, "Priority: " + std::to_string(allCityBases[i].priority), 14);
        priorityText.setFillColor(sf::Color(200, 200, 100));
        priorityText.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 85.0f, static_cast<float>(yPos) + 35.0f));
        window.draw(priorityText);
        
        yPos += 80;
    }
    
    sf::Text selectedCount(font, "Selected: " + std::to_string(selectedBaseIndices.size()) + " / 4", 18);
    selectedCount.setFillColor(selectedBaseIndices.size() == 4 ? sf::Color(0, 255, 100) : sf::Color(255, 200, 100));
    selectedCount.setStyle(sf::Text::Bold);
    selectedCount.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 160.0f, static_cast<float>(WINDOW_HEIGHT) - 50.0f));
    window.draw(selectedCount);
}

void drawPriorityConfirmationUI(sf::RenderWindow& window, sf::Font& font) {
    sf::RectangleShape sidebar(sf::Vector2f(static_cast<float>(SIDEBAR_WIDTH), static_cast<float>(WINDOW_HEIGHT)));
    sidebar.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE), 0.0f));
    sidebar.setFillColor(SIDEBAR_COLOR);
    window.draw(sidebar);
    
    sf::Text title(font, "CONFIRM MISSION PLAN", 22);
    title.setFillColor(sf::Color(255, 100, 100));
    title.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 80.0f, 20.0f));
    window.draw(title);
    
    sf::Text fuelInfo(font, "Available Fuel: " + std::to_string((int)totalFuel) + "%", 14);
    fuelInfo.setFillColor(sf::Color(0, 255, 100));
    fuelInfo.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 80.0f, 55.0f));
    window.draw(fuelInfo);
    
    sf::Text missionStatus(font, "Targets will be evaluated in priority order", 14);
    missionStatus.setFillColor(sf::Color(200, 200, 200));
    missionStatus.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 50.0f, 75.0f));
    window.draw(missionStatus);
    
    sf::Text instruction(font, "Press SPACE to START mission", 16);
    instruction.setFillColor(sf::Color(0, 255, 100));
    instruction.setStyle(sf::Text::Bold);
    instruction.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 60.0f, 105.0f));
    window.draw(instruction);
    
    sf::Text instruction2(font, "Press R to go back", 14);
    instruction2.setFillColor(TEXT_COLOR);
    instruction2.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 130.0f, 130.0f));
    window.draw(instruction2);
    
    sf::Text priorityTitle(font, "Attack Priority Order:", 18);
    priorityTitle.setFillColor(sf::Color(200, 200, 100));
    priorityTitle.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 30.0f, 170.0f));
    window.draw(priorityTitle);
    
    int yPos = 210;
    
    for (size_t i = 0; i < selectedBases.size(); i++) {
        sf::RectangleShape targetBox(sf::Vector2f(440.0f, 60.0f));
        targetBox.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 30.0f, static_cast<float>(yPos)));
        targetBox.setFillColor(BOX_COLOR);
        targetBox.setOutlineThickness(2);
        targetBox.setOutlineColor(sf::Color(150, 150, 150));
        window.draw(targetBox);
        
        sf::Text orderNum(font, "#" + std::to_string(i + 1), 24);
        orderNum.setFillColor(sf::Color(0, 255, 100));
        orderNum.setStyle(sf::Text::Bold);
        orderNum.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 45.0f, static_cast<float>(yPos) + 15.0f));
        window.draw(orderNum);
        
        sf::Text targetText(font, selectedBases[i].name, 14);
        targetText.setFillColor(sf::Color::White);
        targetText.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 95.0f, static_cast<float>(yPos) + 8.0f));
        window.draw(targetText);
        
        sf::Text priorityText(font, "Priority: " + std::to_string(selectedBases[i].priority), 12);
        priorityText.setFillColor(sf::Color(255, 200, 100));
        priorityText.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 95.0f, static_cast<float>(yPos) + 30.0f));
        window.draw(priorityText);
        
        yPos += 70;
    }
}

void drawMissionLogView(sf::RenderWindow& window, sf::Font& font) {
    sf::RectangleShape sidebar(sf::Vector2f(static_cast<float>(SIDEBAR_WIDTH), static_cast<float>(WINDOW_HEIGHT)));
    sidebar.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE), 0.0f));
    sidebar.setFillColor(SIDEBAR_COLOR);
    window.draw(sidebar);
    
    sf::Text title(font, "MISSION LOG", 24);
    title.setFillColor(sf::Color(0, 200, 255));
    title.setStyle(sf::Text::Bold);
    title.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 140.0f, 20.0f));
    window.draw(title);
    
    sf::Text instruction(font, "Press ESC to return", 14);
    instruction.setFillColor(sf::Color(200, 200, 200));
    instruction.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 130.0f, 60.0f));
    window.draw(instruction);
    
    sf::RectangleShape logBox(sf::Vector2f(460.0f, 670.0f));
    logBox.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 20.0f, 90.0f));
    logBox.setFillColor(BOX_COLOR);
    logBox.setOutlineThickness(2);
    logBox.setOutlineColor(sf::Color(100, 150, 200));
    window.draw(logBox);
    
    sf::Text logText(font, missionLog, 11);
    logText.setFillColor(TEXT_COLOR);
    logText.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 30.0f, 100.0f));
    window.draw(logText);
}

void drawMissionExecutionUI(sf::RenderWindow& window, sf::Font& font) {
    sf::RectangleShape sidebar(sf::Vector2f(static_cast<float>(SIDEBAR_WIDTH), static_cast<float>(WINDOW_HEIGHT)));
sidebar.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE), 0.0f));
sidebar.setFillColor(SIDEBAR_COLOR);
window.draw(sidebar);
string statusMsg = returningHome ? "RETURNING HOME" : "MISSION EXECUTING";
sf::Color statusColor = returningHome ? sf::Color(255, 255, 0) : sf::Color(0, 255, 100);

sf::Text title(font, statusMsg, 22);
title.setFillColor(statusColor);
title.setStyle(sf::Text::Bold);
title.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 90.0f, 20.0f));
window.draw(title);

sf::Text fuelTitle(font, "FUEL STATUS", 16);
fuelTitle.setFillColor(sf::Color(255, 200, 100));
fuelTitle.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 20.0f, 60.0f));
window.draw(fuelTitle);

sf::RectangleShape fuelBg(sf::Vector2f(400.0f, 20.0f));
fuelBg.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 50.0f, 85.0f));
fuelBg.setFillColor(sf::Color(60, 60, 60));
window.draw(fuelBg);

float fuelPercent = currentFuel / totalFuel;
sf::RectangleShape fuelBar(sf::Vector2f(400.0f * fuelPercent, 20.0f));
fuelBar.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 50.0f, 85.0f));
fuelBar.setFillColor(fuelPercent > 0.5f ? sf::Color(0, 200, 100) : 
                     fuelPercent > 0.25f ? sf::Color(255, 200, 0) : sf::Color(255, 50, 50));
window.draw(fuelBar);

stringstream fuelSS;
fuelSS << (int)currentFuel << "%";
sf::Text fuelText(font, fuelSS.str(), 14);
fuelText.setFillColor(sf::Color::White);
fuelText.setStyle(sf::Text::Bold);
fuelText.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 230.0f, 87.0f));
window.draw(fuelText);

sf::Text targetsTitle(font, "MISSION TARGETS", 16);
targetsTitle.setFillColor(sf::Color(255, 200, 100));
targetsTitle.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 20.0f, 130.0f));
window.draw(targetsTitle);

int yPos = 160;
for (size_t i = 0; i < selectedBases.size(); i++) {
    TargetStatus status = targetStatuses[i];
    bool isCurrent = static_cast<int>(i) == currentTargetIndex;
    
    sf::RectangleShape targetBox(sf::Vector2f(440.0f, 90.0f));
    targetBox.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 30.0f, static_cast<float>(yPos)));
    targetBox.setFillColor(isCurrent ? sf::Color(60, 60, 100) : BOX_COLOR);
    targetBox.setOutlineThickness(2);
    targetBox.setOutlineColor(status == DESTROYED ? sf::Color(0, 255, 0) : 
                              status == SKIPPED ? sf::Color(150, 150, 0) :
                              isCurrent ? sf::Color(255, 255, 0) : sf::Color(100, 100, 100));
    window.draw(targetBox);
    
    string statusIcon = status == DESTROYED ? "[X]" : 
                       status == SKIPPED ? "[S]" : 
                       isCurrent ? "[>]" : "[ ]";
    sf::Color iconColor = status == DESTROYED ? sf::Color::Green : 
                         status == SKIPPED ? sf::Color::Yellow :
                         isCurrent ? sf::Color::Yellow : sf::Color(150, 150, 150);
    
    sf::Text statusText(font, statusIcon, 16);
    statusText.setFillColor(iconColor);
    statusText.setStyle(sf::Text::Bold);
    statusText.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 45.0f, static_cast<float>(yPos) + 35.0f));
    window.draw(statusText);
    
    sf::Text targetText(font, selectedBases[i].name, 14);
    targetText.setFillColor(status != PENDING ? sf::Color(180, 180, 180) : sf::Color::White);
    targetText.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 95.0f, static_cast<float>(yPos) + 8.0f));
    window.draw(targetText);
    
    if (status == PENDING) {
        sf::Vector2f targetPos(selectedBases[i].x, selectedBases[i].y);
        float fuelToTarget = calculateFuelRequired(dronePosition, targetPos);
        float fuelToHome = calculateFuelRequired(targetPos, homeBasePosition);
        
        sf::Text fuelToTargetText(font, "To Target: " + std::to_string((int)fuelToTarget) + "%", 11);
        fuelToTargetText.setFillColor(sf::Color(150, 200, 255));
        fuelToTargetText.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 95.0f, static_cast<float>(yPos) + 30.0f));
        window.draw(fuelToTargetText);
        
        sf::Text fuelToHomeText(font, "To Home: " + std::to_string((int)fuelToHome) + "%", 11);
        fuelToHomeText.setFillColor(sf::Color(255, 200, 150));
        fuelToHomeText.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 95.0f, static_cast<float>(yPos) + 48.0f));
        window.draw(fuelToHomeText);
        
        float totalRequired = fuelToTarget + fuelToHome;
        bool canReach = currentFuel >= totalRequired;
        sf::Text reachText(font, canReach ? "REACHABLE" : "UNREACHABLE", 10);
        reachText.setFillColor(canReach ? sf::Color(0, 255, 100) : sf::Color(255, 50, 50));
        reachText.setStyle(sf::Text::Bold);
        reachText.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 95.0f, static_cast<float>(yPos) + 66.0f));
        window.draw(reachText);
    } else if (status == DESTROYED) {
        sf::Text destroyedText(font, "DESTROYED", 12);
        destroyedText.setFillColor(sf::Color(0, 255, 0));
        destroyedText.setStyle(sf::Text::Bold);
        destroyedText.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 95.0f, static_cast<float>(yPos) + 40.0f));
        window.draw(destroyedText);
    } else if (status == SKIPPED) {
        sf::Text skippedText(font, "SKIPPED - INSUFFICIENT FUEL", 11);
        skippedText.setFillColor(sf::Color::Yellow);
        skippedText.setStyle(sf::Text::Bold);
        skippedText.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 95.0f, static_cast<float>(yPos) + 40.0f));
        window.draw(skippedText);
    }
    
    yPos += 100;
}

if (returningHome) {
    float fuelToHome = calculateFuelRequired(dronePosition, homeBasePosition);
    sf::Text homeInfo(font, "Fuel to Home: " + std::to_string((int)fuelToHome) + "%", 14);
    homeInfo.setFillColor(sf::Color(255, 255, 100));
    homeInfo.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 120.0f, static_cast<float>(yPos) + 10.0f));
    window.draw(homeInfo);
}

sf::Text instructions(font, "Press L to view Mission Log", 14);
instructions.setFillColor(sf::Color(150, 150, 200));
instructions.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 100.0f, static_cast<float>(WINDOW_HEIGHT) - 50.0f));
window.draw(instructions);}
void drawMissionCompleteUI(sf::RenderWindow& window, sf::Font& font) {
sf::RectangleShape sidebar(sf::Vector2f(static_cast<float>(SIDEBAR_WIDTH), static_cast<float>(WINDOW_HEIGHT)));
sidebar.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE), 0.0f));
sidebar.setFillColor(SIDEBAR_COLOR);
window.draw(sidebar);
int destroyedCount = 0;
for (TargetStatus status : targetStatuses) {
    if (status == DESTROYED) destroyedCount++;
}

bool allDestroyed = destroyedCount == static_cast<int>(selectedBases.size());

sf::Text title(font, allDestroyed ? "MISSION COMPLETE" : "MISSION INCOMPLETE", 24);
title.setFillColor(allDestroyed ? sf::Color(0, 255, 100) : sf::Color(255, 200, 0));
title.setStyle(sf::Text::Bold);
title.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 70.0f, 30.0f));
window.draw(title);

sf::Text targetsSummary(font, "Targets Destroyed: " + std::to_string(destroyedCount) + " / " + 
                       std::to_string(selectedBases.size()), 18);
targetsSummary.setFillColor(sf::Color::White);
targetsSummary.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 90.0f, 80.0f));
window.draw(targetsSummary);

sf::Text fuelRemaining(font, "Fuel Remaining: " + std::to_string((int)currentFuel) + "%", 16);
fuelRemaining.setFillColor(sf::Color(150, 200, 255));
fuelRemaining.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 100.0f, 110.0f));
window.draw(fuelRemaining);

int yPos = 160;
for (size_t i = 0; i < selectedBases.size(); i++) {
    TargetStatus status = targetStatuses[i];
    
    sf::RectangleShape statusBox(sf::Vector2f(440.0f, 50.0f));
    statusBox.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 30.0f, static_cast<float>(yPos)));
    statusBox.setFillColor(BOX_COLOR);
    statusBox.setOutlineThickness(2);
    statusBox.setOutlineColor(status == DESTROYED ? sf::Color(0, 255, 0) : sf::Color(150, 150, 0));
    window.draw(statusBox);
    
    string statusIcon = status == DESTROYED ? "[X]" : "[S]";
    sf::Color iconColor = status == DESTROYED ? sf::Color::Green : sf::Color::Yellow;
    
    sf::Text iconText(font, statusIcon, 16);
    iconText.setFillColor(iconColor);
    iconText.setStyle(sf::Text::Bold);
    iconText.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 45.0f, static_cast<float>(yPos) + 15.0f));
    window.draw(iconText);
    
    sf::Text baseName(font, selectedBases[i].name, 14);
    baseName.setFillColor(sf::Color::White);
    baseName.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 90.0f, static_cast<float>(yPos) + 5.0f));
    window.draw(baseName);
    
    sf::Text statusLabel(font, status == DESTROYED ? "DESTROYED" : "SKIPPED", 12);
    statusLabel.setFillColor(iconColor);
    statusLabel.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 90.0f, static_cast<float>(yPos) + 26.0f));
    window.draw(statusLabel);
    
    yPos += 60;
}

sf::Text instruction1(font, "Press L to view detailed log", 14);
instruction1.setFillColor(sf::Color(150, 200, 255));
instruction1.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 100.0f, static_cast<float>(WINDOW_HEIGHT) - 80.0f));
window.draw(instruction1);

sf::Text instruction2(font, "Press R to restart", 14);
instruction2.setFillColor(sf::Color(200, 200, 200));
instruction2.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 130.0f, static_cast<float>(WINDOW_HEIGHT) - 50.0f));
window.draw(instruction2);
}
int main() {
sf::RenderWindow window(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "Drone Attack System");
window.setFramerateLimit(60);
sf::Font font;
if (!font.openFromFile("arial.ttf")) {
    cout << "Error loading font!" << endl;
    return -1;
}

sf::Clock clock;

while (window.isOpen()) {
    while (auto event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
        }
        
        if (const auto* keyPress = event->getIf<sf::Event::KeyPressed>()) {
            if (currentState == CITY_SELECTION) {
                if (keyPress->code >= sf::Keyboard::Key::Num1 && keyPress->code <= sf::Keyboard::Key::Num5) {
                    int cityIndex = static_cast<int>(keyPress->code) - static_cast<int>(sf::Keyboard::Key::Num1);
                    vector<City> cities = targetSystem.getAllCities();
                    if (cityIndex < static_cast<int>(cities.size())) {
                        selectedCity = cities[cityIndex].name;
                        allCityBases = targetSystem.getBasesFromCity(selectedCity);
                        
                        Base homeBase;
                        homeBase.name = "Home Base";
                        homeBase.x = homeBasePosition.x;
                        homeBase.y = homeBasePosition.y;
                        homeBase.priority = 0;
                        allCityBases.push_back(homeBase);
                        
                        currentState = BASE_SELECTION;
                        selectedBaseIndices.clear();
                        selectedBases.clear();
                    }
                }
            }
            else if (currentState == BASE_SELECTION) {
                // FIXED: Now supports 0-9 keys dynamically based on available bases
                int numBases = static_cast<int>(allCityBases.size()) - 1; // Exclude home base
                int maxSelectableIndex = min(9, numBases - 1); // Max index that can be selected (0-9)
                
                if (keyPress->code >= sf::Keyboard::Key::Num0 && 
                    keyPress->code <= sf::Keyboard::Key::Num9) {
                    int baseIndex = static_cast<int>(keyPress->code) - static_cast<int>(sf::Keyboard::Key::Num0);
                    
                    // Check if this index is valid for current city
                    if (baseIndex < numBases) {
                        auto it = std::find(selectedBaseIndices.begin(), selectedBaseIndices.end(), baseIndex);
                        if (it != selectedBaseIndices.end()) {
                            selectedBaseIndices.erase(it);
                        } else {
                            if (selectedBaseIndices.size() < 4) {
                                selectedBaseIndices.push_back(baseIndex);
                            }
                        }
                    }
                }
                if (keyPress->code == sf::Keyboard::Key::Enter && !selectedBaseIndices.empty()) {
                    selectedBases.clear();
                    for (int idx : selectedBaseIndices) {
                        selectedBases.push_back(allCityBases[idx]);
                    }
                    
                    std::sort(selectedBases.begin(), selectedBases.end(), 
                        [](const Base& a, const Base& b) {
                            return a.priority > b.priority;
                        });
                    
                    targetStatuses.clear();
                    targetStatuses.resize(selectedBases.size(), PENDING);
                    
                    currentState = PRIORITY_CONFIRMATION;
                }
            }
            else if (currentState == PRIORITY_CONFIRMATION) {
                if (keyPress->code == sf::Keyboard::Key::Space) {
                    currentState = MISSION_EXECUTION;
                    missionStarted = true;
                    
                    // Initialize priority queue
                    while (!missionQueue.empty()) missionQueue.pop();
                    for (size_t i = 0; i < selectedBases.size(); i++) {
                        PriorityTarget pt;
                        pt.index = static_cast<int>(i);
                        pt.base = selectedBases[i];
                        missionQueue.push(pt);
                    }
                    
                    missionLog = "=== MISSION LOG ===\n\n";
                    missionLog += "Mission Start\n";
                    missionLog += "City: " + selectedCity + "\n";
                    missionLog += "Targets Selected: " + std::to_string(selectedBases.size()) + "\n";
                    missionLog += "Starting Fuel: " + std::to_string((int)currentFuel) + "%\n\n";
                    missionLog += "--- Priority Order ---\n";
                    for (size_t i = 0; i < selectedBases.size(); i++) {
                        missionLog += std::to_string(i + 1) + ". " + selectedBases[i].name + 
                                     " (Priority: " + std::to_string(selectedBases[i].priority) + ")\n";
                    }
                    missionLog += "\n--- Mission Execution ---\n\n";
                    
                    currentTargetIndex = -1;
                }
                if (keyPress->code == sf::Keyboard::Key::R) {
                    currentState = BASE_SELECTION;
                }
            }
            else if (currentState == MISSION_EXECUTION) {
                if (keyPress->code == sf::Keyboard::Key::L) {
                    currentState = MISSION_LOG_VIEW;
                }
            }
            else if (currentState == MISSION_COMPLETE) {
                if (keyPress->code == sf::Keyboard::Key::L) {
                    currentState = MISSION_LOG_VIEW;
                }
                if (keyPress->code == sf::Keyboard::Key::R) {
                    currentState = CITY_SELECTION;
                    selectedCity = "";
                    selectedBaseIndices.clear();
                    selectedBases.clear();
                    allCityBases.clear();
                    targetStatuses.clear();
                    while (!missionQueue.empty()) missionQueue.pop();
                    dronePosition = homeBasePosition;
                    currentFuel = totalFuel;
                    missionStarted = false;
                    returningHome = false;
                    currentTargetIndex = -1;
                    currentPath.clear();
                    pathStep = 0;
                    movingToTarget = false;
                }
            }
            else if (currentState == MISSION_LOG_VIEW) {
                if (keyPress->code == sf::Keyboard::Key::Escape) {
                    currentState = MISSION_COMPLETE;
                }
            }
        }
    }
    
    float deltaTime = clock.restart().asSeconds();
    droneLightPulse += deltaTime * 3.0f;
    
    if (showDestroyAnimation) {
        destroyAnimationTimer += deltaTime;
        if (destroyAnimationTimer >= 2.0f) {
            showDestroyAnimation = false;
            destroyAnimationTimer = 0.0f;
        }
    }
    
    if (currentState == MISSION_EXECUTION && missionStarted) {
        // FIXED: Added check to prevent accessing empty queue
        if (!returningHome && !missionQueue.empty()) {
            // Check if we need to get next target
            if (!movingToTarget && currentTargetIndex == -1) {
                // Get next target from priority queue
                PriorityTarget nextTarget = missionQueue.top();
                missionQueue.pop();
                
                currentTargetIndex = nextTarget.index;
                
                // Check if target is reachable
                sf::Vector2f targetPos(nextTarget.base.x, nextTarget.base.y);
                float fuelToTarget = calculateFuelRequired(dronePosition, targetPos);
                float fuelToHome = calculateFuelRequired(targetPos, homeBasePosition);
                float totalFuelNeeded = fuelToTarget + fuelToHome;
                
                if (currentFuel >= totalFuelNeeded) {
                    // Target is reachable
                    missionLog += "Target: " + nextTarget.base.name + "\n";
                    missionLog += "Status: REACHABLE\n";
                    missionLog += "Priority: " + std::to_string(nextTarget.base.priority) + "\n";
                    missionLog += "Fuel to Target: " + std::to_string((int)fuelToTarget) + "%\n";
                    missionLog += "Fuel to Home: " + std::to_string((int)fuelToHome) + "%\n";
                    
                    // Find target in allCityBases
                    int targetIdx = -1;
                    for (size_t i = 0; i < allCityBases.size() - 1; i++) {
                        if (allCityBases[i].x == nextTarget.base.x && 
                            allCityBases[i].y == nextTarget.base.y) {
                            targetIdx = static_cast<int>(i);
                            break;
                        }
                    }
                    
                    if (targetIdx != -1) {
                        currentPath.clear();
                        currentPath.push_back(targetIdx);
                        pathStep = 0;
                        movingToTarget = true;
                    }
                } else {
                    // Target is unreachable - skip it
                    targetStatuses[currentTargetIndex] = SKIPPED;
                    
                    missionLog += "Target: " + nextTarget.base.name + "\n";
                    missionLog += "Status: UNREACHABLE - SKIPPED\n";
                    missionLog += "Priority: " + std::to_string(nextTarget.base.priority) + "\n";
                    missionLog += "Required Fuel: " + std::to_string((int)totalFuelNeeded) + "%\n";
                    missionLog += "Available Fuel: " + std::to_string((int)currentFuel) + "%\n\n";
                    
                    currentTargetIndex = -1; // Reset to check next target
                }
            }
            
            // Move drone to target
            if (movingToTarget && !currentPath.empty() && pathStep < currentPath.size()) {
                Base& targetBase = allCityBases[currentPath[pathStep]];
                sf::Vector2f targetPos(targetBase.x, targetBase.y);
                
                sf::Vector2f direction = targetPos - dronePosition;
                float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);
                
                if (distance < 0.1f) {
                    dronePosition = targetPos;
                    pathStep++;
                    
                    if (pathStep >= currentPath.size()) {
                        // Target reached and destroyed
                        targetStatuses[currentTargetIndex] = DESTROYED;
                        
                        missionLog += "Action: TARGET DESTROYED\n";
                        missionLog += "Remaining Fuel: " + std::to_string((int)currentFuel) + "%\n\n";
                        
                        showDestroyAnimation = true;
                        destroyAnimationTimer = 0.0f;
                        destroyAnimationPos = sf::Vector2f(targetPos.x * CELL_SIZE, targetPos.y * CELL_SIZE);
                        
                        movingToTarget = false;
                        currentPath.clear();
                        pathStep = 0;
                        currentTargetIndex = -1; // Reset to get next target
                    }
                } else {
                    float speed = 2.0f * deltaTime;
                    if (distance > 0) {
                        direction.x = direction.x / distance * speed;
                        direction.y = direction.y / distance * speed;
                    }
                    dronePosition += direction;
                    
                    currentFuel -= 0.5f * deltaTime;
                    if (currentFuel < 0) currentFuel = 0;
                }
            }
        }
        else if (!returningHome && missionQueue.empty() && currentTargetIndex == -1) {
            // All targets processed, return home
            returningHome = true;
            currentPath.clear();
            pathStep = 0;
            movingToTarget = false;
            
            int destroyedCount = 0;
            for (TargetStatus status : targetStatuses) {
                if (status == DESTROYED) destroyedCount++;
            }
            
            missionLog += "--- Mission Summary ---\n";
            missionLog += "Targets Destroyed: " + std::to_string(destroyedCount) + " / " + 
                         std::to_string(selectedBases.size()) + "\n";
            missionLog += "Returning to home base...\n\n";
        }
        
        // Return to home
        if (returningHome) {
            sf::Vector2f direction = homeBasePosition - dronePosition;
            float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);
            
            if (distance < 0.1f) {
                dronePosition = homeBasePosition;
                currentState = MISSION_COMPLETE;
                
                int destroyedCount = 0;
                for (TargetStatus status : targetStatuses) {
                    if (status == DESTROYED) destroyedCount++;
                }
                
                if (destroyedCount == static_cast<int>(selectedBases.size())) {
                    missionLog += "=== MISSION COMPLETE ===\n";
                } else {
                    missionLog += "=== MISSION INCOMPLETE ===\n";
                }
                missionLog += "Returned to home base\n";
                missionLog += "Final Fuel: " + std::to_string((int)currentFuel) + "%\n";
            } else {
                float speed = 2.0f * deltaTime;
                if (distance > 0) {
                    direction.x = direction.x / distance * speed;
                    direction.y = direction.y / distance * speed;
                }
                dronePosition += direction;
                
                currentFuel -= 0.3f * deltaTime;
                if (currentFuel < 0) currentFuel = 0;
            }
        }
    }
    
    window.clear(BACKGROUND_COLOR);
    
    drawGrid(window);
    drawHomeBase(window, font);
    
    if (currentState >= PRIORITY_CONFIRMATION) {
        drawBases(window, font);
        drawDrone(window);
        drawDottedPath(window);
        drawDestroyAnimation(window);
    }
    
    switch (currentState) {
        case CITY_SELECTION:
            drawCitySelectionUI(window, font);
            break;
        case BASE_SELECTION:
            drawBaseSelectionUI(window, font);
            break;
        case PRIORITY_CONFIRMATION:
            drawPriorityConfirmationUI(window, font);
            break;
        case MISSION_EXECUTION:
            drawMissionExecutionUI(window, font);
            break;
        case MISSION_COMPLETE:
            drawMissionCompleteUI(window, font);
            break;
        case MISSION_LOG_VIEW:
            drawMissionLogView(window, font);
            break;
    }
    
    window.display();
}

return 0;
}