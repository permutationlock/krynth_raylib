#include "generator.h"
#include "game_logic.h"

void add_move(
    game_state_t* gstate, move_t move, int askmark, int revealmark
) {
    int player = gstate->turn % 2;
    int opponent = (gstate->turn + 1) % 2;

    if(move.dig == 1) { 
        gstate->marks[move.ask][player] = -1;
    }

    gstate->marks[move.ask][player] = revealmark;

    if(gstate->marks[move.ask][opponent] == 0) {
        gstate->marks[move.ask][opponent] = askmark;
    }

    gstate->turn += 1;
}

int update_game(
    game_state_t* gstate, const move_t move,
    const game_data_t* gdata
) {
    int player = gstate->turn % 2;
    int opponent = (gstate->turn + 1) % 2;

    if(gstate->marks[move.reveal][player] != 0) {
        return 0;
    }

    if(move.dig == 1) {
        int aval = gdata->clue[player].data[move.ask / 32]
            & (1 << (move.ask % 32));

        if(aval != 0) {
            return 0;
        }

        if(move.ask == gdata->answer) {
            gstate->state = ((gstate->turn % 2) == 0) ? 2 : 1;
        }
    }

    int rval = gdata->clue[player].data[move.reveal / 32]
        & (1 << (move.reveal % 32));
    int revealmark = (rval == 0) ? -1 : 1;

    int aval = gdata->clue[opponent].data[move.ask / 32]
        & (1 << (move.ask % 32));
    int askmark = (aval == 0) ? -1 : 1;

    add_move(gstate, move, askmark, revealmark);

    return 1;
}
