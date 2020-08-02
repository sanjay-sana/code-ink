// Application3.cpp: implementation of the Application3 class.
//
//////////////////////////////////////////////////////////////////////

/*
 * application test code for homework assignment #3
*/

#define INFILE3  "pot4.asc"
#define OUTFILE3 "output.ppm"

#include "Application3.h"
#include "gz.h"
#include "disp.h"
#include "rend.h"
#include <stdio.h>
#include <stdlib.h>

void shade(GzCoord norm, GzCoord color);


int Application3::runRenderer()
{
	GzCamera	camera;
	GzToken		nameListTriangle[3]; 	/* vertex attribute names */
	GzPointer	valueListTriangle[3]; 	/* vertex attribute pointers */
	GzToken         nameListColor[3];       /* color type names */
	GzPointer       valueListColor[3];      /* color type rgb pointers */
	GzColor		color; 
	GzCoord		vertexList[3];		/* vertex position coordinates */ 
	GzCoord		normalList[3];		/* vertex normals */ 
	GzTextureIndex	uvList[3];		/* vertex texture map indices */ 
	char		dummy[256]; 
	int		i; 
	int		xRes, yRes, dispClass;	/* display parameters */ 
	int		status; 
 
	GzMatrix	translateAndScale = 
	{ 
		3.25,	0.0,	0.0,	0.0, 
		0.0,	3.25,	0.0,	-3.25, 
		0.0,	0.0,	3.25,	3.5, 
		0.0,	0.0,	0.0,	1.0 
	}; 
	 
	GzMatrix	rotateX = 
	{ 
		1.0,	0.0,	0.0,	0.0, 
		0.0,	.7071,	.7071,	0.0, 
		0.0,	-.7071,	.7071,	0.0, 
		0.0,	0.0,	0.0,	1.0 
	}; 
	 
	GzMatrix	rotateY = 
	{ 
		.866,	0.0,	-0.5,	0.0, 
		0.0,	1.0,	0.0,	0.0, 
		0.5,	0.0,	.866,	0.0, 
		0.0,	0.0,	0.0,	1.0 
	};

	/* 
	 * initialize the display and the renderer 
	 */ 

	int width = 256;    // frame buffer and display width
	int height = 256;   // frame buffer and display height

	status = 0;

	status |= GzNewDisplay(&display, GZ_RGBAZ_DISPLAY, width, height);

	status |= GzGetDisplayParams(display, &xRes, &yRes, &dispClass); 
	 
	status |= GzInitDisplay(display); 
 
	status |= GzNewRender(&render, GZ_Z_BUFFER_RENDER, display); 

#if 1 	/* set up app-defined camera if desired, else use camera defaults */
	camera.position[X] = 13.2;      
  	camera.position[Y] = -8.7;
  	camera.position[Z] = -14.8;

  	camera.lookat[X] = 0.8;
  	camera.lookat[Y] = 0.7;
  	camera.lookat[Z] = 4.5;

  	camera.worldup[X] = -0.2;
  	camera.worldup[Y] = 1.0;
  	camera.worldup[Z] = 0.0;

	camera.FOV = 53.7;              /* degrees */

	status |= GzPutCamera(render, &camera); 
#endif 

	status |= GzBeginRender(render);

	status |= GzPushMatrix(render, translateAndScale);  
	status |= GzPushMatrix(render, rotateY); 
	status |= GzPushMatrix(render, rotateX);

	if (status) return(GZ_FAILURE); 

	 /* 
	 * Tokens associated with triangle vertex values 
	 */ 
	 nameListTriangle[0] = GZ_POSITION; 
	 
	// I/O File open
	FILE *infile;
	if( (infile  = fopen( INFILE3 , "r" )) == NULL )
	{
		printf("Could not open input from %s \n", INFILE3);
		return GZ_FAILURE;
	}

	FILE *outfile;
	if( (outfile  = fopen( OUTFILE3 , "wb" )) == NULL )
	{
		printf("Could not open output file for writing %s \n", OUTFILE3);
		return GZ_FAILURE;
	}

	/* 
	* Walk through the list of triangles, set color 
	* and render each triangle 
	*/ 
	i = 0; 
	while( fscanf(infile, "%s", dummy) == 1) { 	/* read in tri word */
	    fscanf(infile, "%f %f %f %f %f %f %f %f", 
		&(vertexList[0][0]), &(vertexList[0][1]),  
		&(vertexList[0][2]), 
		&(normalList[0][0]), &(normalList[0][1]), 	
		&(normalList[0][2]), 
		&(uvList[0][0]), &(uvList[0][1]) ); 
	    fscanf(infile, "%f %f %f %f %f %f %f %f", 
		&(vertexList[1][0]), &(vertexList[1][1]), 	
		&(vertexList[1][2]), 
		&(normalList[1][0]), &(normalList[1][1]), 	
		&(normalList[1][2]), 
		&(uvList[1][0]), &(uvList[1][1]) ); 
	    fscanf(infile, "%f %f %f %f %f %f %f %f", 
		&(vertexList[2][0]), &(vertexList[2][1]), 	
		&(vertexList[2][2]), 
		&(normalList[2][0]), &(normalList[2][1]), 	
		&(normalList[2][2]), 
		&(uvList[2][0]), &(uvList[2][1]) ); 

	    /* 
	    * Set up shading attributes for each triangle 
	    */ 
	    shade(normalList[0], color);/* shade based on the norm of vert0 */
	    valueListColor[0] = (GzPointer)color; 
	    nameListColor[0] = GZ_RGB_COLOR; 
	    GzPutAttribute(render, 1, nameListColor, valueListColor); 
 
	     /* 
	     * Set the value pointers to the first vertex of the 	
	     * triangle, then feed it to the renderer 
	     */ 
	     valueListTriangle[0] = (GzPointer)vertexList;
 
	     GzPutTriangle(render, 1, nameListTriangle, valueListTriangle); 
	} 

	GzFlushDisplay2File(outfile, display); 	/* write out or update display to file*/

	/* 
	 * Clean up and exit
	 */ 

	if( fclose( infile ) )
		printf( "The input file was not closed\n" );
  
	if( fclose( outfile ) )
		printf( "The output file was not closed\n" );

	status |= GzFreeRender(render); 
	status |= GzFreeDisplay(display); 
 
	if (status) 
		return(GZ_FAILURE); 
	else 
		return(GZ_SUCCESS); 
}

/* 
This doesn't really belong in the application program, but for this 
simplified case of a renderer that doesn't do any shading itself, this 
is the easiest place to put it.
*/

void shade(GzCoord norm, GzCoord color)
{
  GzCoord	light;
  float		coef;

  light[0] = 0.707f;
  light[1] = 0.5f;
  light[2] = 0.5f;

  coef = light[0]*norm[0] + light[1]*norm[1] + light[2]*norm[2];
  if (coef < 0) 	coef *= -1;

  if (coef > 1.0)	coef = 1.0;
  color[0] = coef*0.95f;
  color[1] = coef*0.65f;
  color[2] = coef*0.88f;
}
