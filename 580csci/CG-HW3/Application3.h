// Application3.h: interface for the Application3 class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class GzDisplay;
class GzRender;

class Application3
{
public:	
	int runRenderer();
protected:
	GzDisplay* display; // the display
	GzRender* render;   // the renderer
};
