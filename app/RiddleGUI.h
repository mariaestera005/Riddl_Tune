#ifndef RIDDLEGUI_H
#define RIDDLEGUI_H

#include <SFML/Graphics.hpp>
#include <string>

#include "Riddle.h"

// Interfata grafica (SFML 3) peste aceeasi logica de joc folosita in consola.
class RiddleGUI {
private:
    GameSession& session;
    sf::RenderWindow window;
    sf::Font font;
    std::size_t selectedRow = 0;
    bool keywordMode = false; // true: raspunsul scris este cuvantul vertical (bonus).
    bool feedbackGood = true;
    std::string input;
    std::string feedback = "Select a row, type the artist and press Enter.";

    void act(std::size_t action);
    void submit();
    void say(const std::string& message, bool good);
    float rowHeight() const;
    void draw();
    void drawCell(const RebusRow& row, std::size_t column, sf::Vector2f pos, float size);
    void drawText(const std::string& text, unsigned size, sf::Vector2f pos, sf::Color color, bool centered = false);

public:
    explicit RiddleGUI(GameSession& gameSession);
    void run();
};

#endif
