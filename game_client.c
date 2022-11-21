#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include <time.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "generator.h"
#include "game_logic.h"

#define BSIZE_START 3
#define MARKS_START 5
#define MARKS_GAIN 6
#define MIN_ANS_START 2
#define TTYPES_START 3

extern int brows;
extern int bcols;
extern int bsize;
extern int bu32size;

game_data_t game_data;
game_state_t game_state;

int level, marks, ttypes, min_ans;

int sradius = 25;
int cwidth = 60;

int mouse_has_released;

void change_board_size(int n) {
    set_board_size(n);
    cwidth = 480 / n;
    sradius = cwidth / 2 - 5;
}

void drawTriangle(Vector2 v, int radius) {
    v.y -= 3;
    DrawPoly(v, 3, radius+2, 0.0, DARKGRAY);
    DrawPoly(v, 3, radius-4, 0.0, GOLD);
}

void drawTriangle2(Vector2 v, int radius) {
    v.y += 3;
    DrawPoly(v, 3, radius+2, 180.0, DARKGRAY);
    DrawPoly(v, 3, radius-4, 180.0, RED);
}

void drawSquare(Vector2 v, int radius) {
    DrawPoly(v, 4, radius, 0.0, DARKGRAY);
    DrawPoly(v, 4, radius-4, 0.0, ORANGE);
}

void drawCircle(Vector2 v, int radius) { 
    DrawCircleV(v, radius-5, DARKGRAY);
    DrawCircleV(v, radius-8, SKYBLUE);
}

void drawPentagon(Vector2 v, int radius) {
    DrawPoly(v, 5, radius-2, 180.0, DARKGRAY);
    DrawPoly(v, 5, radius-5, 180.0, BLUE);
}

void drawHexagon(Vector2 v, int radius) {
    DrawPoly(v, 6, radius-3, 0.0, DARKGRAY);
    DrawPoly(v, 6, radius-6, 0.0, GREEN);
}

void drawDiamond(Vector2 v, int radius) {
    DrawPoly(v, 4, radius, 45.0, DARKGRAY);
    DrawPoly(v, 4, radius-4, 45.0, VIOLET);
}

void drawTerrain(Vector2 v, int radius, int terrain) {
    switch(terrain) {
        case 0:
            drawCircle(v, radius);
            break;
        case 1:
            drawTriangle(v, radius);
            break;
        case 2:
            drawSquare(v, radius);
            break;
        case 3:
            drawPentagon(v, radius);
            break;
        case 4:
            drawHexagon(v, radius);
            break;
        case 5:
            drawTriangle2(v, radius);
            break;
        case 6:
            drawDiamond(v, radius);
            break;
    }
}

int drawClueDescription(int player, int starty) {
    clue_data_t clue_data = game_data.clue_data[player];
    int y = starty + 18, fsize = 20, radius = 25, width = 60;
    if(clue_data.type != IN_A_OR_B) {
        int x = 60;
        int not = 0, dist = 0;
        if(clue_data.type == NOT_WITHIN_3) { not = 1; dist = 3; };
        if(clue_data.type == WITHIN_3) { not = 0; dist = 3; };
        if(clue_data.type == NOT_WITHIN_2) { not = 1; dist = 2; };
        if(clue_data.type == WITHIN_2) { not = 0; dist = 2; };
        if(clue_data.type == NOT_WITHIN_1) { not = 1; dist = 1; };
        if(clue_data.type == WITHIN_1) { not = 0; dist = 1; };
        
        const char* treasure_str = "the treasure is";
        const char* not_str = " not";

        DrawText(treasure_str, x, y, fsize, BLACK);
        x += MeasureText(treasure_str, fsize);
        if(not) {
            DrawText(not_str, x, y, fsize, BLACK);
            x += MeasureText(not_str, fsize);
        }
        char color_buff[32] = { 0 };
        sprintf(color_buff, " within %d of", dist);
        DrawText(color_buff, x, y, fsize, BLACK);
        x += MeasureText(color_buff, fsize);
        Vector2 sv = { x + (width / 2), y + radius - 15 };
        drawTerrain(sv, radius, clue_data.color1);
    } else { 
        int x = 80;
        const char* in_str = "the treasure is in";
        const char* or_str = "or";
        DrawText(in_str, x, y, fsize, BLACK);
        x += MeasureText(in_str, fsize);
        Vector2 sv = { x + (width / 2), y + radius - 15 };
        drawTerrain(sv, radius, clue_data.color1);
        x += width;
        DrawText(or_str, x, y, fsize, BLACK);
        x += MeasureText(or_str, fsize);
        sv.x = x + (width / 2);
        drawTerrain(sv, radius, clue_data.color2);
    }
    return width;
}

void drawTile(int i, int value, int starty) {
    Vector2 v = {
        (i % bcols) * cwidth + cwidth / 2,
        starty + ((i / bcols)) * cwidth + cwidth / 2
    };
    drawTerrain(v, sradius, value);
}

void drawMark(int i, int mark, int starty) {
    if(mark == 1) {
        int soffset = (cwidth / 2) - 6;
        int eoffset = (cwidth / 2) + 6;
        Vector2 start = { 
            (i % bcols) * cwidth + soffset,
            starty + (i / bcols) * cwidth + soffset,
        };
        Vector2 end = {
            (i % bcols) * cwidth + eoffset,
            starty + (i / bcols) * cwidth + eoffset,
        };
        DrawLineEx(start, end, 5.0, BLACK);
        start.x = (i % bcols) * cwidth + soffset;
        start.y = starty + (i / bcols) * cwidth + eoffset;
        end.x = (i % bcols) * cwidth + eoffset;
        end.y = starty + (i / bcols) * cwidth + soffset;
        DrawLineEx(start, end, 5.0, BLACK);
    } else if(mark == -1) {
        Vector2 center = { 
            (i % bcols) * cwidth + cwidth / 2,
            starty + (i / bcols) * cwidth + cwidth / 2
        };
        DrawCircleV(center, 7, BLACK);
    }
}

int getClue(int i, int j) {
    return game_data.clue[j].data[i / 32] & (1 << (i % 32));
}

void drawBorder(int i, int j, Color color, int starty) {
    if(getClue(i, j) == 0) {
        if(i % bcols == 0 || getClue(i - 1, j) != 0) {
            Vector2 start = {
                (i % bcols) * cwidth,
                starty + (i / bcols) * cwidth
            };
            Vector2 end = {
                (i % bcols) * cwidth,
                starty + (i / bcols) * cwidth + cwidth
            };
            DrawLineEx(start, end, 3.0, color);
        }
        if(i % bcols == (bcols - 1) || getClue(i + 1, j) != 0) {
            Vector2 start = {
                (i % bcols) * cwidth + cwidth,
                starty + (i / bcols) * cwidth
            };
            Vector2 end = {
                (i % bcols) * cwidth + cwidth,
                starty + (i / bcols) * cwidth + cwidth
            };
            DrawLineEx(start, end, 3.0, color);
        }
        if(i / bcols == 0 || getClue(i - bcols, j) != 0) {
            Vector2 start = {
                (i % bcols) * cwidth,
                starty + (i / bcols) * cwidth
            };
            Vector2 end = {
                (i % bcols) * cwidth + cwidth,
                starty + (i / bcols) * cwidth
            };
            DrawLineEx(start, end, 3.0, color);
        }
        if(i / bcols == (brows - 1) || getClue(i + bcols, j) != 0) {
            Vector2 start = {
                (i % bcols) * cwidth,
                starty + (i / bcols) * cwidth + cwidth
            };
            Vector2 end = {
                (i % bcols) * cwidth + cwidth,
                starty + (i / bcols) * cwidth + cwidth
            };
            DrawLineEx(start, end, 3.0, color);
        }
    }
}

int drawBoard(int starty) {
    for(int i = 0; i < bsize; ++i) {
        Vector2 v = {
            (i % bcols) * cwidth,
            starty + (i / bcols) * cwidth
        };
        Vector2 s = { cwidth, cwidth };
        if(getClue(i,0) == 0) {
            DrawRectangleV(v, s, LIGHTGRAY);
        } else if(game_state.state != 0 && getClue(i, 1) == 0) {
            DrawRectangleV(v, s, GRAY);
        }
    }
    {
        int mx = GetMouseX(), my = GetMouseY() - starty;
        Vector2 v, s = { cwidth, cwidth };
        if(my >= 0 && my < bcols * cwidth) {
            v.x = cwidth * (mx / cwidth);
            v.y = starty + cwidth * (my / cwidth);
            DrawRectangleV(v, s, BEIGE);
        }
        if(game_state.state == 1) {
            v.x = cwidth * (game_data.answer % bcols);
            v.y = starty + cwidth * (game_data.answer / bcols);
            DrawRectangleV(v, s, MAROON);
        } else if(game_state.state == 2) {
            v.x = cwidth * (game_data.answer % bcols);
            v.y = starty + cwidth * (game_data.answer / bcols);
            DrawRectangleV(v, s, LIME);
        }
    }

    for(int i = 0; i < bsize; ++i) {
        drawBorder(i, 0, DARKGRAY, starty);
        if(game_state.state != 0) {
            drawBorder(i, 1, DARKGRAY, starty);
        }
        drawTile(i, game_data.map.terrain[i], starty);
        drawMark(i, game_state.marks[i][1], starty);
    }
    return brows * cwidth;
}

void drawEndMessage(int y) {
    int x = -100;
    char* str;
    if(game_state.state == 2) {
        x = 25;
        str = "LEVEL COMPLETE! (CLICK TO CONTINUE)";
    } else if(game_state.state == 1) {
        x = 60;
        str = "GAME OVER! (CLICK TO CONTINUE)";
    } else {
        return;
    }
    int width = MeasureText(str, 20);
    Vector2 v = { x - 2, y - 2 };
    Vector2 sq = { width + 4, 20 + 3 };
    DrawRectangleV(v, sq, RAYWHITE); 
    DrawText(
        str, x, y, 20, BLACK
    );
}

int drawGUI(int starty) {
    if(game_state.state != 0) {
        drawClueDescription(1, starty);
    } else {
        char slevel[32] = { 0 }, smarks[32] = { 0 };
        sprintf(slevel, "level: %d", level + 1);
        sprintf(smarks, "marks: %d", marks);
        DrawText(slevel, 25, starty + 18, 20, BLACK);
        DrawText(smarks, 360, starty + 18, 20, BLACK);
    }
    return 60;
}

void readInput(int starty) {
    int x = GetMouseX() / cwidth;
    int y = GetMouseY() - starty;
    if(y < 0) y = -1;
    else y /= cwidth;

    int i = y * bcols + x;
    if(IsMouseButtonDown(MOUSE_BUTTON_LEFT) == 1) {
        if((x >= 0 && x < bcols) && (y >= 0 && y < brows)) {
            if(mouse_has_released == 1) {
                if(game_state.state == 0) {
                    if(i < bsize) {
                        if(getClue(i, 0) != 0) {
                            marks -= 1;
                        } else {
                            marks -= 3;
                        }
                        update_game(&game_state, i, &game_data);
                        if(game_state.state == 2) {
                            marks += MARKS_GAIN;
                        }
                        if(marks < 0) {
                            game_state.state = 1;
                        }
                    }
                } else {
                    if(game_state.state == 1) {
                        change_board_size(BSIZE_START);
                        marks = MARKS_START;
                        ttypes = TTYPES_START;
                        min_ans = MIN_ANS_START;
                        level = 0;
                    } else {
                        level += 1;
                        if(level % 3 == 1 && ttypes < TTYPES) {
                            ttypes += 1;
                        }
                        if(level % 3 == 0 && min_ans < MINANS) {
                            min_ans += 1;
                        }
                        if(level % 3 == 2 && bcols < BCOLS) {
                            change_board_size(bcols + 1);
                        }
                    }

                    memset(&game_state, 0, sizeof(game_state_t));
                    game_data = generate_game(ttypes, min_ans, rand());
#ifdef DEBUG
                    printf("game:\n");
                    printf("\tclue1: %8x%8x\n", game_data.clue[0].data[1], game_data.clue[0].data[0]);
                    printf("\ttype1: %d\n", game_data.clue_data[0].type);
                    printf("\tclue2: %8x%8x\n", game_data.clue[1].data[1], game_data.clue[1].data[0]);
                    printf("\ttype2: %d\n", game_data.clue_data[1].type);
                    printf(
                        "\tcombined: %8x%8x\n",
                        game_data.clue[0].data[1] | game_data.clue[1].data[1],
                        game_data.clue[0].data[0] | game_data.clue[1].data[0]
                    );
                    printf("\tanswer: %d\n", game_data.answer);
#endif
                }
            }
        }
        mouse_has_released = 0;
    }

    if(IsMouseButtonDown(MOUSE_BUTTON_LEFT) == 0) {
        mouse_has_released = 1;
    }
}

void drawFrame() {
    int y = 0;
    BeginDrawing();
    ClearBackground(RAYWHITE);
    y += drawClueDescription(0, y);
    readInput(y);
    y += drawBoard(y);
    y += drawGUI(y);
    drawEndMessage(290);
    EndDrawing();
}

void update() {
    drawFrame();
}

int main(int argc, char *argv[]) {
    change_board_size(BSIZE_START);
    level = 0;
    marks = MARKS_START;
    ttypes = TTYPES_START;
    min_ans = MIN_ANS_START;

    SetTraceLogLevel(LOG_NONE);

    srand(time(NULL));
    game_data = generate_game(ttypes, min_ans, rand());
    game_data = generate_game(ttypes, min_ans, rand());

    // create a window to draw in
    InitWindow(480, 600, "Krynth");
    SetTargetFPS(30);

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(update, 30, 1);
#else
    while (!WindowShouldClose()) {
        update();
    }
#endif

    CloseWindow();
    return EXIT_SUCCESS;
}
