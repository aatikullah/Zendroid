//
//  Original Author: S. M. Shahriar Nirjon
//
//  Last Modified by: Mr. Mohammad Imrul Jubair [Assistant Professor (AUST CSE)]
//  Last Updated: 16 December 2017 
//
//  Version: 4.0
//

#pragma once
# include <stdio.h>
# include <stdlib.h>
#pragma comment(lib, "glut32.lib")
#pragma comment(lib, "glaux.lib")
#include "glut.h"
#include <time.h>
#include <math.h>
#include <windows.h>
#include "glaux.h"
# define STB_IMAGE_IMPLEMENTATION
# include "stb_image.h"
#include <map>
#include <string>
#include <utility>

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

// ---------------------------------------------------------------------------
//  RESOLUTION SCALING (added for the 1400x1000 window)
//
//  The game keeps working in its original 700x500 "logical" coordinate space,
//  so every position, speed, gravity value and hitbox in the level files stays
//  valid.  The window is simply ZEN_WINDOW_SCALE times bigger and OpenGL scales
//  everything up.  To change the window size later, change this ONE number:
//      1 = 700x500     2 = 1400x1000     (3 = 2100x1500, ...)
//  If the screen is too small for the window, the window is shrunk to fit.
//
//  ZEN_STROKE_TEXT: bitmap fonts cannot be scaled by OpenGL, so text is drawn
//  with a scalable vector font when the window is larger than the logical size.
//  Set it to 0 to go back to the original bitmap fonts (they will look small).
// ---------------------------------------------------------------------------
#define ZEN_WINDOW_SCALE 2
#define ZEN_STROKE_TEXT  1

int iWindowWidth, iWindowHeight;   // real window size in pixels
double iPixelScale = 1.0;          // window pixels per logical unit

int iScreenHeight, iScreenWidth;
int iMouseX, iMouseY;
int ifft=0;
void (*iAnimFunction[10])(void)={0};
int iAnimCount=0;
int iAnimDelays[10];
int iAnimPause[10];

unsigned int keyPressed[512];
unsigned int specialKeyPressed[512];

void iDraw();
void fixedUpdate();
void iMouseMove(int, int);
void iPassiveMouseMove(int, int);
void iMouse(int button, int state, int x, int y);

static void  __stdcall iA0(HWND,unsigned int, unsigned int, unsigned long){if(!iAnimPause[0])iAnimFunction[0]();}
static void  __stdcall iA1(HWND,unsigned int, unsigned int, unsigned long){if(!iAnimPause[1])iAnimFunction[1]();}
static void  __stdcall iA2(HWND,unsigned int, unsigned int, unsigned long){if(!iAnimPause[2])iAnimFunction[2]();}
static void  __stdcall iA3(HWND,unsigned int, unsigned int, unsigned long){if(!iAnimPause[3])iAnimFunction[3]();}
static void  __stdcall iA4(HWND,unsigned int, unsigned int, unsigned long){if(!iAnimPause[4])iAnimFunction[4]();}
static void  __stdcall iA5(HWND,unsigned int, unsigned int, unsigned long){if(!iAnimPause[5])iAnimFunction[5]();}
static void  __stdcall iA6(HWND,unsigned int, unsigned int, unsigned long){if(!iAnimPause[6])iAnimFunction[6]();}
static void  __stdcall iA7(HWND,unsigned int, unsigned int, unsigned long){if(!iAnimPause[7])iAnimFunction[7]();}
static void  __stdcall iA8(HWND,unsigned int, unsigned int, unsigned long){if(!iAnimPause[8])iAnimFunction[8]();}
static void  __stdcall iA9(HWND,unsigned int, unsigned int, unsigned long){if(!iAnimPause[9])iAnimFunction[9]();}
static void  __stdcall keypressHandler(HWND, unsigned int, unsigned int, unsigned long){ fixedUpdate(); }

int isKeyPressed(unsigned char key) {
	return keyPressed[key];
}

int isSpecialKeyPressed(unsigned char key) {
	return specialKeyPressed[key];
}

int iSetTimer(int msec, void (*f)(void))
{
    int i = iAnimCount;

    if(iAnimCount>=10){printf("Error: Maximum number of already timer used.\n");return -1;}

    iAnimFunction[i] = f;
    iAnimDelays[i] = msec;
    iAnimPause[i] = 0;

    if(iAnimCount == 0) SetTimer(0, 0, msec, iA0);
    if(iAnimCount == 1) SetTimer(0, 0, msec, iA1);
    if(iAnimCount == 2) SetTimer(0, 0, msec, iA2);
    if(iAnimCount == 3) SetTimer(0, 0, msec, iA3);
    if(iAnimCount == 4) SetTimer(0, 0, msec, iA4);

    if(iAnimCount == 5) SetTimer(0, 0, msec, iA5);
    if(iAnimCount == 6) SetTimer(0, 0, msec, iA6);
    if(iAnimCount == 7) SetTimer(0, 0, msec, iA7);
    if(iAnimCount == 8) SetTimer(0, 0, msec, iA8);
    if(iAnimCount == 9) SetTimer(0, 0, msec, iA9);
    iAnimCount++;

    return iAnimCount-1;
}

void iPauseTimer(int index){
    if(index>=0 && index <iAnimCount){
        iAnimPause[index] = 1;
    }
}

void iResumeTimer(int index){
    if(index>=0 && index <iAnimCount){
        iAnimPause[index] = 0;
    }
}

//
// Puts a BMP image on screen
//
// parameters:
//  x - x coordinate
//  y - y coordinate
//  filename - name of the BMP file
//  ignoreColor - A specified color that should not be rendered. If you have an
//                image strip that should be rendered on top of another back
//                ground image, then the background of the image strip should
//                not get rendered. Use the background color of the image strip
//                in ignoreColor parameter. Then the strip's background does
//                not get rendered.
//
//                To disable this feature, put -1 in this parameter
//
// BMPs are decoded once, uploaded as textures and drawn as quads, so they scale
// with the window (glDrawPixels cannot) and are no longer re-read from disk on
// every frame.  Colour-keyed images (ignoreColor != -1) use nearest filtering so
// no light fringe appears around sprites; opaque images use smooth filtering.
struct iBMPTexEntry { unsigned int tex; int w, h; };
typedef std::pair<std::string, int> iBMPKey;
static std::map<iBMPKey, iBMPTexEntry> iBMPCache;

void iShowBMP2(int x, int y, char filename[], int ignoreColor)
{
    iBMPKey key(std::string(filename), ignoreColor);
    std::map<iBMPKey, iBMPTexEntry>::iterator it = iBMPCache.find(key);

    if (it == iBMPCache.end())
    {
        iBMPTexEntry e;
        e.tex = 0; e.w = 0; e.h = 0;

        AUX_RGBImageRec *TextureImage = auxDIBImageLoad(filename);
        if (TextureImage && TextureImage->data)
        {
            int width = TextureImage->sizeX;
            int height = TextureImage->sizeY;
            int nPixels = width * height;
            unsigned char *px = new unsigned char[nPixels * 4];

            for (int i = 0, j = 0; i < nPixels; i++, j += 3)
            {
                unsigned char r = TextureImage->data[j];
                unsigned char g = TextureImage->data[j + 1];
                unsigned char b = TextureImage->data[j + 2];
                int rgb = (b << 16) | (g << 8) | r;   // same value the old code compared
                px[i * 4]     = r;
                px[i * 4 + 1] = g;
                px[i * 4 + 2] = b;
                px[i * 4 + 3] = (rgb == ignoreColor) ? 0 : 255;
            }

            glGenTextures(1, &e.tex);
            glBindTexture(GL_TEXTURE_2D, e.tex);
            GLint filter = (ignoreColor == -1) ? GL_LINEAR : GL_NEAREST;
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, px);
            e.w = width;
            e.h = height;
            delete[] px;
        }
        if (TextureImage)
        {
            free(TextureImage->data);
            free(TextureImage);
        }
        it = iBMPCache.insert(std::make_pair(key, e)).first;
    }

    if (!it->second.tex) return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, it->second.tex);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    // BMP rows are stored bottom-up, so t = 0 is the bottom edge.
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(x, y);
        glTexCoord2f(1, 0); glVertex2f(x + it->second.w, y);
        glTexCoord2f(1, 1); glVertex2f(x + it->second.w, y + it->second.h);
        glTexCoord2f(0, 1); glVertex2f(x, y + it->second.h);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

void iShowBMP(int x, int y, char filename[])
{
    iShowBMP2(x, y, filename, -1 /* ignoreColor */);
}

unsigned int iLoadImage(char filename[])
{
	int width, height, bpp;

	unsigned int texture;

	BYTE* data(0);
	data = stbi_load(filename, &width, &height, &bpp, 4);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D,
		0,
		GL_RGBA,
		width, height,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		data);

	stbi_image_free(data);

	return texture;
}

void iShowImage(int x, int y, int width, int height, unsigned int texture)
{

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// NOTE: keep GL_REPEAT here. The quad below uses t = 0 .. -1 to flip the image
	// upright, which only works with wrap-around. (CLAMP would break every image.)
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glBegin(GL_QUADS);

		glTexCoord2f(0, 0);
		glVertex2f(x, y);

		glTexCoord2f(1, 0);
		glVertex2f(x + width, y);

		glTexCoord2f(1, -1);
		glVertex2f(x + width, y+height);

		glTexCoord2f(0, -1);
		glVertex2f(x, y + height);

	glEnd();

	glDisable(GL_TEXTURE_2D);

}

void iGetPixelColor (int cursorX, int cursorY, int rgb[])
{
    GLubyte pixel[3];
    glReadPixels(cursorX, cursorY,1,1,
        GL_RGB,GL_UNSIGNED_BYTE,(void *)pixel);

    rgb[0] = pixel[0];
    rgb[1] = pixel[1];
    rgb[2] = pixel[2];

    //printf("%d %d %d\n",pixel[0],pixel[1],pixel[2]);
}

// Approximate height (in logical units) of each bitmap font, used to size the
// vector replacement font so text keeps its original proportions on screen.
static float iFontSize(void *font)
{
    if (font == GLUT_BITMAP_8_BY_13)        return 13.0f;
    if (font == GLUT_BITMAP_9_BY_15)        return 15.0f;
    if (font == GLUT_BITMAP_TIMES_ROMAN_10) return 10.0f;
    if (font == GLUT_BITMAP_TIMES_ROMAN_24) return 24.0f;
    if (font == GLUT_BITMAP_HELVETICA_10)   return 10.0f;
    if (font == GLUT_BITMAP_HELVETICA_12)   return 12.0f;
    if (font == GLUT_BITMAP_HELVETICA_18)   return 18.0f;
    return 13.0f;
}

void iText(GLdouble x, GLdouble y, char *str, void* font=GLUT_BITMAP_8_BY_13)
{
#if ZEN_STROKE_TEXT
    if (iPixelScale > 1.01)
    {
        float size = iFontSize(font);
        float s = size * 0.75f / 100.0f;          // stroke capitals are 100 units tall
        GLfloat oldWidth = 1.0f;
        glGetFloatv(GL_LINE_WIDTH, &oldWidth);
        float lw = (float)(size * 0.09 * iPixelScale);
        if (lw < 1.5f) lw = 1.5f;
        glLineWidth(lw);

        glPushMatrix();
        glTranslated(x, y, 0);
        glScalef(s, s, 1.0f);
        for (int k = 0; str[k]; k++) {
            glutStrokeCharacter(GLUT_STROKE_ROMAN, (unsigned char)str[k]);
        }
        glPopMatrix();

        glLineWidth(oldWidth);
        return;
    }
#endif
    glRasterPos3d(x, y, 0);
    int i;
    for (i=0; str[i]; i++) {
        glutBitmapCharacter(font, str[i]); //,GLUT_BITMAP_8_BY_13, GLUT_BITMAP_TIMES_ROMAN_24
    }
}

void iPoint(double x, double y, int size=0)
{
    int i, j;
    glBegin(GL_POINTS);
    glVertex2f(x, y);
    for(i=x-size;i<x+size;i++)
    {
        for(j=y-size; j<y+size;j++)
        {
            glVertex2f(i, j);
        }
    }
    glEnd();
}

void iLine(double x1, double y1, double x2, double y2)
{
    glBegin(GL_LINE_STRIP);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
}

void iFilledPolygon(double x[], double y[], int n)
{
    int i;
    if(n<3)return;
    glBegin(GL_POLYGON);
    for(i = 0; i < n; i++){
        glVertex2f(x[i], y[i]);
    }
    glEnd();
}

void iPolygon(double x[], double y[], int n)
{
    int i;
    if(n<3)return;
    glBegin(GL_LINE_STRIP);
    for(i = 0; i < n; i++){
        glVertex2f(x[i], y[i]);
    }
    glVertex2f(x[0], y[0]);
    glEnd();
}

void iRectangle(double left, double bottom, double dx, double dy)
{
    double x1, y1, x2, y2;

    x1 = left;
    y1 = bottom;
    x2=x1+dx;
    y2=y1+dy;

    iLine(x1, y1, x2, y1);
    iLine(x2, y1, x2, y2);
    iLine(x2, y2, x1, y2);
    iLine(x1, y2, x1, y1);
}

void iFilledRectangle(double left, double bottom, double dx, double dy)
{
    double xx[4], yy[4];
    double x1, y1, x2, y2;

    x1 = left;
    y1 = bottom;
    x2=x1+dx;
    y2=y1+dy;

    xx[0]=x1;
    yy[0]=y1;
    xx[1]=x2;
    yy[1]=y1;
    xx[2]=x2;
    yy[2]=y2;
    xx[3]=x1;
    yy[3]=y2;

    iFilledPolygon(xx, yy, 4);
}

void iFilledCircle(double x, double y, double r, int slices=100)
{
    double t, PI=acos(-1.0), dt, x1,y1, xp, yp;
    dt = 2*PI/slices;
    xp = x+r;
    yp = y;
    glBegin(GL_POLYGON);
    for(t = 0; t <= 2*PI; t+=dt)
    {
        x1 = x + r * cos(t);
        y1 = y + r * sin(t);

        glVertex2f(xp, yp);
        xp = x1;
        yp = y1;
    }
    glEnd();
}

void iCircle(double x, double y, double r, int slices=100)
{
    double t, PI=acos(-1.0), dt, x1,y1, xp, yp;
    dt = 2*PI/slices;
    xp = x+r;
    yp = y;
    for(t = 0; t <= 2*PI; t+=dt)
    {
        x1 = x + r * cos(t);
        y1 = y + r * sin(t);
        iLine(xp, yp, x1, y1);
        xp = x1;
        yp = y1;
    }
}

void iEllipse(double x, double y, double a, double b, int slices=100)
{
    double t, PI=acos(-1.0), dt, x1,y1, xp, yp;
    dt = 2*PI/slices;
    xp = x+a;
    yp = y;
    for(t = 0; t <= 2*PI; t+=dt)
    {
        x1 = x + a * cos(t);
        y1 = y + b * sin(t);
        iLine(xp, yp, x1, y1);
        xp = x1;
        yp = y1;
    }
}

void iFilledEllipse(double x, double y, double a, double b, int slices=100)
{
    double t, PI=acos(-1.0), dt, x1,y1, xp, yp;
    dt = 2*PI/slices;
    xp = x+a;
    yp = y;
    glBegin(GL_POLYGON);
    for(t = 0; t <= 2*PI; t+=dt)
    {
        x1 = x + a * cos(t);
        y1 = y + b * sin(t);
        glVertex2f(xp, yp);
        xp = x1;
        yp = y1;
    }
    glEnd();
}

// Rotates the co-ordinate system
// Parameters:
//  (x, y) - The pivot point for rotation
//  degree - degree of rotation
//
// After calling iRotate(), evrey subsequent rendering will
// happen in rotated fashion. To stop rotation of subsequent rendering,
// call iUnRotate(). Typical call pattern would be:
//      iRotate();
//      Render your objects, that you want rendered as rotated
//      iUnRotate();
//
void iRotate(double x, double y, double degree)
{
	glPushMatrix();

	glTranslatef(x, y, 0.0);

	glRotatef(degree, 0, 0, 1.0);

	glTranslatef(-x, -y, 0.0);
}

void iUnRotate()
{
	glPopMatrix();
}

void iSetColor(double r, double g, double b)
{
    double mmx;
    mmx = r;
    if(g > mmx)mmx = g;
    if(b > mmx)mmx = b;
    mmx = 255;
    if(mmx > 0){
        r /= mmx;
        g /= mmx;
        b /= mmx;
    }
    glColor3f(r, g, b);
}

void iDelay(int sec)
{
    int t1, t2;
    t1 = time(0);
    while(1){
        t2 = time(0);
        if(t2-t1>=sec)
            break;
    }
}

void iDelayMS(int msec)
{
	clock_t end;
	end = clock() + msec * CLOCKS_PER_SEC / 1000;
	while (end > clock());
}

void iClear()
{
    glClear(GL_COLOR_BUFFER_BIT) ;
    glMatrixMode(GL_MODELVIEW) ;
    glClearColor(0,0,0,0);
    glFlush();
}

// window pixels per logical unit, horizontally / vertically
double iPixelScaleX() { return (double)iWindowWidth  / iScreenWidth;  }
double iPixelScaleY() { return (double)iWindowHeight / iScreenHeight; }

// Called by GLUT whenever the window is created or resized.  The whole window
// is used for the game, so resizing the window just scales the picture.
void reshapeFF(int w, int h)
{
    if (w < 1) w = 1;
    if (h < 1) h = 1;
    iWindowWidth = w;
    iWindowHeight = h;
    iPixelScale = (iPixelScaleX() + iPixelScaleY()) / 2.0;

    glViewport(0, 0, w, h);
    glLineWidth((GLfloat)iPixelScale);   // keep lines/points as thick as before,
    glPointSize((GLfloat)iPixelScale);   // relative to the picture
}

void displayFF(void){

    iDraw();
    glutSwapBuffers() ;
}

void animFF(void)
{
    if(ifft == 0){
        ifft = 1;
        iClear();
    }
    glutPostRedisplay();
}

void keyboardHandlerUp1FF(unsigned char key, int x, int y)
{
	keyPressed[key] = 0;
    glutPostRedisplay();
}
void keyboardHandlerUp2FF(int key, int x, int y)
{
	specialKeyPressed[key] = 0;
    glutPostRedisplay();
}

void keyboardHandler1FF(unsigned char key, int x, int y)
{
	keyPressed[key] = 1;
	glutPostRedisplay();
}
void keyboardHandler2FF(int key, int x, int y)
{
	specialKeyPressed[key] = 1;
	glutPostRedisplay();
}

void mouseMoveHandlerFF(int mx, int my)
{
    iMouseX = (int)(mx / iPixelScaleX());
    iMouseY = iScreenHeight - (int)(my / iPixelScaleY());
    iMouseMove(iMouseX, iMouseY);

    glFlush();
}

void mousePassiveMoveHandlerFF(int mx, int my)
{
	iMouseX = (int)(mx / iPixelScaleX());
	iMouseY = iScreenHeight - (int)(my / iPixelScaleY());
	iPassiveMouseMove(iMouseX, iMouseY);

	glFlush();
}

void mouseHandlerFF(int button, int state, int x, int y)
{
    iMouseX = (int)(x / iPixelScaleX());
    iMouseY = iScreenHeight - (int)(y / iPixelScaleY());

    iMouse(button, state, iMouseX, iMouseY);

    glFlush();
}

void iInitialize(int width=500, int height=500, char *title="iGraphics", int keyboardSamplingRate = 16)
{
	SetTimer(0, 0, keyboardSamplingRate, keypressHandler);

    iScreenHeight = height;    // logical size (what the game code works in)
    iScreenWidth = width;

    // Real window size = logical size * scale, shrunk if the screen is too small.
    int winW = width * ZEN_WINDOW_SCALE;
    int winH = height * ZEN_WINDOW_SCALE;
    int availW = GetSystemMetrics(SM_CXSCREEN) - 40;
    int availH = GetSystemMetrics(SM_CYSCREEN) - 100;   // title bar + taskbar
    if (availW > 100 && availH > 100 && (winW > availW || winH > availH))
    {
        double fw = (double)availW / winW;
        double fh = (double)availH / winH;
        double f = (fw < fh) ? fw : fh;
        winW = (int)(winW * f);
        winH = (int)(winH * f);
    }
    iWindowWidth = winW;
    iWindowHeight = winH;
    iPixelScale = (iPixelScaleX() + iPixelScaleY()) / 2.0;

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_ALPHA) ;
    glutInitWindowSize(winW , winH ) ;
    glutInitWindowPosition( 10 , 10 ) ;
    glutCreateWindow(title) ;
    glClearColor( 0.0 , 0.0 , 0.0 , 0.0 ) ;
    glMatrixMode( GL_PROJECTION) ;
    glLoadIdentity() ;
    glOrtho(0.0 , width , 0.0 , height , -1.0 , 1.0) ;
    //glOrtho(-100.0 , 100.0 , -100.0 , 100.0 , -1.0 , 1.0) ;
    //SetTimer(0, 0, 10, timer_proc);


}

void iStart()
{
    iClear();

    glutReshapeFunc(reshapeFF);
    glLineWidth((GLfloat)iPixelScale);
    glPointSize((GLfloat)iPixelScale);
    glutDisplayFunc(displayFF) ;
    glutKeyboardFunc(keyboardHandler1FF); //normal
    glutSpecialFunc(keyboardHandler2FF); //special keys
	glutKeyboardUpFunc(keyboardHandlerUp1FF);
	glutSpecialUpFunc(keyboardHandlerUp2FF);
    glutMouseFunc(mouseHandlerFF);
    glutMotionFunc(mouseMoveHandlerFF);
	glutPassiveMotionFunc(mousePassiveMoveHandlerFF);
    glutIdleFunc(animFF) ;

    //
    // Setup Alpha channel testing.
    // If alpha value is greater than 0, then those
    // pixels will be rendered. Otherwise, they would not be rendered
    //
    glAlphaFunc(GL_GREATER,0.0f);
    glEnable(GL_ALPHA_TEST);

    glutMainLoop();
}
