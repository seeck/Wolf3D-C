//
//  constants.h
//  Wolf3D
//
//  Created by Chaitanya Kochhar on 8/20/20.
//  Copyright © 2020 Chaitanya Kochhar. All rights reserved.
//

#ifndef constants_h
#define constants_h

#define FALSE 0
#define TRUE 1

#define PI 3.14159265
#define TWO_PI 6.28318530

#define TILE_SIZE 64
#define MAP_NUM_ROWS 13
#define MAP_NUM_COLS 20

#define MINIMAP_SCALE_FACTOR 0.3

#define WINDOW_WIDTH (MAP_NUM_COLS * TILE_SIZE)
#define WINDOW_HEIGHT (MAP_NUM_ROWS * TILE_SIZE)

#define TEXTURE_WIDTH 64
#define TEXTURE_HEIGHT 64

#define FOV_ANGLE (90 * (PI / 180))

#define NUM_RAYS WINDOW_WIDTH

#define FPS 60
#define FRAME_TIME_LENGTH (1000 / FPS)

#endif /* constants_h */
