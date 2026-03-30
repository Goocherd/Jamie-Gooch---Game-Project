#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define clear_background BLACK
#define character_colour WHITE
#define screen_width 1000
#define screen_height 800
#define character_speed 400.0f
#define character_size 40.0f
#define max_bullets 10
#define bullet_speed 500.0f
#define bullets_radius 5.0f
#define enemy_size 40.0f
#define enemy_rows 5
#define enemy_cols 10
#define max_enemies (enemy_rows * enemy_cols)
#define enemy_move_interval 0.5f
#define initial_enemy_speed 20.0f
#define initial_enemy_bullet_speed 300.0f
#define character_lives 3
#define heart_size 20.0f
#define enemy_wave_interval 5.0f
#define enemy_wave_speed_increment 5.0f
#define Block_no 4
#define block_width 100
#define block_height 40
#define BLOCK_SEGMENT_SIZE 6
#define BLOCK_SEGMENTS_X 10 
#define BLOCK_SEGMENTS_Y 5 

typedef struct Enemy {
    Vector2 position;
    bool active;
} Enemy;

typedef struct Bullet {
    Vector2 position;
    bool active;
} Bullet;

typedef struct Enemy_bullet {
    Vector2 position;
    bool active;
} Enemy_bullet;

typedef struct Block {
    float x, y;
    bool segment[BLOCK_SEGMENTS_X][BLOCK_SEGMENTS_Y];
} Block;

int main(void) {
    int score = 0;
    //* Initialisation *//
    InitWindow(screen_width, screen_height, "Space Invaders");
    SetTargetFPS(60);
    Vector2 character_position = {screen_width/2.0f, screen_height/1.1f};

    //* Bullet setup *//
    Bullet bullets[max_bullets];
    Enemy_bullet enemy_bullets[max_bullets];
    for (int i=0; i<max_bullets; i++) {
        bullets[i].active = false;
        enemy_bullets[i].active = false;
    }

    //* Enemy grid setup *//
    float startX = 100.0f;
    float startY = 60.0f;
    float spacingX = 80.0f;
    float spacingY = 75.0f;

    Enemy enemies[enemy_rows][enemy_cols];
    for (int row = 0; row < enemy_rows; row++) {
        for (int col = 0; col < enemy_cols; col++) {
            enemies[row][col].position.x = startX + col * spacingX;
            enemies[row][col].position.y = startY + row * spacingY;
            enemies[row][col].active = true;
        }
    }

    //* Enemy logic initialisation *//
    int enemy_direction = 1;
    int need_reverse = 0;
    float enemy_move_timer = 0.0f;

    float enemy_speed = initial_enemy_speed;
    float enemy_bullet_speed = initial_enemy_bullet_speed;

    int gameOver = 0;
    int lives = character_lives;
    int enemy_wave = 1;

    //* Block setup *//
    Block blocks[Block_no];
    int total_blocks_width = Block_no * block_width;
    int available_space = screen_width - total_blocks_width;
    int block_spacing = available_space / (Block_no + 1);
    for (int i=0; i<Block_no; i++) {
        blocks[i].x = block_spacing + i * (block_width + block_spacing);
        blocks[i].y = screen_height - 180;
        for (int sx = 0; sx < BLOCK_SEGMENTS_X; sx++) {
            for (int sy = 0; sy < BLOCK_SEGMENTS_Y; sy++) {
                blocks[i].segment[sx][sy] = true;
            }
        }
    }

    while (!WindowShouldClose() && !gameOver)
    {
        float delta = GetFrameTime();

        //* Character movement logic *//
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) character_position.x -= character_speed * delta;
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) character_position.x += character_speed * delta;
        if (character_position.x < 0) character_position.x = 0;
        if (character_position.x > screen_width - character_size) character_position.x = screen_width - character_size;

        //* Character shooting logic *//
        if (IsKeyPressed(KEY_SPACE)) {
            for (int i=0; i<max_bullets; i++) {
                if (!bullets[i].active) {
                    bullets[i].position.x = character_position.x + character_size/2 - bullets_radius;
                    bullets[i].position.y = character_position.y;
                    bullets[i].active = true;
                    break;
                }
            }
        }

        //* Shooting collision logic *//
        for (int i=0; i<max_bullets; i++) {
            if (bullets[i].active) {
                for (int row = 0; row < enemy_rows; row++) {
                    for (int col = 0; col < enemy_cols; col++) {
                        if (enemies[row][col].active) {
                            if (CheckCollisionCircleRec(bullets[i].position, bullets_radius, (Rectangle){enemies[row][col].position.x, enemies[row][col].position.y, enemy_size, enemy_size})) {
                                enemies[row][col].active = false;
                                bullets[i].active = false;
                                score += 20;
                                break;
                            }
                        }
                    }
                    if (!bullets[i].active) break;
                }
            }
        }

        //* Update player bullets *//
        for (int i=0; i<max_bullets; i++) {
            if (bullets[i].active) {
                bullets[i].position.y -= bullet_speed * delta;
                if (bullets[i].position.y < 0) bullets[i].active = false;
            }
        }

        //* Timer-based enemy movement *//
        enemy_move_timer += delta;
        if (enemy_move_timer >= enemy_move_interval) {
            enemy_move_timer = 0.0f;
            need_reverse = 0;
            //* Check if any enemy will hit the edge on next move *//
            for (int row = 0; row < enemy_rows; row++) {
                for (int col = 0; col < enemy_cols; col++) {
                    if (enemies[row][col].active) {
                        float next_x = enemies[row][col].position.x + enemy_direction * enemy_speed;
                        if (next_x < 0 || next_x > screen_width - enemy_size) {
                            need_reverse = 1;
                            break;
                        }
                    }
                }
                if (need_reverse) break;
            }
            //* Stopping enemies going off screen *//
            if (need_reverse) {
                enemy_direction *= -1;
                for (int row = 0; row < enemy_rows; row++) {
                    for (int col = 0; col < enemy_cols; col++) {
                        enemies[row][col].position.y += enemy_size / 2;
                    }
                }
            } else {
                //* Otherwise, move all enemies horizontally *//
                for (int row = 0; row < enemy_rows; row++) {
                    for (int col = 0; col < enemy_cols; col++) {
                        if (enemies[row][col].active) {
                            enemies[row][col].position.x += enemy_direction * enemy_speed;
                        }
                    }
                }
            }
        }

        //* Enemy reaching bottom logic *//
        for (int row = 0; row < enemy_rows; row++) {
            for (int col = 0; col < enemy_cols; col++) {
                if (enemies[row][col].active && enemies[row][col].position.y + enemy_size >= character_position.y) {
                    gameOver = 1;
                }
            }
        }

        //* Enemy shooting back logic *//
        int enemies_left = 0;
        int shooting_chance = 2 + (max_enemies - enemies_left)/3;
        if (rand() % 100 < 2) {
            //*Targeting logic *//
            int closest_col = 0;
            float min_dist = 1e9f;
            for (int col = 0; col < enemy_cols; col++) {
                float enemy_col_x = enemies[0][col].position.x + enemy_size/2;
                float dist = fabsf(enemy_col_x - (character_position.x + character_size/2));
                if (dist < min_dist) {
                    min_dist = dist;
                    closest_col = col;
                }
            }
            int col_range[3];
            int col_count = 0;
            for (int d = -1; d <= 1; d++) {
                int c = closest_col + d;
                if (c >= 0 && c < enemy_cols) {
                    col_range[col_count++] = c;
                }
            }
            int shooter_col = col_range[rand() % col_count];
            int shooter_row = -1;
            for (int row = enemy_rows - 1; row >= 0; row--) {
                if (enemies[row][shooter_col].active) {
                    shooter_row = row;
                    break;
                }
            }
            if (shooter_row != -1) {
                for (int i=0; i<max_bullets; i++) {
                    if (!enemy_bullets[i].active) {
                        enemy_bullets[i].position.x = enemies[shooter_row][shooter_col].position.x + enemy_size/2 - bullets_radius;
                        enemy_bullets[i].position.y = enemies[shooter_row][shooter_col].position.y + enemy_size;
                        enemy_bullets[i].active = true;
                        break;
                    }
                }
            }
        }

        //* Update enemy bullets *//
        for (int i=0; i<max_bullets; i++) {
            if (enemy_bullets[i].active) {
                enemy_bullets[i].position.y += enemy_bullet_speed * delta;
                if (enemy_bullets[i].position.y > screen_height) enemy_bullets[i].active = false;
                //* Check collision with player *//
                if (CheckCollisionCircleRec(enemy_bullets[i].position, bullets_radius, (Rectangle){character_position.x, character_position.y - character_size, character_size, character_size})) {
                    enemy_bullets[i].active = false;
                    lives--;
                    if (lives <= 0) {
                        gameOver = 1;
                    }
                }
            }
        }

        //* Enemy wave cleared logic *//
        for (int row = 0; row < enemy_rows; row++) {
            for (int col = 0; col < enemy_cols; col++) {
                if (enemies[row][col].active) {
                    enemies_left++;
                }
            }
        }
        if (enemies_left == 0) {
            enemy_wave++;
            //* Reset enemies *//
            for (int row = 0; row < enemy_rows; row++) {
                for (int col = 0; col < enemy_cols; col++) {
                    enemies[row][col].position.x = startX + col * spacingX;
                    enemies[row][col].position.y = startY + row * spacingY;
                    enemies[row][col].active = true;
                }
            }
            //* Increase difficulty *//
            enemy_speed += enemy_wave_speed_increment;
            enemy_bullet_speed += enemy_wave_speed_increment;
        }

        //* Block damage logic *//
        for (int i=0; i<max_bullets; i++) {
            if (bullets[i].active) {
                for (int j=0; j<Block_no; j++) {
                    //* Check collision with each block segment *//
                    for (int sx = 0; sx < BLOCK_SEGMENTS_X; sx++) {
                        for (int sy = 0; sy < BLOCK_SEGMENTS_Y; sy++) {
                            if (blocks[j].segment[sx][sy]) {
                                float seg_x = blocks[j].x + sx * BLOCK_SEGMENT_SIZE;
                                float seg_y = blocks[j].y + sy * BLOCK_SEGMENT_SIZE;
                                Rectangle seg_rect = {seg_x, seg_y, BLOCK_SEGMENT_SIZE, BLOCK_SEGMENT_SIZE};
                                if (CheckCollisionCircleRec(bullets[i].position, bullets_radius, seg_rect)) {
                                    blocks[j].segment[sx][sy] = false;
                                    bullets[i].active = false;
                                    goto next_player_bullet;
                                }
                            }
                        }
                    }
                }
                next_player_bullet: ;
            }
        }
        //* Enemy bullets damaging blocks *//
        for (int i=0; i<max_bullets; i++) {
            if (enemy_bullets[i].active) {
                for (int j=0; j<Block_no; j++) {
                    for (int sx = 0; sx < BLOCK_SEGMENTS_X; sx++) {
                        for (int sy = 0; sy < BLOCK_SEGMENTS_Y; sy++) {
                            if (blocks[j].segment[sx][sy]) {
                                float seg_x = blocks[j].x + sx * BLOCK_SEGMENT_SIZE;
                                float seg_y = blocks[j].y + sy * BLOCK_SEGMENT_SIZE;
                                Rectangle seg_rect = {seg_x, seg_y, BLOCK_SEGMENT_SIZE, BLOCK_SEGMENT_SIZE};
                                if (CheckCollisionCircleRec(enemy_bullets[i].position, bullets_radius, seg_rect)) {
                                    blocks[j].segment[sx][sy] = false;
                                    enemy_bullets[i].active = false;
                                    goto next_enemy_bullet;
                                }
                            }
                        }
                    }
                }
                next_enemy_bullet: ;
            }
        }

        //* Score bonus for clearing waves *//
        if (enemies_left == 0) {
            score += 100;
        }
        if (enemy_wave +=5) {
            score += 500;
        }

        //* Draw everything *//
        BeginDrawing();
        ClearBackground(clear_background);
        DrawText("Use A and D or arrow keys to move the character", 10, 10, 20, YELLOW);
        DrawText("Press SPACE to shoot", 10, 40, 20, YELLOW);
        DrawText(TextFormat("Score: %d", score), screen_width - 200, 10, 24, WHITE);
        DrawTriangle((Vector2){character_position.x, character_position.y}, (Vector2){character_position.x + character_size, character_position.y}, (Vector2){character_position.x + character_size/2, character_position.y - character_size}, character_colour);
        //* Draw bullets *//
        for (int i=0; i<max_bullets; i++) {
            if (bullets[i].active) {
                DrawCircleV(bullets[i].position, bullets_radius, YELLOW);
            }
        }
        for (int i=0; i<max_bullets; i++) {
            if (enemy_bullets[i].active) {
                DrawCircleV(enemy_bullets[i].position, bullets_radius, RED);
            }
        }
        //* Draw enemies *//
        for (int row = 0; row < enemy_rows; row++) {
            for (int col = 0; col < enemy_cols; col++) {
                if (enemies[row][col].active) {
                    DrawRectangle(enemies[row][col].position.x, enemies[row][col].position.y, enemy_size, enemy_size, PURPLE);
                }
            }
        }
        //* Draw lives *//
        for (int i=0; i<lives; i++) {
            float heart_x = 20 + i * (character_size + 10) + character_size/2;
            float heart_y = screen_height - 30;
            DrawCircle(heart_x, heart_y, heart_size, RED);
        }
        //* Draw blocks as segments *//
        for (int i=0; i<Block_no; i++) {
            for (int sx = 0; sx < BLOCK_SEGMENTS_X; sx++) {
                for (int sy = 0; sy < BLOCK_SEGMENTS_Y; sy++) {
                    if (blocks[i].segment[sx][sy]) {
                        float seg_x = blocks[i].x + sx * BLOCK_SEGMENT_SIZE;
                        float seg_y = blocks[i].y + sy * BLOCK_SEGMENT_SIZE;
                        DrawRectangle(seg_x, seg_y, BLOCK_SEGMENT_SIZE, BLOCK_SEGMENT_SIZE, GREEN);
                    }
                }
            }
        }
        EndDrawing();
    }

    //* Game Over screen *//
    if (gameOver) {
        while (!WindowShouldClose()) {
            BeginDrawing();
            ClearBackground(clear_background);
            DrawText("Game Over!", screen_width/2 - 100, screen_height/2 - 20, 40, RED);
            DrawText(TextFormat("Final Score: %d", score), screen_width/2 - 120, screen_height/2 + 30, 30, WHITE);  
            EndDrawing();
        }
    }
    CloseWindow();
    return 0;
}