#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>

#include "generator.h"

#define SAMPLES 10

int main() {
    srand(clock());
    clock_t start = clock(), diff;
    set_board_size(6);
    for(int i = 0; i < SAMPLES; ++i) { 
        int clues[NCLUES] = { 0 };
        int good_clues1[CTYPES] = { 0 };
        board_w_clues_t bwc = generate_board_w_clues(6, 4, 3, 7, 14, 16, rand());
        printf("Board with %d pairs using %d unique clues\n", bwc.npairs, bwc.unique_clues);
        for(int j = 0; j < MAXPPA; ++j) {
            printf("\tand %d answers with at least %d corresponding pair(s)\n", bwc.answers[j], j+1);
        }

        for(int j = 0; j < bwc.npairs; ++j) {
            for(int k = 0; k < 2; ++k) {
                int clue_index = ((int*)&(bwc.good_clue_pairs[j]))[k];
                if(clues[clue_index] != 0) {
                    continue;
                }
                clues[clue_index] = 1;
                clue_data_t cdata = get_clue_data(clue_index);
                enum clue_type ctype = cdata.type;
                good_clues1[ctype] += 1;
            }
        }
        
        printf("\tgood clues by type:\n");
        for(int i = 0; i < CTYPES; ++i) {
            printf("\t %02d", i);
        }
        printf("\n");
        for(int i = 0; i < CTYPES; ++i) {
            printf("\t %02d", good_clues1[i]);
        }
        printf("\n\n");
    }
    diff = clock() - start;

    printf("Generated %d sample games:\n\n", SAMPLES);

    int msec = diff * 1000 / CLOCKS_PER_SEC;
    printf("Time taken per 100 samples: %f\n\n", msec/(SAMPLES / 100.0));
}
