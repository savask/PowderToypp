#include "Graphics.h"
#include "font.h"

#ifdef OGLI

Graphics::Graphics():
sdl_scale(1)
{
	Reset();
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	//Texture for main UI
	glEnable(GL_TEXTURE_2D);
	
	glGenTextures(1, &vidBuf);
	glBindTexture(GL_TEXTURE_2D, vidBuf);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, XRES+BARSIZE, YRES+MENUSIZE, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glGenTextures(1, &textTexture);
	glBindTexture(GL_TEXTURE_2D, textTexture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glDisable(GL_TEXTURE_2D);
}

Graphics::~Graphics()
{
}

void Graphics::Reset()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	//glOrtho(0, (XRES+BARSIZE)*sdl_scale, 0, (YRES+MENUSIZE)*sdl_scale, -1, 1);
	glOrtho(0, (XRES+BARSIZE)*sdl_scale, (YRES+MENUSIZE)*sdl_scale, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//glRasterPos2i(0, (YRES+MENUSIZE));
	glRasterPos2i(0, 0);
	glPixelZoom(1, 1);
}

void Graphics::Clear()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void Graphics::Finalise()
{
    glFlush();
}

#define VIDXRES XRES+BARSIZE
#define VIDYRES YRES+MENUSIZE
#define PIXELMETHODS_CLASS Graphics
#include "OpenGLDrawMethods.inc"
#undef VIDYRES
#undef VIDXRES
#undef PIXELMETHODS_CLASS


#endif
