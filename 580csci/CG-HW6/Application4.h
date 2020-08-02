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
	int combineMult();
	GzDisplay* final_display;
protected:
	GzDisplay** display; // the display
	GzRender** render;   // the renderer
};
