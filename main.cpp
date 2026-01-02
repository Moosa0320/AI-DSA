#include <SFML/Graphics.hpp>
#include "TargetLocations.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <queue>
#include <sstream>

using namespace std;

// Constants
const unsigned int WINDOW_WIDTH = 1200;
const unsigned int WINDOW_HEIGHT = 800;
const unsigned int GRID_SIZE = 700;
const unsigned int CELL_SIZE = 20;
const unsigned int SIDEBAR_WIDTH = 500;

// Colors
const sf::Color GRID_COLOR(50, 50, 50, 100);
const sf::Color BACKGROUND_COLOR(15, 20, 30);
const sf::Color SIDEBAR_COLOR(25, 30, 40);
const sf::Color DRONE_BODY_COLOR(100, 200, 255);
const sf::Color DRONE_DOME_COLOR(80, 180, 240);
const sf::Color DRONE_LIGHT_COLOR(255, 255, 150);
const sf::Color TARGET_COLOR(255, 50, 50);
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
    MISSION_COMPLETE
};

// Global variables
TargetLocations targetSystem;
GameState currentState = CITY_SELECTION;
string selectedCity = "";
vector<int> selectedBaseIndices;
vector<Base> selectedBases;
priority_queue<pair<int, int>> attackQueue; // <priority, base_index>
vector<Base> allCityBases;

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

// Draw functions
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
            
            sf::RectangleShape targetBase(sf::Vector2f(CELL_SIZE * 1.2f, CELL_SIZE * 0.8f));
            targetBase.setPosition(sf::Vector2f(
                base.x * CELL_SIZE - CELL_SIZE * 0.1f, 
                base.y * CELL_SIZE + CELL_SIZE * 0.2f));
            targetBase.setFillColor(TARGET_COLOR);
            targetBase.setOutlineThickness(2);
            targetBase.setOutlineColor(sf::Color::White);
            window.draw(targetBase);
            
            sf::CircleShape priorityCircle(10.0f);
            priorityCircle.setPosition(sf::Vector2f(
                base.x * CELL_SIZE + CELL_SIZE * 0.4f, 
                base.y * CELL_SIZE - 10.0f));
            priorityCircle.setFillColor(sf::Color::Black);
            priorityCircle.setOutlineThickness(2);
            priorityCircle.setOutlineColor(sf::Color::Yellow);
            window.draw(priorityCircle);
            
            sf::Text priorityText(font, std::to_string(base.priority), 12);
            priorityText.setFillColor(sf::Color::Yellow);
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
    
    sf::Text instruction(font, "Press 0-3 to toggle selection", 14);
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
    
    sf::Text instruction(font, "Press SPACE to START mission", 16);
    instruction.setFillColor(sf::Color(0, 255, 100));
    instruction.setStyle(sf::Text::Bold);
    instruction.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 60.0f, 70.0f));
    window.draw(instruction);
    
    sf::Text instruction2(font, "Press R to go back", 14);
    instruction2.setFillColor(TEXT_COLOR);
    instruction2.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 130.0f, 100.0f));
    window.draw(instruction2);
    
    sf::Text priorityTitle(font, "Attack Order (by Priority):", 18);
    priorityTitle.setFillColor(sf::Color(200, 200, 100));
    priorityTitle.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 30.0f, 150.0f));
    window.draw(priorityTitle);
    
    int yPos = 200;
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
        
        stringstream ss;
        ss << selectedBases[i].name;
        
        sf::Text targetText(font, ss.str(), 16);
        targetText.setFillColor(sf::Color::White);
        targetText.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 95.0f, static_cast<float>(yPos) + 10.0f));
        window.draw(targetText);
        
        sf::Text priorityText(font, "Priority: " + std::to_string(selectedBases[i].priority), 14);
        priorityText.setFillColor(sf::Color(255, 200, 100));
        priorityText.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 95.0f, static_cast<float>(yPos) + 35.0f));
        window.draw(priorityText);
        
        yPos += 75;
    }
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
    fuelTitle.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 20.0f, 70.0f));
    window.draw(fuelTitle);
    
    sf::RectangleShape fuelBg(sf::Vector2f(400.0f, 20.0f));
    fuelBg.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 50.0f, 100.0f));
    fuelBg.setFillColor(sf::Color(60, 60, 60));
    window.draw(fuelBg);
    
    float fuelPercent = currentFuel / totalFuel;
    sf::RectangleShape fuelBar(sf::Vector2f(400.0f * fuelPercent, 20.0f));
    fuelBar.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 50.0f, 100.0f));
    fuelBar.setFillColor(fuelPercent > 0.5f ? sf::Color(0, 200, 100) : 
                         fuelPercent > 0.25f ? sf::Color(255, 200, 0) : sf::Color(255, 50, 50));
    window.draw(fuelBar);
    
    stringstream fuelSS;
    fuelSS << (int)currentFuel << "%";
    sf::Text fuelText(font, fuelSS.str(), 14);
    fuelText.setFillColor(sf::Color::White);
    fuelText.setStyle(sf::Text::Bold);
    fuelText.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 230.0f, 100.0f));
    window.draw(fuelText);
    
    sf::Text logTitle(font, "MISSION LOG:", 16);
    logTitle.setFillColor(sf::Color(150, 200, 255));
    logTitle.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 20.0f, 150.0f));
    window.draw(logTitle);
    
    sf::RectangleShape logBox(sf::Vector2f(460.0f, 550.0f));
    logBox.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 20.0f, 180.0f));
    logBox.setFillColor(BOX_COLOR);
    logBox.setOutlineThickness(1);
    logBox.setOutlineColor(sf::Color(100, 100, 100));
    window.draw(logBox);
    
    sf::Text logText(font, missionLog, 11);
    logText.setFillColor(TEXT_COLOR);
    logText.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 30.0f, 190.0f));
    window.draw(logText);
    
    sf::Text controls(font, "Press R to reset and return to menu", 12);
    controls.setFillColor(sf::Color(150, 150, 150));
    controls.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 90.0f, static_cast<float>(WINDOW_HEIGHT) - 30.0f));
    window.draw(controls);
}

void calculateFuelAndAttack() {
    if (attackQueue.empty()) {
        returningHome = true;
        missionLog += "\n=== All targets processed ===\n";
        missionLog += "Returning to home base...\n";
        return;
    }
    
    int targetPriority = attackQueue.top().first;
    int targetIndex = attackQueue.top().second;
    attackQueue.pop();
    
    Base& targetBase = selectedBases[targetIndex];
    
    int targetBaseIndex = -1;
    for (size_t i = 0; i < allCityBases.size(); i++) {
        if (allCityBases[i].name == targetBase.name) {
            targetBaseIndex = static_cast<int>(i);
            break;
        }
    }
    
    if (targetBaseIndex == -1) {
        missionLog += "\nERROR: Target not found!\n";
        calculateFuelAndAttack();
        return;
    }
    
    int homeIndex = static_cast<int>(allCityBases.size()) - 1;
    currentPath = targetSystem.diskstra(selectedCity, homeIndex, targetBaseIndex);
    
    if (currentPath.empty()) {
        missionLog += "\nNo path to " + targetBase.name + "!\n";
        calculateFuelAndAttack();
        return;
    }
    
    float fuelNeeded = 0.0f;
    for (size_t i = 0; i < currentPath.size() - 1; i++) {
        int from = currentPath[i];
        int to = currentPath[i + 1];
        
        for (auto& neighbor : allCityBases[from].neighbors) {
            if (neighbor.first == to) {
                fuelNeeded += neighbor.second;
                break;
            }
        }
    }
    
    if (fuelNeeded <= currentFuel) {
        stringstream ss;
        ss << "\n--- Target " << (selectedBases.size() - attackQueue.size()) << " ---\n";
        ss << "Name: " << targetBase.name << "\n";
        ss << "Priority: " << targetPriority << "\n";
        ss << "Fuel needed: " << (int)fuelNeeded << "%\n";
        ss << "STATUS: ENOUGH FUEL!\n";
        ss << ">>> ATTACKING TARGET <<<\n";
        missionLog += ss.str();
        
        currentFuel -= fuelNeeded;
        currentTargetIndex = targetIndex;
        pathStep = 0;
        movingToTarget = true;
    } else {
        stringstream ss;
        ss << "\n--- Target " << (selectedBases.size() - attackQueue.size()) << " ---\n";
        ss << "Name: " << targetBase.name << "\n";
        ss << "Fuel needed: " << (int)fuelNeeded << "%\n";
        ss << "Current fuel: " << (int)currentFuel << "%\n";
        ss << "STATUS: INSUFFICIENT FUEL!\n";
        ss << "Skipping to next target...\n";
        missionLog += ss.str();
        
        calculateFuelAndAttack();
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "Drone Attack System - Integrated");
    window.setFramerateLimit(60);
    
    sf::Font font;
    if (!font.openFromFile("arial.ttf")) {
        if (!font.openFromFile("C:/Windows/Fonts/arial.ttf")) {
            if (!font.openFromFile("/System/Library/Fonts/Helvetica.ttc")) {
                cout << "Warning: Could not load font\n";
            }
        }
    }
    
    cout << "=== Drone Attack System Started ===" << endl;
    cout << "Current State: CITY_SELECTION" << endl;
    
    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (currentState == CITY_SELECTION) {
                    if (keyPressed->code >= sf::Keyboard::Key::Num1 && keyPressed->code <= sf::Keyboard::Key::Num5) {
                        int cityIndex = static_cast<int>(keyPressed->code) - static_cast<int>(sf::Keyboard::Key::Num1);
                        vector<City> cities = targetSystem.getAllCities();
                        if (cityIndex < static_cast<int>(cities.size())) {
                            selectedCity = cities[cityIndex].name;
                            allCityBases = targetSystem.getBasesFromCity(selectedCity);
                            currentState = BASE_SELECTION;
                            cout << "Selected City: " << selectedCity << endl;
                            cout << "Total bases: " << allCityBases.size() << endl;
                        }
                    }
                }
                else if (currentState == BASE_SELECTION) {
                    if (keyPressed->code >= sf::Keyboard::Key::Num0 && keyPressed->code <= sf::Keyboard::Key::Num3) {
                        int baseIndex = static_cast<int>(keyPressed->code) - static_cast<int>(sf::Keyboard::Key::Num0);
                        if (baseIndex < static_cast<int>(allCityBases.size()) - 1) {
                            auto it = std::find(selectedBaseIndices.begin(), selectedBaseIndices.end(), baseIndex);
                            if (it != selectedBaseIndices.end()) {
                                selectedBaseIndices.erase(it);
                                cout << "Deselected base " << baseIndex << endl;
                            } else if (selectedBaseIndices.size() < 4) {
                                selectedBaseIndices.push_back(baseIndex);
                                cout << "Selected base " << baseIndex << ": " << allCityBases[baseIndex].name << endl;
                            }
                        }
                    }
                    else if (keyPressed->code == sf::Keyboard::Key::Enter && !selectedBaseIndices.empty()) {
                        selectedBases.clear();
                        for (int idx : selectedBaseIndices) {
                            selectedBases.push_back(allCityBases[idx]);
                        }
                        
                        sort(selectedBases.begin(), selectedBases.end(), 
                             [](const Base& a, const Base& b) { return a.priority > b.priority; });
                        
                        currentState = PRIORITY_CONFIRMATION;
                        cout << "Moving to priority confirmation" << endl;
                    }
                }
                else if (currentState == PRIORITY_CONFIRMATION) {
                    if (keyPressed->code == sf::Keyboard::Key::Space) {
                        for (size_t i = 0; i < selectedBases.size(); i++) {
                            attackQueue.push({selectedBases[i].priority, static_cast<int>(i)});
                        }
                        
                        currentState = MISSION_EXECUTION;
                        missionStarted = true;
                        missionLog = "=== MISSION STARTED ===\n";
                        missionLog += "City: " + selectedCity + "\n";
                        missionLog += "Targets: " + std::to_string(selectedBases.size()) + "\n";
                        missionLog += "Initial Fuel: 100%\n\n";
                        
                        cout << "Mission started!" << endl;
                        calculateFuelAndAttack();
                    }
                    else if (keyPressed->code == sf::Keyboard::Key::R) {
                        currentState = BASE_SELECTION;
                        selectedBaseIndices.clear();
                        selectedBases.clear();
                    }
                }
                else if (currentState == MISSION_EXECUTION || currentState == MISSION_COMPLETE) {
                    if (keyPressed->code == sf::Keyboard::Key::R) {
                        currentState = CITY_SELECTION;
                        selectedCity = "";
                        selectedBaseIndices.clear();
                        selectedBases.clear();
                        while (!attackQueue.empty()) attackQueue.pop();
                        dronePosition = homeBasePosition;
                        currentFuel = totalFuel;
                        missionStarted = false;
                        returningHome = false;
                        currentTargetIndex = -1;
                        pathStep = 0;
                        movingToTarget = false;
                        missionLog = "";
                        cout << "Reset to city selection" << endl;
                    }
                }
            }
        }
        
        if (currentState == MISSION_EXECUTION && missionStarted) {
            if (movingToTarget && !currentPath.empty() && pathStep < currentPath.size()) {
                int targetNodeIndex = currentPath[pathStep];
                Base& targetNode = allCityBases[targetNodeIndex];
                
                sf::Vector2f targetPos(targetNode.x, targetNode.y);
                
                float speed = 0.08f;
                if (dronePosition.x < targetPos.x) dronePosition.x += speed;
                if (dronePosition.y < targetPos.y) dronePosition.y += speed;
                if (dronePosition.x > targetPos.x) dronePosition.x -= speed;
                if (dronePosition.y > targetPos.y) dronePosition.y -= speed;
                
                float diffX = dronePosition.x - targetPos.x;
                float diffY = dronePosition.y - targetPos.y;
                if (diffX * diffX + diffY * diffY < 0.09f) {
                    pathStep++;
                    if (pathStep >= currentPath.size()) {
                        movingToTarget = false;
                        missionLog += ">>> Target destroyed!\n";
                        calculateFuelAndAttack();
                    }
                }
            }
            else if (returningHome) {
                float speed = 0.08f;
                if (dronePosition.x < homeBasePosition.x) dronePosition.x += speed;
                if (dronePosition.y < homeBasePosition.y) dronePosition.y += speed;
                if (dronePosition.x > homeBasePosition.x) dronePosition.x -= speed;
                if (dronePosition.y > homeBasePosition.y) dronePosition.y -= speed;
                
                float diffX = dronePosition.x - homeBasePosition.x;
                float diffY = dronePosition.y - homeBasePosition.y;
                if (diffX * diffX + diffY * diffY < 0.09f) {
                    missionLog += "\n=== MISSION COMPLETE ===\n";
                    missionLog += "Returned to home base.\n";
                    missionLog += "Final fuel: " + std::to_string((int)currentFuel) + "%\n";
                    currentState = MISSION_COMPLETE;
                    missionStarted = false;
                    returningHome = false;
                    cout << "Mission complete!" << endl;
                }
            }
        }
        
        droneLightPulse += 0.1f;
        if (droneLightPulse > 6.28318f) droneLightPulse = 0.0f;
        
        window.clear(BACKGROUND_COLOR);
        
        if (currentState == CITY_SELECTION) {
            drawGrid(window);
            drawHomeBase(window, font);
            drawCitySelectionUI(window, font);
        }
        else if (currentState == BASE_SELECTION) {
            drawGrid(window);
            drawHomeBase(window, font);
            for (size_t i = 0; i < allCityBases.size() - 1; i++) {
                sf::CircleShape baseMarker(8.0f);
                baseMarker.setPosition(sf::Vector2f(allCityBases[i].x * CELL_SIZE - 8.0f, 
                                                     allCityBases[i].y * CELL_SIZE - 8.0f));
                baseMarker.setFillColor(TARGET_COLOR);
                window.draw(baseMarker);
            }
            drawBaseSelectionUI(window, font);
        }
        else if (currentState == PRIORITY_CONFIRMATION) {
            drawGrid(window);
            drawHomeBase(window, font);
            drawBases(window, font);
            drawPriorityConfirmationUI(window, font);
        }
        else if (currentState == MISSION_EXECUTION || currentState == MISSION_COMPLETE) {
            drawGrid(window);
            drawHomeBase(window, font);
            drawBases(window, font);
            drawDrone(window);
            drawMissionExecutionUI(window, font);
        }
        
        window.display();
    }
    
    return 0;
}