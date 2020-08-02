#include    <iostream>
#include        "stdio.h"
#include        "math.h"
#include        "Gz.h"

GzColor *image;
int xs, ys;
int reset = 1;

/* Image texture function */
int tex_fun(float u, float v, GzColor color)
{
 	unsigned char	pixel[3];
	unsigned char	dummy;
	char		foo[8];
	int		i;
	FILE		*fd;

	if (reset) {          /* open and load texture file */
		fd = fopen ("texture", "rb");
		if (fd == NULL) {
			fprintf (stderr, "texture file not found\n");
			return GZ_FAILURE;
		}
		fscanf (fd, "%s %d %d %c", foo, &xs, &ys, &dummy);
		image = (GzColor*)malloc(sizeof(GzColor)*(xs+1)*(ys+1));
		if (image == NULL) {
			fprintf (stderr, "malloc for texture image failed\n");
			return GZ_FAILURE;
		}

		for (i = 0; i < xs*ys; i++) {	/* create array of GzColor values */
			fread(pixel, sizeof(pixel), 1, fd);
			image[i][RED] = (float)((int)pixel[RED]) * (1.0 / 255.0);
			image[i][GREEN] = (float)((int)pixel[GREEN]) * (1.0 / 255.0);
			image[i][BLUE] = (float)((int)pixel[BLUE]) * (1.0 / 255.0);
		}

		reset = 0;          /* init is done */
		fclose(fd);
	}

	/* bounds-test u,v to make sure nothing will overflow image array bounds */
	/* determine texture cell corner values and perform bilinear interpolation */
	/* set color to interpolated GzColor value and return */

	if (u < 0)
	{
		u = 0;
	}
	else if (u > 1)
	{
		u = 1;
	}
	if (v < 0)
	{
		v = 0;
	}
	else if (v > 1)
	{
		v = 1;
	}
	// for look up and interpolating
	float mapU = u * (xs - 1);
	float mapV = v * (ys - 1);

	int xMin = floor(mapU);
	int yMin = floor(mapV);
	int xMax = ceil(mapU);
	int yMax = ceil(mapV);

	float s, t;
	s = mapU - floor(mapU);
	t = mapV - floor(mapV);

	GzColor colorA, colorB, colorC, colorD;

	// Bilinear Interpolation
	for (int idx = 0; idx < 3; idx++)
	{

		colorA[idx]=image[xMin + (yMin * xs)][idx];
		colorB[idx]=image[xMax + (yMin * xs)][idx];
		colorC[idx]=image[xMax + (yMax * xs)][idx];
		colorD[idx]=image[xMin + (yMax * xs)][idx];

		color[idx] = s*t*colorC[idx] + (1-s)*t*colorD[idx] + s*(1-t)*colorB[idx] + (1-s)*(1-t)*colorA[idx];
    }

    return GZ_SUCCESS;
}

class complexNumber
{
public:
	float r; 
	float i;
};

int ptex_fun(float u, float v, GzColor color)
{
	int N = 500;

	complexNumber x;
	complexNumber c;
	
	c.r = -0.333;
	c.i = 0.434;

	x.r = 2 * u - 1.2;
    x.i = 2 * v - 1.3;
	int i;
	float clr;
    for(i = 0; i < N; i++) 
	{
        float xr = x.r * x.r - x.i * x.i + c.r;
        float xi = x.r * x.i + x.i * x.r + c.i;
		if ((x.r * x.r + x.i * x.i) > 4.0)
		{
			break;
		}
        x.r = xr;
        x.i = xi;
    }

	float length = sqrt(x.r * x.r + x.i * x.i);
	if (i == N)
	{
		color[0] = length / 2;
		color[1] = length / 2;
		color[2] = length / 2;
	}
	else
	{
		clr = ((float)i) / N;
		color[0] = clr * 10;
		color[1] = clr * 8;
		color[2] = clr * 10;

	}
    return GZ_SUCCESS;
}