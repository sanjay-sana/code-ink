/**
 * Application
 * USC csci 580 *
*/

#pragma once

#include "disp.h"
#include "gz.h"

typedef struct{
  
  GzDisplay * display;
  
  char      * frameBuffer;
  
  unsigned int width;
  
  unsigned int height;
  
}GzDisplayApplication;

int runRenderer(GzDisplayApplication* application);