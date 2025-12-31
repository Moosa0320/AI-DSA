#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

#include "WeatherAPI.h"       // Module 1
#include "TargetManager.h"    // Module 3
#include "DroneState.h"       // Module 4
#include "MissionManager.h"  // Module 5
#include "AnalysisReport.h"  // Module 6

const int WINDOW_WIDTH  = 1280;
const int WINDOW_HEIGHT = 720;
const int FPS_LIMIT     = 60;
const std::string WINDOW_TITLE = "Drone Mission Simulator";

const int GRID_ROWS = 20;
const int GRID_COLS = 20;
const int CELL_SIZE = 30;

const int GRID_OFFSET_X = 50;
const int GRID_OFFSET_Y = 50;

const int SIDEBAR_WIDTH   = 300;
const int SIDEBAR_PADDING = 15;

const int TEXT_SIZE_LARGE  = 20;
const int TEXT_SIZE_MEDIUM = 16;
const int TEXT_SIZE_SMALL  = 12;

const sf::Color BACKGROUND_COLOR = sf::Color(20, 20, 25);
const sf::Color GRID_COLOR       = sf::Color(50, 50, 50);
const sf::Color SIDEBAR_BG       = sf::Color(30, 30, 35);
const sf::Color TEXT_COLOR       = sf::Color::White;

const sf::Color DRONE_COLOR      = sf::Color::Cyan;
const sf::Color PATH_COLOR       = sf::Color(255, 255, 0, 150);
const sf::Color COMPLETED_PATH_COLOR = sf::Color(0, 255, 0, 100);

// Base priority colors
const sf::Color PRIORITY_HIGH   = sf::Color::Red;
const sf::Color PRIORITY_MEDIUM = sf::Color(255, 165, 0);
const sf::Color PRIORITY_LOW    = sf::Color::Green;

// Fuel status colors
const sf::Color FUEL_GOOD     = sf::Color::Green;
const sf::Color FUEL_WARNING = sf::Color::Yellow;
const sf::Color FUEL_CRITICAL= sf::Color::Red;

class Environment {
private:
    sf::RenderWindow window;
    sf::Font font;
    sf::Clock frameClock;
    sf::Clock weatherClock;

    int gridWidth;
    int gridHeight;
=
    WeatherAPI     weather;
    TargetManager  targetManager;
    DroneState     droneState;
    MissionManager missionManager;
    AnalysisReport analysisReport;
=
    int  selectedBaseIndex;     // -1 if none selected
    bool missionActive;         // Mission running or paused
    bool showAnalysisReport;    // Analysis screen toggle

    float weatherUpdateInterval;

    void handleEvents();
    void handleMouseClick(int x, int y);
    void handleKeyPress(sf::Keyboard::Key key);

    void update();
    void updateWeatherData();
    bool updateDroneMovement();
    void checkMissionCompletion();

    float calculateFuelConsumption(float distance);

    void render();
    void renderGrid();
    void renderBases();
    void renderDrone();
    void renderMissionPath();
    void renderSidebar();

    // Sidebar Sections
    void renderDroneInfo(float startY);
    void renderWeatherInfo(float startY);
    void renderMissionInfo(float startY);
    void renderTimestamp(float startY);

    sf::Vector2f gridToPixel(int row, int col) const;
    sf::Vector2i pixelToGrid(int x, int y) const;

    sf::Color getPriorityColor(int priority) const;
    sf::Color getFuelColor(float fuelPercent) const;

    bool isValidGridPosition(int row, int col) const;

    void drawText(const std::string& text,
                  float x, float y,
                  int size,
                  sf::Color color = TEXT_COLOR);

    void drawProgressBar(float percent,
                         float x, float y,
                         float width, float height);

    void drawCircle(float x, float y,
                    float radius,
                    sf::Color fillColor,
                    sf::Color outlineColor = sf::Color::Transparent);

    void initializeWindow();
    bool loadFont();
    void initializeModules();

public:
    Environment();
    ~Environment();

    void run();
    void resetSimulation();
    void toggleMission();
    void displayAnalysisReport();
};

#endif // ENVIRONMENT_H
