#include "generator.h"
#include "game_logic.h"

int update_game(
    game_state_t* gstate, int ask,
    const game_data_t* gdata
) {
    int player = gstate->turn % 2;
    int opponent = (gstate->turn + 1) % 2;

    int rval = gdata->clue[player].data[ask / 32]
        & (1 << (ask % 32));

    if(rval == 0) {
        int aval = gdata->clue[player].data[ask / 32]
            & (1 << (ask % 32));

        if(aval != 0) {
            return 0;
        }

        if(ask == gdata->answer) {
            gstate->state = ((gstate->turn % 2) == 0) ? 2 : 1;
        } else {
            gstate->state = ((gstate->turn % 2) == 0) ? 1 : 2;
        }
    }

    if(gstate->marks[ask][opponent] != 0) {
        return 0;
    }

    int aval = gdata->clue[opponent].data[ask / 32]
        & (1 << (ask % 32));
    int askmark = (aval == 0) ? -1 : 1;

    if(gstate->marks[ask][opponent] == 0) {
        gstate->marks[ask][opponent] = askmark;
    }

    return 1;
}
