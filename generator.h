#ifndef KRYNTH_GENERATOR_HPP
#define KRYNTH_GENERATOR_HPP

#define BROWS 8
#define BCOLS 8
#define BSIZE BROWS * BCOLS
#define BU8SIZE (BSIZE / 8)
#define BU32SIZE (1 + ((BSIZE - 1) / 32))
#define TTYPES 7
#define NCLUES 49
#define AMSIZE NCLUES * NCLUES
#define MAXGCP 8
#define MINANS 8

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define TLIST1 0
#define TLIST2 TLIST1, 1
#define TLIST3 TLIST2, 2
#define TLIST4 TLIST3, 3
#define TLIST5 TLIST4, 4
#define TLIST6 TLIST5, 5
#define TLIST7 TLIST6, 6

// types

enum clue_type {
    NOT_WITHIN_3,
    NOT_WITHIN_2,
    NOT_WITHIN_1,
    WITHIN_1,
    IN_A_OR_B
};

struct clue_t {
    unsigned int data[BU32SIZE];
};

typedef struct clue_t clue_t;

struct map_t {
    int terrain[BSIZE];
};

typedef struct map_t map_t;

struct clue_list_t {
    clue_t clue[NCLUES];
};

typedef struct clue_list_t clue_list_t;

struct matrix_t {
    int n;
    int* data;
};

typedef struct matrix_t matrix_t;

struct array_t {
    int size;
    int* data;
};

typedef struct array_t array_t;

struct clue_pair_t {
    int clue1;
    int clue2;
    int answer;
};

typedef struct clue_pair_t clue_pair_t;

struct clue_pair_list_t {
    int size;
    clue_pair_t* cpair;
};

typedef struct clue_pair_list_t clue_pair_list_t;

struct game_data_t {
    map_t map;
    enum clue_type clue_type[2];
    clue_t clue[2];
    int answer;
};

typedef struct game_data_t game_data_t;


// functions

void generate_map(map_t* map, unsigned int seed);

void add_near_rec(clue_t* clue, int r, int c, int n);

void add_near(clue_t* clue, int i, int n);

void near_clue(
    clue_t* clue, const map_t* map, int terrain, int dist
);

void add_clues(
    clue_t* res, const clue_t clue1, const clue_t clue2
);

void negate_clue(clue_t* clue);

int get_answer_index(const clue_t clue);

void generate_answer_matrix(
    matrix_t* amatrix, const clue_list_t* clist
);

void count_compatible_clues(
    int* row_sums, const matrix_t* amatrix
);

void count_unique_answers(
    array_t* row_sums, const matrix_t* amatrix
);

void generate_minimum_matrix(
    matrix_t* mmatrix, const matrix_t* amatrix,
    const array_t* clue_counts
);

void generate_difference_matrix(
    matrix_t* dmatrix, const matrix_t* amatrix,
    const array_t* clue_counts
);

void generate_clues(clue_list_t* clist, const map_t* map);

int compute_max_answers_eliminated(
    const map_t* map,
    const clue_list_t* clist,
    const array_t* valid_clues,
    const matrix_t* amatrix,
    int clue
);

void compute_good_clue_pairs(
    clue_pair_list_t* good_pairs,
    const map_t* map,
    const clue_list_t* clist
);

game_data_t generate_game(int seed);

#endif // KRYNTH_GENERATOR_HPP
