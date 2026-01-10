#include <SFML/Graphics.hpp>
#include "targetLocations.h"
#include "drone s & r.h"
#include "analysisReport.h"
#include "weatherAPI.h"
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
enum GameState
{
    CITY_SELECTION,
    BASE_SELECTION,
    PRIORITY_CONFIRMATION,
    MISSION_EXECUTION,
    MISSION_COMPLETE,
    MISSION_LOG_VIEW
};

// Target Status
enum TargetStatus
{
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

// DroneState instance
DroneState droneState;

// Analysis Report instance
AnalysisReport analysisReport;

// Weather API instance
WeatherAPI *weatherAPI = nullptr;
bool weatherInitialized = false;

// Fuel tracking variables
float missionStartFuelForCurrentTarget = 0.0f;
sf::Vector2f initialDronePositionForMission;

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
struct PriorityTarget
{
    int index;
    Base base;

    bool operator<(const PriorityTarget &other) const
    {
        return base.priority < other.base.priority;
    }
};

priority_queue<PriorityTarget> missionQueue;

// Function declarations
float calculateDistance(sf::Vector2f pos1, sf::Vector2f pos2);
float calculateFuelRequired(sf::Vector2f from, sf::Vector2f to);
int getDroneGridIndex();
void drawGrid(sf::RenderWindow &window);
void drawHomeBase(sf::RenderWindow &window, sf::Font &font);
void drawDottedPath(sf::RenderWindow &window);
void drawDestroyAnimation(sf::RenderWindow &window);
void drawDrone(sf::RenderWindow &window);
void drawBases(sf::RenderWindow &window, sf::Font &font);
void drawCitySelectionUI(sf::RenderWindow &window, sf::Font &font);
void drawBaseSelectionUI(sf::RenderWindow &window, sf::Font &font);
void drawPriorityConfirmationUI(sf::RenderWindow &window, sf::Font &font);
void drawMissionExecutionUI(sf::RenderWindow &window, sf::Font &font);
void drawMissionCompleteUI(sf::RenderWindow &window, sf::Font &font);
void drawMissionLogView(sf::RenderWindow &window, sf::Font &font);
void drawWeatherInfo(sf::RenderWindow &window, sf::Font &font); // New function for weather display

// Calculate distance
float calculateDistance(sf::Vector2f pos1, sf::Vector2f pos2)
{
    float dx = pos2.x - pos1.x;
    float dy = pos2.y - pos1.y;
    return std::sqrt(dx * dx + dy * dy);
}

// Calculate fuel required
float calculateFuelRequired(sf::Vector2f from, sf::Vector2f to)
{
    return calculateDistance(from, to) * FUEL_PER_GRID_UNIT;
}

int getDroneGridIndex()
{
    for (size_t i = 0; i < allCityBases.size(); i++)
    {
        float diffX = dronePosition.x - allCityBases[i].x;
        float diffY = dronePosition.y - allCityBases[i].y;
        if (diffX * diffX + diffY * diffY < 0.5f)
        {
            return static_cast<int>(i);
        }
    }
    return static_cast<int>(allCityBases.size()) - 1;
}

void drawGrid(sf::RenderWindow &window)
{
    for (unsigned int x = 0; x <= GRID_SIZE; x += CELL_SIZE)
    {
        sf::VertexArray line(sf::PrimitiveType::Lines, 2);
        line[0].position = sf::Vector2f(static_cast<float>(x), 0.0f);
        line[0].color = GRID_COLOR;
        line[1].position = sf::Vector2f(static_cast<float>(x), static_cast<float>(GRID_SIZE));
        line[1].color = GRID_COLOR;
        window.draw(line);
    }

    for (unsigned int y = 0; y <= GRID_SIZE; y += CELL_SIZE)
    {
        sf::VertexArray line(sf::PrimitiveType::Lines, 2);
        line[0].position = sf::Vector2f(0.0f, static_cast<float>(y));
        line[0].color = GRID_COLOR;
        line[1].position = sf::Vector2f(static_cast<float>(GRID_SIZE), static_cast<float>(y));
        line[1].color = GRID_COLOR;
        window.draw(line);
    }
}

void drawHomeBase(sf::RenderWindow &window, sf::Font &font)
{
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

void drawDottedPath(sf::RenderWindow &window)
{
    if (missionQueue.empty() && !movingToTarget)
        return;

    // Draw path to current target if moving
    if (movingToTarget && !currentPath.empty() && pathStep < currentPath.size())
    {
        Base &targetBase = selectedBases[currentPath[pathStep]];
        sf::Vector2f startPos(dronePosition.x * CELL_SIZE, dronePosition.y * CELL_SIZE);
        sf::Vector2f endPos(targetBase.x * CELL_SIZE, targetBase.y * CELL_SIZE);

        float length = std::sqrt((endPos.x - startPos.x) * (endPos.x - startPos.x) +
                                 (endPos.y - startPos.y) * (endPos.y - startPos.y));
        float dashLength = 10.0f;
        float gapLength = 5.0f;
        float totalSegment = dashLength + gapLength;
        int numSegments = static_cast<int>(length / totalSegment);

        sf::Vector2f direction = endPos - startPos;
        float dirLength = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (dirLength > 0)
        {
            direction.x /= dirLength;
            direction.y /= dirLength;
        }

        for (int j = 0; j < numSegments; j++)
        {
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

    // Draw path back to home if returning
    if (returningHome)
    {
        sf::Vector2f startPos(dronePosition.x * CELL_SIZE, dronePosition.y * CELL_SIZE);
        sf::Vector2f endPos(homeBasePosition.x * CELL_SIZE, homeBasePosition.y * CELL_SIZE);

        float length = std::sqrt((endPos.x - startPos.x) * (endPos.x - startPos.x) +
                                 (endPos.y - startPos.y) * (endPos.y - startPos.y));
        float dashLength = 10.0f;
        float gapLength = 5.0f;
        float totalSegment = dashLength + gapLength;
        int numSegments = static_cast<int>(length / totalSegment);

        sf::Vector2f direction = endPos - startPos;
        float dirLength = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (dirLength > 0)
        {
            direction.x /= dirLength;
            direction.y /= dirLength;
        }

        for (int j = 0; j < numSegments; j++)
        {
            sf::Vector2f dashStart = startPos + direction * (j * totalSegment);
            sf::Vector2f dashEnd = dashStart + direction * dashLength;

            sf::VertexArray dash(sf::PrimitiveType::Lines, 2);
            dash[0].position = dashStart;
            dash[0].color = sf::Color(255, 200, 0, 150);
            dash[1].position = dashEnd;
            dash[1].color = sf::Color(255, 200, 0, 150);
            window.draw(dash);
        }
    }
}

void drawDestroyAnimation(sf::RenderWindow &window)
{
    if (!showDestroyAnimation)
        return;

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

void drawDrone(sf::RenderWindow &window)
{
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

void drawBases(sf::RenderWindow &window, sf::Font &font)
{
    if (currentState >= PRIORITY_CONFIRMATION)
    {
        for (size_t i = 0; i < selectedBases.size(); i++)
        {
            const Base &base = selectedBases[i];
            TargetStatus status = targetStatuses[i];

            sf::Color baseColor = TARGET_COLOR;
            if (status == DESTROYED)
                baseColor = DESTROYED_TARGET_COLOR;
            else if (status == SKIPPED)
                baseColor = SKIPPED_TARGET_COLOR;

            sf::RectangleShape targetBase(sf::Vector2f(CELL_SIZE * 1.2f, CELL_SIZE * 0.8f));
            targetBase.setPosition(sf::Vector2f(
                base.x * CELL_SIZE - CELL_SIZE * 0.1f,
                base.y * CELL_SIZE + CELL_SIZE * 0.2f));
            targetBase.setFillColor(baseColor);
            targetBase.setOutlineThickness(2);
            targetBase.setOutlineColor(sf::Color::White);
            window.draw(targetBase);

            if (status == DESTROYED)
            {
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
            }
            else if (status == SKIPPED)
            {
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
            if (status == DESTROYED)
                outlineColor = sf::Color(150, 150, 150);
            else if (status == SKIPPED)
                outlineColor = sf::Color(200, 200, 100);

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

void drawWeatherInfo(sf::RenderWindow &window, sf::Font &font)
{
    if (!weatherAPI || !weatherInitialized) return;
    
    Weather currentWeather = weatherAPI->getCurrentWeather();
    
    // Weather display box - compact version at bottom
    sf::RectangleShape weatherBox(sf::Vector2f(440.0f, 110.0f));
    weatherBox.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 30.0f, static_cast<float>(WINDOW_HEIGHT) - 180.0f));
    weatherBox.setFillColor(sf::Color(30, 35, 50, 220));
    weatherBox.setOutlineThickness(2);
    weatherBox.setOutlineColor(sf::Color(70, 130, 180));
    window.draw(weatherBox);
    
    // Weather header
    sf::RectangleShape weatherHeader(sf::Vector2f(440.0f, 25.0f));
    weatherHeader.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 30.0f, static_cast<float>(WINDOW_HEIGHT) - 180.0f));
    weatherHeader.setFillColor(sf::Color(40, 80, 120));
    window.draw(weatherHeader);
    
    // Weather title
    sf::Text weatherTitle(font, "CURRENT WEATHER", 14);
    weatherTitle.setFillColor(sf::Color(200, 230, 255));
    weatherTitle.setStyle(sf::Text::Bold);
    weatherTitle.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 145.0f, static_cast<float>(WINDOW_HEIGHT) - 177.0f));
    window.draw(weatherTitle);
    
    // City name
    sf::Text cityText(font, selectedCity, 13);
    cityText.setFillColor(sf::Color::White);
    cityText.setStyle(sf::Text::Bold);
    cityText.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 45.0f, static_cast<float>(WINDOW_HEIGHT) - 148.0f));
    window.draw(cityText);
    
    // Temperature
    sf::Text tempText(font, "Temp: " + std::to_string(static_cast<int>(currentWeather.temperature)) + "C", 12);
    if (currentWeather.temperature < 10.0f) {
        tempText.setFillColor(sf::Color(100, 150, 255));
    } else if (currentWeather.temperature < 25.0f) {
        tempText.setFillColor(sf::Color(100, 220, 100));
    } else {
        tempText.setFillColor(sf::Color(255, 100, 100));
    }
    tempText.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 45.0f, static_cast<float>(WINDOW_HEIGHT) - 125.0f));
    window.draw(tempText);
    
    // Wind speed
    sf::Text windText(font, "Wind: " + std::to_string(static_cast<int>(currentWeather.windspeed)) + " km/h", 12);
    if (currentWeather.windspeed > 20.0f) {
        windText.setFillColor(sf::Color(255, 100, 100));
    } else if (currentWeather.windspeed > 10.0f) {
        windText.setFillColor(sf::Color(255, 200, 100));
    } else {
        windText.setFillColor(sf::Color(100, 200, 255));
    }
    windText.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 240.0f, static_cast<float>(WINDOW_HEIGHT) - 125.0f));
    window.draw(windText);
    
    // Flight conditions
    sf::RectangleShape conditionsBox(sf::Vector2f(400.0f, 22.0f));
    conditionsBox.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 50.0f, static_cast<float>(WINDOW_HEIGHT) - 100.0f));
    
    string conditionsText;
    sf::Color conditionsColor;
    
    if (currentWeather.windspeed > 25.0f) {
        conditionsText = "POOR: High winds";
        conditionsColor = sf::Color(255, 50, 50, 180);
    } else if (currentWeather.windspeed > 15.0f) {
        conditionsText = "MODERATE: Windy";
        conditionsColor = sf::Color(255, 150, 50, 180);
    } else {
        conditionsText = "GOOD: Optimal flight";
        conditionsColor = sf::Color(50, 200, 50, 180);
    }
    
    conditionsBox.setFillColor(conditionsColor);
    window.draw(conditionsBox);
    
    sf::Text conditionsTextDisplay(font, conditionsText, 11);
    conditionsTextDisplay.setFillColor(sf::Color::White);
    conditionsTextDisplay.setStyle(sf::Text::Bold);
    conditionsTextDisplay.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 150.0f, static_cast<float>(WINDOW_HEIGHT) - 97.0f));
    window.draw(conditionsTextDisplay);
}


void drawCitySelectionUI(sf::RenderWindow &window, sf::Font &font)
{
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

    for (size_t i = 0; i < cities.size(); i++)
    {
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

    // Draw weather info if initialized (for city selection, shows default)
    if (weatherInitialized)
    {
        drawWeatherInfo(window, font);
    }
}

void drawBaseSelectionUI(sf::RenderWindow &window, sf::Font &font)
{
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
    for (size_t i = 0; i < allCityBases.size() - 1; i++)
    {
        bool isSelected = std::find(selectedBaseIndices.begin(), selectedBaseIndices.end(), static_cast<int>(i)) != selectedBaseIndices.end();

        sf::RectangleShape baseBox(sf::Vector2f(440.0f, 65.0f));
        baseBox.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 30.0f, static_cast<float>(yPos)));
        baseBox.setFillColor(isSelected ? sf::Color(50, 80, 50) : BOX_COLOR);
        baseBox.setOutlineThickness(2);
        baseBox.setOutlineColor(isSelected ? sf::Color(0, 255, 100) : sf::Color(100, 100, 100));
        window.draw(baseBox);

        if (isSelected)
        {
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

    // Draw weather info
    if (weatherInitialized)
    {
        drawWeatherInfo(window, font);
    }
}

void drawPriorityConfirmationUI(sf::RenderWindow &window, sf::Font &font)
{
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

    for (size_t i = 0; i < selectedBases.size(); i++)
    {
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

    // Draw weather info
    if (weatherInitialized)
    {
        drawWeatherInfo(window, font);
    }
}

void drawMissionExecutionUI(sf::RenderWindow &window, sf::Font &font)
{
    sf::RectangleShape sidebar(sf::Vector2f(static_cast<float>(SIDEBAR_WIDTH), static_cast<float>(WINDOW_HEIGHT)));
    sidebar.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE), 0.0f));
    sidebar.setFillColor(SIDEBAR_COLOR);
    window.draw(sidebar);

    sf::Text title(font, "MISSION EXECUTING", 24);
    title.setFillColor(sf::Color(255, 50, 50));
    title.setStyle(sf::Text::Bold);
    title.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 90.0f, 20.0f));
    window.draw(title);

    // Fuel bar
    float fuelPercent = currentFuel / totalFuel;
    sf::RectangleShape fuelBarBg(sf::Vector2f(440.0f, 30.0f));
    fuelBarBg.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 30.0f, 70.0f));
    fuelBarBg.setFillColor(sf::Color(50, 50, 50));
    fuelBarBg.setOutlineThickness(2);
    fuelBarBg.setOutlineColor(sf::Color::White);
    window.draw(fuelBarBg);

    sf::RectangleShape fuelBar(sf::Vector2f(440.0f * fuelPercent, 30.0f));
    fuelBar.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 30.0f, 70.0f));

    sf::Color fuelColor;
    if (fuelPercent > 0.5f)
        fuelColor = sf::Color(0, 255, 100);
    else if (fuelPercent > 0.25f)
        fuelColor = sf::Color(255, 200, 0);
    else
        fuelColor = sf::Color(255, 50, 50);

    fuelBar.setFillColor(fuelColor);
    window.draw(fuelBar);

    sf::Text fuelText(font, "Fuel: " + std::to_string(static_cast<int>(currentFuel)) + "%", 14);
    fuelText.setFillColor(sf::Color::White);
    fuelText.setStyle(sf::Text::Bold);
    fuelText.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 210.0f, 75.0f));
    window.draw(fuelText);

    // Next target info
    int yPos = 120;

    if (!missionQueue.empty() && !movingToTarget)
    {
        priority_queue<PriorityTarget> tempQueue = missionQueue;
        PriorityTarget nextTarget = tempQueue.top();

        sf::Text nextTargetTitle(font, "NEXT TARGET:", 16);
        nextTargetTitle.setFillColor(sf::Color(255, 200, 100));
        nextTargetTitle.setStyle(sf::Text::Bold);
        nextTargetTitle.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 30.0f, static_cast<float>(yPos)));
        window.draw(nextTargetTitle);
        yPos += 30;

        sf::Text targetName(font, nextTarget.base.name, 14);
        targetName.setFillColor(sf::Color::White);
        targetName.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 50.0f, static_cast<float>(yPos)));
        window.draw(targetName);
        yPos += 25;

        sf::Text targetPriority(font, "Priority: " + std::to_string(nextTarget.base.priority), 12);
        targetPriority.setFillColor(sf::Color(200, 200, 100));
        targetPriority.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 50.0f, static_cast<float>(yPos)));
        window.draw(targetPriority);
        yPos += 25;

        sf::Vector2f targetPos(nextTarget.base.x, nextTarget.base.y);
        float fuelToTarget = calculateFuelRequired(dronePosition, targetPos);
        float fuelToHome = calculateFuelRequired(targetPos, homeBasePosition);
        float totalRequired = fuelToTarget + fuelToHome;

        sf::Text fuelToTargetText(font, "Fuel to Target: " + std::to_string(static_cast<int>(fuelToTarget)) + "%", 12);
        fuelToTargetText.setFillColor(sf::Color(150, 200, 255));
        fuelToTargetText.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 50.0f, static_cast<float>(yPos)));
        window.draw(fuelToTargetText);
        yPos += 20;

        sf::Text fuelReturnText(font, "Fuel to Return: " + std::to_string(static_cast<int>(fuelToHome)) + "%", 12);
        fuelReturnText.setFillColor(sf::Color(150, 200, 255));
        fuelReturnText.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 50.0f, static_cast<float>(yPos)));
        window.draw(fuelReturnText);
        yPos += 20;

        sf::Text totalRequiredText(font, "Total Required: " + std::to_string(static_cast<int>(totalRequired)) + "%", 12);
        totalRequiredText.setFillColor(sf::Color(255, 200, 100));
        totalRequiredText.setStyle(sf::Text::Bold);
        totalRequiredText.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 50.0f, static_cast<float>(yPos)));
        window.draw(totalRequiredText);
        yPos += 25;

        string reachStatus;
        sf::Color reachColor;
        if (currentFuel >= totalRequired)
        {
            reachStatus = "STATUS: REACHABLE";
            reachColor = sf::Color(0, 255, 100);
        }
        else
        {
            reachStatus = "STATUS: INSUFFICIENT FUEL";
            reachColor = sf::Color(255, 50, 50);
        }

        sf::Text reachText(font, reachStatus, 14);
        reachText.setFillColor(reachColor);
        reachText.setStyle(sf::Text::Bold);
        reachText.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 50.0f, static_cast<float>(yPos)));
        window.draw(reachText);
        yPos += 35;
    }
    else if (movingToTarget)
    {
        sf::Text currentAction(font, "APPROACHING TARGET...", 16);
        currentAction.setFillColor(sf::Color(255, 200, 100));
        currentAction.setStyle(sf::Text::Bold);
        currentAction.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 30.0f, static_cast<float>(yPos)));
        window.draw(currentAction);
        yPos += 35;
    }
    else if (returningHome)
    {
        sf::Text currentAction(font, "RETURNING TO BASE...", 16);
        currentAction.setFillColor(sf::Color(255, 200, 0));
        currentAction.setStyle(sf::Text::Bold);
        currentAction.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 30.0f, static_cast<float>(yPos)));
        window.draw(currentAction);
        yPos += 35;
    }

    sf::Text logTitle(font, "MISSION LOG:", 16);
    logTitle.setFillColor(sf::Color(200, 200, 100));
    logTitle.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 30.0f, static_cast<float>(yPos)));
    window.draw(logTitle);
    yPos += 30;

    sf::Text logContent(font, missionLog, 11);
    logContent.setFillColor(TEXT_COLOR);
    logContent.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 30.0f, static_cast<float>(yPos)));
    window.draw(logContent);

    // Draw weather info
    if (weatherInitialized)
    {
        drawWeatherInfo(window, font);
    }
}

void drawMissionCompleteUI(sf::RenderWindow &window, sf::Font &font)
{
    sf::RectangleShape sidebar(sf::Vector2f(static_cast<float>(SIDEBAR_WIDTH), static_cast<float>(WINDOW_HEIGHT)));
    sidebar.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE), 0.0f));
    sidebar.setFillColor(SIDEBAR_COLOR);
    window.draw(sidebar);

    sf::Text title(font, "MISSION COMPLETE", 26);
    title.setFillColor(sf::Color(0, 255, 100));
    title.setStyle(sf::Text::Bold);
    title.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 90.0f, 30.0f));
    window.draw(title);

    int destroyed = 0;
    int skipped = 0;
    for (const auto &status : targetStatuses)
    {
        if (status == DESTROYED)
            destroyed++;
        else if (status == SKIPPED)
            skipped++;
    }

    sf::Text statsTitle(font, "MISSION STATISTICS", 18);
    statsTitle.setFillColor(sf::Color(200, 200, 100));
    statsTitle.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 30.0f, 90.0f));
    window.draw(statsTitle);

    int yPos = 130;

    sf::Text destroyedText(font, "Targets Destroyed: " + std::to_string(destroyed), 16);
    destroyedText.setFillColor(sf::Color(0, 255, 100));
    destroyedText.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 50.0f, static_cast<float>(yPos)));
    window.draw(destroyedText);
    yPos += 35;

    sf::Text skippedText(font, "Targets Skipped: " + std::to_string(skipped), 16);
    skippedText.setFillColor(sf::Color(255, 200, 100));
    skippedText.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 50.0f, static_cast<float>(yPos)));
    window.draw(skippedText);
    yPos += 35;

    sf::Text fuelText(font, "Remaining Fuel: " + std::to_string(static_cast<int>(currentFuel)) + "%", 16);
    fuelText.setFillColor(sf::Color(100, 200, 255));
    fuelText.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 50.0f, static_cast<float>(yPos)));
    window.draw(fuelText);
    yPos += 50;

    sf::Text logTitle(font, "MISSION LOG:", 18);
    logTitle.setFillColor(sf::Color(200, 200, 100));
    logTitle.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 30.0f, static_cast<float>(yPos)));
    window.draw(logTitle);
    yPos += 35;

    sf::Text logContent(font, missionLog, 12);
    logContent.setFillColor(TEXT_COLOR);
    logContent.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 30.0f, static_cast<float>(yPos)));
    window.draw(logContent);

    sf::Text instruction(font, "Press A for Analysis Report | R to restart | ESC to exit", 14);
    instruction.setFillColor(sf::Color::White);
    instruction.setStyle(sf::Text::Bold);
    instruction.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 100.0f, static_cast<float>(WINDOW_HEIGHT) - 40.0f));
    window.draw(instruction);

    // Draw weather info
    if (weatherInitialized)
    {
        drawWeatherInfo(window, font);
    }
}

void drawMissionLogView(sf::RenderWindow &window, sf::Font &font)
{
    sf::RectangleShape sidebar(sf::Vector2f(static_cast<float>(SIDEBAR_WIDTH), static_cast<float>(WINDOW_HEIGHT)));
    sidebar.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE), 0.0f));
    sidebar.setFillColor(SIDEBAR_COLOR);
    window.draw(sidebar);

    sf::Text title(font, "ANALYSIS REPORT", 24);
    title.setFillColor(sf::Color(0, 200, 255));
    title.setStyle(sf::Text::Bold);
    title.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 140.0f, 20.0f));
    window.draw(title);

    sf::Text instruction(font, "Press ESC to return", 14);
    instruction.setFillColor(TEXT_COLOR);
    instruction.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 150.0f, 60.0f));
    window.draw(instruction);

    sf::Text logContent(font, analysisReport.generateReport(), 12);
    logContent.setFillColor(TEXT_COLOR);
    logContent.setPosition(sf::Vector2f(static_cast<float>(GRID_SIZE) + 30.0f, 100.0f));
    window.draw(logContent);

    // Draw weather info
    if (weatherInitialized)
    {
        drawWeatherInfo(window, font);
    }
}

int main()
{
    sf::RenderWindow window(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "Drone Attack System");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.openFromFile("arial.ttf"))
    {
        cout << "Error loading font!" << endl;
        return -1;
    }

    // Initialize Weather API
    string apiKey = "d8bc07a569c441f0ab9192544252909"; // WeatherAPI.com key
    weatherAPI = new WeatherAPI(apiKey);

    sf::Clock clock;

    while (window.isOpen())
    {
        while (auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }

            if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (currentState == CITY_SELECTION)
                {
                    if (keyPressed->code >= sf::Keyboard::Key::Num1 && keyPressed->code <= sf::Keyboard::Key::Num5)
                    {
                        int cityIndex = static_cast<int>(keyPressed->code) - static_cast<int>(sf::Keyboard::Key::Num1);
                        vector<City> cities = targetSystem.getAllCities();

                        if (cityIndex < static_cast<int>(cities.size()))
                        {
                            selectedCity = cities[cityIndex].name;
                            allCityBases = targetSystem.getBasesFromCity(selectedCity);
                            selectedBaseIndices.clear();

                            // Set weather API location to the selected city
                            if (weatherAPI)
                            {
                                weatherAPI->setLocation(selectedCity);
                                weatherInitialized = true;
                            }

                            currentState = BASE_SELECTION;
                        }
                    }
                }
                else if (currentState == BASE_SELECTION)
                {
                    if (keyPressed->code >= sf::Keyboard::Key::Num0 && keyPressed->code <= sf::Keyboard::Key::Num9)
                    {
                        int baseIndex = static_cast<int>(keyPressed->code) - static_cast<int>(sf::Keyboard::Key::Num0);

                        if (baseIndex < static_cast<int>(allCityBases.size()) - 1)
                        {
                            auto it = std::find(selectedBaseIndices.begin(), selectedBaseIndices.end(), baseIndex);

                            if (it != selectedBaseIndices.end())
                            {
                                selectedBaseIndices.erase(it);
                            }
                            else
                            {
                                if (selectedBaseIndices.size() < 4)
                                {
                                    selectedBaseIndices.push_back(baseIndex);
                                }
                            }
                        }
                    }
                    else if (keyPressed->code == sf::Keyboard::Key::Enter)
                    {
                        if (!selectedBaseIndices.empty())
                        {
                            selectedBases.clear();
                            targetStatuses.clear();

                            for (int idx : selectedBaseIndices)
                            {
                                selectedBases.push_back(allCityBases[idx]);
                                targetStatuses.push_back(PENDING);
                            }

                            std::sort(selectedBases.begin(), selectedBases.end(),
                                      [](const Base &a, const Base &b)
                                      {
                                          return a.priority > b.priority;
                                      });

                            currentState = PRIORITY_CONFIRMATION;
                        }
                    }
                }
                else if (currentState == PRIORITY_CONFIRMATION)
                {
                    if (keyPressed->code == sf::Keyboard::Key::Space)
                    {
                        analysisReport = AnalysisReport();
                        analysisReport.updateFuelData(static_cast<int>(totalFuel), static_cast<int>(totalFuel));

                        for (const auto &base : selectedBases)
                        {
                            analysisReport.logMissionAssigned(base.name);
                        }

                        missionStarted = true;
                        currentFuel = totalFuel;
                        dronePosition = homeBasePosition;
                        missionStartFuelForCurrentTarget = totalFuel;
                        initialDronePositionForMission = dronePosition;

                        missionLog = "=== MISSION STARTED ===\n";
                        missionLog += "Initial Fuel: 100%\n";
                        missionLog += "Targets Selected: " + std::to_string(selectedBases.size()) + "\n";
                        missionLog += "Home Base: (" + std::to_string(static_cast<int>(homeBasePosition.x)) + ", " + std::to_string(static_cast<int>(homeBasePosition.y)) + ")\n\n";

                        while (!missionQueue.empty())
                            missionQueue.pop();

                        for (size_t i = 0; i < selectedBases.size(); i++)
                        {
                            PriorityTarget pt;
                            pt.index = static_cast<int>(i);
                            pt.base = selectedBases[i];
                            missionQueue.push(pt);
                        }

                        currentState = MISSION_EXECUTION;
                    }
                    else if (keyPressed->code == sf::Keyboard::Key::R)
                    {
                        currentState = BASE_SELECTION;
                        selectedBaseIndices.clear();
                    }
                }
                else if (currentState == MISSION_COMPLETE)
                {
                    if (keyPressed->code == sf::Keyboard::Key::R)
                    {
                        currentState = CITY_SELECTION;
                        selectedCity = "";
                        selectedBaseIndices.clear();
                        selectedBases.clear();
                        allCityBases.clear();
                        targetStatuses.clear();
                        missionStarted = false;
                        returningHome = false;
                        currentTargetIndex = -1;
                        pathStep = 0;
                        movingToTarget = false;
                        currentPath.clear();
                        dronePosition = homeBasePosition;
                        currentFuel = totalFuel;
                        missionLog = "";
                    }
                    else if (keyPressed->code == sf::Keyboard::Key::Escape)
                    {
                        window.close();
                    }
                    else if (keyPressed->code == sf::Keyboard::Key::A)
                    {
                        currentState = MISSION_LOG_VIEW;
                    }
                }
                else if (currentState == MISSION_LOG_VIEW)
                {
                    if (keyPressed->code == sf::Keyboard::Key::Escape)
                    {
                        currentState = MISSION_COMPLETE;
                    }
                }
            }
        }

        // Update weather data regularly
        if (weatherAPI && weatherInitialized)
        {
            weatherAPI->update();
        }

        if (currentState == MISSION_EXECUTION && missionStarted)
        {
            float deltaTime = clock.restart().asSeconds();

            if (showDestroyAnimation)
            {
                destroyAnimationTimer += deltaTime;
                if (destroyAnimationTimer >= 2.0f)
                {
                    showDestroyAnimation = false;
                    destroyAnimationTimer = 0.0f;
                }
            }

            if (!returningHome && !missionQueue.empty())
            {
                if (!movingToTarget)
                {
                    PriorityTarget nextTarget = missionQueue.top();
                    missionQueue.pop();

                    currentTargetIndex = nextTarget.index;
                    sf::Vector2f targetPos(nextTarget.base.x, nextTarget.base.y);

                    float fuelNeeded = calculateFuelRequired(dronePosition, targetPos);
                    float returnFuel = calculateFuelRequired(targetPos, homeBasePosition);

                    missionLog += "==================\n";
                    missionLog += "Target: " + nextTarget.base.name + "\n";
                    missionLog += "Priority: " + std::to_string(nextTarget.base.priority) + "\n";
                    missionLog += "Fuel to target: " + std::to_string(static_cast<int>(fuelNeeded)) + "%\n";
                    missionLog += "Fuel to return: " + std::to_string(static_cast<int>(returnFuel)) + "%\n";
                    missionLog += "Total needed: " + std::to_string(static_cast<int>(fuelNeeded + returnFuel)) + "%\n";
                    missionLog += "Current fuel: " + std::to_string(static_cast<int>(currentFuel)) + "%\n";

                    if (currentFuel >= fuelNeeded + returnFuel)
                    {
                        missionLog += "Status: ENGAGING TARGET\n";
                        movingToTarget = true;
                        currentPath.clear();

                        for (size_t i = 0; i < selectedBases.size(); i++)
                        {
                            if (selectedBases[i].x == nextTarget.base.x &&
                                selectedBases[i].y == nextTarget.base.y)
                            {
                                currentPath.push_back(static_cast<int>(i));
                                break;
                            }
                        }
                        pathStep = 0;

                        // Store CURRENT fuel and position before starting this mission
                        missionStartFuelForCurrentTarget = currentFuel;
                        initialDronePositionForMission = dronePosition;
                    }
                    else
                    {
                        targetStatuses[currentTargetIndex] = SKIPPED;
                        missionLog += "Status: SKIPPED - Insufficient fuel!\n";
                        analysisReport.logMissionFailed(nextTarget.base.name);
                    }

                    if (!missionQueue.empty())
                    {
                        PriorityTarget peekNext = missionQueue.top();
                        missionLog += "------------------\n";
                        missionLog += "Next in queue: " + peekNext.base.name + "\n";
                        missionLog += "Priority: " + std::to_string(peekNext.base.priority) + "\n";
                    }
                    else if (currentFuel >= fuelNeeded + returnFuel)
                    {
                        missionLog += "------------------\n";
                        missionLog += "Last target - will return to base\n";
                    }
                    missionLog += "\n";
                }

                if (movingToTarget && !currentPath.empty())
                {
                    Base &targetBase = selectedBases[currentPath[pathStep]];
                    sf::Vector2f targetPos(targetBase.x, targetBase.y);

                    sf::Vector2f direction = targetPos - dronePosition;
                    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

                    if (distance > 0.05f)
                    {
                        direction.x /= distance;
                        direction.y /= distance;

                        float moveSpeed = 2.0f * deltaTime;
                        dronePosition.x += direction.x * moveSpeed;
                        dronePosition.y += direction.y * moveSpeed;

                        currentFuel -= FUEL_PER_GRID_UNIT * moveSpeed;
                    }
else
{
    dronePosition = targetPos;
    targetStatuses[currentTargetIndex] = DESTROYED;

    // UPDATED: Calculate actual fuel consumed from mission start to target
    float fuelUsedFloat = missionStartFuelForCurrentTarget - currentFuel;
    int fuelUsedForMission = static_cast<int>(fuelUsedFloat);
    
    // Ensure we don't report negative or zero fuel if there was actual movement
    if (fuelUsedForMission <= 0) {
        float theoreticalFuel = calculateFuelRequired(initialDronePositionForMission, targetPos);
        fuelUsedForMission = static_cast<int>(theoreticalFuel);
    }
    
    analysisReport.logMissionCompleted(targetBase.name, fuelUsedForMission);
    analysisReport.updateFuelData(static_cast<int>(totalFuel), static_cast<int>(currentFuel));

    missionLog += ">>> TARGET DESTROYED: " + targetBase.name + " <<<\n";
    missionLog += "Fuel used for mission: " + std::to_string(fuelUsedForMission) + "%\n\n";

    showDestroyAnimation = true;
    destroyAnimationTimer = 0.0f;
    destroyAnimationPos = sf::Vector2f(targetBase.x * CELL_SIZE, targetBase.y * CELL_SIZE);

    movingToTarget = false;
    currentPath.clear();
    pathStep = 0;
}
                }
            }
            else if (!returningHome && missionQueue.empty())
            {
                if (!movingToTarget)
                {
                    missionLog += "==================\n";
                    missionLog += "All targets processed\n";
                    missionLog += "Returning to HOME BASE\n";
                    missionLog += "==================\n\n";
                }
                returningHome = true;
            }

            if (returningHome)
            {
                sf::Vector2f direction = homeBasePosition - dronePosition;
                float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

                if (distance > 0.05f)
                {
                    direction.x /= distance;
                    direction.y /= distance;

                    float moveSpeed = 2.0f * deltaTime;
                    dronePosition.x += direction.x * moveSpeed;
                    dronePosition.y += direction.y * moveSpeed;

                    currentFuel -= FUEL_PER_GRID_UNIT * moveSpeed;

                    analysisReport.updateFuelData(static_cast<int>(totalFuel), static_cast<int>(currentFuel));
                }
                else
                {
                    dronePosition = homeBasePosition;
                    missionLog += ">>> DRONE SAFELY RETURNED TO HOME BASE <<<\n";
                    missionLog += "Mission fuel remaining: " + std::to_string(static_cast<int>(currentFuel)) + "%\n";

                    analysisReport.updateFuelData(static_cast<int>(totalFuel), static_cast<int>(currentFuel));

                    currentState = MISSION_COMPLETE;
                }
            }
        }
        else
        {
            clock.restart();
        }

        droneLightPulse += 0.1f;

        window.clear(BACKGROUND_COLOR);

        if (currentState >= PRIORITY_CONFIRMATION)
        {
            drawGrid(window);
            drawHomeBase(window, font);
            drawBases(window, font);
            drawDottedPath(window);
            drawDrone(window);
            drawDestroyAnimation(window);
        }

        if (currentState == CITY_SELECTION)
        {
            drawCitySelectionUI(window, font);
        }
        else if (currentState == BASE_SELECTION)
        {
            drawBaseSelectionUI(window, font);
        }
        else if (currentState == PRIORITY_CONFIRMATION)
        {
            drawPriorityConfirmationUI(window, font);
        }
        else if (currentState == MISSION_EXECUTION)
        {
            drawMissionExecutionUI(window, font);
        }
        else if (currentState == MISSION_COMPLETE)
        {
            drawMissionCompleteUI(window, font);
        }
        else if (currentState == MISSION_LOG_VIEW)
        {
            drawMissionLogView(window, font);
        }

        window.display();
    }

    // Clean up weather API
    if (weatherAPI)
    {
        delete weatherAPI;
    }

    return 0;
}