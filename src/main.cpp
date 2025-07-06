#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include <iostream>
#include <vector>
#include "settings.h"


int main(int argc, char* argv[]) {
    Game game;
    
    // Initialize everything
    if (!initSDL(game)) return 1;
    if (!loadResources(game)) {
        cleanup(game);
        return 1;
    }
    initGame(game);
    
    // Main game loop
    bool running = true;
    Uint32 lastTime = SDL_GetTicks();
    
    while (running) {
        // Calculate delta time
        Uint32 currentTime = SDL_GetTicks();
        double deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        
        // Event handling
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                    running = false;
                }
            }
        }
        
        // Get keyboard state
        const Uint8* keys = SDL_GetKeyboardState(NULL);
        
        // Handle input
        handleInput(game, keys);
        
        // Update game state
        updateGame(game, deltaTime);
        
        // Render game
        renderGame(game);
        
        // Cap frame rate
        SDL_Delay(16); // ~60 FPS
    }
    
    // Cleanup
    cleanup(game);
    return 0;
}
