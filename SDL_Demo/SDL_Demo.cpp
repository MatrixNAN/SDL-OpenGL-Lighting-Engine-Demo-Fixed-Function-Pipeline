#include <stdlib.h>

#include <math.h>

#include "Light.h"
#include "Texture.h"
#include "SDL.h"

#ifdef WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#endif

#if defined(__APPLE__) && defined(__MACH__)
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#else
	//#include <gl/OpenGL.h>
	#include <gl/GL.h>
	#include <gl/GLU.h>
	//#include <GL\glut.h>
#endif

const GLsizei windowWidth = 500;
const GLsizei windowHeight = 500;

GLfloat cubeRotateX = 45.0f;
GLfloat cubeRotateY = 45.0f;

Uint8 *keys = NULL; 

Light *mainLight = NULL;
Texture *texture = NULL;

int *seed = NULL;

GLvoid establishProjectionMatrix(GLsizei width, GLsizei height)
{
	glViewport(0,0,width,height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 200.0f);
}

GLvoid initGL(GLsizei width, GLsizei height)
{
	Light::Initialize();

	establishProjectionMatrix(width, height);

	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_PERSPECTIVE_CORRECTION_HINT);

	glEnable(GL_LIGHTING);

	texture = new Texture("Textures/Sitework.Planting.Gravel.Loose.tga", "Gravel");

}

GLvoid displayFPS(GLvoid)
{
	static long lastTime = SDL_GetTicks(); 
	static long loops = 0;
	static GLfloat fps = 0.0f;

	int newTime = SDL_GetTicks();
	
	if(newTime - lastTime > 100) 
	{
		float newFPS = (float)loops / float(newTime / lastTime) * 1000.0f;  

		fps = (fps + newFPS) / 2.0f;

		char title[80];
		sprintf_s(title, "OpenGL Demo - %.2f", fps);
		SDL_WM_SetCaption(title, NULL);

		lastTime = newTime;

		loops = 0;
	}

	loops++;
}

GLvoid drawGrid(GLvoid)
{
	const float width = 80.0f;
	const float height = 80.0f;
	const int divisions = 100;

	float incX = width / (float) divisions;
	float incY = height / (float) divisions;

	glColor3f(0.5f, 0.5f, 0.5f);
	glNormal3f(0,1,0);
	for (float x = -width / 2; x < width / 2; x += incX)
	{
		for (float y = -height / 2; y < height / 2; y += incY)
		{
			glBegin(GL_TRIANGLE_STRIP);
			// Top Face
				glVertex3f(x + incX, 0.0f, y + incY);
				glVertex3f(x,		 0.0f, y + incY);
				glVertex3f(x + incX, 0.0f, y);
				glVertex3f(x,		 0.0f, y);
			glEnd();
		}
	}
}


GLvoid drawScene(GLvoid)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f,0.0f,-80.0f);
	glRotatef(cubeRotateX, 1, 0, 0);
	glRotatef(cubeRotateY, 0, 1, 0);
	for(int i = 0; i < (int)Light::lights.size(); i++)
	{
		double randomNumber = (double)SDL_GetTicks() + seed[i];
		float x = (float)sin( randomNumber / 1600.0f ) * (float)cos( randomNumber / 1200.0f ) * 50.0f; 
		float y = (float)sin( randomNumber / 900.0f ) * (float)cos( randomNumber / 1400.0f ) * 50.0f; 

		Light::lights[i]->setPosition( x, 30.0f, y );
		Light::lights[i]->updateLight();
	}

	drawGrid();

	glColor3f(1.0f, 1.0f, 1.0f);

	glFlush();
	// this line is modified depending on which window manager is used...
	SDL_GL_SwapBuffers();

	displayFPS();
}

GLboolean checkKeys(GLvoid)
{
	static long lastTime = SDL_GetTicks();
	const GLfloat speed = 1.0f;
	const long updateTime = 10;
	long newTime = SDL_GetTicks();

	if (newTime - lastTime > updateTime)
	{
		if(keys[SDLK_ESCAPE])
			return true;

		if(keys[SDLK_LEFT])
			cubeRotateY -= speed;

		if(keys[SDLK_RIGHT])
			cubeRotateY += speed;

		if(keys[SDLK_UP])
			cubeRotateX -= speed;

		if(keys[SDLK_DOWN])
			cubeRotateX += speed;
	}

	return false;
}

int main(int argc,  char **argv)
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		// error
		fprintf(stderr, "Unable to initalize SDL %s", SDL_GetError());
		exit(1);
	}

	if(SDL_SetVideoMode(windowWidth, windowHeight, 0, SDL_OPENGL) == NULL)
	{
		// error
		fprintf(stderr, "Unable to Create OpenGL Scene %s", SDL_GetError());
		exit(2);
	}

	initGL(windowWidth, windowHeight);

	seed = new int[Light::numLights];
	for ( int i = 0; i < Light::numLights; i++ )
	{
		Light *light = new Light(LIGHT_SPOT);

		float r = (float)rand() / (float) RAND_MAX;
		float g = (float)rand() / (float) RAND_MAX;
		float b = (float)rand() / (float) RAND_MAX;

		light->setDiffuse(r,g,b,1);
		light->setAmbient(0,0,0,1);
		light->setSpotDirection(0, -1, 0);

		float cutOff = 20.0f + (float) rand() / (float) RAND_MAX * 60.0f;
		light->setCutoff(cutOff);
		light->setExponent(20.0f * cutOff);

		seed[i] = rand();
	}

	int done = 0;

	while(!done)
	{
		drawScene();
		SDL_Event event;
		while (SDL_PollEvent (&event))
		{
			if(event.type == SDL_QUIT)
			{
				done = 1;
			}

			keys = SDL_GetKeyState(NULL);
		}

		if(checkKeys())
		{
			done = 1;
		}
	}

	for(int i = 0; i < (int) Light::lights.size(); i++)
		delete Light::lights[i];
	
	delete seed;

	SDL_Quit();

	return 1;
}