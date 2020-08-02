/**
 * Application
 * USC csci 580 *
*/

#include "Application1.h"
#include <stdio.h>
#include <stdlib.h>

#define INFILE  "./rects"
#define OUTFILE "./output.ppm"

int runRenderer(GzDisplayApplication* application)
{
  int i, j;
  int xRes, yRes, dispClass; //display parameters
  int status;
  
  status = 0;
  
  /*
   * initialize the display and the renderer
   */
  
  application->width  = 512;  // frame buffer and display width
  application->height = 512;  // frame buffer and display height
  
  status |= GzNewFrameBuffer(&application->frameBuffer, application->width, application->height);
  
  status |= GzNewDisplay(&application->display, GZ_RGBAZ_DISPLAY, application->width, application->height);
  
  status |= GzGetDisplayParams(application->display, &xRes, &yRes, &dispClass);
  
  status |= GzInitDisplay(application->display);  /* init for new frame */
  
  if (status) exit(GZ_FAILURE);
  
  // I/O File open
  FILE *infile;
  if( (infile = fopen( INFILE , "r" )) == NULL )
  {
    printf("Could not open input from %s \n", INFILE);
    return GZ_FAILURE;
  }

  FILE *outfile;
  if( (outfile = fopen( OUTFILE , "wb" )) == NULL )
  {
    printf("Could not open output file for writing %s \n", OUTFILE);
    return GZ_FAILURE;
  }
	
  int ulx, uly, lrx, lry, r, g, b;
  while( fscanf(infile, "%d %d %d %d %d %d %d", &ulx, &uly, &lrx, &lry, &r, &g, &b) == 7)
  {
    for (j = uly; j <= lry; j++)
    {
      for (i = ulx; i <= lrx; i++)
      {
        GzPutDisplay(application->display, i, j, r, g, b, 1, 0);
      }
    }
  }
	
  GzFlushDisplay2File(outfile, application->display); 	/* write out or update display to file*/

  /*
   * Clean up and exit
   */
  
  if( fclose( infile ) )
    printf( "The input file was not closed\n" );
  
  if( fclose( outfile ) )
    printf( "The output file was not closed\n" );
  
  status |= GzFreeDisplay(application->display);
  
  if (status)
    return GZ_FAILURE;
  else
    return GZ_SUCCESS;
}