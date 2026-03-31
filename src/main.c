#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// defines for basic game setting
#define colour_background BLACK
#define colour_character WHITE
#define screenwidth 1000
#define screenheight 800
// bullet and character defines
#define speed_player 400.0f
#define size_player 40.0f
#define maximum_bullets 10
#define bullet_speed 500.0f
#define bullets_radius 5.0f
// alien defines
#define size_alien 40.0f
#define alien_rows 5
#define alien_cols 10
#define maximum_aliens (alien_rows * alien_cols)
#define interval_alien_move 0.5f
#define startalien_speed 20.0f
#define startalien_bulletspeed 300.0f
#define alien_wave_interval 5.0f
#define wavespeed_increment 5.0f
// player lives defines
#define lives_player 3
#define heart_size 20.0f
// block defines
#define Block_no 4
#define block_width 80
#define block_height 30
// basic structs for game objects
typedef struct Alien {
    Vector2 position;
    bool active;
} Alien;

typedef struct Bullet {
    Vector2 position;
    bool active;
} Bullet;

typedef struct Alien_bullet {
    Vector2 position;
    bool active;
} Alien_bullet;


typedef struct Block {
    float x, y;
} Block;

int main(void) {
    int score = 0;
    // Initialisation
    InitWindow(screenwidth, screenheight, "Space Invaders");
    SetTargetFPS(60);
    Vector2 position_player = {screenwidth/2.0f, screenheight/1.1f};

    // Setup for bullets
    Bullet bullets[maximum_bullets];
    Alien_bullet alien_bullets[maximum_bullets];
    for (int i=0; i<maximum_bullets; i++) {
        bullets[i].active = false;
        alien_bullets[i].active = false;
    }

    // Creating the aliens grid 
    float startX = 100.0f;
    float startY = 60.0f;
    float spacingX = 80.0f;
    float spacingY = 75.0f;

    Alien aliens[alien_rows][alien_cols];
    for (int row = 0; row < alien_rows; row++) {
        for (int col = 0; col < alien_cols; col++) {
            aliens[row][col].position.x = startX + col * spacingX;
            aliens[row][col].position.y = startY + row * spacingY;
            aliens[row][col].active = true;
        }
    }

    // Alien logic initialisation
    int alien_direction = 1;
    int reverse = 0;
    float alien_move_timer = 0.0f;

    float alien_speed = startalien_speed;
    float alien_bulletspeed = startalien_bulletspeed;

    int gameover = 0;
    int lives = lives_player;
    int alien_wave = 1;

    // Basic blocks setup (no damage, just rectangles)
    Block blocks[Block_no];
    int total_blocks_width = Block_no * block_width;
    int available_space = screenwidth - total_blocks_width;
    int block_spacing = available_space / (Block_no + 1);
    for (int i = 0; i < Block_no; i++) {
        blocks[i].x = block_spacing + i * (block_width + block_spacing);
        blocks[i].y = screenheight - 180;
    }

    // main game loop
    while (!WindowShouldClose() && !gameover)
    {
        float dt = GetFrameTime();

        // Creating the movement of the character
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) position_player.x -= speed_player * dt;
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) position_player.x += speed_player * dt;
        // stopping you going off screen
        if (position_player.x < 0) position_player.x = 0;
        if (position_player.x > screenwidth - size_player) position_player.x = screenwidth - size_player;

        // basic shooting logic
        if (IsKeyPressed(KEY_SPACE)) {
            for (int i=0; i<maximum_bullets; i++) {
                if (!bullets[i].active) {
                    bullets[i].position.x = position_player.x + size_player/2 - bullets_radius;
                    bullets[i].position.y = position_player.y;
                    bullets[i].active = true;
                    break;
                }
            }
        }

        // bullet collisions killing aliens and adding to the score
        for (int i=0; i<maximum_bullets; i++) {
            if (bullets[i].active) {
                for (int row = 0; row < alien_rows; row++) {
                    for (int col = 0; col < alien_cols; col++) {
                        if (aliens[row][col].active) {
                            if (CheckCollisionCircleRec(bullets[i].position, bullets_radius, (Rectangle){aliens[row][col].position.x, aliens[row][col].position.y, size_alien, size_alien})) {
                                aliens[row][col].active = false;
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

        // this updates the players bullets
        for (int i=0; i<maximum_bullets; i++) {
            if (bullets[i].active) {
                bullets[i].position.y -= bullet_speed * dt;
                if (bullets[i].position.y < 0) bullets[i].active = false;
            }
        }

        // adding an interval between each aliens movements
        alien_move_timer += dt;
        if (alien_move_timer >= interval_alien_move) {
            alien_move_timer = 0.0f;
            reverse = 0;
            // This checks if an alien is about to go off screen
            for (int row = 0; row < alien_rows; row++) {
                for (int col = 0; col < alien_cols; col++) {
                    if (aliens[row][col].active) {
                        float next_x = aliens[row][col].position.x + alien_direction * alien_speed;
                        if (next_x < 0 || next_x > screenwidth - size_alien) {
                            reverse = 1;
                            break;
                        }
                    }
                }
                if (reverse) break;
            }
            // this stops the aliens going off screen
            if (reverse) {
                alien_direction *= -1;
                for (int row = 0; row < alien_rows; row++) {
                    for (int col = 0; col < alien_cols; col++) {
                        aliens[row][col].position.y += size_alien / 2;
                    }
                }
            } else {
                // Moving the aliens at the same time
                for (int row = 0; row < alien_rows; row++) {
                    for (int col = 0; col < alien_cols; col++) {
                        if (aliens[row][col].active) {
                            aliens[row][col].position.x += alien_direction * alien_speed;
                        }
                    }
                }
            }
        }

        // checking if an alien reaches the bottom
        for (int row = 0; row < alien_rows; row++) {
            for (int col = 0; col < alien_cols; col++) {
                if (aliens[row][col].active && aliens[row][col].position.y + size_alien >= position_player.y) {
                    gameover = 1;
                }
            }
        }

        // Alien shooting back and basic alien ai
        int aliens_left = 0;
        int shooting_chance = 2 + (maximum_aliens- aliens_left)/3;
        if (rand() % 100 < 2) {
            //*Targeting logic *//
            int closest_col = 0;
            float min_dist = 1e9f;
            for (int col = 0; col < alien_cols; col++) {
                float aliens_col_x = aliens[0][col].position.x + size_alien/2;
                float dist = fabsf(aliens_col_x - (position_player.x + size_player/2));
                if (dist < min_dist) {
                    min_dist = dist;
                    closest_col = col;
                }
            }
            int col_range[3];
            int col_count = 0;
            for (int d = -1; d <= 1; d++) {
                int c = closest_col + d;
                if (c >= 0 && c < alien_cols) {
                    col_range[col_count++] = c;
                }
            }
            int shooter_col = col_range[rand() % col_count];
            int shooter_row = -1;
            for (int row = alien_rows - 1; row >= 0; row--) {
                if (aliens[row][shooter_col].active) {
                    shooter_row = row;
                    break;
                }
            }
            if (shooter_row != -1) {
                for (int i=0; i<maximum_bullets; i++) {
                    if (!alien_bullets[i].active) {
                        alien_bullets[i].position.x = aliens[shooter_row][shooter_col].position.x + size_alien/2 - bullets_radius;
                        alien_bullets[i].position.y = aliens[shooter_row][shooter_col].position.y + size_alien;
                        alien_bullets[i].active = true;
                        break;
                    }
                }
            }
        }

        // This updates the alien bullets
        for (int i=0; i<maximum_bullets; i++) {
            if (alien_bullets[i].active) {
                alien_bullets[i].position.y += alien_bulletspeed * dt;
                if (alien_bullets[i].position.y > screenheight) alien_bullets[i].active = false;
                // Check collision with player and reduce lives
                if (CheckCollisionCircleRec(alien_bullets[i].position, bullets_radius, (Rectangle){position_player.x, position_player.y - size_player, size_player, size_player})) {
                    alien_bullets[i].active = false;
                    lives--;
                    if (lives <= 0) {
                        gameover = 1;
                    }
                }
            }
        }

        // what happens if all aliens defeated
        for (int row = 0; row < alien_rows; row++) {
            for (int col = 0; col < alien_cols; col++) {
                if (aliens[row][col].active) {
                    aliens_left++;
                }
            }
        }
        if (aliens_left == 0) {
            alien_wave++;
            // Respawn new grid of aliens
            for (int row = 0; row < alien_rows; row++) {
                for (int col = 0; col < alien_cols; col++) {
                    aliens[row][col].position.x = startX + col * spacingX;
                    aliens[row][col].position.y = startY + row * spacingY;
                    aliens[row][col].active = true;
                }
            }
            // this increases the difficuty
            alien_speed += alien_wave_interval;
            alien_bulletspeed += alien_wave_interval;
        }


        // Player bullet-block collision
        for (int i = 0; i < maximum_bullets; i++) {
            if (bullets[i].active) {
                for (int j = 0; j < Block_no; j++) {
                    Rectangle block_rect = {blocks[j].x, blocks[j].y, block_width, block_height};
                    if (CheckCollisionCircleRec(bullets[i].position, bullets_radius, block_rect)) {
                        bullets[i].active = false;
                        break;
                    }
                }
            }
        }
        // Alien bullet-block collision
        for (int i = 0; i < maximum_bullets; i++) {
            if (alien_bullets[i].active) {
                for (int j = 0; j < Block_no; j++) {
                    Rectangle block_rect = {blocks[j].x, blocks[j].y, block_width, block_height};
                    if (CheckCollisionCircleRec(alien_bullets[i].position, bullets_radius, block_rect)) {
                        alien_bullets[i].active = false;
                        break;
                    }
                }
            }
        }

        // Score bonuses for defeating waves of aliens
        if (aliens_left == 0) {
            score += 100;
        }
        if (alien_wave % 5 ==0) {
            score += 500;
        }

        // drawing everything here
        BeginDrawing();
        ClearBackground(colour_background);
        DrawText("Use A and D or arrow keys to move the character", 10, 10, 20, YELLOW);
        DrawText("Press SPACE to shoot", 10, 40, 20, YELLOW);
        DrawText(TextFormat("Score: %d", score), screenwidth - 200, 10, 24, WHITE);
        DrawText(TextFormat("Wave %d", alien_wave), screenwidth - 200, 40, 24, WHITE);
        DrawTriangle((Vector2){position_player.x, position_player.y}, (Vector2){position_player.x + size_player, position_player.y}, (Vector2){position_player.x + size_player/2, position_player.y - size_player}, colour_character);
        // draw the bulllets for both player and the aliens
        for (int i=0; i<maximum_bullets; i++) {
            if (bullets[i].active) {
                DrawCircleV(bullets[i].position, bullets_radius, YELLOW);
            }
        }
        for (int i=0; i<maximum_bullets; i++) {
            if (alien_bullets[i].active) {
                DrawCircleV(alien_bullets[i].position, bullets_radius, RED);
            }
        }
        // drawing the aliens
        for (int row = 0; row < alien_rows; row++) {
            for (int col = 0; col < alien_cols; col++) {
                if (aliens[row][col].active) {
                    DrawRectangle(aliens[row][col].position.x, aliens[row][col].position.y, size_alien, size_alien, PURPLE);
                }
            }
        }
        // draw the lives in the corner
        for (int i=0; i<lives; i++) {
            float heart_x = 20 + i * (size_player + 10) + size_player/2;
            float heart_y = screenheight - 30;
            DrawCircle(heart_x, heart_y, heart_size, RED);
        }
        // Draw basic blocks
        for (int i = 0; i < Block_no; i++) {
            DrawRectangle(blocks[i].x, blocks[i].y, block_width, block_height, GREEN);
        }
        EndDrawing();
    }

    // basic game over screen
    if (gameover) {
        while (!WindowShouldClose()) {
            BeginDrawing();
            ClearBackground(colour_background);
            DrawText("Game Over!", screenwidth/2 - 100, screenheight/2 - 20, 40, RED);
            DrawText(TextFormat("Final Score: %d", score), screenwidth/2 - 120, screenheight/2 + 30, 30, WHITE);  
            DrawText(TextFormat("You reached wave %d", alien_wave), screenwidth/2 - 120, screenheight/2 + 70, 30, WHITE);
            DrawText("Press ESC to exit", screenwidth/2 - 100, screenheight/2 + 110, 20, YELLOW);
            EndDrawing();
        }
    }
    CloseWindow();
    return 0;
}