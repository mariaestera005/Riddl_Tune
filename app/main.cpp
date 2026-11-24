#include <exception>
#include <iostream>
#include <limits>
#include <string>

#include "Riddle.h"

int main() {
    try {
        GameSession session("assets/tracks.json", "assets/config.json");
        // Copie a tablei de la inceput (constructor de copiere), afisata la final.
        RebusBoard initialBoard(session.getBoard());

        int option = -1;
        while (option != 0) {
            std::cout << session << "\n1. Guess row | 2. Hint | 3. Guess A-B | 4. Reveal all | 5. New round | 0. Exit\nChoice: ";
            if (!(std::cin >> option)) {
                // La finalul fisierului tastatura.txt iesim din joc.
                if (std::cin.eof()) {
                    break;
                }
                // Intrare invalida: resetam eroarea si ignoram restul liniei.
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }

            if (option == 0) {
                break;
            } else if (option == 1) {
                std::size_t row = 0;
                std::string guess;
                std::cout << "Row & Artist: ";
                if (std::cin >> row) {
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::getline(std::cin, guess);
                    std::cout << (session.submitRowGuess(row, guess) ? "Correct!\n\n" : "Wrong!\n\n");
                }
            } else if (option == 2) {
                std::size_t row = 0;
                std::cout << "Row: ";
                if (std::cin >> row) {
                    std::cout << (session.requestHint(row) ? "Hint revealed!\n\n" : "Unavailable.\n\n");
                }
            } else if (option == 3) {
                std::string kw;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "A-B Keyword: ";
                std::getline(std::cin, kw);
                std::cout << (session.submitKeywordGuess(kw) ? "Solved A-B! Bonus added!\n\n" : "Wrong!\n\n");
            } else if (option == 4) {
                session.revealSolution();
            } else if (option == 5 && session.startNewRound()) {
                // Atribuire (operator=) pentru noua tabla initiala.
                initialBoard = session.getBoard();
                std::cout << "New round started!\n\n";
            }

            if (session.isRoundOver()) {
                std::cout << session << "\nCompleted! Final score: " << session.getScore() << "\n";
                if (initialBoard.isValid()) {
                    std::cout << "\nInitial board was:\n" << initialBoard;
                }
                break;
            }
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }
    return 0;
}