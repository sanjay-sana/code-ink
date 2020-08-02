// Application4.h: interface for the Application4 class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

struct GzDisplay;
struct GzRender;

class Application4
{
public:	
	int runRenderer();
protected:
	GzDisplay* display; // the display
	GzRender* render;   // the renderer
};
