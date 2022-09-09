struct game_state_t {
    int marks[BSIZE][2];
    int turn;
    int state;
};

typedef struct game_state_t game_state_t;

struct move_t {
    int reveal;
    int ask;
    int dig;
};

typedef struct move_t move_t;

int update_game(
    game_state_t* gstate, const move_t move,
    const game_data_t* gdata
);
