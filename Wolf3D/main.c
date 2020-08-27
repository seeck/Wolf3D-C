//
//  main.c
//  Wolf3D
//
//  Created by Chaitanya Kochhar on 8/20/20.
//  Copyright © 2020 Chaitanya Kochhar. All rights reserved.
//

#include <stdio.h>
#include <SDL2/SDL.h>
#include "constants.h"
#include <limits.h>

const int map[MAP_NUM_ROWS][MAP_NUM_COLS] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ,1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

struct Player {
    float x;
    float y;
    float width;
    float height;
    int turnDirection; //-1: left, 1:right
    int walkDirection; //-1: back, 1:front
    float rotationAngle;
    float walkSpeed;
    float turnSpeed;
} player;

struct Ray {
    float rayAngle;
    float wallHitX;
    float wallHitY;
    int wasHitVertical;
    float distance;
    int isRayFacingUp;
    int isRayFacingDown;
    int isRayFacingLeft;
    int isRayFacingRight;
    int wallHitContent;
} rays[NUM_RAYS];

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
Uint32* colorBuffer = NULL;
SDL_Texture* colorBufferTexture = NULL;
Uint32* wallTexture = NULL;
int isGameRunning = FALSE;
int ticksLastFrame;



int initializeWindow() {
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initializing SDL. \n");
        return FALSE;
    }
    window = SDL_CreateWindow(
                              "Wolf3d",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              WINDOW_WIDTH,
                              WINDOW_HEIGHT,
                              SDL_WINDOW_BORDERLESS
                              );
    if(!window) {
        fprintf(stderr, "Error creating SDL Window \n");
        return FALSE;
    }
    
    renderer = SDL_CreateRenderer(window, -1, 0);
    if(!renderer) {
        fprintf(stderr, "Error creating SDL Renderer \n");
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    return TRUE;
}

void destroyWindow() {
    free(colorBuffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void setup() {
    //TODO: initialize and set up game objects
    player.x = WINDOW_WIDTH/2;
    player.y = WINDOW_HEIGHT/2;
    player.width = 5;
    player.height = 5;
    player.turnDirection = 0;
    player.walkDirection = 0;
    player.rotationAngle = PI/2;
    player.walkSpeed = 100;
    player.turnSpeed = 45 * (PI/180);
    
    //Allocate the frame buffer
    colorBuffer = (Uint32*) malloc(sizeof(Uint32) * (Uint32)(WINDOW_WIDTH * WINDOW_HEIGHT));
    colorBufferTexture =SDL_CreateTexture(
                                          renderer,
                                          SDL_PIXELFORMAT_ARGB8888,
                                          SDL_TEXTUREACCESS_STREAMING,
                                          WINDOW_WIDTH,
                                          WINDOW_HEIGHT
                                          );
    wallTexture = (Uint32*) malloc(sizeof(Uint32) * (Uint32)(TEXTURE_WIDTH * TEXTURE_HEIGHT));
    for(int x = 0; x < TEXTURE_WIDTH; x++)
        for(int y = 0; y < TEXTURE_HEIGHT; y++) {
            wallTexture[y*TEXTURE_WIDTH + x] = (x%8 && y%8) ? 0xff0000ff : 0xff000000;
        }
}

int mapHasWallAt(float x, float y) {
    if(x < 0 || x > WINDOW_WIDTH || y < 0 || y > WINDOW_HEIGHT) {
        return TRUE;
    }
    int _x = floor(x/TILE_SIZE);
    int _y = floor(y/TILE_SIZE);
    return map[_y][_x] != 0;
}

void movePlayer(float deltaTime) {
    player.rotationAngle += player.turnDirection * player.turnSpeed * deltaTime;
    int moveStep = player.walkDirection * player.walkSpeed * deltaTime;
    float newX = player.x + cos(player.rotationAngle) * moveStep;
    float newY = player.y + sin(player.rotationAngle) * moveStep;
    
    //Wall collision.
    if(!mapHasWallAt(newX, newY)) {
        player.x = newX;
        player.y = newY;
    }
    
    
}

void renderPlayer() {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect playerRect = {
        player.x * MINIMAP_SCALE_FACTOR,
        player.y * MINIMAP_SCALE_FACTOR,
        player.width * MINIMAP_SCALE_FACTOR,
        player.height * MINIMAP_SCALE_FACTOR
    };
    SDL_RenderFillRect(renderer, &playerRect);
    SDL_RenderDrawLine
    (
     renderer,
     player.x * MINIMAP_SCALE_FACTOR,
     player.y * MINIMAP_SCALE_FACTOR,
     MINIMAP_SCALE_FACTOR * (player.x + cos(player.rotationAngle) * 40),
     MINIMAP_SCALE_FACTOR * (player.y + sin(player.rotationAngle) * 40)
     );
}

float normalizeAngle(float angle) {
    angle = remainderf(angle, TWO_PI);
    if(angle < 0) {
        angle += TWO_PI;
    }
    return angle;
}

float distanceBetweenPoints(float x1, float y1, float x2, float y2) {
    return sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
}

void castRay(float rayAngle, int stripId) {
    rayAngle = normalizeAngle(rayAngle);
    
    int isRayFacingDown = rayAngle > 0 && rayAngle < PI;
    int isRayFacingUp = !isRayFacingDown;
    
    int isRayFacingRight = rayAngle < 0.5 * PI || rayAngle > 1.5 * PI;
    int isRayFacingLeft = !isRayFacingRight;
    
    float xintercept, yintercept;
    float xstep, ystep;
    
    ///////////////////////////////////////////
    // HORIZONTAL RAY-GRID INTERSECTION CODE
    ///////////////////////////////////////////
    int foundHorzWallHit = FALSE;
    float horzWallHitX = 0;
    float horzWallHitY = 0;
    int horzWallContent = 0;
    
    // Find the y-coordinate of the closest horizontal grid intersection
    yintercept = floor(player.y / TILE_SIZE) * TILE_SIZE;
    yintercept += isRayFacingDown ? TILE_SIZE : 0;
    
    // Find the x-coordinate of the closest horizontal grid intersection
    xintercept = player.x + (yintercept - player.y) / tan(rayAngle);
    
    // Calculate the increment xstep and ystep
    ystep = TILE_SIZE;
    ystep *= isRayFacingUp ? -1 : 1;
    
    xstep = TILE_SIZE / tan(rayAngle);
    xstep *= (isRayFacingLeft && xstep > 0) ? -1 : 1;
    xstep *= (isRayFacingRight && xstep < 0) ? -1 : 1;
    
    float nextHorzTouchX = xintercept;
    float nextHorzTouchY = yintercept;
    
    // Increment xstep and ystep until we find a wall
    while (nextHorzTouchX >= 0 && nextHorzTouchX <= WINDOW_WIDTH && nextHorzTouchY >= 0 && nextHorzTouchY <= WINDOW_HEIGHT) {
        float xToCheck = nextHorzTouchX;
        float yToCheck = nextHorzTouchY + (isRayFacingUp ? -1 : 0);
        
        if (mapHasWallAt(xToCheck, yToCheck)) {
            // found a wall hit
            horzWallHitX = nextHorzTouchX;
            horzWallHitY = nextHorzTouchY;
            horzWallContent = map[(int)floor(yToCheck / TILE_SIZE)][(int)floor(xToCheck / TILE_SIZE)];
            foundHorzWallHit = TRUE;
            break;
        } else {
            nextHorzTouchX += xstep;
            nextHorzTouchY += ystep;
        }
    }
    
    ///////////////////////////////////////////
    // VERTICAL RAY-GRID INTERSECTION CODE
    ///////////////////////////////////////////
    int foundVertWallHit = FALSE;
    float vertWallHitX = 0;
    float vertWallHitY = 0;
    int vertWallContent = 0;
    
    // Find the x-coordinate of the closest horizontal grid intersection
    xintercept = floor(player.x / TILE_SIZE) * TILE_SIZE;
    xintercept += isRayFacingRight ? TILE_SIZE : 0;
    
    // Find the y-coordinate of the closest horizontal grid intersection
    yintercept = player.y + (xintercept - player.x) * tan(rayAngle);
    
    // Calculate the increment xstep and ystep
    xstep = TILE_SIZE;
    xstep *= isRayFacingLeft ? -1 : 1;
    
    ystep = TILE_SIZE * tan(rayAngle);
    ystep *= (isRayFacingUp && ystep > 0) ? -1 : 1;
    ystep *= (isRayFacingDown && ystep < 0) ? -1 : 1;
    
    float nextVertTouchX = xintercept;
    float nextVertTouchY = yintercept;
    
    // Increment xstep and ystep until we find a wall
    while (nextVertTouchX >= 0 && nextVertTouchX <= WINDOW_WIDTH && nextVertTouchY >= 0 && nextVertTouchY <= WINDOW_HEIGHT) {
        float xToCheck = nextVertTouchX + (isRayFacingLeft ? -1 : 0);
        float yToCheck = nextVertTouchY;
        
        if (mapHasWallAt(xToCheck, yToCheck)) {
            // found a wall hit
            vertWallHitX = nextVertTouchX;
            vertWallHitY = nextVertTouchY;
            vertWallContent = map[(int)floor(yToCheck / TILE_SIZE)][(int)floor(xToCheck / TILE_SIZE)];
            foundVertWallHit = TRUE;
            break;
        } else {
            nextVertTouchX += xstep;
            nextVertTouchY += ystep;
        }
    }
    
    // Calculate both horizontal and vertical hit distances and choose the smallest one
    float horzHitDistance = foundHorzWallHit
    ? distanceBetweenPoints(player.x, player.y, horzWallHitX, horzWallHitY)
    : INT_MAX;
    float vertHitDistance = foundVertWallHit
    ? distanceBetweenPoints(player.x, player.y, vertWallHitX, vertWallHitY)
    : INT_MAX;
    
    if (vertHitDistance < horzHitDistance) {
        rays[stripId].distance = vertHitDistance;
        rays[stripId].wallHitX = vertWallHitX;
        rays[stripId].wallHitY = vertWallHitY;
        rays[stripId].wallHitContent = vertWallContent;
        rays[stripId].wasHitVertical = TRUE;
    } else {
        rays[stripId].distance = horzHitDistance;
        rays[stripId].wallHitX = horzWallHitX;
        rays[stripId].wallHitY = horzWallHitY;
        rays[stripId].wallHitContent = horzWallContent;
        rays[stripId].wasHitVertical = FALSE;
    }
    rays[stripId].rayAngle = rayAngle;
    rays[stripId].isRayFacingDown = isRayFacingDown;
    rays[stripId].isRayFacingUp = isRayFacingUp;
    rays[stripId].isRayFacingLeft = isRayFacingLeft;
    rays[stripId].isRayFacingRight = isRayFacingRight;
}

void castAllRays() {
    //Start first ray subtracting half of our FOV
    float rayAngle = player.rotationAngle - (FOV_ANGLE/2);
    for(int stripId = 0; stripId < NUM_RAYS; stripId++) {
        castRay(rayAngle,stripId);
        rayAngle += FOV_ANGLE / NUM_RAYS;
    }
}

void renderMap() {
    for(int i = 0 ; i < MAP_NUM_ROWS; i++) {
        for(int j = 0; j < MAP_NUM_COLS; j++) {
            int tileX = j * TILE_SIZE;
            int tileY = i * TILE_SIZE;
            int tileColor = map[i][j] != 0 ? 255 : 0;
            SDL_SetRenderDrawColor(renderer, tileColor, tileColor, tileColor, 255);
            SDL_Rect mapTileRect = {
                tileX * MINIMAP_SCALE_FACTOR,
                tileY * MINIMAP_SCALE_FACTOR,
                TILE_SIZE * MINIMAP_SCALE_FACTOR,
                TILE_SIZE * MINIMAP_SCALE_FACTOR
            };
            SDL_RenderFillRect(renderer, &mapTileRect);
        }
    }
}

void renderRays() {
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for(int i = 0; i < NUM_RAYS; i++) {
        SDL_RenderDrawLine(renderer,
                           MINIMAP_SCALE_FACTOR * player.x,
                           MINIMAP_SCALE_FACTOR * player.y,
                           MINIMAP_SCALE_FACTOR * rays[i].wallHitX,
                           MINIMAP_SCALE_FACTOR * rays[i].wallHitY
                           );
    }
}

void processInput() {
    SDL_Event event;
    SDL_PollEvent(&event);
    switch(event.type) {
        case SDL_QUIT:
        {
            isGameRunning = FALSE;
            break;
        }
        case SDL_KEYDOWN:{
            if(event.key.keysym.sym == SDLK_ESCAPE) {
                isGameRunning = FALSE;
            }
            if(event.key.keysym.sym == SDLK_UP) {
                player.walkDirection = 1;
            }
            if(event.key.keysym.sym == SDLK_DOWN) {
                player.walkDirection = -1;
            }
            if(event.key.keysym.sym == SDLK_LEFT) {
                player.turnDirection  = -1;
            }
            if(event.key.keysym.sym == SDLK_RIGHT) {
                player.turnDirection = 1;
            }
            break;
        }
        case SDL_KEYUP: {
            if(event.key.keysym.sym == SDLK_UP) {
                player.walkDirection = 0;
            }
            if(event.key.keysym.sym == SDLK_DOWN) {
                player.walkDirection = 0;
            }
            if(event.key.keysym.sym == SDLK_LEFT) {
                player.turnDirection  = 0;
            }
            if(event.key.keysym.sym == SDLK_RIGHT) {
                player.turnDirection = 0;
            }
            break;
        }
            
            
    }
}

void update() {
    int timeToWait = FRAME_TIME_LENGTH - (SDL_GetTicks() - ticksLastFrame);
    if(timeToWait > 0 && timeToWait <=FRAME_TIME_LENGTH) {
        SDL_Delay(timeToWait);
    }
    float deltaTime = (SDL_GetTicks() - ticksLastFrame)/1000.0f;
    ticksLastFrame = SDL_GetTicks();
    movePlayer(deltaTime);
    castAllRays();
    
}

void generate3DProjection() {
    for(int i = 0; i < NUM_RAYS; i++) {
        float distanceProjPlane = (WINDOW_WIDTH) / 2 / tan(FOV_ANGLE/2);
        float perpDistance = rays[i].distance * cos(rays[i].rayAngle - player.rotationAngle);
        float projectedWallHeight = (TILE_SIZE/perpDistance) * distanceProjPlane;
        int wallStripHeight = (int) projectedWallHeight;
        
        int wallTopPixel = (WINDOW_HEIGHT/2) - (wallStripHeight/2);
        wallTopPixel = wallTopPixel < 0 ? 0 : wallTopPixel;
        
        int wallBottomPixel = (WINDOW_HEIGHT/2) + (wallStripHeight/2);
        wallBottomPixel = wallBottomPixel > WINDOW_HEIGHT ? WINDOW_HEIGHT : wallBottomPixel;
        
        for(int y = 0; y < wallTopPixel; y++)
            colorBuffer[(WINDOW_WIDTH)*y + i] = 0xff333333;
        
        //Calculate X offset.
        int textureOffsetX;
        if(rays[i].wasHitVertical) {
            textureOffsetX = (int)rays[i].wallHitY % TILE_SIZE;
        }
        else {
            textureOffsetX = (int)rays[i].wallHitX % TILE_SIZE;
        }
        
        
        for(int y = wallTopPixel; y < wallBottomPixel; y++) {
            //TODO: Calculate texture Offset Y.
            int distanceFromTop = (y + (wallStripHeight/2) - (WINDOW_HEIGHT/2));
            int textureOffsetY = distanceFromTop * ((float)TEXTURE_HEIGHT/wallStripHeight);
            Uint32 texelColor = wallTexture[(TEXTURE_WIDTH * textureOffsetY) + textureOffsetX];
            colorBuffer[WINDOW_WIDTH * y + i] = texelColor;
        }
        
        for(int y = wallBottomPixel; y < WINDOW_HEIGHT; y++)
            colorBuffer[WINDOW_WIDTH*y + i] = 0xff777777;
    }
}

void clearColorBuffer(Uint32 color) {
    for(int i = 0; i < WINDOW_WIDTH; i++) {
        for(int j = 0; j < WINDOW_HEIGHT; j++) {
            colorBuffer[(j * WINDOW_WIDTH) + i] = color;
        }
    }
}

void renderColorBuffer() {
    SDL_UpdateTexture(
                      colorBufferTexture,
                      NULL,
                      colorBuffer,
                      (int)((Uint32) WINDOW_WIDTH * sizeof(Uint32))
                      );
    SDL_RenderCopy(renderer, colorBufferTexture, NULL, NULL);
    
}

void render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    generate3DProjection();
    
    renderColorBuffer();
    clearColorBuffer(0xff000000);
    
    renderMap();
    renderRays();
    renderPlayer();
    
    SDL_RenderPresent(renderer);
}

int main(int argc, const char * argv[]) {
    isGameRunning = initializeWindow();
    setup();
    while(isGameRunning) {
        processInput();
        update();
        render();
    }
    destroyWindow();
    return 0;
}
