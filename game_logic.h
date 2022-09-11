struct game_state_t {
    int marks[BSIZE][2];
    int turn;
    int state;
};

typedef struct game_state_t game_state_t;

int update_game(
    game_state_t* gstate, int ask,
    const game_data_t* gdata
);
