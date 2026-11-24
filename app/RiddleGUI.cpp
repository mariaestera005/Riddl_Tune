#include "RiddleGUI.h"

#include <algorithm>
#include <stdexcept>

namespace {
constexpr float GRID_X = 330.f;
constexpr float GRID_Y = 116.f;
constexpr float INPUT_Y = 552.f;
constexpr std::size_t INPUT_LIMIT = 30;
const char* const LABELS[] = {"Hint (Ctrl+H)", "Bonus word (Ctrl+B)", "Reveal (Ctrl+R)", "New round (Ctrl+N)"};

// Paleta: nuante de mov (fundal, casute ascunse) si verde (litere descoperite).
const sf::Color BACKGROUND(22, 14, 36), PANEL(44, 28, 72), HIGHLIGHT(70, 46, 112);
const sf::Color HIDDEN(96, 66, 156), HIDDEN_PIVOT(160, 120, 235);
const sf::Color SHOWN(40, 150, 100), SHOWN_PIVOT(120, 230, 160);
const sf::Color LIGHT(238, 230, 252), MUTED(178, 158, 214), DARK(18, 38, 28);
const sf::Color GREEN(90, 230, 150), PURPLE(184, 140, 250), PINK(232, 124, 196);

sf::FloatRect buttonRect(std::size_t i) { return {{40.f + 235.f * static_cast<float>(i), 640.f}, {215.f, 44.f}}; }
}

RiddleGUI::RiddleGUI(GameSession& gameSession)
    : session(gameSession), window(sf::VideoMode({1000, 700}), "Riddl Tune - Music Rebus") {
    window.setFramerateLimit(60);
    if (!font.openFromFile("assets/font.ttf")) {
        throw std::runtime_error("Could not load assets/font.ttf");
    }
}

void RiddleGUI::run() {
    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            } else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                const auto k = key->code;
                const std::size_t rows = session.getBoard().getRowCount();
                if (k == sf::Keyboard::Key::Escape) window.close();
                else if (k == sf::Keyboard::Key::Up && selectedRow > 0) { --selectedRow; input.clear(); }
                else if (k == sf::Keyboard::Key::Down && selectedRow + 1 < rows) { ++selectedRow; input.clear(); }
                else if (k == sf::Keyboard::Key::Backspace && !input.empty()) input.pop_back();
                else if (k == sf::Keyboard::Key::Enter) submit();
                else if (key->control && k == sf::Keyboard::Key::H) act(0);
                else if ((key->control && k == sf::Keyboard::Key::B) || k == sf::Keyboard::Key::Tab) act(1);
                else if (key->control && k == sf::Keyboard::Key::R) act(2);
                else if (key->control && k == sf::Keyboard::Key::N) act(3);
            } else if (const auto* text = event->getIf<sf::Event::TextEntered>()) {
                if (text->unicode >= 32 && text->unicode < 127 && input.size() < INPUT_LIMIT && !session.isRoundOver()) {
                    input.push_back(static_cast<char>(text->unicode));
                }
            } else if (const auto* click = event->getIf<sf::Event::MouseButtonPressed>()) {
                const sf::Vector2f p(static_cast<float>(click->position.x), static_cast<float>(click->position.y));
                for (std::size_t i = 0; i < 4; ++i) {
                    if (buttonRect(i).contains(p)) act(i);
                }
                const float row = (p.y - GRID_Y) / rowHeight();
                if (row >= 0 && row < static_cast<float>(session.getBoard().getRowCount())) {
                    selectedRow = static_cast<std::size_t>(row);
                    input.clear();
                }
            }
        }
        draw();
    }
}

void RiddleGUI::say(const std::string& message, bool good) {
    feedback = message;
    feedbackGood = good;
}

void RiddleGUI::submit() {
    if (input.empty() || session.isRoundOver()) {
        return;
    }
    if (keywordMode) {
        keywordMode = !session.submitKeywordGuess(input);
        say(keywordMode ? "Wrong bonus word." : "Bonus word solved! Points added.", !keywordMode);
    } else if (session.getBoard().getRow(selectedRow).isSolved()) {
        say("This row is already solved.", false);
    } else {
        const bool ok = session.submitRowGuess(selectedRow + 1, input);
        say(ok ? "Correct!" : "Wrong, try again.", ok);
    }
    input.clear();
}

void RiddleGUI::act(std::size_t action) {
    input.clear();
    if (action == 0) {
        const bool ok = session.requestHint(selectedRow + 1);
        say(ok ? "Hint revealed (small penalty)." : "No hint available for this row.", ok);
    } else if (action == 1) {
        keywordMode = !keywordMode;
        say(keywordMode ? "Type the vertical bonus word." : "Back to row guesses.", true);
    } else if (action == 2) {
        session.revealSolution();
        say("Solution revealed.", true);
    } else if (session.startNewRound()) {
        selectedRow = 0;
        keywordMode = false;
        say("New round started!", true);
    } else {
        say("Could not start a new round.", false);
    }
}

float RiddleGUI::rowHeight() const {
    return std::min(50.f, 400.f / static_cast<float>(std::max<std::size_t>(1, session.getBoard().getRowCount())));
}

void RiddleGUI::drawText(const std::string& text, unsigned size, sf::Vector2f pos, sf::Color color, bool centered) {
    sf::Text label(font, text, size);
    label.setFillColor(color);
    if (centered) {
        const sf::FloatRect b = label.getLocalBounds();
        label.setOrigin({b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f});
    }
    label.setPosition(pos);
    window.draw(label);
}

void RiddleGUI::drawCell(const RebusRow& row, std::size_t column, sf::Vector2f pos, float size) {
    const bool pivot = column == row.getLetterPosition();
    const char letter = row.getDisplayChar(column);
    const bool shown = letter != '_';
    sf::RectangleShape cell({size, size});
    cell.setPosition(pos);
    cell.setFillColor(pivot ? (shown ? SHOWN_PIVOT : HIDDEN_PIVOT) : (shown ? SHOWN : HIDDEN));
    cell.setOutlineThickness(pivot ? 2.f : 0.f);
    cell.setOutlineColor(LIGHT);
    window.draw(cell);
    if (shown) {
        drawText(std::string(1, letter), static_cast<unsigned>(size * 0.55f), pos + sf::Vector2f(size, size) / 2.f,
                 pivot ? DARK : LIGHT, true);
    }
}

void RiddleGUI::draw() {
    window.clear(BACKGROUND);
    drawText("RIDDL TUNE - Music Rebus", 30, {40.f, 16.f}, GREEN);
    drawText("Guess the artist of each song. The highlighted column spells the bonus word.", 16, {40.f, 62.f}, MUTED);
    drawText("Score: " + std::to_string(session.getScore()), 26, {800.f, 20.f}, PURPLE);

    const RebusBoard& board = session.getBoard();
    const float height = rowHeight();
    std::size_t columns = 1;
    for (std::size_t r = 0; r < board.getRowCount(); ++r) {
        columns = std::max(columns, board.getRow(r).getLeftIndent() + board.getRow(r).getLength());
    }
    const float pitch = std::min(48.f, 630.f / static_cast<float>(columns));
    const float size = std::min(pitch - 4.f, height - 6.f);

    for (std::size_t r = 0; r < board.getRowCount(); ++r) {
        const RebusRow& row = board.getRow(r);
        const float y = GRID_Y + static_cast<float>(r) * height;
        if (r == selectedRow) {
            sf::RectangleShape bar({940.f, height - 4.f});
            bar.setPosition({30.f, y});
            bar.setFillColor(HIGHLIGHT);
            window.draw(bar);
        }
        std::string title = row.getTitle();
        if (title.size() > 26) {
            title.resize(23);
            title += "...";
        }
        drawText(std::to_string(r + 1) + ". " + title, 18, {40.f, y + height / 2.f - 14.f},
                 row.isSolved() ? GREEN : LIGHT);
        for (std::size_t c = 0; c < row.getLength(); ++c) {
            drawCell(row, c, {GRID_X + static_cast<float>(row.getLeftIndent() + c) * pitch, y + (height - size) / 2.f},
                     size);
        }
    }
    if (board.getRowCount() > 0) { // A si B marcheaza capetele coloanei verticale.
        const RebusRow& first = board.getRow(0);
        const float x = GRID_X + static_cast<float>(first.getLeftIndent() + first.getLetterPosition()) * pitch;
        drawText("A", 18, {x + size / 2.f - 6.f, GRID_Y - 26.f}, PURPLE);
        drawText("B", 18, {x + size / 2.f - 6.f, GRID_Y + height * static_cast<float>(board.getRowCount())}, PURPLE);
    }

    sf::RectangleShape box({920.f, 46.f});
    box.setPosition({40.f, INPUT_Y});
    box.setFillColor(PANEL);
    box.setOutlineThickness(2.f);
    box.setOutlineColor(keywordMode ? GREEN : PURPLE);
    window.draw(box);
    const std::string prompt = keywordMode ? "Bonus word > " : "Row " + std::to_string(selectedRow + 1) + " > ";
    drawText(session.isRoundOver() ? "Round complete! Score: " + std::to_string(session.getScore()) + " - press Ctrl+N"
                                   : prompt + input + "_",
             22, {56.f, INPUT_Y + 9.f}, session.isRoundOver() ? GREEN : LIGHT);
    drawText(feedback, 18, {44.f, INPUT_Y + 54.f}, feedbackGood ? GREEN : PINK);

    for (std::size_t i = 0; i < 4; ++i) {
        const sf::FloatRect area = buttonRect(i);
        sf::RectangleShape button(area.size);
        button.setPosition(area.position);
        button.setFillColor(PANEL);
        button.setOutlineThickness(2.f);
        button.setOutlineColor(PURPLE);
        window.draw(button);
        drawText(LABELS[i], 18, area.position + area.size / 2.f, LIGHT, true);
    }
    window.display();
}
