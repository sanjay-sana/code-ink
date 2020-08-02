/* 
*  disp.cpp -- definition file for Display
*  USC csci 580 
*/

#include <iostream>
#include "gz.h"
#include "disp.h"

int GzNewFrameBuffer(char** framebuffer, int width, int height)
{
  /* create a framebuffer:
   -- allocate memory for framebuffer : (sizeof)GzPixel x width x height
   -- pass back pointer
  */
  if (!framebuffer)
  {
	  return GZ_FAILURE;
  }
  *framebuffer = new char[sizeof(GzPixel)*width*height];
  return GZ_SUCCESS;
}

int GzNewDisplay(GzDisplay **display, GzDisplayClass dispClass, int xRes, int yRes)
{
  /* create a display:
   -- allocate memory for indicated class and resolution
   -- pass back pointer to GzDisplay object in display
   */
  if (!display)
  {
	  return GZ_FAILURE;
  }
  *display = new GzDisplay();
  (*display)->xres=xRes;
  (*display)->yres=yRes;
  (*display)->dispClass=dispClass;
  (*display)->fbuf = new GzPixel[sizeof(GzPixel)*xRes*yRes];
  return GZ_SUCCESS;
}


int GzFreeDisplay(GzDisplay *display)
{
  /* clean up, free memory */
  free (display);
  return GZ_SUCCESS;
}


int GzGetDisplayParams(GzDisplay *display, int *xRes, int *yRes, GzDisplayClass	*dispClass)
{
  /* pass back values for an open display */
  *xRes=display->xres;
  *yRes=display->yres;
  *dispClass=display->dispClass;
  return GZ_SUCCESS;
}


int GzInitDisplay(GzDisplay *display)
{
  /* set everything to some default values - start a new frame */
  for(int i=0;i<display->xres;i++)
  {
	  for(int j=0;j<display->yres;j++)
	  {
		  display->fbuf[ARRAY(i,j)].red=2048;
		  display->fbuf[ARRAY(i,j)].green=1792;
		  display->fbuf[ARRAY(i,j)].blue=1536;
		  display->fbuf[ARRAY(i,j)].alpha=0;
		  display->fbuf[ARRAY(i,j)].z=0;
	  }
  }

  return GZ_SUCCESS;
}


int GzPutDisplay(GzDisplay *display, int i, int j, GzIntensity r, GzIntensity g, GzIntensity b, GzIntensity a, GzDepth z)
{
  /* write pixel values into the display */
  if(i>255)
  {
  i=255;
  }
  else if(i<0)
	  {
		i=0;
	  }
  if(j>255)
  {
	j=255;
  }
  else if(j<0)
	  {
		j=0;
	  }
  if(r>4095)
  {
  r=4095;
  }
  else if(r<0)
	  {
		r=0;
	  }
  if(g>4095)
  {
  g=4095;
  }
  else if(g<0)
	  {
		g=0;
	  } 
  if(b>4095)
  {
  b=4095;
  }
  else if(b<0)
	  {
		b=0;
	  }
  display->fbuf[ARRAY(i,j)].red=r;
  display->fbuf[ARRAY(i,j)].green=g;
  display->fbuf[ARRAY(i,j)].blue=b;
  display->fbuf[ARRAY(i,j)].alpha=a;
  display->fbuf[ARRAY(i,j)].z=z;
  return GZ_SUCCESS;
}


int GzGetDisplay(GzDisplay *display, int i, int j, GzIntensity *r, GzIntensity *g, GzIntensity *b, GzIntensity *a, GzDepth *z)
{
  /* pass back pixel value in the display */
  /* check display class to see what vars are valid */
  if(i>255)
  {
  i=255;
  }
  else if(i<0)
	  {
		i=0;
	  }
  if(j>255)
  {
	j=255;
  }
  else if(j<0)
	  {
		j=0;
	  }
  if(*r>4095)
  {
  *r=4095;
  }
  else if(*r<0)
	  {
		*r=0;
	  }
  if(*g>4095)
  {
  *g=4095;
  }
  else if(*g<0)
	  {
		*g=0;
	  } 
  if(*b>4095)
  {
  *b=4095;
  }
  else if(*b<0)
	  {
		*b=0;
	  } 
  *r=display->fbuf[ARRAY(i,j)].red;
  *g=display->fbuf[ARRAY(i,j)].green;
  *b=display->fbuf[ARRAY(i,j)].blue;
  *a=display->fbuf[ARRAY(i,j)].alpha;
  *z=display->fbuf[ARRAY(i,j)].z;
  return GZ_SUCCESS;
}


int GzFlushDisplay2File(FILE* outfile, GzDisplay *display)
{
  /* write pixels to ppm file based on display class -- "P6 %d %d 255\r" */
  char rgb[3];
  fprintf(outfile,"P6 %d %d 255\n",256,256);
  for(int j=0;j<display->yres;j++)
  {
	  for(int i=0;i<display->xres;i++)
	  {
		  rgb[0]=(char)(display->fbuf[ARRAY(i,j)].red >> 4);
		  rgb[1]=(char)(display->fbuf[ARRAY(i,j)].green >> 4);
		  rgb[2]=(char)(display->fbuf[ARRAY(i,j)].blue >> 4);
		  fwrite(rgb,sizeof(rgb),1,outfile);
	  }
  }
  return GZ_SUCCESS;
}

int GzFlushDisplay2FrameBuffer(char* framebuffer, GzDisplay *display)
{
  /* write pixels to framebuffer:
   - Put the pixels into the frame buffer
   - Caution: store the pixel to the frame buffer as the order of blue, green, and red
   - Not red, green, and blue !!!
  */
  int counter=0;
  for(int j=0;j<display->yres;j++)
  {
	  for(int i=0;i<display->xres;i++)
	  {
		  framebuffer[counter++]=(display->fbuf[ARRAY(i,j)].blue>>4);
		  framebuffer[counter++]=(display->fbuf[ARRAY(i,j)].green>>4);
		  framebuffer[counter++]=(display->fbuf[ARRAY(i,j)].red>>4);
	  }
  }
  return GZ_SUCCESS;
}
