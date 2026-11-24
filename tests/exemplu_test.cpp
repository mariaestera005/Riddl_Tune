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
    assert(board.generate(tracks, false));
    assert(board.isValid() && board.getRowCount() == 2);

    const RebusBoard copy(board);
    assert(copy.getRowCount() == board.getRowCount());

    assert(!board.guessRow(0, "Prince"));
    assert(board.guessRow(0, "alpha"));
    assert(!board.isCompleted());

    assert(board.guessKeyword("ab"));
    assert(board.getRow(1).getDisplayChar(board.getRow(1).getLetterPosition()) != '_');
    assert(board.guessRow(1, "Bravo"));
    assert(board.isCompleted());
    return 0;
}
