#include <SFML/Graphics.hpp>

int main()
{
    const int windowWidth  = 800;
    const int windowHeight = 600;
    const int cellSize = 40;

    const int cols = windowWidth / cellSize;
    const int rows = windowHeight / cellSize;

    sf::RenderWindow window(
        sf::VideoMode({windowWidth, windowHeight}),
        "SFML 2.6 Grid"
    );

    sf::VertexArray grid(sf::PrimitiveType::Lines);

    // Vertical lines
    for (int i = 0; i <= cols; ++i)
    {
        float x = i * cellSize;

        sf::Vertex v1;
        v1.position = {x, 0.f};
        v1.color = sf::Color::White;

        sf::Vertex v2;
        v2.position = {x, static_cast<float>(windowHeight)};
        v2.color = sf::Color::White;

        grid.append(v1);
        grid.append(v2);
    }

    // Horizontal lines
    for (int i = 0; i <= rows; ++i)
    {
        float y = i * cellSize;

        sf::Vertex v1;
        v1.position = {0.f, y};
        v1.color = sf::Color::White;

        sf::Vertex v2;
        v2.position = {static_cast<float>(windowWidth), y};
        v2.color = sf::Color::White;

        grid.append(v1);
        grid.append(v2);
    }

    while (window.isOpen())
    {
        while (auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        window.clear(sf::Color(30, 30, 30));
        window.draw(grid);
        window.display();
    }

    return 0;
}
