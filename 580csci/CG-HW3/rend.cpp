/* CS580 Homework 3 */

#include	<iostream>
#include	<stdio.h>
#include	<math.h>
#include	"gz.h"
#include	"disp.h"
#include	"rend.h"
#include    <cstdlib>

#define PI 3.14159265

int GzRotXMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along x axis
// Pass back the matrix using mat value

	if (!mat)
	{
		return GZ_FAILURE;
	}
	/*
	- Convert degree to radian
	*/
	float radian = degree * (PI / 180);

	mat[0][0] = 1;
	mat[0][1] = 0;
	mat[0][2] = 0;
	mat[0][3] = 0;

	mat[1][0] = 0;
	mat[1][1] = cos(radian);
	mat[1][2] = -sin(radian);
	mat[1][3] = 0;

	mat[2][0] = 0;
	mat[2][1] = sin(radian);
	mat[2][2] = cos(radian);
	mat[2][3] = 0;

	mat[3][0] = 0;
	mat[3][1] = 0;
	mat[3][2] = 0;
	mat[3][3] = 1;

	return GZ_SUCCESS;
}


int GzRotYMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along y axis
// Pass back the matrix using mat value
	
	if (!mat)
	{
		return GZ_FAILURE;
	}
	/*
	- Convert degree to radian
	*/
	float radian = degree * (PI / 180);

	mat[0][0] = cos(radian);
	mat[0][1] = 0;
	mat[0][2] = sin(radian);
	mat[0][3] = 0;

	mat[1][0] = 0;
	mat[1][1] = 1;
	mat[1][2] = 0;
	mat[1][3] = 0;

	mat[2][0] = -sin(radian);
	mat[2][1] = 0;
	mat[2][2] = cos(radian);
	mat[2][3] = 0;

	mat[3][0] = 0;
	mat[3][1] = 0;
	mat[3][2] = 0;
	mat[3][3] = 1;

	return GZ_SUCCESS;
}


int GzRotZMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along z axis
// Pass back the matrix using mat value

	if (!mat)
	{
		return GZ_FAILURE;
	}
	/*
	- Convert degree to radian
	*/
	float radian = degree * (PI / 180);

	mat[0][0] = cos(radian);
	mat[0][1] = -sin(radian);
	mat[0][2] = 0;
	mat[0][3] = 0;

	mat[1][0] = sin(radian);
	mat[1][1] = cos(radian);
	mat[1][2] = 0;
	mat[1][3] = 0;

	mat[2][0] = 0;
	mat[2][1] = 0;
	mat[2][2] = 1;
	mat[2][3] = 0;

	mat[3][0] = 0;
	mat[3][1] = 0;
	mat[3][2] = 0;
	mat[3][3] = 1;

	return GZ_SUCCESS;
}


int GzTrxMat(GzCoord translate, GzMatrix mat)
{
// Create translation matrix
// Pass back the matrix using mat value

	mat[0][0] = 1;
	mat[0][1] = 0;
	mat[0][2] = 0;
	mat[0][3] = translate[X];

	mat[1][0] = 0;
	mat[1][1] = 1;
	mat[1][2] = 0;
	mat[1][3] = translate[Y];

	mat[2][0] = 0;
	mat[2][1] = 0;
	mat[2][2] = 1;
	mat[2][3] = translate[Z];

	mat[3][0] = 0;
	mat[3][1] = 0;
	mat[3][2] = 0;
	mat[3][3] = 1;

	return GZ_SUCCESS;
}


int GzScaleMat(GzCoord scale, GzMatrix mat)
{
// Create scaling matrix
// Pass back the matrix using mat value

	mat[0][0] = scale[X];
	mat[0][1] = 0;
	mat[0][2] = 0;
	mat[0][3] = 0;

	mat[1][0] = 0;
	mat[1][1] = scale[Y];
	mat[1][2] = 0;
	mat[1][3] = 0;

	mat[2][0] = 0;
	mat[2][1] = 0;
	mat[2][2] = scale[Z];
	mat[2][3] = 0;

	mat[3][0] = 0;
	mat[3][1] = 0;
	mat[3][2] = 0;
	mat[3][3] = 1;

	return GZ_SUCCESS;
}


//----------------------------------------------------------
// Begin main functions

int GzNewRender(GzRender **render, GzRenderClass renderClass, GzDisplay	*display)
{
/*  
- malloc a renderer struct 
- keep closed until all inits are done 
- setup Xsp and anything only done once 
- span interpolator needs pointer to display 
- check for legal class GZ_Z_BUFFER_RENDER 
- init default camera 
*/ 
	if (!render)
	{
		return GZ_FAILURE;
	}
	*render = new GzRender();
	(*render)->renderClass = renderClass;
	(*render)->display=display;
	/*
	- Keep the renderer closed initially 
	*/
	(*render)->open = 0;
	(*render)->camera.FOV = DEFAULT_FOV;
	/*
	- Initialize camera position 
	*/
	(*render)->camera.position[X] = DEFAULT_IM_X;
	(*render)->camera.position[Y] = DEFAULT_IM_Y;
	(*render)->camera.position[Z] = DEFAULT_IM_Z;

	(*render)->camera.lookat[X] = 0;
	(*render)->camera.lookat[Y] = 0;
	(*render)->camera.lookat[Z] = 0;

	(*render)->camera.worldup[X] = 0;
	(*render)->camera.worldup[Y] = 1;
	(*render)->camera.worldup[Z] = 0;

	/*
	- Setup Xsp
	*/

	float FOVradian = (*render)->camera.FOV * (PI / 180);
	float d = 1 / (tan (FOVradian / 2));
	(*render)->Xsp[0][0] = (*render)->display->xres / 2;
	(*render)->Xsp[0][1] = 0;
	(*render)->Xsp[0][2] = 0;
	(*render)->Xsp[0][3] = (*render)->display->xres / 2;	

	(*render)->Xsp[1][0] = 0;	
	(*render)->Xsp[1][1] = -((*render)->display->yres / 2);
	(*render)->Xsp[1][2] = 0;
	(*render)->Xsp[1][3] = (*render)->display->yres / 2;

	(*render)->Xsp[2][0] = 0;
	(*render)->Xsp[2][1] = 0;
	(*render)->Xsp[2][2] = INT_MAX / d;
	(*render)->Xsp[2][3] = 0;

	(*render)->Xsp[3][0] = 0;
	(*render)->Xsp[3][1] = 0;
	(*render)->Xsp[3][2] = 0;
	(*render)->Xsp[3][3] = 1;

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
	render->display = NULL;
	return GZ_SUCCESS;
}

int GzBeginRender(GzRender *render)
{
/*  
- set up for start of each frame - clear frame buffer 
- compute Xiw and projection xform Xpi from camera definition 
- init Ximage - put Xsp at base of stack, push on Xpi and Xiw 
- now stack contains Xsw and app can push model Xforms if it want to. 
*/ 
	if (!render)
	{
		return GZ_FAILURE;
	}
	render->flatcolor[RED] = 1;
	render->flatcolor[GREEN] = 0;
	render->flatcolor[BLUE] = 0;

	/*
	- Calculate Xpi
	*/
	float FOVradian = render->camera.FOV * (PI / 180);
//	float d = 1 / tan (FOVradian / 2);
	render->camera.Xpi[0][0] = 1;
	render->camera.Xpi[0][1] = 0;
	render->camera.Xpi[0][2] = 0;
	render->camera.Xpi[0][3] = 0;

	render->camera.Xpi[1][0] = 0;
	render->camera.Xpi[1][1] = 1;
	render->camera.Xpi[1][2] = 0;
	render->camera.Xpi[1][3] = 0;

	render->camera.Xpi[2][0] = 0;
	render->camera.Xpi[2][1] = 0;
	render->camera.Xpi[2][2] = 1;
	render->camera.Xpi[2][3] = 0;

	render->camera.Xpi[3][0] = 0;
	render->camera.Xpi[3][1] = 0;
//	render->camera.Xpi[3][2] = 1 / d;
	render->camera.Xpi[3][2] = tan(FOVradian / 2);
	render->camera.Xpi[3][3] = 1;
	/*
	- Calculate Xiw
	*/
	GzCoord cameraX, cameraY, cameraZ;
	float length;
	/*
	- Camera Z vector
	*/
	cameraZ[X] = render->camera.lookat[X] - render->camera.position[X];
	cameraZ[Y] = render->camera.lookat[Y] - render->camera.position[Y];
	cameraZ[Z] = render->camera.lookat[Z] - render->camera.position[Z];
	/*
	- Normalize Camera Z vector
	*/
	length = sqrt ((cameraZ[X]*cameraZ[X]) + (cameraZ[Y]*cameraZ[Y]) + (cameraZ[Z]*cameraZ[Z]));
	cameraZ[X] = cameraZ[X] / length;
	cameraZ[Y] = cameraZ[Y] / length;
	cameraZ[Z] = cameraZ[Z] / length;
	/*
	- Camera Y vector
	*/
	float worldupDotZ;
	worldupDotZ = (render->camera.worldup[X] * cameraZ[X]) + (render->camera.worldup[Y] * cameraZ[Y]) + 
				  (render->camera.worldup[Z] * cameraZ[Z]);
	cameraY[X] = render->camera.worldup[X] - (worldupDotZ * cameraZ[X]);
	cameraY[Y] = render->camera.worldup[Y] - (worldupDotZ * cameraZ[Y]);
	cameraY[Z] = render->camera.worldup[Z] - (worldupDotZ * cameraZ[Z]);
	/*
	- Normalize Camera Y vector
	*/
	length = sqrt ((cameraY[X]*cameraY[X]) + (cameraY[Y]*cameraY[Y]) + (cameraY[Z]*cameraY[Z]));
	cameraY[X] = cameraY[X] / length;
	cameraY[Y] = cameraY[Y] / length;
	cameraY[Z] = cameraY[Z] / length;
	/*
	- Camera X vector => X = (Y x Z)
	*/
	cameraX[X] = (cameraY[Y] * cameraZ[Z]) - (cameraY[Z] * cameraZ[Y]);
	cameraX[Y] = (cameraY[Z] * cameraZ[X]) - (cameraY[X] * cameraZ[Z]);
	cameraX[Z] = (cameraY[X] * cameraZ[Y]) - (cameraY[Y] * cameraZ[X]);

	render->camera.Xiw[0][0] = cameraX[X];
	render->camera.Xiw[0][1] = cameraX[Y];
	render->camera.Xiw[0][2] = cameraX[Z];
	render->camera.Xiw[0][3] = -((cameraX[X] * render->camera.position[X]) +
								 (cameraX[Y] * render->camera.position[Y]) +
								 (cameraX[Z] * render->camera.position[Z]));

	render->camera.Xiw[1][0] = cameraY[X];
	render->camera.Xiw[1][1] = cameraY[Y];
	render->camera.Xiw[1][2] = cameraY[Z];
	render->camera.Xiw[1][3] = -((cameraY[X] * render->camera.position[X]) +
								 (cameraY[Y] * render->camera.position[Y]) +
								 (cameraY[Z] * render->camera.position[Z]));;

	render->camera.Xiw[2][0] = cameraZ[X];
	render->camera.Xiw[2][1] = cameraZ[Y];
	render->camera.Xiw[2][2] = cameraZ[Z];
	render->camera.Xiw[2][3] = -((cameraZ[X] * render->camera.position[X]) +
								 (cameraZ[Y] * render->camera.position[Y]) +
								 (cameraZ[Z] * render->camera.position[Z]));;

	render->camera.Xiw[3][0] = 0;
	render->camera.Xiw[3][1] = 0;
	render->camera.Xiw[3][2] = 0;
	render->camera.Xiw[3][3] = 1;
	/*
	- Push Xsp, Xpi and Xiw into the stack
	*/
	render->matlevel = 0;
	GzPushMatrix(render, render->Xsp);
	GzPushMatrix(render, render->camera.Xpi);
	GzPushMatrix(render, render->camera.Xiw);
	/*
	- Open the renderer now
	*/
	render->open = 1;

	return GZ_SUCCESS;
}

int GzPutCamera(GzRender *render, GzCamera *camera)
{
/*
- overwrite renderer camera structure with new camera definition
*/
	if (!render || !camera)
	{
		return GZ_FAILURE;
	}
	render->camera.FOV = camera->FOV;

	render->camera.lookat[X] = camera->lookat[X];
	render->camera.lookat[Y] = camera->lookat[Y];
	render->camera.lookat[Z] = camera->lookat[Z];
	
	render->camera.position[X] = camera->position[X];
	render->camera.position[Y] = camera->position[Y];
	render->camera.position[Z] = camera->position[Z];
	
	render->camera.worldup[X] = camera->worldup[X];
	render->camera.worldup[Y] = camera->worldup[Y];
	render->camera.worldup[Z] = camera->worldup[Z];
	/*
	- Normalize Camera world-up vector
	*/
	float length;
	length = sqrt ((render->camera.worldup[X]*render->camera.worldup[X]) + 
				   (render->camera.worldup[Y]*render->camera.worldup[Y]) + 
				   (render->camera.worldup[Z]*render->camera.worldup[Z]));
	render->camera.worldup[X] = render->camera.worldup[X] / length;
	render->camera.worldup[Y] = render->camera.worldup[Y] / length;
	render->camera.worldup[Z] = render->camera.worldup[Z] / length;
	/* 
	- Override modified Xsp using the changed camera FOV
	*/
	float FOVradian = render->camera.FOV * (PI / 180);
	float d = 1 / tan (FOVradian / 2);
	render->Xsp[2][2] = INT_MAX / d;

	return GZ_SUCCESS;	
}

bool matrixproduct(GzMatrix mat1, GzMatrix mat2, GzMatrix Xproduct)
{
	int i, j, k;
	for (i=0; i<4; i++)
	{
		for (j=0; j<4; j++)
		{
			Xproduct[i][j] = 0;
		}
	}
	for (i=0; i<4; i++)
	{
		for (j=0; j<4; j++)
		{
			for (k=0; k<4; k++)
			{
				Xproduct[i][j] += mat1[i][k] * mat2[k][j];
			}
		}
	}
	return true;	
}
int GzPushMatrix(GzRender *render, GzMatrix matrix)
{
/*
- push a matrix onto the Ximage stack
- check for stack overflow
*/	
	if (!render || render->matlevel == MATLEVELS)
	{
		return GZ_FAILURE;
	}
	if (render->matlevel == 0)
	{
		memcpy(render->Ximage[render->matlevel], matrix, sizeof(GzMatrix));
	}
	else
	{
		GzMatrix Xproduct;
		matrixproduct(render->Ximage[render->matlevel-1], matrix, Xproduct);
		memcpy(render->Ximage[render->matlevel], Xproduct, sizeof(GzMatrix));
	}
	render->matlevel++;
	return GZ_SUCCESS;
}

int GzPopMatrix(GzRender *render)
{
/*
- pop a matrix off the Ximage stack
- check for stack underflow
*/
	if(!render)
	{
		return GZ_FAILURE;
	}
	if (render->matlevel < 0)
	{
		return GZ_FAILURE;
	}
	else
	{
		render->matlevel--;
	}
	return GZ_SUCCESS;
}


int GzPutAttribute(GzRender *render, int numAttributes, GzToken *nameList, 
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

/* NOT part of API - just for general assistance */

short	ctoi(float color)		/* convert float color to GzIntensity short */
{
  return(short)((int)(color * ((1 << 12) - 1)));
}

int GzPutTriangle(GzRender *render, int numParts, GzToken *nameList, 
				  GzPointer *valueList)
/* numParts : how many names and values */
{
/*  
- pass in a triangle description with tokens and values corresponding to 
      GZ_POSITION:3 vert positions in model space 
- Xform positions of verts  
- Clip - just discard any triangle with verts behind view plane 
       - test for triangles with all three verts off-screen 
- invoke triangle rasterizer  
*/ 
	if (!render || !nameList || !valueList )
	{
		return GZ_FAILURE;
	}
	/*
	- Convert object space coordinates into Xform => triangle_vertex
	*/
	GzCoord *objspace_vertex = (GzCoord*) valueList[0];
	GzCoord *triangle_vertex = new GzCoord[3];
	GzMatrix stacktop;
	memcpy(stacktop, render->Ximage[render->matlevel - 1], sizeof(GzMatrix));
	float w;
	bool ignore = false;
	for (int id = 0; id < 3; id++) 
	{
		triangle_vertex[id][X] = stacktop[0][0]*objspace_vertex[id][X] + stacktop[0][1]*objspace_vertex[id][Y] + stacktop[0][2]*objspace_vertex[id][Z]
						+ stacktop[0][3]*1.0;
		triangle_vertex[id][Y] = stacktop[1][0]*objspace_vertex[id][X] + stacktop[1][1]*objspace_vertex[id][Y] + stacktop[1][2]*objspace_vertex[id][Z]
						+ stacktop[1][3]*1.0;
		triangle_vertex[id][Z] = stacktop[2][0]*objspace_vertex[id][X] + stacktop[2][1]*objspace_vertex[id][Y] + stacktop[2][2]*objspace_vertex[id][Z]
						+ stacktop[2][3]*1.0;
		w = stacktop[3][0]*objspace_vertex[id][X] + stacktop[3][1]*objspace_vertex[id][Y] + stacktop[3][2]*objspace_vertex[id][Z]
						+ stacktop[3][3]*1.0;
		triangle_vertex[id][X] /= w;
		triangle_vertex[id][Y] /= w;
		triangle_vertex[id][Z] /= w;
		if (triangle_vertex[id][Z] <= 0)
		{
			ignore = true;
			break;
		}
		// check if any vets in behind view plane
	}
	if (ignore == true)
	{
		return GZ_SUCCESS;
	}
	/*     *********** Implemented LEE Algorithm ***********     */
	/*
	- Calculate the bounding box for a single triangle
	*/
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
	/*
	- Sort the vertices of a triangle based on the Y coordinate - low to high
	*/
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
	/* 
	- Obtain the values A, B and C for the line equation Ax + By + C = 0 
	*/
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
	/*
	- Convert floating point values to integers to check for pixel value
	*/
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
	/*
	- Interpolate z value using the cross product of edge 0-2
	*/
	float aCross, bCross, cCross, dCross;

	aCross = (triangle_vertex[1][Y] - triangle_vertex[0][Y]) * (triangle_vertex[2][Z] - triangle_vertex[1][Z])
			 - (triangle_vertex[1][Z] - triangle_vertex[0][Z]) * (triangle_vertex[2][Y] - triangle_vertex[1][Y]);
	bCross = (triangle_vertex[1][Z] - triangle_vertex[0][Z]) * (triangle_vertex[2][X] - triangle_vertex[1][X])
				 - (triangle_vertex[1][X] - triangle_vertex[0][X]) * (triangle_vertex[2][Z] - triangle_vertex[1][Z]);
	cCross = (triangle_vertex[1][X] - triangle_vertex[0][X]) * (triangle_vertex[2][Y] - triangle_vertex[1][Y])
				 - (triangle_vertex[1][Y] - triangle_vertex[0][Y]) * (triangle_vertex[2][X] - triangle_vertex[1][X]);

	/* 
	- Replace A, B and C values in Ax + By + Cz + D = 0 expression to obtain the value of D 
	*/
	dCross = -1*((aCross * triangle_vertex[0][X]) + (bCross * triangle_vertex[0][Y]) + (cCross * triangle_vertex[0][Z]));
	float interpolateZ;
	/* 
	- Insert each pixel value to the line equation ax + by + c = 0 where x and y are the pixel coordinates
	*/
	for (j=startY; j<=endY;j++)
	{
		for (i=startX; i<=endX;i++)
		{
			color20 = (A02 * (float)i) + (B02 * (float)j) + C02;
			color01 = (A10 * (float)i) + (B10 * (float)j) + C10;
			color12 = (A21 * (float)i) + (B21 * (float)j) + C21;
			GzIntensity r, g, b, a;
			GzDepth	 z = 0;
				/* 
				- Color the pixel if pixel is on any one of the line equation, or inside the triangle
				*/
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



