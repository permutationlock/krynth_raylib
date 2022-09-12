#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>

#include "generator.h"

#define SAMPLES 10000

int main() {
    int good_clues1[CTYPES] = { 0 };
    int good_clues2[CTYPES] = { 0 };
    int good_clue_pairs[CTYPES][CTYPES] = { 0 };

    srand(clock());
    clock_t start = clock(), diff;
    for(int i = 0; i < SAMPLES; ++i) { 
        game_data_t game = generate_game(7, 6, rand());
        enum clue_type c1t = game.clue_type[0];
        enum clue_type c2t = game.clue_type[1];
        good_clues1[c1t] += 1;
        good_clues2[c2t] += 1;
        if(c1t >= c2t) {
            c1t = c2t;
            c2t = game.clue_type[0];
        }
        good_clue_pairs[c1t][c2t] += 1;
    }
    diff = clock() - start;

    printf("Generated %d sample games:\n\n", SAMPLES);

    int msec = diff * 1000 / CLOCKS_PER_SEC;
    printf("Time taken per 100 samples: %d\n\n", msec/(SAMPLES / 100));

    printf("Good clues by type:\n");
    for(int i = 0; i < CTYPES; ++i) {
        printf(" %06d", i);
    }
    printf("\n");
    for(int i = 0; i < CTYPES; ++i) {
        printf(" %06d", good_clues1[i]);
    }
    printf("\n");
    for(int i = 0; i < CTYPES; ++i) {
        printf(" %06d", good_clues2[i]);
    }
    printf("\n\n");

    printf("Good clue pairs by type:\n");
    for(int i = 0; i < CTYPES; ++i) {
        for(int j = i; j < CTYPES; ++j) {
            printf("  %d, %d", i, j);
        }
    }
    printf("\n");
    for(int i = 0; i < CTYPES; ++i) {
        for(int j = i; j < CTYPES; ++j) {
            printf(" %05d", good_clue_pairs[i][j]);
        }
    }
    printf("\n\n");
}
