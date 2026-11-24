// Teste simple cu assert, fara framework (ramura main nu include unul).
// assert() dispare cand NDEBUG e definit, deci il dezactivam ca testele sa ruleze mereu.
#undef NDEBUG
#include <cassert>
#include <vector>

#include "Riddle.h"

int main() {
    assert(SongTrack::normalizeText("Dragostea din Tei!") == "DRAGOSTEADINTEI");

    const SongTrack track("Titlu", "Madonna");
    assert(track.matchesArtist("madonna!"));
    assert(!track.matchesArtist("Prince"));
    assert(!track.matchesArtist("123"));

    const std::vector<SongTrack> tracks = {SongTrack("T1", "Alpha"), SongTrack("T2", "Bravo")};
    RebusBoard board("AB");
    const bool generated = board.generate(tracks, false);
    assert(generated);
    assert(board.isValid() && board.getRowCount() == 2);

    const RebusBoard copy(board);
    assert(copy.getRowCount() == board.getRowCount());

    const bool wrongGuess = board.guessRow(0, "Prince");
    assert(!wrongGuess);
    const bool rightGuess = board.guessRow(0, "alpha");
    assert(rightGuess);
    assert(!board.isCompleted());

    const bool keywordOk = board.guessKeyword("ab");
    assert(keywordOk);
    assert(board.getRow(1).getDisplayChar(board.getRow(1).getLetterPosition()) != '_');
    const bool lastGuess = board.guessRow(1, "Bravo");
    assert(lastGuess);
    assert(board.isCompleted());
    return 0;
}
