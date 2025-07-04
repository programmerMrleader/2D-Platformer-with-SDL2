#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include <vector>
#include <chrono>
#include <utility>
#include <iostream>

// Constants
#define SCREEN_HEIGHT 480
#define SCREEN_WIDTH 640
#define GRAVITY 0.5f
#define JUMP_FORCE -12.0f
#define SDL_MAIN_HANDLED

const double FRAME_DURATION = 150.0;

typedef std::chrono::high_resolution_clock Clock;

typedef struct {
    float x, y;
    float velx, vely;
    bool isJumping;
    int width, height;
} Player;

// Animation states
enum class AnimationState {
    IDLE,
    RUNNING,
    JUMPING,
    CROUCHING
};

int WinMain(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    if (IMG_Init(IMG_INIT_PNG) == 0) {
        std::cerr << "IMG_Init failed: " << IMG_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    
    // Create window and renderer
    SDL_Window* window = SDL_CreateWindow("Platformer Game", 
                                        SDL_WINDOWPOS_UNDEFINED, 
                                        SDL_WINDOWPOS_UNDEFINED,
                                        SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    
    // Load texture
    SDL_Texture* hero = IMG_LoadTexture(renderer, "assets/adventurer-Sheet.png");
    if (!hero) {
        std::cerr << "Failed to load texture: " << IMG_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Game variables
    bool run = true;
    Player player = {100, 100, 0, 0, false, 32, 32};
    const float groundY = 400;
    int player_speed = 5;
    
    // Animation setup
    const size_t nbRow = 11;
    const size_t nbCol = 7;
    const size_t spriteWidth = 50;
    const size_t spriteHeight = 37;
    
    std::vector<SDL_Rect> rects;
    for (size_t i = 0; i < nbRow; i++) {
        for (size_t j = 0; j < nbCol; j++) {
            rects.push_back(SDL_Rect{
                static_cast<int>(j * spriteWidth),
                static_cast<int>(i * spriteHeight),
                static_cast<int>(spriteWidth),
                static_cast<int>(spriteHeight)
            });
        }
    }
    
    // Animation frames - FIXED JUMP ANIMATION
    std::vector<std::pair<size_t, size_t>> idle{{0,0}, {0,1}, {0,2}, {0,3}};
    std::vector<std::pair<size_t, size_t>> crouch{{0,4}, {0,5}, {0,6}, {1,0}};
    std::vector<std::pair<size_t, size_t>> running{{1,1}, {1,2}, {1,3}, {1,4}, {1,5}, {1,6}};
    std::vector<std::pair<size_t, size_t>> jump{{2,0}, {2,1}, {2,2}, {2,3}}; // FIXED
    
    AnimationState currentState = AnimationState::IDLE;
    auto* currentAnimation = &idle;
    size_t animationIndex = 0;
    double timeBuffer = 0;
    auto lastTime = Clock::now();
    
    // Direction handling
    bool facingRight = true;
    bool moving = false;

    // Main game loop
    while (run) {
        auto now = Clock::now();
        double deltaTime = std::chrono::duration<double, std::milli>(now - lastTime).count(); // FIXED
        lastTime = now;

        // Event handling
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                run = false;
            }
            else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.scancode) {
                    case SDL_SCANCODE_ESCAPE:
                        run = false;
                        break;
                    case SDL_SCANCODE_SPACE:
                        if (!player.isJumping) {
                            player.vely = JUMP_FORCE;
                            player.isJumping = true;
                            currentState = AnimationState::JUMPING;
                            currentAnimation = &jump;
                            animationIndex = 0;
                        }
                        break;
                    case SDL_SCANCODE_LCTRL:
                        currentState = AnimationState::CROUCHING;
                        currentAnimation = &crouch;
                        animationIndex = 0;
                        break;
                }
            }
            else if (event.type == SDL_KEYUP) {
                switch (event.key.keysym.scancode) {
                    case SDL_SCANCODE_LCTRL:
                        if (!player.isJumping) {
                            currentState = AnimationState::IDLE;
                            currentAnimation = &idle;
                            animationIndex = 0;
                        }
                        break;
                }
            }
        }

        // Keyboard state handling
        const Uint8* key = SDL_GetKeyboardState(NULL);
        moving = false;
        
        // Handle movement
        if (key[SDL_SCANCODE_D]) {
            player.x += player_speed;
            facingRight = true;
            moving = true;
            if (!player.isJumping && currentState != AnimationState::CROUCHING) {
                currentState = AnimationState::RUNNING;
                currentAnimation = &running;
            }
        }
        if (key[SDL_SCANCODE_A]) {
            player.x -= player_speed;
            facingRight = false;
            moving = true;
            if (!player.isJumping && currentState != AnimationState::CROUCHING) {
                currentState = AnimationState::RUNNING;
                currentAnimation = &running;
            }
        }
        
        // If not moving and not jumping, return to idle
        if (!moving && !player.isJumping && currentState != AnimationState::CROUCHING) {
            currentState = AnimationState::IDLE;
            currentAnimation = &idle;
            animationIndex = 0;
        }

        // Physics
        player.vely += GRAVITY;
        player.y += player.vely;

        // Ground collision
        if (player.y >= groundY) {
            player.y = groundY;
            player.vely = 0;
            player.isJumping = false;
            
            // Reset to idle or running after landing - FIXED ANIMATION RESET
            if (currentState == AnimationState::JUMPING) {
                currentState = moving ? AnimationState::RUNNING : AnimationState::IDLE;
                currentAnimation = moving ? &running : &idle;
                animationIndex = 0;
            }
        }

        // Animation update
        timeBuffer += deltaTime;
        if (timeBuffer > FRAME_DURATION) {
            timeBuffer = 0;
            animationIndex = (animationIndex + 1) % currentAnimation->size();
        }

        // Rendering
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Get current frame
        if (currentAnimation && animationIndex < currentAnimation->size()) {
            auto currentPair = (*currentAnimation)[animationIndex];
            size_t position = currentPair.second + currentPair.first * nbCol;
            
            if (position < rects.size()) {
                // Draw player
                SDL_Rect dest_rect = {
                    static_cast<int>(player.x),
                    static_cast<int>(player.y),
                    player.width,
                    player.height
                };

                SDL_RenderCopyEx(
                    renderer,
                    hero,
                    &rects[position],
                    &dest_rect,
                    0,
                    NULL,
                    facingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL
                );
            }
        }

        // Draw ground line (for debugging)
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawLine(renderer, 0, static_cast<int>(groundY), 
                          SCREEN_WIDTH, static_cast<int>(groundY));

        SDL_RenderPresent(renderer);
        SDL_Delay(10);  // Cap frame rate
    }

    // Cleanup
    SDL_DestroyTexture(hero);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    
    return 0;
}