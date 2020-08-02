/* CS580 Homework 3 */

#include	<iostream>
#include	<stdio.h>
#include	<math.h>
#include	"gz.h"
#include	"disp.h"
#include	"rend.h"
#include    <cstdlib>

#define PI 3.14159265

typedef struct
{
	GzCoord vertex;
	int origIdx;
}triVector;

typedef struct GzTriangleEdge
{
	triVector begin;
	triVector end;
}edges;

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

int GzNewRender(GzRender **render, GzRenderClass renderClass, GzDisplay *display)
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
	- Initialize ambient light
	*/
	(*render)->ambientlight.color[RED] = 0;
	(*render)->ambientlight.color[GREEN] = 0;
	(*render)->ambientlight.color[BLUE] = 0;

	(*render)->numlights = 0;
	/*
	- Initialize Ka, Kd, Ks
	*/
	GzColor valueKa = DEFAULT_AMBIENT;
	GzColor valueKd = DEFAULT_DIFFUSE;
	GzColor valueKs = DEFAULT_SPECULAR;
	(*render)->Ka[RED] = valueKa[RED];
	(*render)->Ka[GREEN] = valueKa[GREEN];
	(*render)->Ka[BLUE] = valueKa[BLUE];
	(*render)->Kd[RED] = valueKd[RED];
	(*render)->Kd[GREEN] = valueKd[GREEN];
	(*render)->Kd[BLUE] = valueKd[BLUE];
	(*render)->Ks[RED] = valueKs[RED];
	(*render)->Ks[GREEN] = valueKs[GREEN];
	(*render)->Ks[BLUE] = valueKs[BLUE];
	/*
	- Initialize Specular power
	*/
	(*render)->spec = DEFAULT_SPEC;
	/*
	- Set default interpolation mode as flat shading
	*/
	(*render)->interp_mode = GZ_RGB_COLOR;
	/*
	- Set default interpolation mode as flat shading
	*/
	(*render)->tex_fun = 0;
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

int GzPushMatrix(GzRender *render, GzMatrix	matrix)
{
/*
- push a matrix onto the Ximage stack
- check for stack overflow
*/	
	GzMatrix Xproduct;
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
		matrixproduct(render->Ximage[render->matlevel-1], matrix, Xproduct);
		memcpy(render->Ximage[render->matlevel], Xproduct, sizeof(GzMatrix));
	}
	if (render->matlevel == 0 || render->matlevel == 1)
	{
		/*
		- Identity Matrix
		*/
		GzMatrix	identity = 
		{ 
			{1.0,	0.0,	0.0,	0.0}, 
			{0.0,	1.0,	0.0,	0.0}, 
			{0.0,	0.0,	1.0,	0.0}, 
			{0.0,	0.0,	0.0,	1.0} 
		};
		memcpy(render->Xnorm[render->matlevel], identity, sizeof(GzMatrix));
	}
	else
	{
		GzMatrix temp;
		memcpy(temp, matrix, sizeof(GzMatrix));
		// temp[0][3] = 0;
		// temp[1][3] = 0;
		// temp[2][3] = 0;
		// temp[3][3] = 1;
		float k = 1 / sqrt((temp[0][0] * temp[0][0]) +
						   (temp[1][0] * temp[1][0]) +
						   (temp[2][0] * temp[2][0]));
//						   (matrix[0][3] * matrix[0][3]));
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				temp[i][j] = temp[i][j] * k;
			}
			temp[i][3] = 0;
			temp[3][i] = 0;
		}
		temp[3][3] = 1;
		matrixproduct(render->Xnorm[render->matlevel - 1], temp, Xproduct);
		memcpy(render->Xnorm[render->matlevel], Xproduct, sizeof(GzMatrix));
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

bool normalizeVector(GzCoord vec)
{
	float length = sqrt((vec[X]*vec[X]) + (vec[Y]*vec[Y]) + (vec[Z]*vec[Z]));
	vec[X] /= length;
	vec[Y] /= length;
	vec[Z] /= length;
	return true;
}

int GzPutAttribute(GzRender	*render, int numAttributes, GzToken	*nameList, 
	GzPointer	*valueList) /* void** valuelist */
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
		if (nameList[i] == GZ_RGB_COLOR)
		{
			GzColor *color = (GzColor*)valueList[i];
			float r = color[i][0];
			float g = color[i][1];
			float b = color[i][2];
			render->flatcolor[RED] = r;
			render->flatcolor[GREEN] = g;
			render->flatcolor[BLUE] = b;
		}
		else if (nameList[i] == GZ_DIRECTIONAL_LIGHT)
		{
			if (render->numlights < MAX_LIGHTS)
			{
				GzLight *directional_light = (GzLight*)valueList[i];
				render->lights[render->numlights] = *directional_light;
				// Normalize direction vector of the light
				normalizeVector(render->lights[render->numlights].direction);
				render->numlights++;
			}
		}
		else if (nameList[i] == GZ_AMBIENT_LIGHT)
		{
			GzLight *ambient_light = (GzLight*)valueList[i];
			render->ambientlight = *ambient_light;
		}
		else if (nameList[i] == GZ_DIFFUSE_COEFFICIENT)
		{
			GzColor *diffuse_coeff = (GzColor*)valueList[i];
			render->Kd[RED]   = diffuse_coeff[0][RED];
			render->Kd[GREEN] = diffuse_coeff[0][GREEN];
			render->Kd[BLUE]  = diffuse_coeff[0][BLUE];
		}
		else if (nameList[i] == GZ_INTERPOLATE)
		{
			int *interpolation_mode = (int*)valueList[i];
			render->interp_mode = *interpolation_mode;
		}
		else if (nameList[i] == GZ_AMBIENT_COEFFICIENT)
		{
			GzColor *ambient_coeff = (GzColor*)valueList[i];
			render->Ka[RED]   = ambient_coeff[0][RED];
			render->Ka[GREEN] = ambient_coeff[0][GREEN];
			render->Ka[BLUE]  = ambient_coeff[0][BLUE];
		}
		else if (nameList[i] == GZ_SPECULAR_COEFFICIENT)
		{
			GzColor *spec_coeff = (GzColor*)valueList[i];
			render->Ks[RED]   = spec_coeff[0][RED];
			render->Ks[GREEN] = spec_coeff[0][GREEN];
			render->Ks[BLUE]  = spec_coeff[0][BLUE];
		}
		else if (nameList[i] == GZ_DISTRIBUTION_COEFFICIENT)
		{
			float *dist_coeff= (float*)valueList[i];
			render->spec = *dist_coeff;
		}
		else if (nameList[i] == GZ_TEXTURE_MAP)
		{
			render->tex_fun = (GzTexture) valueList[i];
		}
	}	
	return GZ_SUCCESS;
}

short	ctoi(float color)		/* convert float color to GzIntensity short */
{
  return(short)((int)(color * ((1 << 12) - 1)));
}

bool vectorObjectToScreen(GzMatrix topstack, GzCoord vec, GzCoord product)
{
	product[X] = topstack[0][0]*vec[X] + topstack[0][1]*vec[Y] + topstack[0][2]*vec[Z]
					+ topstack[0][3]*1.0;
	product[Y] = topstack[1][0]*vec[X] + topstack[1][1]*vec[Y] + topstack[1][2]*vec[Z]
					+ topstack[1][3]*1.0;
	product[Z] = topstack[2][0]*vec[X] + topstack[2][1]*vec[Y] + topstack[2][2]*vec[Z]
					+ topstack[2][3]*1.0;
	float w = topstack[3][0]*vec[X] + topstack[3][1]*vec[Y] + topstack[3][2]*vec[Z]
					+ topstack[3][3]*1.0;
	product[X] /= w;
	product[Y] /= w;
	product[Z] /= w;
	return true;
}

bool shadingCalculation(GzRender *render, GzColor colorV, GzCoord imgspace_normal, GzTextureIndex texture_vertex)
{
	colorV[RED] = 0;
	colorV[GREEN] = 0;
	colorV[BLUE] = 0;
	/*
	Initialize eye vector
	*/
	GzCoord eVector;
	eVector[X] = 0;
	eVector[Y] = 0;
	eVector[Z] = -1;
	float NdotL, NdotE, RdotE;
	GzCoord reflectionRay, NdotLofN;
	GzColor ka, kd, ks;
	ka[RED] = 0;
	ka[GREEN] = 0;
	ka[BLUE] = 0;
	kd[RED] = 0;
	kd[GREEN] = 0;
	kd[BLUE] = 0;
	ks[RED] = 0;
	ks[GREEN] = 0;
	ks[BLUE] = 0;
	for (int idx = 0; idx < render->numlights; idx++)
	{
		NdotL = ((imgspace_normal[X] * render->lights[idx].direction[X]) +
				 (imgspace_normal[Y] * render->lights[idx].direction[Y]) +
				 (imgspace_normal[Z] * render->lights[idx].direction[Z]));
		NdotE = ((imgspace_normal[X] * eVector[X]) +
				 (imgspace_normal[Y] * eVector[Y]) +
				 (imgspace_normal[Z] * eVector[Z]));
		if (NdotL < 0 && NdotE < 0)
		{
			imgspace_normal[X] *= -1;
			imgspace_normal[Y] *= -1;
			imgspace_normal[Z] *= -1;
			NdotL *= -1;
			NdotE *= -1;
		}
		if ((NdotL > 0 && NdotE < 0) || (NdotL < 0 && NdotE > 0))
		{
			continue;
		}
		/*
		- Compute Reflection ray R = 2(N.L)N - L
		*/
		NdotLofN[X] = imgspace_normal[X] * 2 * NdotL;
		NdotLofN[Y] = imgspace_normal[Y] * 2 * NdotL;
		NdotLofN[Z] = imgspace_normal[Z] * 2 * NdotL;
		reflectionRay[X] = NdotLofN[X] - render->lights[idx].direction[X];
		reflectionRay[Y] = NdotLofN[Y] - render->lights[idx].direction[Y];
		reflectionRay[Z] = NdotLofN[Z] - render->lights[idx].direction[Z];
		normalizeVector(reflectionRay);
		RdotE = ((reflectionRay[X] * eVector[X]) +
				 (reflectionRay[Y] * eVector[Y]) +
				 (reflectionRay[Z] * eVector[Z]));
		if (RdotE < 0)
		{
			RdotE = 0;
		}
		else if (RdotE > 1)
		{
			RdotE = 1;
		}
		GzColor tempKd, tempKs;
		/*
		- Summing all the lights to calculate Ks
		*/
		tempKs[RED] = render->lights[idx].color[RED] * pow(RdotE,render->spec);
		tempKs[GREEN] = render->lights[idx].color[GREEN] * pow(RdotE,render->spec);
		tempKs[BLUE] = render->lights[idx].color[BLUE] * pow(RdotE,render->spec);
		ks[RED] += tempKs[RED];
		ks[GREEN] += tempKs[GREEN];
		ks[BLUE] += tempKs[BLUE];
		/*
		- Summing all the lights to calculate Kd
		*/
		tempKd[RED] = render->lights[idx].color[RED] * NdotL;
		tempKd[GREEN] = render->lights[idx].color[GREEN] * NdotL;
		tempKd[BLUE] = render->lights[idx].color[BLUE] * NdotL;
		kd[RED] += tempKd[RED];
		kd[GREEN] += tempKd[GREEN];
		kd[BLUE] += tempKd[BLUE];
	}
	GzColor modifiedKa, modifiedKd, modifiedKs;
	/*
	- Calculate Ks, Kd and Ka
	*/
	if (render->tex_fun)
	{
		GzColor uv_Color;
		render->tex_fun(texture_vertex[U], texture_vertex[V], uv_Color);
		modifiedKd[RED]   = uv_Color[RED];
		modifiedKd[GREEN] = uv_Color[GREEN];
		modifiedKd[BLUE]  = uv_Color[BLUE];
		modifiedKa[RED]   = uv_Color[RED];
		modifiedKa[GREEN] = uv_Color[GREEN];
		modifiedKa[BLUE]  = uv_Color[BLUE];
		if (render-> interp_mode == GZ_COLOR)
		{
			modifiedKs[RED]   = uv_Color[RED];
			modifiedKs[GREEN] = uv_Color[GREEN];
			modifiedKs[BLUE]  = uv_Color[BLUE];
		}
		else
		{
			modifiedKs[RED]   = render->Ks[RED];
			modifiedKs[GREEN] = render->Ks[GREEN];
			modifiedKs[BLUE]  = render->Ks[BLUE];
		}
	}
	else
	{
		modifiedKd[RED]   = render->Kd[RED];
		modifiedKd[GREEN] = render->Kd[GREEN];
		modifiedKd[BLUE]  = render->Kd[BLUE];
		modifiedKa[RED]   = render->Ka[RED];
		modifiedKa[GREEN] = render->Ka[GREEN];
		modifiedKa[BLUE]  = render->Ka[BLUE];
		modifiedKs[RED]   = render->Ks[RED];
		modifiedKs[GREEN] = render->Ks[GREEN];
		modifiedKs[BLUE]  = render->Ks[BLUE];
	}
	ks[RED]   = modifiedKs[RED] * ks[RED];
	ks[GREEN] = modifiedKs[GREEN] * ks[GREEN];
	ks[BLUE]  = modifiedKs[BLUE] * ks[BLUE];
	kd[RED]   = modifiedKd[RED] * kd[RED];
	kd[GREEN] = modifiedKd[GREEN] * kd[GREEN];
	kd[BLUE]  = modifiedKd[BLUE] * kd[BLUE];
	ka[RED]   = modifiedKa[RED] * render->ambientlight.color[RED];
	ka[GREEN] = modifiedKa[GREEN] * render->ambientlight.color[GREEN];
	ka[BLUE]  = modifiedKa[BLUE] * render->ambientlight.color[BLUE];
	/*
	- Obtaining summation of Ks, Kd and Ka components
	*/
	colorV[RED] = ka[RED] + kd[RED] + ks[RED];
	colorV[GREEN] = ka[GREEN] + kd[GREEN] + ks[GREEN];
	colorV[BLUE] = ka[BLUE] + kd[BLUE] + ks[BLUE];

	return true;
}

int GzPutTriangle(GzRender	*render, int numParts, GzToken *nameList, 
				  GzPointer	*valueList)
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
	GzCoord *objspace_vertex;
	GzCoord *objspace_normal;
	GzCoord *triangle_vertex = new GzCoord[3];
	GzCoord *imgspace_normal = new GzCoord[3];
	GzTextureIndex * texture_vertex;

	GzMatrix stacktop;
	memcpy(stacktop, render->Ximage[render->matlevel - 1], sizeof(GzMatrix));
	GzMatrix stacktop_normal;
	memcpy(stacktop_normal, render->Xnorm[render->matlevel - 1], sizeof(GzMatrix));
	stacktop_normal[3][3] = 1;
	for (int n = 0; n < numParts; n++)
	{
		if (nameList[n] == GZ_POSITION)
		{
			bool ignore = false;
			objspace_vertex = (GzCoord*) valueList[n];
			for (int id = 0; id < 3; id++) 
			{
				vectorObjectToScreen(stacktop, objspace_vertex[id], triangle_vertex[id]);
				if (triangle_vertex[id][Z] <= 0)
				{
					ignore = true;
					break;
				}
			}
			// check if any vertices are behind view plane
			if (ignore == true)
			{
				return GZ_SUCCESS;
			}
		}
		else if (nameList[n] == GZ_NORMAL)
		{
			objspace_normal = (GzCoord*) valueList[n];
		}
		else if (nameList[n] == GZ_TEXTURE_INDEX)
		{
			texture_vertex = (GzTextureIndex*) valueList[n];
		}
	}

	for (int id = 0; id < 3; id++)
	{
		vectorObjectToScreen(stacktop_normal, objspace_normal[id], imgspace_normal[id]);
	}

	/*     *********** Implemented LEE Algorithm ***********     */
	/*
	- Calculate the bounding box for a single triangle
	*/
    float minX, maxX, minY, maxY;
	triVector tri_vertex[3];
    for( int idx = 0; idx < 3; idx++ )
    {
        memcpy( tri_vertex[idx].vertex, triangle_vertex[idx], sizeof( GzCoord ) );
        tri_vertex[idx].origIdx = idx;
    }

	if (tri_vertex[0].vertex[X] < tri_vertex[1].vertex[X])
	{
		minX = ((tri_vertex[0].vertex[X] <= tri_vertex[2].vertex[X])?tri_vertex[0].vertex[X]:tri_vertex[2].vertex[X]);
	}
	else
	{
		minX = ((tri_vertex[1].vertex[X] <= tri_vertex[2].vertex[X])?tri_vertex[1].vertex[X]:tri_vertex[2].vertex[X]);
	}
	if (tri_vertex[0].vertex[X] > tri_vertex[1].vertex[X])
	{
		maxX = ((tri_vertex[0].vertex[X] >= tri_vertex[2].vertex[X])?tri_vertex[0].vertex[X]:tri_vertex[2].vertex[X]);
	}
	else
	{
		maxX = ((tri_vertex[1].vertex[X] >= tri_vertex[2].vertex[X])?tri_vertex[1].vertex[X]:tri_vertex[2].vertex[X]);
	}
	if (tri_vertex[0].vertex[Y] < tri_vertex[1].vertex[Y])
	{
		minY = ((tri_vertex[0].vertex[Y] <= tri_vertex[2].vertex[Y])?tri_vertex[0].vertex[Y]:tri_vertex[2].vertex[Y]);
	}
	else
	{
		minY = ((tri_vertex[1].vertex[Y] <= tri_vertex[2].vertex[Y])?tri_vertex[1].vertex[Y]:tri_vertex[2].vertex[Y]);
	}
	if (tri_vertex[0].vertex[Y] > tri_vertex[1].vertex[Y])
	{
		maxY = ((tri_vertex[0].vertex[Y] >= tri_vertex[2].vertex[Y])?tri_vertex[0].vertex[Y]:tri_vertex[2].vertex[Y]);
	}
	else
	{
		maxY = ((tri_vertex[1].vertex[Y] >= tri_vertex[2].vertex[Y])?tri_vertex[1].vertex[Y]:tri_vertex[2].vertex[Y]);
	}

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
	- Define each edge vertices based on the position of the third vertex 
	*/
    edges triangle_edge[3];
	float midPoint = 0;
	float slope = 0;
	slope = ( tri_vertex[2].vertex[Y] - tri_vertex[0].vertex[Y] ) / ( tri_vertex[2].vertex[X] - tri_vertex[0].vertex[X] );
	midPoint = ( ( tri_vertex[1].vertex[Y] - tri_vertex[0].vertex[Y] ) / slope ) + tri_vertex[0].vertex[X];

    if(tri_vertex[0].vertex[Y] == tri_vertex[1].vertex[Y])
    {
		triangle_edge[0].begin = tri_vertex[1];
		triangle_edge[0].end = tri_vertex[0];

		triangle_edge[1].begin = tri_vertex[0];
		triangle_edge[1].end = tri_vertex[2];

		triangle_edge[2].begin = tri_vertex[2];
		triangle_edge[2].end = tri_vertex[1];
    }
    else if(tri_vertex[1].vertex[Y] == tri_vertex[2].vertex[Y])
    {
		triangle_edge[0].begin = tri_vertex[0];
		triangle_edge[0].end = tri_vertex[1];

		triangle_edge[1].begin = tri_vertex[1];
		triangle_edge[1].end = tri_vertex[2];

		triangle_edge[2].begin = tri_vertex[2];
		triangle_edge[2].end = tri_vertex[0];
    }
    else
    {
        if( midPoint < tri_vertex[1].vertex[X] )
		{
			triangle_edge[0].begin = tri_vertex[0];
			triangle_edge[0].end = tri_vertex[2];

			triangle_edge[1].begin = tri_vertex[2];
			triangle_edge[1].end = tri_vertex[1];

			triangle_edge[2].begin = tri_vertex[1];
			triangle_edge[2].end = tri_vertex[0];
		}
        else if( midPoint > tri_vertex[1].vertex[X] )
        {
			triangle_edge[0].begin = tri_vertex[0];
			triangle_edge[0].end = tri_vertex[1];

			triangle_edge[1].begin = tri_vertex[1];
			triangle_edge[1].end = tri_vertex[2];

			triangle_edge[2].begin = tri_vertex[2];
			triangle_edge[2].end = tri_vertex[0];
        }
    }
	/* 
	- Obtain the values A, B and C for the line equation Ax + By + C = 0 
	*/
	float A02, B02, C02, A10, B10, C10, A21, B21, C21;
	float color20 = 0;
	float color01 = 0;
	float color12 = 0;

	A02 = triangle_edge[0].end.vertex[1] - triangle_edge[0].begin.vertex[1];
	B02 = -(triangle_edge[0].end.vertex[0] - triangle_edge[0].begin.vertex[0]);
	C02 = ((triangle_edge[0].end.vertex[0] - triangle_edge[0].begin.vertex[0]) * triangle_edge[0].begin.vertex[1]) - ((triangle_edge[0].end.vertex[1] - triangle_edge[0].begin.vertex[1]) * triangle_edge[0].begin.vertex[0]);
	A10 = triangle_edge[1].end.vertex[1] - triangle_edge[1].begin.vertex[1];
	B10 = -(triangle_edge[1].end.vertex[0] - triangle_edge[1].begin.vertex[0]);
	C10 = ((triangle_edge[1].end.vertex[0] - triangle_edge[1].begin.vertex[0]) * triangle_edge[1].begin.vertex[1]) - ((triangle_edge[1].end.vertex[1] - triangle_edge[1].begin.vertex[1]) * triangle_edge[1].begin.vertex[0]);
	A21 = triangle_edge[2].end.vertex[1] - triangle_edge[2].begin.vertex[1];
	B21 = -(triangle_edge[2].end.vertex[0] - triangle_edge[2].begin.vertex[0]);
	C21 = ((triangle_edge[2].end.vertex[0] - triangle_edge[2].begin.vertex[0]) * triangle_edge[2].begin.vertex[1]) - ((triangle_edge[2].end.vertex[1] - triangle_edge[2].begin.vertex[1]) * triangle_edge[2].begin.vertex[0]);
	/*
	- Interpolate z value using the cross product of edge 0-2
	*/
    float aCross, bCross, cCross, dCross;

    aCross = (triangle_edge[0].end.vertex[Y] - triangle_edge[0].begin.vertex[Y]) * (triangle_edge[1].end.vertex[Z] - triangle_edge[1].begin.vertex[Z]) - (triangle_edge[0].end.vertex[Z] - triangle_edge[0].begin.vertex[Z]) * (triangle_edge[1].end.vertex[Y] - triangle_edge[1].begin.vertex[Y]);
    bCross = -1 *((triangle_edge[0].end.vertex[X] - triangle_edge[0].begin.vertex[X]) * (triangle_edge[1].end.vertex[Z] - triangle_edge[1].begin.vertex[Z]) - (triangle_edge[0].end.vertex[Z] - triangle_edge[0].begin.vertex[Z]) * (triangle_edge[1].end.vertex[X] - triangle_edge[1].begin.vertex[X]));
    cCross = (triangle_edge[0].end.vertex[X] - triangle_edge[0].begin.vertex[X]) * (triangle_edge[1].end.vertex[Y] - triangle_edge[1].begin.vertex[Y]) - (triangle_edge[0].end.vertex[Y] - triangle_edge[0].begin.vertex[Y]) * (triangle_edge[1].end.vertex[X] - triangle_edge[1].begin.vertex[X]);
	/* 
	- Replace A, B and C values in Ax + By + Cz + D = 0 expression to obtain the value of D 
	*/
    dCross = -1*((aCross * triangle_vertex[0][X]) + (bCross * triangle_vertex[0][Y]) + (cCross * triangle_vertex[0][Z]));
	/*
	- Interpolate z value for Gouraud/Phong shading
	*/
    GzColor aCrossColor, bCrossColor, cCrossColor, dCrossColor;
    GzCoord aCrossNormal, bCrossNormal, cCrossNormal, dCrossNormal;
    GzTextureIndex aCrosstexture, bCrosstexture, cCrosstexture, dCrosstexture;

    float differenceX1, differenceX2, differenceY1, differenceY2, differenceZ1, differenceZ2;
    differenceX1 = triangle_edge[0].end.vertex[X] - triangle_edge[0].begin.vertex[X];
    differenceY1 = triangle_edge[0].end.vertex[Y] - triangle_edge[0].begin.vertex[Y];
    differenceX2 = triangle_edge[1].end.vertex[X] - triangle_edge[1].begin.vertex[X];
    differenceY2 = triangle_edge[1].end.vertex[Y] - triangle_edge[1].begin.vertex[Y];

    float screenSpaceZ[3];
    GzColor colorV[3];
    if (render->interp_mode == GZ_COLOR) // For Gouraud shading
    {
        for( int idx = 0; idx < 3; idx++ )
        {
            shadingCalculation(render, colorV[idx], imgspace_normal[idx], texture_vertex[idx]);
        }
        for( int idx = 0; idx < 3; idx++ )
        {
            differenceZ1 = colorV[triangle_edge[0].end.origIdx][idx] - colorV[triangle_edge[0].begin.origIdx][idx];
            differenceZ2 = colorV[triangle_edge[1].end.origIdx][idx] - colorV[triangle_edge[1].begin.origIdx][idx];

			aCrossColor[idx] = (differenceY1 * differenceZ2) - (differenceZ1 * differenceY2);
			bCrossColor[idx] = (differenceZ1 * differenceX2) - (differenceX1 * differenceZ2);
			cCrossColor[idx] = (differenceX1 * differenceY2) - (differenceY1 * differenceX2);

            dCrossColor[idx] = -1*((aCrossColor[idx] * triangle_edge[0].begin.vertex[X]) + (bCrossColor[idx] * triangle_edge[0].begin.vertex[Y]) + (cCrossColor[idx] * colorV[triangle_edge[0].begin.origIdx][idx]));
        }
    }
    else if (render->interp_mode ==  GZ_NORMALS) // For Phong shading
    {
        for( int idx = 0; idx < 3; idx++ )
        {
            differenceZ1 = imgspace_normal[triangle_edge[0].end.origIdx][idx] - imgspace_normal[triangle_edge[0].begin.origIdx][idx];
            differenceZ2 = imgspace_normal[triangle_edge[1].end.origIdx][idx] - imgspace_normal[triangle_edge[1].begin.origIdx][idx];

            aCrossNormal[idx] = (differenceY1 * differenceZ2) - (differenceZ1 * differenceY2);
	        bCrossNormal[idx] = (differenceZ1 * differenceX2) - (differenceX1 * differenceZ2);
    	    cCrossNormal[idx] = (differenceX1 * differenceY2) - (differenceY1 * differenceX2);

            dCrossNormal[idx] = -1*((aCrossNormal[idx] * triangle_edge[0].begin.vertex[X]) + (bCrossNormal[idx] * triangle_edge[0].begin.vertex[Y]) + (cCrossNormal[idx] * imgspace_normal[triangle_edge[0].begin.origIdx][idx]));

        }
        GzTextureIndex persective_tex_vert[3];
        for( int idx = 0; idx < 3; idx++ )
        {
			screenSpaceZ[idx] = (-1 * (aCross * triangle_vertex[idx][X] ) - (bCross * triangle_vertex[idx][Y]) - dCross) / cCross;

			persective_tex_vert[idx][U] = texture_vertex[idx][U] / ( (screenSpaceZ[idx] / ( INT_MAX - screenSpaceZ[idx] )) + 1 );
			persective_tex_vert[idx][V] = texture_vertex[idx][V] / ( (screenSpaceZ[idx] / ( INT_MAX - screenSpaceZ[idx] )) + 1 );
        }
        for( int idx = 0; idx < 2; idx++ )
        {
			differenceZ1 = persective_tex_vert[triangle_edge[0].end.origIdx][idx] - persective_tex_vert[triangle_edge[0].begin.origIdx][idx];
			differenceZ2 = persective_tex_vert[triangle_edge[1].end.origIdx][idx] - persective_tex_vert[triangle_edge[1].begin.origIdx][idx];

			aCrosstexture[idx] = (differenceY1 * differenceZ2) - (differenceZ1 * differenceY2);
			bCrosstexture[idx] = (differenceZ1 * differenceX2) - (differenceX1 * differenceZ2);
			cCrosstexture[idx] = (differenceX1 * differenceY2) - (differenceY1 * differenceX2);
			dCrosstexture[idx] = -1*((aCrosstexture[idx] * triangle_edge[0].begin.vertex[X]) + (bCrosstexture[idx] * triangle_edge[0].begin.vertex[Y]) + (cCrosstexture[idx] * persective_tex_vert[triangle_edge[0].begin.origIdx][idx]));
        }
    }
    float interpolateZ;
    GzColor color;
    for( int j = startY; j <= endY; j++ )
    {
        for( int i = startX; i <= endX; i++ )
        {
            GzIntensity r, g, b, a;
            GzDepth z;

            interpolateZ = (-1 * (aCross * i ) - (bCross * j) - dCross) / cCross;

			color20 = (A02 * (float)i) + (B02 * (float)j) + C02;
			color01 = (A10 * (float)i) + (B10 * (float)j) + C10;
			color12 = (A21 * (float)i) + (B21 * (float)j) + C21;

			GzCoord interpImageSpaceNormal;
			GzTextureIndex interpZ_texture;

            if ((color20 < 0 && color01 < 0 && color12 < 0) || (color20 > 0 && color01 > 0 && color12 > 0)
					|| color20 == 0 || color01 == 0 || color12 == 0)

            {
	            GzGetDisplay( render->display, i, j, &r, &g, &b, &a, &z );
	            if( interpolateZ < z || z == 0)
	            {
	                if (render->interp_mode == GZ_COLOR)
	                {
	                    for( int idx = 0; idx < 3; idx++ )
	                    {
	                        color[idx] = (-1 * (aCrossColor[idx] * i ) - (bCrossColor[idx] * j) - dCrossColor[idx]) / cCrossColor[idx];
	                    }
	                    r = ctoi(color[RED]);
						g = ctoi(color[GREEN]);
						b = ctoi(color[BLUE]);
	                }
	                else if (render->interp_mode == GZ_NORMALS)
	                {
	                    for( int idx = 0; idx < 3; idx++ )
	                    {
	                    	interpImageSpaceNormal[idx] = (-1 * (aCrossNormal[idx] * i ) - (bCrossNormal[idx] * j) - dCrossNormal[idx]) / cCrossNormal[idx];
	                    }
	                    for( int idx = 0; idx < 2; idx++ )
	                    {	                            
	                        interpZ_texture[idx] = (-1 * (aCrosstexture[idx] * i ) - (bCrosstexture[idx] * j) - dCrosstexture[idx]) / cCrosstexture[idx];
	                        interpZ_texture[idx] = interpZ_texture[idx] * ( (interpolateZ / ( INT_MAX - interpolateZ )) + 1 );
	                    }
	                    normalizeVector(interpImageSpaceNormal);
	                    shadingCalculation(render, color, interpImageSpaceNormal, interpZ_texture);
						r = ctoi(color[RED]);
						g = ctoi(color[GREEN]);
						b = ctoi(color[BLUE]);
	                }
	                else
	                {
						r = (GzIntensity) ctoi((float)render->flatcolor[RED]);
						g = (GzIntensity) ctoi((float)render->flatcolor[GREEN]);
						b = (GzIntensity) ctoi((float)render->flatcolor[BLUE]);
	                }
	                GzPutDisplay(render->display, i, j, r, g, b, a, (GzDepth) interpolateZ);
	            }
	        }

        } 
    }
    return GZ_SUCCESS;
}
