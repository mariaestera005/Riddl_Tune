#ifndef RIDDLE_H
#define RIDDLE_H

#include <cstddef>
#include <ostream>
#include <string>
#include <vector>

class SongTrack {
private:
    std::string title;
    std::string artist;

    static std::string removeDiacritics(const std::string& rawText);
    static std::string Uppercase(const std::string& text);

public:
    SongTrack(const std::string& trackTitle, const std::string& mainArtist);

    // Pastreaza doar literele A-Z majuscule, fara diacritice.
    static std::string normalizeText(const std::string& rawText);
    const std::string& getTitle() const { return title; }
    std::string getNormalizedArtist() const;
    bool matchesArtist(const std::string& guess) const;

    friend std::ostream& operator<<(std::ostream& os, const SongTrack& track);
};

class RebusRow {
private:
    SongTrack track;
    std::size_t letterPosition; // Pozitia literei din cuvantul vertical A-B.
    std::size_t leftIndent;     // Casute goale in stanga, pentru aliniere.
    std::vector<bool> revealed;
    bool solved;

    std::size_t findNextHintIndex() const;

public:
    RebusRow(const SongTrack& rowTrack, std::size_t keywordLetterPos);

    bool isSolved() const { return solved; }
    std::size_t getLetterPosition() const { return letterPosition; }
    std::size_t getLeftIndent() const { return leftIndent; }
    std::size_t getLength() const { return revealed.size(); }
    const std::string& getTitle() const { return track.getTitle(); }
    char getDisplayChar(std::size_t index) const; // Litera la index, sau _ daca e ascunsa.
    std::size_t countHiddenLetters() const;

    void setLeftIndent(std::size_t padding) { leftIndent = padding; }
    bool revealHint();
    void revealKeywordLetter();
    bool tryGuess(const std::string& guess);
    void revealAll();

    friend std::ostream& operator<<(std::ostream& os, const RebusRow& row);
};

class RebusBoard {
private:
    std::string keyword;
    std::vector<RebusRow> rows;
    bool keywordGuessed;

    std::size_t computeMaxPivot() const;
    void alignRows();
    static bool isArtistUsed(const std::string& artist, const std::vector<std::string>& used);
    bool buildBacktrack(std::size_t idx, const std::vector<SongTrack>& list,
                        std::vector<bool>& usedTracks, std::vector<std::string>& usedArtists);

public:
    // Rule of Three: copiere, atribuire si destructor.
    explicit RebusBoard(const std::string& targetKeyword);
    RebusBoard(const RebusBoard& other);
    RebusBoard& operator=(const RebusBoard& other);
    ~RebusBoard();

    bool generate(const std::vector<SongTrack>& tracks, bool shuffleTracks);
    bool isValid() const { return !keyword.empty() && rows.size() == keyword.size(); }
    bool isCompleted() const;
    bool isKeywordGuessed() const { return keywordGuessed; }
    std::size_t getRowCount() const { return rows.size(); }
    const RebusRow& getRow(std::size_t rowIndex) const { return rows.at(rowIndex); }
    std::size_t getRowHiddenCount(std::size_t rowIndex) const;

    bool revealRowHint(std::size_t rowIndex);
    bool guessRow(std::size_t rowIndex, const std::string& artistGuess);
    bool guessKeyword(const std::string& keywordGuess);
    void revealAll();

    friend std::ostream& operator<<(std::ostream& os, const RebusBoard& board);
};

class GameSession {
private:
    std::vector<SongTrack> catalog;
    std::vector<std::string> keywords;
    RebusBoard board;
    bool shuffleMode;
    int pointsPerLetter;
    int keywordBonus;
    int hintPenalty;
    int wrongPenalty;
    int score;

    void loadConfigFile(const std::string& configPath);
    void loadTracksFile(const std::string& tracksPath);
    void applyPenalty(int amount) { score = score > amount ? score - amount : 0; }

public:
    GameSession(const std::string& tracksPath, const std::string& configPath);

    bool startNewRound();
    bool submitRowGuess(std::size_t rowNumber, const std::string& artistGuess);
    bool submitKeywordGuess(const std::string& keywordGuess);
    bool requestHint(std::size_t rowNumber);
    void revealSolution() { board.revealAll(); }

    bool isRoundOver() const { return board.isCompleted(); }
    int getScore() const { return score; }
    const RebusBoard& getBoard() const { return board; }

    friend std::ostream& operator<<(std::ostream& os, const GameSession& session);
};

#endif