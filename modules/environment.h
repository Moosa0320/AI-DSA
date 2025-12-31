#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <string>
#include <vector>

// Include other module headers
#include "WeatherAPI.h"
#include "TargetManager.h"
#include "DroneState.h"
#include "MissionManager.h"
#include "AnalysisReport.h"

// Window settings
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
const int FPS_LIMIT = 60;
const std::string WINDOW_TITLE = "Drone Mission Simulator";

// Grid settings
const int GRID_ROWS = 20;
const int GRID_COLS = 20;
const int CELL_SIZE = 30;
const int GRID_OFFSET_X = 50;      // Left margin
const int GRID_OFFSET_Y = 50;      // Top margin

// UI settings
const int SIDEBAR_WIDTH = 300;
const int SIDEBAR_PADDING = 15;
const int TEXT_SIZE_LARGE = 20;
const int TEXT_SIZE_MEDIUM = 16;
const int TEXT_SIZE_SMALL = 12;

// Colors
const sf::Color GRID_COLOR = sf::Color(50, 50, 50);
const sf::Color BACKGROUND_COLOR = sf::Color(20, 20, 25);
const sf::Color DRONE_COLOR = sf::Color::Cyan;
const sf::Color PATH_COLOR = sf::Color(255, 255, 0, 150);  // Yellow with alpha
const sf::Color COMPLETED_PATH_COLOR = sf::Color(0, 255, 0, 100);  // Green
const sf::Color SIDEBAR_BG = sf::Color(30, 30, 35);
const sf::Color TEXT_COLOR = sf::Color::White;

// Base priority colors
const sf::Color PRIORITY_HIGH = sf::Color::Red;
const sf::Color PRIORITY_MEDIUM = sf::Color(255, 165, 0);  // Orange
const sf::Color PRIORITY_LOW = sf::Color::Green;

// Fuel level colors
const sf::Color FUEL_GOOD = sf::Color::Green;
const sf::Color FUEL_WARNING = sf::Color::Yellow;
const sf::Color FUEL_CRITICAL = sf::Color::Red;

class Environment {
private:
    sf::RenderWindow window;        // Main application window
    sf::Font font;                  // Font for text rendering
    sf::Clock clock;                // Time tracking for updates
    sf::Clock weatherUpdateClock;   // Separate clock for weather API calls

    int gridWidth;                  // Total grid width in pixels
    int gridHeight;                 // Total grid height in pixels
    
    WeatherAPI weather;             // Module 1: Weather data
    TargetManager targetManager;    // Module 3: Target bases
    DroneState droneState;          // Module 4: Drone state & resources
    MissionManager missionManager;  // Module 5: Mission & path management
    AnalysisReport report;          // Module 6: Analysis & reporting

    int selectedBaseIndex;          // Currently selected base (-1 if none)
    bool missionActive;             // Is mission currently running?
    bool showAnalysis;              // Display analysis report?
    float weatherUpdateInterval;    // Seconds between weather updates
    
    // Main event processing loop
    void handleEvents();
    
    // Handle mouse click events (base selection)
    // Parameters: x, y - mouse click coordinates
    void handleMouseClick(int x, int y);
    
    // Handle keyboard input
    // Parameters: key - pressed key code
    void handleKeyPress(sf::Keyboard::Key key);
    
    // Main update function called each frame
    void update();
    
    // Update weather data from API (periodic calls)
    void updateWeatherData();
    
    // Update drone movement along mission path
    // Returns: true if target reached
    bool updateDroneMovement();
    
    // Check if current mission is completed
    void checkMissionCompletion();
    
    // Calculate fuel consumption based on weather
    // Parameters: distance - distance traveled this frame
    // Returns: fuel amount to deduct
    float calculateFuelConsumption(float distance);
    
    // Main render function
    void render();
    
    // Render 2D grid background
    void renderGrid();
    
    // Render all target bases on map
    void renderBases();
    
    // Render drone at current position
    void renderDrone();
    
    // Render mission path (current and completed)
    void renderMissionPath();
    
    // Render sidebar with all info panels
    void renderSidebar();
    
    void renderDroneInfo(float startY);      // Fuel, missions completed
    void renderWeatherInfo(float startY);    // Temp, wind, alerts
    void renderMissionInfo(float startY);    // Targets, current mission
    void renderTimestamp(float startY);      // Current date/time

    // Convert grid coordinates to screen pixel position
    // Parameters: row, col - grid coordinates
    // Returns: pixel position as Vector2f
    sf::Vector2f gridToPixel(int row, int col) const;
    
    // Convert screen pixel position to grid coordinates
    // Parameters: x, y - screen coordinates
    // Returns: grid coordinates as Vector2i (-1,-1 if out of bounds)
    sf::Vector2i pixelToGrid(int x, int y) const;
    
    // Get color based on priority level
    // Parameters: priority - priority value (1=high, 2=medium, 3=low)
    // Returns: corresponding color
    sf::Color getPriorityColor(int priority) const;
    
    // Get color based on fuel percentage
    // Parameters: fuelPercent - fuel level (0-100)
    // Returns: color (green/yellow/red)
    sf::Color getFuelColor(float fuelPercent) const;
    
    // Check if a point is inside grid bounds
    // Parameters: row, col - grid coordinates
    // Returns: true if valid position
    bool isValidGridPosition(int row, int col) const;
    
    // Draw text at specified position
    // Parameters: text, x, y, size, color
    void drawText(const std::string& text, float x, float y, 
                  int size, sf::Color color = TEXT_COLOR);
    
    // Draw progress bar (for fuel display)
    // Parameters: percent (0-100), x, y, width, height
    void drawProgressBar(float percent, float x, float y, 
                        float width, float height);
    
    // Draw a circle (for bases and drone)
    // Parameters: x, y, radius, color, outline
    void drawCircle(float x, float y, float radius, 
                   sf::Color fillColor, sf::Color outlineColor = sf::Color::Transparent);

    // Initialize SFML window and settings
    void initializeWindow();
    
    // Load font from file
    // Returns: true if successful
    bool loadFont();
    
    // Initialize all module objects
    void initializeModules();
    
public:
    // Constructor: Initialize SFML and all modules
    Environment();
    
    // Destructor: Cleanup resources
    ~Environment();
  
    // Main application loop
    // Runs until window is closed
    void run();

    // Reset simulation to initial state
    void resetSimulation();
    
    // Toggle mission active/pause
    void toggleMission();
    
    // Display analysis report
    void displayAnalysisReport();
};

#endif // ENVIRONMENT_H