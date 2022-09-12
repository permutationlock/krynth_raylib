#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>

#include "generator.h"

#define SAMPLES 100000

int main() {
    clue_t clue;
    clue.data[0] = ~0;
    clue.data[1] = ~(1 << 23);

    printf("%d\n", get_answer_index(clue));

    clue.data[0] = ~(1 << 23);
    clue.data[1] = ~0;

    printf("%d\n", get_answer_index(clue));

    clue.data[0] = ~(1 << 23);
    clue.data[1] = ~(1 << 23);

    printf("%d\n", get_answer_index(clue));
}
