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
#define player_move_frame_delay 0.12f
#define sprite_animation_frame_interval 0.18f
#define maximum_bullets 10
#define bullet_speed 500.0f
#define bullets_radius 5.0f
#define shot_width 16.0f
#define shot_height 24.0f
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

typedef enum {
    obj_alien,
    obj_bullet,
    obj_alien_bullet,
    obj_block
} GameObjectType;

typedef struct {
    GameObjectType type;
    union {
        struct { Vector2 position; bool active; } alien;
        struct { Vector2 position; bool active; } bullet;
        struct { Vector2 position; bool active; } alien_bullet;
        struct { float x, y; } block;
    } data;
} GameObject;

// making fallback paths for the textures
static Texture2D LoadTextureFromPaths(const char *primary_path, const char *fallback_path, bool *loaded) {
    if (FileExists(primary_path)) {
        if (loaded) *loaded = true;
        return LoadTexture(primary_path);
    }
    if (FileExists(fallback_path)) {
        if (loaded) *loaded = true;
        return LoadTexture(fallback_path);
    }

    if (loaded) *loaded = false;
    return (Texture2D){0};
}

int main(void) {
    int score = 0;
    // Initialisation
    InitWindow(screenwidth, screenheight, "Space Invaders");
    SetTargetFPS(60);
    Vector2 position_player = {screenwidth/2.0f, screenheight/1.1f};
    
    // this is setting up the textures
    Texture2D background_texture = {0};
    Texture2D enemy_texture_1 = {0};
    Texture2D enemy_texture_2 = {0};
    Texture2D player_straight_texture = {0};
    Texture2D player_left_texture = {0};
    Texture2D player_left_texture_2 = {0};
    Texture2D player_right_texture = {0};
    Texture2D player_right_texture_2 = {0};
    Texture2D player_shot_texture_1 = {0};
    Texture2D player_shot_texture_2 = {0};
    Texture2D enemy_shot_texture_1 = {0};
    Texture2D enemy_shot_texture_2 = {0};
    bool has_background_texture = false;
    bool has_enemy_texture_1 = false;
    bool has_enemy_texture_2 = false;
    bool has_player_straight_texture = false;
    bool has_player_left_texture = false;
    bool has_player_left_texture_2 = false;
    bool has_player_right_texture = false;
    bool has_player_right_texture_2 = false;
    bool has_player_shot_texture_1 = false;
    bool has_player_shot_texture_2 = false;
    bool has_enemy_shot_texture_1 = false;
    bool has_enemy_shot_texture_2 = false;
    int player_facing_direction = 0;
    float player_move_hold_time = 0.0f;
    float sprite_animation_timer = 0.0f;
    bool sprite_animation_frame_2 = false;

    // loading the textures into the game here
    background_texture = LoadTextureFromPaths("Textures/background.png", "../Textures/background.png", &has_background_texture);
    enemy_texture_1 = LoadTextureFromPaths("Textures/enemy_1.png", "../Textures/enemy_1.png", &has_enemy_texture_1);
    enemy_texture_2 = LoadTextureFromPaths("Textures/enemy_2.png", "../Textures/enemy_2.png", &has_enemy_texture_2);
    player_straight_texture = LoadTextureFromPaths("Textures/player_1_straight.png", "../Textures/player_1_straight.png", &has_player_straight_texture);
    player_left_texture = LoadTextureFromPaths("Textures/player_1_left.png", "../Textures/player_1_left.png", &has_player_left_texture);
    player_left_texture_2 = LoadTextureFromPaths("Textures/player_1_left_2.png", "../Textures/player_1_left_2.png", &has_player_left_texture_2);
    player_right_texture = LoadTextureFromPaths("Textures/player_1_right.png", "../Textures/player_1_right.png", &has_player_right_texture);
    player_right_texture_2 = LoadTextureFromPaths("Textures/player_1_right_2.png", "../Textures/player_1_right_2.png", &has_player_right_texture_2);
    player_shot_texture_1 = LoadTextureFromPaths("Textures/player_shot_1.png", "../Textures/player_shot_1.png", &has_player_shot_texture_1);
    player_shot_texture_2 = LoadTextureFromPaths("Textures/player_shot_2.png", "../Textures/player_shot_2.png", &has_player_shot_texture_2);
    enemy_shot_texture_1 = LoadTextureFromPaths("Textures/enemy_shot_1.png", "../Textures/enemy_shot_1.png", &has_enemy_shot_texture_1);
    enemy_shot_texture_2 = LoadTextureFromPaths("Textures/enemy_shot_2.png", "../Textures/enemy_shot_2.png", &has_enemy_shot_texture_2);

    // Setup for the bullets
    GameObject bullets[maximum_bullets];
    GameObject alien_bullets[maximum_bullets];
    for (int i=0; i<maximum_bullets; i++) {
        bullets[i].type = obj_bullet;
        bullets[i].data.bullet.active = false;
        alien_bullets[i].type = obj_alien_bullet;
        alien_bullets[i].data.alien_bullet.active = false;
    }

    // Creating the aliens grid 
    float startX = 100.0f;
    float startY = 60.0f;
    float spacingX = 80.0f;
    float spacingY = 75.0f;

    GameObject aliens[alien_rows][alien_cols];
    for (int row = 0; row < alien_rows; row++) {
        for (int col = 0; col < alien_cols; col++) {
            aliens[row][col].type = obj_alien;
            aliens[row][col].data.alien.position.x = startX + col * spacingX;
            aliens[row][col].data.alien.position.y = startY + row * spacingY;
            aliens[row][col].data.alien.active = true;
        }
    }

    // Alien logic start
    int alien_direction = 1;
    int reverse = 0;
    float alien_move_timer = 0.0f;

    float alien_speed = startalien_speed;
    float alien_bulletspeed = startalien_bulletspeed;

    int gameover = 0;
    int lives = lives_player;
    int alien_wave = 1;

    // setting up the blocks
    GameObject blocks[Block_no];
    int total_blocks_width = Block_no * block_width;
    int available_space = screenwidth - total_blocks_width;
    int block_spacing = available_space / (Block_no + 1);
    for (int i = 0; i < Block_no; i++) {
        blocks[i].type = obj_block;
        blocks[i].data.block.x = block_spacing + i * (block_width + block_spacing);
        blocks[i].data.block.y = screenheight - 180;
    }


    // this is where the main game loop is
    while (!WindowShouldClose() && !gameover)
    {
        // handling the player input and updating the animation timer
        float dt = GetFrameTime();
        bool moving_left = IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A);
        bool moving_right = IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D);
        int move_direction = 0;
        if (moving_left && !moving_right) move_direction = -1;
        if (moving_right && !moving_left) move_direction = 1;

        sprite_animation_timer += dt;
        while (sprite_animation_timer >= sprite_animation_frame_interval) {
            sprite_animation_timer -= sprite_animation_frame_interval;
            sprite_animation_frame_2 = !sprite_animation_frame_2;
        }

        if (move_direction != 0) {
            if (move_direction == player_facing_direction) {
                player_move_hold_time += dt;
            } else {
                player_facing_direction = move_direction;
                player_move_hold_time = 0.0f;
            }
        } else {
            player_facing_direction = 0;
            player_move_hold_time = 0.0f;
        }

        // Creating the movement of the character
        if (moving_left && !moving_right) position_player.x -= speed_player * dt;
        if (moving_right && !moving_left) position_player.x += speed_player * dt;
        // stopping you going off screen
        if (position_player.x < 0) position_player.x = 0;
        if (position_player.x > screenwidth - size_player) position_player.x = screenwidth - size_player;

        // basic shooting logic
        if (IsKeyPressed(KEY_SPACE)) {
            for (int i=0; i<maximum_bullets; i++) {
                if (!bullets[i].data.bullet.active) {
                    bullets[i].data.bullet.position.x = position_player.x + size_player/2 - bullets_radius;
                    bullets[i].data.bullet.position.y = position_player.y;
                    bullets[i].data.bullet.active = true;
                    break;
                }
            }
        }

        // bullet collisions killing aliens and adding to the score
        for (int i=0; i<maximum_bullets; i++) {
            if (bullets[i].data.bullet.active) {
                for (int row = 0; row < alien_rows; row++) {
                    for (int col = 0; col < alien_cols; col++) {
                        if (aliens[row][col].data.alien.active) {
                            if (CheckCollisionCircleRec(bullets[i].data.bullet.position, bullets_radius, (Rectangle){aliens[row][col].data.alien.position.x, aliens[row][col].data.alien.position.y, size_alien, size_alien})) {
                                aliens[row][col].data.alien.active = false;
                                bullets[i].data.bullet.active = false;
                                score += 20;
                                break;
                            }
                        }
                    }
                    if (!bullets[i].data.bullet.active) break;
                }
            }
        }

        // this updates the players bullets
        for (int i=0; i<maximum_bullets; i++) {
            if (bullets[i].data.bullet.active) {
                bullets[i].data.bullet.position.y -= bullet_speed * dt;
                if (bullets[i].data.bullet.position.y < 0) bullets[i].data.bullet.active = false;
            }
        }

        // adding a short interval between each aliens movements
        alien_move_timer += dt;
        if (alien_move_timer >= interval_alien_move) {
            alien_move_timer = 0.0f;
            reverse = 0;
            // This checks if an alien is about to go off screen 
            for (int row = 0; row < alien_rows; row++) {
                for (int col = 0; col < alien_cols; col++) {
                    if (aliens[row][col].data.alien.active) {
                        float next_x = aliens[row][col].data.alien.position.x + alien_direction * alien_speed;
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
                        aliens[row][col].data.alien.position.y += size_alien / 2;
                    }
                }
            } else {
                // Moving the aliens at the same time
                for (int row = 0; row < alien_rows; row++) {
                    for (int col = 0; col < alien_cols; col++) {
                        if (aliens[row][col].data.alien.active) {
                            aliens[row][col].data.alien.position.x += alien_direction * alien_speed;
                        }
                    }
                }
            }
        }

        // checking if an alien reaches the bottom
        for (int row = 0; row < alien_rows; row++) {
            for (int col = 0; col < alien_cols; col++) {
                if (aliens[row][col].data.alien.active && aliens[row][col].data.alien.position.y + size_alien >= position_player.y) {
                    gameover = 1;
                }
            }
        }

        // Alien shooting back and basic alien ai
        int aliens_left = 0;
        int shooting_chance = 2 + (maximum_aliens- aliens_left)/3;
        if (rand() % 100 < 2) {
            // tracking the colomn closet to the player
            int closest_col = 0;
            float min_dist = 1e9f;
            for (int col = 0; col < alien_cols; col++) {
                float aliens_col_x = aliens[0][col].data.alien.position.x + size_alien/2;
                float dist = fabsf(aliens_col_x - (position_player.x + size_player/2));
                if (dist < min_dist) {
                    min_dist = dist;
                    closest_col = col;
                }
            }
            // picking a colomn closest to the player
            int col_range[3];
            int col_count = 0;
            for (int d = -1; d <= 1; d++) {
                int c = closest_col + d;
                // staying within a colomn on the screen
                if (c >= 0 && c < alien_cols) {
                    col_range[col_count++] = c;
                }
            }
            // adds randomness so that it isnt completely targetted
            int shooter_col = col_range[rand() % col_count];
            // this finds the lowest alien in the colomn
            int shooter_row = -1;
            for (int row = alien_rows - 1; row >= 0; row--) {
                if (aliens[row][shooter_col].data.alien.active) {
                    shooter_row = row;
                    break;
                }
            }
            // if there is a valid alien it will shoot
            if (shooter_row != -1) {
                for (int i=0; i<maximum_bullets; i++) {
                    if (!alien_bullets[i].data.alien_bullet.active) {
                        alien_bullets[i].data.alien_bullet.position.x = aliens[shooter_row][shooter_col].data.alien.position.x + size_alien/2 - bullets_radius;
                        alien_bullets[i].data.alien_bullet.position.y = aliens[shooter_row][shooter_col].data.alien.position.y + size_alien;
                        alien_bullets[i].data.alien_bullet.active = true;
                        break;
                    }
                }
            }
        }

        // This updates the alien bullets
        for (int i=0; i<maximum_bullets; i++) {
            if (alien_bullets[i].data.alien_bullet.active) {
                alien_bullets[i].data.alien_bullet.position.y += alien_bulletspeed * dt;
                if (alien_bullets[i].data.alien_bullet.position.y > screenheight) alien_bullets[i].data.alien_bullet.active = false;
                // Check collision with player and reduce lives
                if (CheckCollisionCircleRec(alien_bullets[i].data.alien_bullet.position, bullets_radius, (Rectangle){position_player.x, position_player.y - size_player, size_player, size_player})) {
                    alien_bullets[i].data.alien_bullet.active = false;
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
                if (aliens[row][col].data.alien.active) {
                    aliens_left++;
                }
            }
        }
        if (aliens_left == 0) {
            alien_wave++;
            // Respawn new grid of aliens
            for (int row = 0; row < alien_rows; row++) {
                for (int col = 0; col < alien_cols; col++) {
                    aliens[row][col].data.alien.position.x = startX + col * spacingX;
                    aliens[row][col].data.alien.position.y = startY + row * spacingY;
                    aliens[row][col].data.alien.active = true;
                }
            }
            // this increases the difficuty after each wave
            alien_speed += alien_wave_interval;
            alien_bulletspeed += alien_wave_interval;
        }


        // Player bullet collision with block
        for (int i = 0; i < maximum_bullets; i++) {
            if (bullets[i].data.bullet.active) {
                for (int j = 0; j < Block_no; j++) {
                    Rectangle block_rect = {blocks[j].data.block.x, blocks[j].data.block.y, block_width, block_height};
                    if (CheckCollisionCircleRec(bullets[i].data.bullet.position, bullets_radius, block_rect)) {
                        bullets[i].data.bullet.active = false;
                        break;
                    }
                }
            }
        }
        // Alien bullet collision with block
        for (int i = 0; i < maximum_bullets; i++) {
            if (alien_bullets[i].data.alien_bullet.active) {
                for (int j = 0; j < Block_no; j++) {
                    Rectangle block_rect = {blocks[j].data.block.x, blocks[j].data.block.y, block_width, block_height};
                    if (CheckCollisionCircleRec(alien_bullets[i].data.alien_bullet.position, bullets_radius, block_rect)) {
                        alien_bullets[i].data.alien_bullet.active = false;
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
        if (has_background_texture) {
            DrawTexturePro(
                background_texture,
                (Rectangle){0.0f, 0.0f, (float)background_texture.width, (float)background_texture.height},
                (Rectangle){0.0f, 0.0f, (float)screenwidth, (float)screenheight},
                (Vector2){0.0f, 0.0f},
                0.0f,
                WHITE
            );
        }
        // drawing text on the screen
        DrawText("Use A and D or arrow keys to move the character", 10, 10, 20, YELLOW);
        DrawText("Press SPACE to shoot", 10, 40, 20, YELLOW);
        DrawText(TextFormat("Score: %d", score), screenwidth - 200, 10, 24, WHITE);
        DrawText(TextFormat("Wave %d", alien_wave), screenwidth - 200, 40, 24, WHITE);
        // initialising the pointers for the textures which start at null and are assigned later
        Texture2D *current_player_texture = NULL;
        Texture2D *current_enemy_texture = NULL;
        Texture2D *current_player_shot_texture = NULL;
        Texture2D *current_enemy_shot_texture = NULL;
        // picking which specific texture based on which direction the player is going and for how long
        if (moving_left && !moving_right && has_player_left_texture) {
            if (player_move_hold_time >= player_move_frame_delay && has_player_left_texture_2) {
                current_player_texture = &player_left_texture_2;
            } else {
                current_player_texture = &player_left_texture;
            }
        } else if (moving_right && !moving_left && has_player_right_texture) {
            if (player_move_hold_time >= player_move_frame_delay && has_player_right_texture_2) {
                current_player_texture = &player_right_texture_2;
            } else {
                current_player_texture = &player_right_texture;
            }
        // if no movement just use the default texture
        } else if (has_player_straight_texture) {
            current_player_texture = &player_straight_texture;
        }
        // this switches the enemy texture
        if (sprite_animation_frame_2 && has_enemy_texture_2) {
            current_enemy_texture = &enemy_texture_2;
        }
        else if (has_enemy_texture_1) {
            current_enemy_texture = &enemy_texture_1;
        } 
        else if (has_enemy_texture_2) {
            current_enemy_texture = &enemy_texture_2;
        }
        // selection of player shot textures
        if (sprite_animation_frame_2 && has_player_shot_texture_2) {
            current_player_shot_texture = &player_shot_texture_2;
        } else if (has_player_shot_texture_1) {
            current_player_shot_texture = &player_shot_texture_1;
        } else if (has_player_shot_texture_2) {
            current_player_shot_texture = &player_shot_texture_2;
        }
        // selection of enemy shot textures
        if (sprite_animation_frame_2 && has_enemy_shot_texture_2) {
            current_enemy_shot_texture = &enemy_shot_texture_2;
        } else if (has_enemy_shot_texture_1) {
            current_enemy_shot_texture = &enemy_shot_texture_1;
        } else if (has_enemy_shot_texture_2) {
            current_enemy_shot_texture = &enemy_shot_texture_2;
        }
        // drawing the player if there is a valid texture to use
        if (current_player_texture != NULL) {
            DrawTexturePro(
                *current_player_texture,
                (Rectangle){0.0f, 0.0f, (float)current_player_texture->width, (float)current_player_texture->height},
                (Rectangle){position_player.x, position_player.y - size_player, size_player, size_player},
                (Vector2){0.0f, 0.0f},
                0.0f,
                WHITE
            );
            // if no valid textures then just draw a normal triangle instead
        } else {
            DrawTriangle((Vector2){position_player.x, position_player.y}, (Vector2){position_player.x + size_player, position_player.y}, (Vector2){position_player.x + size_player/2, position_player.y - size_player}, colour_character);
        }
        // draw the bulllets for both player and the aliens
        for (int i=0; i<maximum_bullets; i++) {
            if (bullets[i].data.bullet.active) {
                if (current_player_shot_texture != NULL) {
                    DrawTexturePro(
                        *current_player_shot_texture,
                        (Rectangle){0.0f, 0.0f, (float)current_player_shot_texture->width, (float)current_player_shot_texture->height},
                        (Rectangle){bullets[i].data.bullet.position.x - shot_width/2.0f, bullets[i].data.bullet.position.y - shot_height/2.0f, shot_width, shot_height},
                        (Vector2){0.0f, 0.0f},
                        0.0f,
                        WHITE
                    );
                } else {
                    DrawCircleV(bullets[i].data.bullet.position, bullets_radius, YELLOW);
                }
            }
        }
        for (int i=0; i<maximum_bullets; i++) {
            if (alien_bullets[i].data.alien_bullet.active) {
                if (current_enemy_shot_texture != NULL) {
                    DrawTexturePro(
                        *current_enemy_shot_texture,
                        (Rectangle){0.0f, 0.0f, (float)current_enemy_shot_texture->width, (float)current_enemy_shot_texture->height},
                        (Rectangle){alien_bullets[i].data.alien_bullet.position.x - shot_width/2.0f, alien_bullets[i].data.alien_bullet.position.y - shot_height/2.0f, shot_width, shot_height},
                        (Vector2){0.0f, 0.0f},
                        0.0f,
                        WHITE
                    );
                } else {
                    DrawCircleV(alien_bullets[i].data.alien_bullet.position, bullets_radius, RED);
                }
            }
        }
        // drawing the aliens
        for (int row = 0; row < alien_rows; row++) {
            for (int col = 0; col < alien_cols; col++) {
                if (aliens[row][col].data.alien.active) {
                    if (current_enemy_texture != NULL) {
                        DrawTexturePro(
                            *current_enemy_texture,
                            (Rectangle){0.0f, 0.0f, (float)current_enemy_texture->width, (float)current_enemy_texture->height},
                            (Rectangle){aliens[row][col].data.alien.position.x, aliens[row][col].data.alien.position.y, size_alien, size_alien},
                            (Vector2){0.0f, 0.0f},
                            0.0f,
                            WHITE
                        );
                    } else {
                        DrawRectangle(aliens[row][col].data.alien.position.x, aliens[row][col].data.alien.position.y, size_alien, size_alien, PURPLE);
                    }
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
            DrawRectangle(blocks[i].data.block.x, blocks[i].data.block.y, block_width, block_height, GREEN);
        }
        EndDrawing();
    }

    // basic game over screen
    if (gameover) {
        while (!WindowShouldClose()) {
            BeginDrawing();
            ClearBackground(colour_background);
            if (has_background_texture) {
                DrawTexturePro(
                    background_texture,
                    (Rectangle){0.0f, 0.0f, (float)background_texture.width, (float)background_texture.height},
                    (Rectangle){0.0f, 0.0f, (float)screenwidth, (float)screenheight},
                    (Vector2){0.0f, 0.0f},
                    0.0f,
                    WHITE
                );
            }
            DrawText("Game Over!", screenwidth/2 - 100, screenheight/2 - 20, 40, RED);
            DrawText(TextFormat("Final Score: %d", score), screenwidth/2 - 120, screenheight/2 + 30, 30, WHITE);  
            DrawText(TextFormat("You reached wave %d", alien_wave), screenwidth/2 - 120, screenheight/2 + 70, 30, WHITE);
            DrawText("Press ESC to exit", screenwidth/2 - 100, screenheight/2 + 110, 20, YELLOW);
            EndDrawing();
        }
    }
    // unloading the textures
    if (has_background_texture) UnloadTexture(background_texture);
    if (has_enemy_texture_1) UnloadTexture(enemy_texture_1);
    if (has_enemy_texture_2) UnloadTexture(enemy_texture_2);
    if (has_player_straight_texture) UnloadTexture(player_straight_texture);
    if (has_player_left_texture) UnloadTexture(player_left_texture);
    if (has_player_left_texture_2) UnloadTexture(player_left_texture_2);
    if (has_player_right_texture) UnloadTexture(player_right_texture);
    if (has_player_right_texture_2) UnloadTexture(player_right_texture_2);
    if (has_player_shot_texture_1) UnloadTexture(player_shot_texture_1);
    if (has_player_shot_texture_2) UnloadTexture(player_shot_texture_2);
    if (has_enemy_shot_texture_1) UnloadTexture(enemy_shot_texture_1);
    if (has_enemy_shot_texture_2) UnloadTexture(enemy_shot_texture_2);
    CloseWindow();
    return 0;
}
