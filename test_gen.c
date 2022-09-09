#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>

#include "generator.h"

#define SAMPLES 100000

int main() {
    int good_clues[5];
    int good_clue_pairs[5][5];

    srand(clock());
    clock_t start = clock(), diff;
    for(int i = 0; i < SAMPLES; ++i) { 
        game_data_t game = generate_game(rand());
        enum clue_type c1t = game.clue_type[0];
        enum clue_type c2t = game.clue_type[1];
        if(c1t >= c2t) {
            c1t = c2t;
            c2t = game.clue_type[0];
        }
        good_clues[c1t] += 1;
        good_clues[c2t] += 1;
        good_clue_pairs[c1t][c2t] += 1;
    }
    diff = clock() - start;

    printf("Generated %d sample games:\n\n", SAMPLES);

    int msec = diff * 1000 / CLOCKS_PER_SEC;
    printf("Time taken per 100 samples: %d\n\n", msec/(SAMPLES / 100));

    printf("Good clues by type:\n");
    for(int i = 0; i < 5; ++i) {
        printf(" %06d", i);
    }
    printf("\n");
    for(int i = 0; i < 5; ++i) {
        printf(" %06d", good_clues[i]);
    }
    printf("\n\n");

    printf("Good clue pairs by type:\n");
    for(int i = 0; i < 5; ++i) {
        for(int j = i; j < 5; ++j) {
            printf("  %d, %d", i, j);
        }
    }
    printf("\n");
    for(int i = 0; i < 5; ++i) {
        for(int j = i; j < 5; ++j) {
            printf(" %05d", good_clue_pairs[i][j]);
        }
    }
    printf("\n\n");
}
