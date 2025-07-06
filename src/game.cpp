#include "settings.h"

#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"

#include <iostream>
bool initSDL(Game& game) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return false;
    }
    
    if (IMG_Init(IMG_INIT_PNG) == 0) {
        std::cerr << "IMG_Init failed: " << IMG_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    game.window = SDL_CreateWindow("Platformer Game", 
                                  SDL_WINDOWPOS_UNDEFINED, 
                                  SDL_WINDOWPOS_UNDEFINED,
                                  SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!game.window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        IMG_Quit();
        SDL_Quit();
        return false;
    }
    
    game.renderer = SDL_CreateRenderer(game.window, -1, SDL_RENDERER_ACCELERATED);
    if (!game.renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(game.window);
        IMG_Quit();
        SDL_Quit();
        return false;
    }
    
    return true;
}

// Load game resources
bool loadResources(Game& game) {
    game.playerTexture = IMG_LoadTexture(game.renderer, "assets/adventurer-Sheet.png");
    if (!game.playerTexture) {
        std::cerr << "Failed to load texture: " << IMG_GetError() << std::endl;
        return false;
    }

    // Initialize animations
    // Format: {row, {frame1, frame2, ...}}
    game.animations = {
        {0, 4, false, 0},    // IDLE: row 0, 4 frames
        {1, 6, false, 0},    // RUNNING: row 1, 6 frames
        {2, 4, false, 0},    // JUMPING: row 2, 4 frames
        {0, 6, false, 0},    // CROUCHING: row 0, frames 4-6 (adjust as needed)
        {5, 12, true, 6}     // ATTACKING: starts at row 3, 12 frames, 6 per row (2 rows)
    };

    return true;
}

// Initialize game state
void initGame(Game& game) {
    game.player = {100, 100, 0, 0, false, 50, 50, AnimationState::IDLE, true};
    game.groundY = 400;
    game.currentAnimIndex = 0;
    game.animFrame = 0;
    game.animTimer = 0;
}

// Handle keyboard input
void handleInput(Game& game, const Uint8* keys) {
    Player& p = game.player;
    
    // Reset movement flag
    bool moving = false;
    
    // Handle movement
    if (keys[SDL_SCANCODE_D] && p.x < SCREEN_WIDTH - p.width) {
        p.x += PLAYER_SPEED;
        p.facingRight = true;
        moving = true;
        if (!p.isJumping && p.state != AnimationState::CROUCHING) {
            p.state = AnimationState::RUNNING;
        }
    }
    if (keys[SDL_SCANCODE_A] && p.x > 0) {
        p.x -= PLAYER_SPEED;
        p.facingRight = false;
        moving = true;
        if (!p.isJumping && p.state != AnimationState::CROUCHING) {
            p.state = AnimationState::RUNNING;
        }
    }
    
    // Jumping
    if (keys[SDL_SCANCODE_SPACE] && !p.isJumping && p.state != AnimationState::ATTACKING) {
        p.vely = JUMP_FORCE;
        p.isJumping = true;
        p.state = AnimationState::JUMPING;
    }
    
    // Crouching
    if (keys[SDL_SCANCODE_LCTRL] && !p.isJumping) {
        p.state = AnimationState::CROUCHING;
    }
    
    // Attacking
    if (keys[SDL_SCANCODE_E] && !p.isJumping) {
        p.state = AnimationState::ATTACKING;
    }
    
    // Return to idle if not moving
    if (!moving && !p.isJumping && p.state != AnimationState::CROUCHING && p.state != AnimationState::ATTACKING) {
        p.state = AnimationState::IDLE;
    }
}

// Update game state
void updateGame(Game& game, double deltaTime) {
    Player& p = game.player;
    
    // Apply physics
    p.vely += GRAVITY;
    p.y += p.vely;
    
    // Ground collision
    if (p.y >= game.groundY) {
        p.y = game.groundY;
        p.vely = 0;
        p.isJumping = false;
        
        // Reset to idle after landing
        if (p.state == AnimationState::JUMPING) {
            p.state = AnimationState::IDLE;
        }
    }
    
    // Update animation
    game.animTimer += deltaTime;
    const Animation& anim = game.animations[static_cast<int>(game.player.state)];
    if (game.animTimer > ANIMATION_FRAME_DURATION) {
        game.animTimer = 0;
        game.animFrame++;
        if (game.animFrame >= anim.frameCount) {
            game.animFrame = 0;
            if (game.player.state == AnimationState::ATTACKING) {
                game.player.state = AnimationState::IDLE;
            }
        }
    }
}

// Render the game
void renderGame(Game& game) {
    // Clear screen
    SDL_SetRenderDrawColor(game.renderer, 0, 0, 0, 255);
    SDL_RenderClear(game.renderer);
    
    // Get current animation frame
    const Animation& anim = game.animations[static_cast<int>(game.player.state)];
    int frame = game.animFrame;

    int row = anim.startRow;
    int col = frame;

    if (anim.multiRow) {
        row += frame / anim.framesPerRow;
        col = frame % anim.framesPerRow;
    }

    SDL_Rect srcRect = {
        col * 50,  // Assuming 50px wide frames
        row * 37,  // Assuming 37px tall frames
        50,
        37
    };
    
    // Destination rectangle for player
    SDL_Rect destRect = {
        static_cast<int>(game.player.x),
        static_cast<int>(game.player.y),
        game.player.width,
        game.player.height
    };
    
    // Draw player
    SDL_RenderCopyEx(
        game.renderer,
        game.playerTexture,
        &srcRect,
        &destRect,
        0,
        NULL,
        game.player.facingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL
    );
    
    // Draw ground line
    SDL_SetRenderDrawColor(game.renderer, 255, 255, 255, 255);
    SDL_RenderDrawLine(game.renderer, 0, static_cast<int>(game.groundY), 
                      SCREEN_WIDTH, static_cast<int>(game.groundY));
    
    // Present to screen
    SDL_RenderPresent(game.renderer);
}

// Clean up resources
void cleanup(Game& game) {
    SDL_DestroyTexture(game.playerTexture);
    SDL_DestroyRenderer(game.renderer);
    SDL_DestroyWindow(game.window);
    IMG_Quit();
    SDL_Quit();
}
