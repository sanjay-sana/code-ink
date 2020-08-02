#include <iostream>
#include <math.h>
#include "disp.h"
#include	"gz.h"
#include	"rend.h"

int GzNewRender(GzRender **render, GzRenderClass renderClass, GzDisplay *display)
{
/* 
- malloc a renderer struct
- keep closed until BeginRender inits are done
- span interpolator needs pointer to display for pixel writes
- check for legal class GZ_Z_BUFFER_RENDER
*/

	if (!render)
	{
		return GZ_FAILURE;
	}
	*render = new GzRender();
	(*render)->renderClass = renderClass;
	(*render)->display=display;
	/*Keep the renderer closed initially */
	(*render)->open = 0;
	return GZ_SUCCESS;
}


int GzFreeRender(GzRender *render)
{
/* 
-free all renderer resources
*/
	if (!render)
	{
		return GZ_FAILURE;
	}
	free(render);
	return GZ_SUCCESS;
}


int GzBeginRender(GzRender	*render)
{
/* 
- set up for start of each frame - init frame buffer
*/
	if (!render)
	{
		return GZ_FAILURE;
	}
	render->flatcolor[RED] = 1;
	render->flatcolor[GREEN] = 0;
	render->flatcolor[BLUE] = 0;
/*Open the renderer now*/
	render->open = 1;
	return GZ_SUCCESS;
}


int GzPutAttribute(GzRender	*render, int numAttributes, GzToken	*nameList, 
	GzPointer *valueList) /* void** valuelist */
{
/*
- set renderer attribute states (e.g.: GZ_RGB_COLOR default color)
- later set shaders, interpolaters, texture maps, and lights
*/
	if (!render || !nameList || !valueList )
	{
		return GZ_FAILURE;
	}
	for (int i=0; i < numAttributes; i++)
	{
		if (nameList[i] <= GZ_RGB_COLOR)
		{
			GzColor *color = (GzColor*)valueList[i];
			float r = color[i][0];
			float g = color[i][1];
			float b = color[i][2];
			render->flatcolor[RED] = r;
			render->flatcolor[GREEN] = g;
			render->flatcolor[BLUE] = b;
		}
	}
	return GZ_SUCCESS;
}

short ctoi(float color)		/* convert float color to GzIntensity short */
{
  return(short)((int)(color * ((1 << 12) - 1)));
}

int GzPutTriangle(GzRender *render, int	numParts, GzToken *nameList,
	GzPointer *valueList) 
/* numParts - how many names and values */
{
/* 
- pass in a triangle description with tokens and values corresponding to
      GZ_NULL_TOKEN:		do nothing - no values
      GZ_POSITION:		3 vert positions in model space
- Invoke the scan converter and return an error code
*/
	if (!render || !nameList || !valueList )
	{
		return GZ_FAILURE;
	}

	GzCoord *triangle_vertex = (GzCoord*) valueList[0];
	/*Calculate the bounding box for a single triangle*/
	float minX, minY, maxX, maxY;
	if (triangle_vertex[0][X] < triangle_vertex[1][X])
	{
		minX = ((triangle_vertex[0][X] <= triangle_vertex[2][X])?triangle_vertex[0][X]:triangle_vertex[2][X]);
	}
	else
	{
		minX = ((triangle_vertex[1][X] <= triangle_vertex[2][X])?triangle_vertex[1][X]:triangle_vertex[2][X]);
	}
	if (triangle_vertex[0][X] > triangle_vertex[1][X])
	{
		maxX = ((triangle_vertex[0][X] >= triangle_vertex[2][X])?triangle_vertex[0][X]:triangle_vertex[2][X]);
	}
	else
	{
		maxX = ((triangle_vertex[1][X] >= triangle_vertex[2][X])?triangle_vertex[1][X]:triangle_vertex[2][X]);
	}

	/*Sort the vertices of a triangle based on the Y coordinate - low to high*/
	bool swapped = true;
	int i=0;
	int j=0;
	float tempX,tempY,tempZ;
	while (swapped)
	{
		swapped = false;
		j++;
		for (i=0; i<3-j; i++)
		{
			if (triangle_vertex[i][Y] > triangle_vertex[i+1][Y])
			{
			tempX = triangle_vertex[i+1][X];
			tempY = triangle_vertex[i+1][Y];
			tempZ = triangle_vertex[i+1][Z];
			triangle_vertex[i+1][X] = triangle_vertex[i][X];
			triangle_vertex[i+1][Y] = triangle_vertex[i][Y];
			triangle_vertex[i+1][Z] = triangle_vertex[i][Z];
			triangle_vertex[i][X] = tempX;
			triangle_vertex[i][Y] = tempY;
			triangle_vertex[i][Z] = tempZ;
			swapped = true;
			}
		}
	}
	maxY = triangle_vertex[2][Y];
	minY = triangle_vertex[0][Y];
	/* Obtain the values A, B and C for the line equation Ax + By + C = 0 */
	float A02, B02, C02, A10, B10, C10, A21, B21, C21;
	float color20 = 0;
	float color01 = 0;
	float color12 = 0;
	A02 = triangle_vertex[2][Y] - triangle_vertex[0][Y];
	B02 = -1 * (triangle_vertex[2][X] - triangle_vertex[0][X]);
	C02 = ((triangle_vertex[2][X] - triangle_vertex[0][X]) * triangle_vertex[0][Y]) - ((triangle_vertex[2][Y] - triangle_vertex[0][Y]) * triangle_vertex[0][X]);
	A10 = triangle_vertex[0][Y] - triangle_vertex[1][Y];
	B10 = -1 * (triangle_vertex[0][X] - triangle_vertex[1][X]);
	C10 = ((triangle_vertex[0][X] - triangle_vertex[1][X]) * triangle_vertex[1][Y]) - ((triangle_vertex[0][Y] - triangle_vertex[1][Y]) * triangle_vertex[1][X]);
	A21 = triangle_vertex[1][Y] - triangle_vertex[2][Y];
	B21 = -1 * (triangle_vertex[1][X] - triangle_vertex[2][X]);
	C21 = ((triangle_vertex[1][X] - triangle_vertex[2][X]) * triangle_vertex[2][Y]) - ((triangle_vertex[1][Y] - triangle_vertex[2][Y]) * triangle_vertex[2][X]);
	/*Convert floating point values to integers to check for pixel value*/
	int startX = floor(minX);
	if (startX < 0)
	{
		startX = 0;
	}
	int startY = floor(minY);
	if (startY < 0)
	{
		startY = 0;
	}
	int endX = ceil(maxX);
	if (endX > render->display->xres - 1)
	{
		endX = render->display->xres - 1;
	}
	int endY = ceil(maxY);
	if (endY > render->display->yres - 1)
	{
		endY = render->display->yres - 1;
	}
	/*Interpolate z value using the cross product of edge 0-2*/
	float aCross, bCross, cCross, dCross;

	aCross = (triangle_vertex[1][Y] - triangle_vertex[0][Y]) * (triangle_vertex[2][Z] - triangle_vertex[1][Z])
			 - (triangle_vertex[1][Z] - triangle_vertex[0][Z]) * (triangle_vertex[2][Y] - triangle_vertex[1][Y]);
	bCross = (triangle_vertex[1][Z] - triangle_vertex[0][Z]) * (triangle_vertex[2][X] - triangle_vertex[1][X])
				 - (triangle_vertex[1][X] - triangle_vertex[0][X]) * (triangle_vertex[2][Z] - triangle_vertex[1][Z]);
	cCross = (triangle_vertex[1][X] - triangle_vertex[0][X]) * (triangle_vertex[2][Y] - triangle_vertex[1][Y])
				 - (triangle_vertex[1][Y] - triangle_vertex[0][Y]) * (triangle_vertex[2][X] - triangle_vertex[1][X]);

	/* Replace A, B and C values in Ax + By + Cz + D = 0 expression to obtain the value of D */
	dCross = -1*((aCross * triangle_vertex[0][X]) + (bCross * triangle_vertex[0][Y]) + (cCross * triangle_vertex[0][Z]));
	float interpolateZ;
	/* Insert each pixel value to the line equation to  */
	for (j=startY; j<=endY;j++)
	{
		for (i=startX; i<=endX;i++)
		{
			color20 = (A02 * (float)i) + (B02 * (float)j) + C02;
			color01 = (A10 * (float)i) + (B10 * (float)j) + C10;
			color12 = (A21 * (float)i) + (B21 * (float)j) + C21;
			GzIntensity r, g, b, a;
			GzDepth	 z = 0;
			if ((color20 < 0 && color01 < 0 && color12 < 0) || (color20 > 0 && color01 > 0 && color12 > 0)
					|| color20 == 0 || color01 == 0 || color01 == 0)
			{
				GzGetDisplay(render->display, i, j, &r, &g, &b, &a, &z);
				interpolateZ = (-1 * (aCross * i ) - (bCross * j) - dCross) / cCross;
				if (interpolateZ < z || z == 0)
				{
					r = (GzIntensity) ctoi((float)render->flatcolor[RED]);
					g = (GzIntensity) ctoi((float)render->flatcolor[GREEN]);
					b = (GzIntensity) ctoi((float)render->flatcolor[BLUE]);
					GzPutDisplay(render->display, i, j, r, g, b, a, (GzDepth) interpolateZ);
				}
			}
		}
	}
	return GZ_SUCCESS;
}


/* NOT part of API - just for general assistance */

//short ctoi(float color)		/* convert float color to GzIntensity short */
//{
//  return(short)((int)(color * ((1 << 12) - 1)));
//}

