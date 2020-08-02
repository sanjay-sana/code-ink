/*
 * gz.h - include file for rendering library
 * CSCI 580   USC 
*/

#pragma once

#define GZ_SUCCESS      0
#define GZ_FAILURE      1

/*
 * display classes
 */
#define GZ_RGBAZ_DISPLAY         1

/*
 * rendering classes
 */
#define GZ_Z_BUFFER_RENDER      1

typedef int     GzRenderClass;
typedef int     GzDisplayClass;
typedef void    *GzPointer;
typedef float   GzColor[3];
typedef short   GzIntensity;	/* 0 - 4095 in lower 12-bits */
typedef int	GzDepth;	/* z is signed for clipping */


#define RED     0               /* array indicies for color vector */
#define GREEN   1
#define BLUE    2

#define X       0               /* array indicies for position vector */
#define Y       1
#define Z       2

#define U       0               /* array indicies for texture coords */
#define V       1
