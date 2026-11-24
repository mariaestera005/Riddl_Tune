#include <exception>
#include <iostream>

#include "RiddleGUI.h"

int main() {
    try {
        GameSession session("assets/tracks.json", "assets/config.json");
        RiddleGUI gui(session);
        gui.run();
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }
    return 0;
}
