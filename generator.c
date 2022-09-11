#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "generator.h"

#define DECKSIZE (7 + 7 + 5 + 5 + 3 + 3 + 1 + 1)
const int TDECK[DECKSIZE] = {
    TLIST7, TLIST7,
    TLIST5, TLIST5, TLIST3, TLIST3, TLIST1, TLIST1
};


void generate_map(
    map_t* map, unsigned int seed
) {
    int ti;
    int tdeck[DECKSIZE];
    memcpy(tdeck, TDECK, DECKSIZE * sizeof(int));

    srand(seed);

    int i = 0;
    for(; i < DECKSIZE; ++i) {
        ti = rand() % (DECKSIZE - i);
        map->terrain[i] = tdeck[ti];
        tdeck[ti] = tdeck[DECKSIZE - 1 - i];
    }
    for(; i < BSIZE; ++i) {
        map->terrain[i] = rand() % TTYPES;
    }
}

void add_near_rec(clue_t* clue, int r, int c, int n) {
    int i = (r * BCOLS + c);
    clue->data[i / 32] |= (1 << (i % 32));
    if(n > 0) {
        if(c > 0) {
            add_near_rec(clue, r, c - 1, n - 1);
        }
        if(r > 0) {
            add_near_rec(clue, r - 1, c, n - 1);
        }
        if(c < BCOLS - 1) {
            add_near_rec(clue, r, c + 1, n - 1);
        }
        if(r < BROWS - 1) {
            add_near_rec(clue, r + 1, c, n - 1);
        }
    }
}

void add_near(clue_t* clue, int i, int n) { 
    int c = i % BCOLS;
    int r = i / BCOLS;
    add_near_rec(clue, r, c, n);
}

void near_clue(
    clue_t* clue, const map_t* map, int terrain, int dist
) {
    for(int i = 0; i < BSIZE; ++i) {
        if(map->terrain[i] == terrain) {
            add_near(clue, i, dist);
        }
    }
}

void add_clues(
    clue_t* res, const clue_t clue1, const clue_t clue2
) {
    for(int i = 0; i < BU32SIZE; ++i) {
        res->data[i] = (clue1.data[i] | clue2.data[i]);
    }
}

void negate_clue(clue_t* clue) {
    for(int i=0; i < BU32SIZE; ++i) {
        clue->data[i] = ~(clue->data[i]);
    }
}

int get_answer_index(const clue_t clue) {
    int answer = 0, done = 0, count, pcount;
    for(int k = 0; k < BU32SIZE; ++k) {
        pcount = __builtin_popcount(clue.data[k]);
        count = __builtin_ctz(~(clue.data[k]));
        answer += count * (1 - done) * (1 - (pcount / 32));
        answer += 32 * (pcount / 32) * (1 - done);
        done += 1 - (pcount / 32);
        done += 1 - ((pcount + 1) / 32);
    }

    return (done == 1) ? answer : -1;
}

void generate_answer_matrix(
    matrix_t* amatrix, const clue_list_t* clist
) {
    memset(amatrix->data, -1, NCLUES * NCLUES * sizeof(int));
    clue_t total;
    int answer;

    amatrix->n = NCLUES;
    for(int i = 0; i < NCLUES; ++i) {
        for(int j = i + 1; j < NCLUES; ++j) {
            memset(&total, 0, sizeof(clue_t));
            add_clues(&total, clist->clue[i], clist->clue[j]);

            answer = get_answer_index(total);
            amatrix->data[i * NCLUES + j] = answer;
            amatrix->data[j * NCLUES + i] = answer;
        }
    }
}

void count_compatible_clues(
    int* row_sums, const matrix_t* amatrix
) {
    for(int r = 0; r < amatrix->n; ++r) {
        row_sums[r] = 0;
        for(int c = 0; c < amatrix->n; ++c) {
            if(amatrix->data[r * amatrix->n + c] > -1) {
                row_sums[r] += 1;
            }
        }
    }
}

void count_unique_answers(array_t* row_sums, const matrix_t* amatrix) {
    int answer, sum;
    int aset[BSIZE + 1];
    for(int r = 0; r < amatrix->n; ++r) {
        sum = 0;
        memset(aset, 0, (BSIZE + 1) * sizeof(int));
        for(int c = 0; c < amatrix->n; ++c) {
            answer = amatrix->data[r * amatrix->n + c] + 1;
            sum += 1 - aset[answer];
            aset[answer] = 1;
        }
        row_sums->data[r] = sum - 1;
    }
}

void generate_minimum_matrix(
    matrix_t* mmatrix, const matrix_t* amatrix,
    const array_t* clue_counts
) {
    int i;
    mmatrix->n = amatrix->n;
    for(int r = 0; r < amatrix->n; ++r) {
        for(int c = 0; c < amatrix->n; ++c) {
            i = r * amatrix->n + c;
            if(amatrix->data[i] > -1) {
                mmatrix->data[i] = MIN(
                    clue_counts->data[r], clue_counts->data[c]
                );
            } else {
                mmatrix->data[i] = 0;
            }
        }
    }
}

void generate_difference_matrix(
    matrix_t* dmatrix, const matrix_t* amatrix,
    const array_t* clue_counts
) {
    int i;
    dmatrix->n = amatrix->n;
    for(int r = 0; r < amatrix->n; ++r) {
        for(int c = 0; c < amatrix->n; ++c) {
            i = r * amatrix->n + c;
            if(amatrix->data[i] > -1) {
                dmatrix->data[i] = abs(
                    clue_counts->data[r] - clue_counts->data[c]
                );
            } else {
                dmatrix->data[i] = 0;
            }
        }
    }
}

void generate_clues(clue_list_t* clist, const map_t* map) {
    int cn = 0;
    memset(clist->clue, 0, NCLUES * sizeof(clue_t));
    for(int i = 0; i < TTYPES; ++i) {
        near_clue(&(clist->clue[cn]), map, i, 3);
        ++cn;
    }

    memcpy(&(clist->clue[cn]), &(clist->clue[cn - TTYPES]), TTYPES);
    for(int i = 0; i < TTYPES; ++i) {
        negate_clue(&(clist->clue[cn]));
        ++cn;
    }

    for(int i = 0; i < TTYPES; ++i) {
        near_clue(&(clist->clue[cn]), map, i, 2);
        ++cn;
    }

    memcpy(&(clist->clue[cn]), &(clist->clue[cn - TTYPES]), TTYPES);
    for(int i = 0; i < TTYPES; ++i) {
        negate_clue(&(clist->clue[cn]));
        ++cn;
    }

    for(int i = 0; i < TTYPES; ++i) {
        near_clue(&(clist->clue[cn]), map, i, 1);
        ++cn;
    }

    memcpy(&(clist->clue[cn]), &(clist->clue[cn - TTYPES]), TTYPES);
    for(int i = 0; i < TTYPES; ++i) {
        negate_clue(&(clist->clue[cn]));
        ++cn;
    }

    clue_t in_clues[TTYPES] = { 0 };
    for(int i = 0; i < TTYPES; ++i) {
        near_clue(&(in_clues[i]), map, i, 0);
    }

    for(int i = 0; i < TTYPES; ++i) {
        for(int j = i+1; j < TTYPES; ++j) {
            add_clues(
                &(clist->clue[cn]), in_clues[i], in_clues[j]
            );
            negate_clue(&(clist->clue[cn]));
            ++cn;
        }
    }
}

enum clue_type get_clue_type(int index) {
    if(index < TTYPES) {
        return NOT_WITHIN_3;
    } else if(index < 2 * TTYPES) {
        return WITHIN_3;
    } else if(index < 3 * TTYPES) {
        return NOT_WITHIN_2;
    } else if(index < 4 * TTYPES) {
        return WITHIN_2;
    } else if(index < 5 * TTYPES) {
        return NOT_WITHIN_1;
    } else if(index < 6 * TTYPES) {
        return WITHIN_1;
    }

    return IN_A_OR_B;
}

/*
int compute_max_answers_eliminated(
    const map_t* map,
    const clue_list_t* clist,
    const array_t* valid_clues,
    const matrix_t* amatrix,
    int cindex
) {
    int max = 0;
    uint32_t x_answers[BU32SIZE], o_answers[BU32SIZE];
    clue_t clue = clist->clue[valid_clues->data[cindex]];

    for(int i = 0; i < BSIZE; ++i) {
        int cshift = i / 32;
        uint32_t cmask = 1 << (i % 32);
        if((clue.data[cshift] & cmask) == 0) {
            continue;
        }

        int xcount = 0, ocount = 0;

        for(int j = 0; j < BU32SIZE; ++j) {
            x_answers[j] = 0;
            o_answers[j] = 0;
        }

        for(int j = 0; j < amatrix->n; ++j) {
            int aindex = amatrix->data[cindex * amatrix->n + j];
            if(aindex < 0) {
                continue;
            }
            clue_t jclue = clist->clue[valid_clues->data[j]];

            int ashift = aindex / 32;
            uint32_t amask = 1 << (aindex % 32);

#ifdef DEBUG
            printf("(%d,%d): cindex=%d, cshift=%d, cmask=%x, aindex=%d, ashift=%d, amask=%x\n", i, j, cindex, cshift, cmask, aindex, ashift, amask);
            printf("\txcount=%d, ocount=%d\txanswrs=%x %x, oanswers=%x %x\n", xcount, ocount, x_answers[0], x_answers[1], o_answers[0], o_answers[1]);
#endif
            if((jclue.data[cshift] & cmask) == 0) {
                if((x_answers[ashift] & amask) == 0) {
                    xcount += 1;
                }
                x_answers[ashift] |= amask;
            } else {
                if((o_answers[ashift] & amask) == 0) {
                    ocount += 1;
                }
                o_answers[ashift] |= amask;
            }
        }

        max = MAX(xcount, MAX(ocount, max));
    }

    return max;
}
*/

void compute_good_clue_pairs(
    clue_pair_list_t* good_pairs,
    const map_t* map,
    const clue_list_t* clist
) { 
    int vclist_data[NCLUES];
    array_t vclist = { 0, vclist_data };
    int acount_data[NCLUES];
    array_t acounts = { 0, acount_data };
    int amatrix_data[NCLUES * NCLUES];
    matrix_t amatrix = { 0, amatrix_data };

#ifdef DEBUG
    printf("generating answer matrix\n");
#endif

    generate_answer_matrix(&amatrix, clist);

#ifdef DEBUG
    for(int i = 0; i < NCLUES; ++i) {
        for(int j = 0; j < NCLUES; ++j) {
            printf(" %02d", amatrix.data[i * NCLUES + j]);
        }
        printf("\n");
    }
#endif

#ifdef DEBUG
    printf("counting unique answers\n");
#endif
    count_unique_answers(
        &acounts, &amatrix
    );

#ifdef DEBUG
    for(int i = 0; i < NCLUES; ++i) {
        printf(" %02d", acounts.data[i]);
    }
    printf("\n");

    printf("culling invalid clues\n");
#endif

    for(int i = 0; i < NCLUES; ++i) {
        if(acounts.data[i] > 0) {
            vclist.data[vclist.size] = i;
            acounts.data[vclist.size] = acounts.data[i];
            vclist.size += 1;
        }
    }

    acounts.size = vclist.size;
    amatrix.n = vclist.size;

#ifdef DEBUG
    printf("acounts\t");
    for(int i = 0; i < vclist.size; ++i) {
        printf(" %02d", acounts.data[i]);
    }
    printf("\n");

    printf("vclist\t");
    for(int i = 0; i < vclist.size; ++i) {
        printf(" %02d", vclist.data[i]);
    }
    printf("\n");

    printf("resizing answer matrix\n");
#endif
    for(int i = 0; i < amatrix.n; ++i) {
        for(int j = 0; j < amatrix.n; ++j) {
            amatrix.data[i * amatrix.n + j] =
                amatrix.data[vclist.data[i] * NCLUES + vclist.data[j]];
        }
    }
/*
    printf("computing max answers eliminated\n");
    int max_elim[NCLUES];
    for(int r = 0; r < vclist.size; ++r) {
        max_elim[r] = compute_max_answers_eliminated(
            map,
            clist,
            &vclist,
            &amatrix,
            r
        );
    }
*/

#ifdef DEBUG
    printf("computing good clue pairs\n");
#endif
    good_pairs->size = 0;
    for(int r = 0; r < vclist.size; ++r) {
#ifdef DEBUG
        printf("\t%d: acounts = %d\n", r, acounts.data[r]);
#endif
        if(
            acounts.data[r] < MINANS
        ) {
            continue;
        }

        if(good_pairs->size >= MAXGCP) {
            break;
        }

        for(int c = 0; c < vclist.size; ++c) {
#ifdef DEBUG
            printf("\t\t%d: acounts = %d\n", c, acounts.data[c]);
#endif
            if(amatrix.data[r * amatrix.n + c] < 0) {
                continue;
            }

            good_pairs->cpair[good_pairs->size].clue1 = vclist.data[r];
            good_pairs->cpair[good_pairs->size].clue2 = vclist.data[c];
            good_pairs->cpair[good_pairs->size].answer =
                amatrix.data[r * amatrix.n + c];
            good_pairs->size += 1;

            if(good_pairs->size >= MAXGCP) {
                break;
            }
        }
    }
}

game_data_t generate_game(int seed) {
    game_data_t g;
    clue_list_t clist;
    clue_pair_t gcp_data[MAXGCP];
    clue_pair_list_t gcps = { 0, gcp_data };
    srand(seed);

    while(gcps.size == 0) {
#ifdef DEBUG
        printf("generating map\n");
#endif
        generate_map(&(g.map), rand());
#ifdef DEBUG
        printf("generating clues\n");
#endif
        generate_clues(&clist, &(g.map));
        compute_good_clue_pairs(
            &gcps,
            &(g.map),
            &clist
        );
    }
#ifdef DEBUG
    printf("organizing game data\n");
#endif

    int ntypes = 0;
    int clue_types[CTYPES * CTYPES] = { 0 };
    int indices[CTYPES * CTYPES][MAXGCP] = { 0 }, found[CTYPES * CTYPES] = { 0 };
    for(int i = 0; i < gcps.size; ++i) {
        int c = get_clue_type(gcps.cpair[i].clue1);
        int r = get_clue_type(gcps.cpair[i].clue2);
        int d = c * CTYPES + r;
        if(found[d] == 0) {
            clue_types[ntypes] = d;
            ntypes += 1;
        }
        indices[d][found[d]] = i;
        found[d] += 1;
    }

    int cti = clue_types[rand() % ntypes];
    int index = indices[cti][rand() % found[cti]];

    g.clue[0] = clist.clue[gcps.cpair[index].clue1];
    g.clue_type[0] = get_clue_type(gcps.cpair[index].clue1);
    g.clue[1] = clist.clue[gcps.cpair[index].clue2];
    g.clue_type[1] = get_clue_type(gcps.cpair[index].clue2);
    g.answer = gcps.cpair[index].answer;

    return g;
}

