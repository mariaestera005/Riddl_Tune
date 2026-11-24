#include "Riddle.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <random>
#include <stdexcept>

#include "json.hpp"

namespace {
constexpr std::size_t CELL_WIDTH = 3;
constexpr std::size_t ROW_LABEL_WIDTH = 5;
constexpr int POINTS_PER_LETTER = 10;
constexpr int KEYWORD_BONUS = 50;
constexpr int HINT_PENALTY = 5;
constexpr int WRONG_PENALTY = 3;
}

// ***** SongTrack *****

std::string SongTrack::removeDiacritics(const std::string& rawText) {
    // Secventele UTF-8 ale diacriticelor si echivalentele ASCII, pe aceleasi pozitii.
    const std::vector<std::string> from = {
        "\xC4\x83", "\xC4\x82", "\xC3\xA2", "\xC3\x82",
        "\xC3\xAE", "\xC3\x8E", "\xC8\x99", "\xC8\x98",
        "\xC5\x9F", "\xC5\x9E", "\xC8\x9B", "\xC8\x9A",
        "\xC5\xA3", "\xC5\xA2", "\xC3\xA9", "\xC3\x89"
    };
    const std::vector<std::string> to = {
        "a", "A", "a", "A",
        "i", "I", "s", "S",
        "s", "S", "t", "T",
        "t", "T", "e", "E"
    };
    std::string text = rawText;
    for (std::size_t i = 0; i < from.size(); ++i) {
        std::size_t pos = 0;
        while ((pos = text.find(from[i], pos)) != std::string::npos) {
            text.replace(pos, from[i].size(), to[i]);
            pos += to[i].size();
        }
    }
    return text;
}

std::string SongTrack::Uppercase(const std::string& text) {
    std::string clean;
    for (std::size_t i = 0; i < text.size(); ++i) {
        const auto ch = static_cast<unsigned char>(text[i]);
        if (std::isalpha(ch) != 0) {
            clean.push_back(static_cast<char>(std::toupper(ch)));
        }
    }
    return clean;
}

std::string SongTrack::normalizeText(const std::string &rawText) {
    return Uppercase(removeDiacritics(rawText));
}

SongTrack::SongTrack(const std::string& trackTitle, const std::string& mainArtist)
    : title(removeDiacritics(trackTitle)), artist(mainArtist) {}

std::string SongTrack::getNormalizedArtist() const { return normalizeText(artist); }

bool SongTrack::matchesArtist(const std::string& guess) const {
    const std::string cleanGuess = normalizeText(guess);
    return !cleanGuess.empty() && cleanGuess == getNormalizedArtist();
}

std::ostream& operator<<(std::ostream& os, const SongTrack& track) {
    return os << '"' << track.title << '"';
}

//***** RebusRow *****

RebusRow::RebusRow(const SongTrack& rowTrack, std::size_t keywordLetterPos)
    : track(rowTrack), letterPosition(keywordLetterPos), leftIndent(0),
      revealed(rowTrack.getNormalizedArtist().size(), false), solved(false) {
    // Litera din cuvantul vertical trebuie sa existe in raspuns.
    if (revealed.empty() || letterPosition >= revealed.size()) {
        throw std::invalid_argument("Invalid pivot index for RebusRow.");
    }
}

std::size_t RebusRow::findNextHintIndex() const {
    for (std::size_t i = 0; i < revealed.size(); ++i) {
        if (i != letterPosition && !revealed[i]) {
            return i;
        }
    }
    return !revealed[letterPosition] ? letterPosition : revealed.size();
}

char RebusRow::getDisplayChar(std::size_t index) const {
    return revealed[index] ? track.getNormalizedArtist()[index] : '_';
}

std::size_t RebusRow::countHiddenLetters() const {
    return static_cast<std::size_t>(std::count(revealed.begin(), revealed.end(), false));
}

bool RebusRow::revealHint() {
    const std::size_t nextIdx = solved ? revealed.size() : findNextHintIndex();
    if (nextIdx >= revealed.size()) {
        return false;
    }
    revealed[nextIdx] = true;
    solved = (countHiddenLetters() == 0);
    return true;
}

void RebusRow::revealKeywordLetter() {
    revealed[letterPosition] = true;
    solved = (countHiddenLetters() == 0);
}

bool RebusRow::tryGuess(const std::string& guess) {
    if (solved || track.matchesArtist(guess)) {
        revealAll();
        return true;
    }
    return false;
}

void RebusRow::revealAll() {
    std::fill(revealed.begin(), revealed.end(), true);
    solved = true;
}

std::ostream& operator<<(std::ostream& os, const RebusRow& row) {
    os << std::string(row.leftIndent * CELL_WIDTH, ' ');
    for (std::size_t i = 0; i < row.revealed.size(); ++i) {
        const char ch = row.getDisplayChar(i);
        if (i == row.letterPosition) {
            // [ ] marcheaza coloana cuvantului vertical A-B.
            os << '[' << ch << ']';
        } else {
            os << ' ' << ch << ' ';
        }
    }
    return os << " | Track: " << row.track;
}

// ***** RebusBoard *****

RebusBoard::RebusBoard(const std::string& targetKeyword)
    : keyword(SongTrack::normalizeText(targetKeyword)), keywordGuessed(false) {}

// Rule of Three: copiere, atribuire si destructor pentru tabla de joc.
RebusBoard::RebusBoard(const RebusBoard& other)
    : keyword(other.keyword), rows(other.rows), keywordGuessed(other.keywordGuessed) {}

RebusBoard& RebusBoard::operator=(const RebusBoard& other) {
    if (this != &other) {
        keyword = other.keyword;
        rows = other.rows;
        keywordGuessed = other.keywordGuessed;
    }
    return *this;
}

RebusBoard::~RebusBoard() {
    rows.clear();
    keyword.clear();
}

std::size_t RebusBoard::computeMaxPivot() const {
    // Cea mai din dreapta pozitie a literei A-B; baza pentru aliniere.
    std::size_t maxPivot = 0;
    for (std::size_t i = 0; i < rows.size(); ++i) {
        maxPivot = std::max(maxPivot, rows[i].getLetterPosition());
    }
    return maxPivot;
}

void RebusBoard::alignRows() {
    const std::size_t maxPivot = computeMaxPivot();
    for (std::size_t i = 0; i < rows.size(); ++i) {
        rows[i].setLeftIndent(maxPivot - rows[i].getLetterPosition());
    }
}

bool RebusBoard::isArtistUsed(const std::string& artist, const std::vector<std::string>& used) const {
    return std::find(used.begin(), used.end(), artist) != used.end();
}

// Backtracking recursiv: alegem cate o piesa pentru fiecare litera a cuvantului
// vertical; daca o alegere blocheaza urmatoarele randuri, o anulam si incercam alta.
bool RebusBoard::buildBacktrack(std::size_t idx, const std::vector<SongTrack>& list,
                                std::vector<bool>& usedTracks, std::vector<std::string>& usedArtists) {
    if (idx == keyword.size()) {
        return true;
    }
    for (std::size_t i = 0; i < list.size(); ++i) {
        if (!usedTracks[i]) {
            const std::string artist = list[i].getNormalizedArtist();
            const std::size_t pos = artist.find(keyword[idx]);

            if (pos != std::string::npos && !isArtistUsed(artist, usedArtists)) {
                usedTracks[i] = true;
                usedArtists.push_back(artist);
                rows.emplace_back(list[i], pos);

                if (buildBacktrack(idx + 1, list, usedTracks, usedArtists)) {
                    return true;
                }

                rows.pop_back();
                usedArtists.pop_back();
                usedTracks[i] = false;
            }
        }
    }
    return false;
}

bool RebusBoard::generate(const std::vector<SongTrack>& tracks, bool shuffleTracks) {
    rows.clear();
    keywordGuessed = false;
    if (keyword.empty() || tracks.empty()) {
        return false;
    }
    std::vector<SongTrack> songs = tracks;
    if (shuffleTracks) {
        // Amestecam piesele ca fiecare runda sa fie diferita.
        std::mt19937 gen(std::random_device{}());
        std::shuffle(songs.begin(), songs.end(), gen);
    }
    std::vector<bool> usedTracks(songs.size(), false);
    std::vector<std::string> usedArtists;
    if (!buildBacktrack(0, songs, usedTracks, usedArtists)) {
        rows.clear();
        return false;
    }
    alignRows();
    return true;
}

bool RebusBoard::isCompleted() const {
    if (!isValid()) {
        return false;
    }
    for (std::size_t i = 0; i < rows.size(); ++i) {
        if (!rows[i].isSolved()) {
            return false;
        }
    }
    return true;
}

std::size_t RebusBoard::getRowHiddenCount(std::size_t rowIndex) const {
    return (rowIndex < rows.size()) ? rows[rowIndex].countHiddenLetters() : 0;
}

bool RebusBoard::revealRowHint(std::size_t rowIndex) {
    return (rowIndex < rows.size()) && rows[rowIndex].revealHint();
}

bool RebusBoard::guessRow(std::size_t rowIndex, const std::string& artistGuess) {
    return (rowIndex < rows.size()) && rows[rowIndex].tryGuess(artistGuess);
}

bool RebusBoard::guessKeyword(const std::string& keywordGuess) {
    const std::string clean = SongTrack::normalizeText(keywordGuess);
    if (!clean.empty() && clean == keyword) {
        // Dupa ghicire, completam litera A-B in fiecare rand.
        keywordGuessed = true;
        for (std::size_t i = 0; i < rows.size(); ++i) {
            rows[i].revealKeywordLetter();
        }
        return true;
    }
    return false;
}

void RebusBoard::revealAll() {
    for (std::size_t i = 0; i < rows.size(); ++i) {
        rows[i].revealAll();
    }
    keywordGuessed = true;
}

std::ostream& operator<<(std::ostream& os, const RebusBoard& board) {
    if (!board.isValid()) {
        return os << "Rebus board not generated.\n";
    }
    const std::size_t offset = ROW_LABEL_WIDTH + board.computeMaxPivot() * CELL_WIDTH;
    os << std::string(offset, ' ') << "A\n";
    for (std::size_t i = 0; i < board.rows.size(); ++i) {
        os << std::setw(2) << (i + 1) << ". " << board.rows[i] << '\n';
    }
    return os << std::string(offset, ' ') << "B\n";
}

// ***** GameSession *****

GameSession::GameSession(const std::string& tracksPath, const std::string& configPath)
    : board(""), shuffleMode(true), pointsPerLetter(POINTS_PER_LETTER), keywordBonus(KEYWORD_BONUS),
      hintPenalty(HINT_PENALTY), wrongPenalty(WRONG_PENALTY), score(0) {
    loadConfigFile(configPath);
    loadTracksFile(tracksPath);
    if (!startNewRound()) {
        throw std::runtime_error("Could not generate a valid rebus board.");
    }
}

void GameSession::loadConfigFile(const std::string& configPath) {
    std::ifstream file(configPath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open config file: " + configPath);
    }
    const nlohmann::json cfg = nlohmann::json::parse(file);
    for (const auto& item : cfg.at("keywords")) {
        const std::string kw = SongTrack::normalizeText(item.get<std::string>());
        if (!kw.empty()) {
            keywords.push_back(kw);
        }
    }
    if (keywords.empty()) {
        throw std::runtime_error("No valid keywords in config.json.");
    }
    shuffleMode = cfg.value("shuffle", true);
    pointsPerLetter = cfg.value("pointsPerLetter", POINTS_PER_LETTER);
    keywordBonus = cfg.value("keywordBonus", KEYWORD_BONUS);
    hintPenalty = cfg.value("hintPenalty", HINT_PENALTY);
    wrongPenalty = cfg.value("wrongPenalty", WRONG_PENALTY);
}

void GameSession::loadTracksFile(const std::string& tracksPath) {
    std::ifstream file(tracksPath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open tracks file: " + tracksPath);
    }
    for (const auto& item : nlohmann::json::parse(file)) {
        catalog.emplace_back(item.at("title").get<std::string>(),
                             item.at("artists").at(0).at("name").get<std::string>());
    }
    if (catalog.empty()) {
        throw std::runtime_error("No tracks in tracks.json.");
    }
}

bool GameSession::startNewRound() {
    std::vector<std::string> songKeywords = keywords;
    if (shuffleMode && songKeywords.size() > 1) {
        std::mt19937 gen(std::random_device{}());
        std::shuffle(songKeywords.begin(), songKeywords.end(), gen);
    }
    for (std::size_t i = 0; i < songKeywords.size(); ++i) {
        RebusBoard nextBoard(songKeywords[i]);
        if (nextBoard.generate(catalog, shuffleMode)) {
            board = nextBoard;
            score = 0;
            return true;
        }
    }
    return false;
}

bool GameSession::submitRowGuess(std::size_t rowNumber, const std::string& artistGuess) {
    if (rowNumber == 0 || rowNumber > board.getRowCount()) {
        return false;
    }
    // Punctele se acorda pentru literele ascunse inainte de ghicire.
    const std::size_t hidden = board.getRowHiddenCount(rowNumber - 1);
    if (hidden == 0) {
        return true;
    }
    const bool hadBonus = board.isKeywordGuessed();
    if (board.guessRow(rowNumber - 1, artistGuess)) {
        score += static_cast<int>(hidden) * pointsPerLetter + ((board.isCompleted() && !hadBonus) ? keywordBonus : 0);
        return true;
    }
    applyPenalty(wrongPenalty);
    return false;
}

bool GameSession::submitKeywordGuess(const std::string& keywordGuess) {
    if (board.isKeywordGuessed()) {
        return true;
    }
    if (board.guessKeyword(keywordGuess)) {
        score += keywordBonus;
        return true;
    }
    applyPenalty(wrongPenalty);
    return false;
}

bool GameSession::requestHint(std::size_t rowNumber) {
    if (rowNumber == 0 || rowNumber > board.getRowCount() || !board.revealRowHint(rowNumber - 1)) {
        return false;
    }
    applyPenalty(hintPenalty);
    if (board.isCompleted() && !board.isKeywordGuessed()) {
        score += keywordBonus;
    }
    return true;
}

std::ostream& operator<<(std::ostream& os, const GameSession& session) {
    return os << "********** RIDDL TUNE <3 **********\nScore: " << session.score << " pts\n\n" << session.board;
}