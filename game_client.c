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

#define SRADIUS 25

#define MARKS_START 15
#define MARKS_GAIN 3
#define MIN_ANS_START 2
#define TTYPES_START 3

game_data_t game_data;
game_state_t game_state;

int level, marks, ttypes, min_ans;

int mouse_has_released;

void drawTriangle(Vector2 v) {
    v.y -= 3;
    DrawPoly(v, 3, SRADIUS+2, 0.0, DARKGRAY);
    DrawPoly(v, 3, SRADIUS-4, 0.0, GOLD);
}

void drawTriangle2(Vector2 v) {
    v.y += 3;
    DrawPoly(v, 3, SRADIUS+2, 180.0, DARKGRAY);
    DrawPoly(v, 3, SRADIUS-4, 180.0, RED);
}

void drawSquare(Vector2 v) {
    DrawPoly(v, 4, SRADIUS, 0.0, DARKGRAY);
    DrawPoly(v, 4, SRADIUS-4, 0.0, ORANGE);
}

void drawCircle(Vector2 v) { 
    DrawCircleV(v, SRADIUS-5, DARKGRAY);
    DrawCircleV(v, SRADIUS-8, SKYBLUE);
}

void drawPentagon(Vector2 v) {
    DrawPoly(v, 5, SRADIUS-2, 180.0, DARKGRAY);
    DrawPoly(v, 5, SRADIUS-5, 180.0, BLUE);
}

void drawHexagon(Vector2 v) {
    DrawPoly(v, 6, SRADIUS-3, 0.0, DARKGRAY);
    DrawPoly(v, 6, SRADIUS-6, 0.0, GREEN);
}

void drawDiamond(Vector2 v) {
    DrawPoly(v, 4, SRADIUS, 45.0, DARKGRAY);
    DrawPoly(v, 4, SRADIUS-4, 45.0, VIOLET);
}

void drawTile(int i, int value) {
    Vector2 v = { (i % BCOLS) * 60 + 30, (i / BCOLS) * 60 + 30 };
    switch(value) {
        case 0:
            drawCircle(v);
            break;
        case 1:
            drawTriangle(v);
            break;
        case 2:
            drawSquare(v);
            break;
        case 3:
            drawPentagon(v);
            break;
        case 4:
            drawHexagon(v);
            break;
        case 5:
            drawTriangle2(v);
            break;
        case 6:
            drawDiamond(v);
            break;
    }
}

void drawMark(int i, int mark) {
    if(mark == 1) {
        Vector2 start = { 
            (i % BCOLS) * 60 + 24, (i / BCOLS) * 60 + 24
        };
        Vector2 end = {
            (i % BCOLS) * 60 + 36, (i / BCOLS) * 60 + 36
        };
        DrawLineEx(start, end, 5.0, BLACK);
        start.x = (i % BCOLS) * 60 + 24;
        start.y =(i / BCOLS) * 60 + 36;
        end.x = (i % BCOLS) * 60 + 36;
        end.y =(i / BCOLS) * 60 + 24;
        DrawLineEx(start, end, 5.0, BLACK);
    } else if(mark == -1) {
        Vector2 center = { 
            (i % BCOLS) * 60 + 30, (i / BCOLS) * 60 + 30
        };
        DrawCircleV(center, 7, BLACK);
    }
}

int getClue(int i, int j) {
    return game_data.clue[j].data[i / 32] & (1 << (i % 32));
}

void drawBorder(int i, int j, Color color) {
    if(getClue(i, j) == 0) {
        if(i % BCOLS == 0 || getClue(i - 1, j) != 0) {
            Vector2 start = {
                (i % BCOLS) * 60, (i / BCOLS) * 60
            };
            Vector2 end = {
                (i % BCOLS) * 60, (i / BCOLS) * 60 + 60
            };
            DrawLineEx(start, end, 3.0, color);
        }
        if(i % BCOLS == (BCOLS - 1) || getClue(i + 1, j) != 0) {
            Vector2 start = {
                (i % BCOLS) * 60 + 60, (i / BCOLS) * 60
            };
            Vector2 end = {
                (i % BCOLS) * 60 + 60, (i / BCOLS) * 60 + 60
            };
            DrawLineEx(start, end, 3.0, color);
        }
        if(i / BCOLS == 0 || getClue(i - BCOLS, j) != 0) {
            Vector2 start = {
                (i % BCOLS) * 60, (i / BCOLS) * 60
            };
            Vector2 end = {
                (i % BCOLS) * 60 + 60, (i / BCOLS) * 60
            };
            DrawLineEx(start, end, 3.0, color);
        }
        if(i / BCOLS == (BROWS - 1) || getClue(i + BCOLS, j) != 0) {
            Vector2 start = {
                (i % BCOLS) * 60, (i / BCOLS) * 60 + 60
            };
            Vector2 end = {
                (i % BCOLS) * 60 + 60, (i / BCOLS) * 60 + 60
            };
            DrawLineEx(start, end, 3.0, color);
        }
    }
}

void drawBoard() {
    for(int i = 0; i < BSIZE; ++i) {
        Vector2 v = { (i % BCOLS) * 60, (i / BCOLS) * 60 };
        Vector2 s = { 60, 60 };
        if(getClue(i,0) == 0) {
            DrawRectangleV(v, s, LIGHTGRAY);
        } else if(game_state.state != 0 && getClue(i, 1) == 0) {
            DrawRectangleV(v, s, GRAY);
        }
    }
    {
        int mx = GetMouseX(), my = GetMouseY();
        Vector2 v, s = { 60, 60 };
        if(my < BCOLS * 60) {
            v.x =60 * (mx / 60);
            v. y =60 * (my / 60);
            DrawRectangleV(v, s, BEIGE);
        }
        if(game_state.state == 1) {
            v.x =  60 * (game_data.answer % BCOLS);
            v.y = 60 * (game_data.answer / BCOLS);
            DrawRectangleV(v, s, MAROON);
        } else if(game_state.state == 2) {
            v.x =  60 * (game_data.answer % BCOLS);
            v.y = 60 * (game_data.answer / BCOLS);
            DrawRectangleV(v, s, LIME);
        }
    }

    for(int i = 0; i < BSIZE; ++i) {
        drawBorder(i, 0, DARKGRAY);
        if(game_state.state != 0) {
            drawBorder(i, 1, DARKGRAY);
        }
        drawTile(i, game_data.map.terrain[i]);
        drawMark(i, game_state.marks[i][1]);
    }
}

void drawGUI() {
    if(game_state.state == 2) {
        DrawText("LEVEL COMPLETE! (CLICK TO CONTINUE)", 25, 490, 20, BLACK);
    } else if(game_state.state == 1) {
        DrawText("GAME OVER! (CLICK TO CONTINUE)", 60, 490, 20, BLACK);
    } else {
        char slevel[32], smarks[32];
        sprintf(slevel, "Level: %d", level);
        sprintf(smarks, "Marks: %d", marks);
        DrawText(slevel, 10, 490, 20, BLACK);
        DrawText(smarks, 360, 490, 20, BLACK);
    }
}

void drawFrame() {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    drawBoard();
    drawGUI();
    EndDrawing();
}

void readInput() {
    int x = GetMouseX() / 60, y = GetMouseY() / 60;
    int i = y * BCOLS + x;
    if(IsMouseButtonDown(MOUSE_BUTTON_LEFT) == 1) {
        if(mouse_has_released == 1) {
            if(game_state.state == 0) {
                if(i < BSIZE) {
                    if(marks <= 0 && getClue(i, 0) != 0) {
                        game_state.state = 1;
                    } else {
                        if(getClue(i, 0) != 0) {
                            marks -= 1;
                        }
                        update_game(&game_state, i, &game_data);
                    }
                }
            } else {
                if(game_state.state == 1) {
                    marks = MARKS_START;
                    ttypes = TTYPES_START;
                    min_ans = MIN_ANS_START;
                    level = 1;
                } else {
                    marks += MARKS_GAIN;
                    level += 1;
                    if(level % 3 == 0 && ttypes < TTYPES) {
                        ttypes += 1;
                    }
                    if(level % 3 == 0 && min_ans < MINANS) {
                        min_ans += 1;
                    }
                }

                memset(&game_state, 0, sizeof(game_state_t));
                game_data = generate_game(ttypes, min_ans, rand());
            }
        }
        mouse_has_released = 0;
    }
    if(IsMouseButtonDown(MOUSE_BUTTON_LEFT) == 0) {
        mouse_has_released = 1;
    }
}

void update() {
    readInput();
    drawFrame();
}

int main(int argc, char *argv[])
{
    level = 1;
    marks = MARKS_START;
    ttypes = TTYPES_START;
    min_ans = MIN_ANS_START;

    SetTraceLogLevel(LOG_NONE);

    srand(time(NULL));
    game_data = generate_game(ttypes, min_ans, rand());

    // create a window to draw in
    InitWindow(480, 520, "Krynth");
    SetTargetFPS(30);

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(update, 60, 1);
#else
    while (!WindowShouldClose()) {
        update();
    }
#endif

    CloseWindow();
    return EXIT_SUCCESS;
}
