#pragma "once"
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include <vector>
#include <iostream>
// Game constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const float GRAVITY = 0.5f;
const float JUMP_FORCE = -12.0f;
const float PLAYER_SPEED = 5.0f;
const int ANIMATION_FRAME_DURATION = 150; // ms

//enum
enum class AnimationState {
    IDLE,
    RUNNING,
    JUMPING,
    CROUCHING,
    ATTACKING
};
//structure
struct Player {
    float x, y;
    float velx, vely;
    bool isJumping;
    int width, height;
    AnimationState state;
    bool facingRight;
};

struct Animation {
    int startRow;         // Starting row in sprite sheet
    int frameCount;       // Total number of frames
    bool multiRow;        // Does this animation span multiple rows?
    int framesPerRow;     // Frames per row if multiRow is true
};

struct Game {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* playerTexture;
    Player player;
    float groundY;
    std::vector<Animation> animations;
    int currentAnimIndex;
    int animFrame;
    double animTimer;
};
//function definaction
bool initSDL(Game& game);
bool loadResources(Game& game);
void initGame(Game& game);
void handleInput(Game& game, const Uint8* keys);
void updateGame(Game& game, double deltaTime);
void renderGame(Game& game);
void cleanup(Game& game);
