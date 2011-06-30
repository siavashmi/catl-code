#include "display2D.h"
#include "math.h"
#include <stdio.h>
#include <GL/glut.h>
#include "common.h"
#include  <ctype.h>
#include "sensor.h"
#include "field.h"
double ooffsetx=200;
double ooffsety=300;
double changeStep=20;
int model=0;
double displayScale=1;
double scale=1;
double angle =0;

void myKeyboardFunc2D(unsigned char key, int x, int y)
{
	switch ( key ) {
		case 'c':
			changDisplayMode2D();
			break;
		case 'z':
			angle = angle + 1;
			if (angle >=360)
				angle =0;
			break;
		case 'x':
			angle =angle -1;
			if (angle <0)
				angle =0;
			break;
	    case 27:	// ESC
		    exit(1);
	}
}

void drawScene2D(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode( GL_MODELVIEW );
	glTranslatef( 0, 0, 0.0 );// Current matrix affects objects positions
	glLoadIdentity();						// Initialize to the identity
	glPushMatrix();
	glScalef( 0.0025, 0.0025, 0.0025);
	glScalef(scale,scale,scale);
	//rotate around the vector
	drawPoint2D();
	glPopMatrix();
	// Flush the pipeline, swap the buffers
	glFlush();
}
void initRendering2D()
{
	glEnable ( GL_DEPTH_TEST );
	glClearColor(1,1,1,1);
	glPointSize(2);
	glLineWidth(2);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);	// Make round points, not square points
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);		// Antialias the lines
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
void resizeWindow2D(int w, int h)
{
	const double Xmin = 0.0, Xmax = 3.0;
	const double Ymin = 0.0, Ymax = 3.0;

	double scale, center;
	double windowXmin, windowXmax, windowYmin, windowYmax;

	// Define the portion of the window used for OpenGL rendering.
	glViewport( 0, 0,2* w, 2*h );	// View port uses whole window
	w = (w==0) ? 1 : w;
	h = (h==0) ? 1 : h;
	if ( (Xmax-Xmin)/w < (Ymax-Ymin)/h ) {
		scale = ((Ymax-Ymin)/h)/((Xmax-Xmin)/w);
		center = (Xmax+Xmin)/2;
		windowXmin = center - (center-Xmin)*scale;
		windowXmax = center + (Xmax-center)*scale;
		windowYmin = Ymin;
		windowYmax = Ymax;
	}
	else {
		scale = ((Xmax-Xmin)/w)/((Ymax-Ymin)/h);
		center = (Ymax+Ymin)/2;
		windowYmin = center - (center-Ymin)*scale;
		windowYmax = center + (Ymax-center)*scale;
		windowXmin = Xmin;
		windowXmax = Xmax;
	}
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( windowXmin, windowXmax, windowYmin, windowYmax, -1, 1 );;
}
 void drawPoint2D()
{
	double scaleDisplay;
	if(model == 1)
	{
		displayOriginalTopology();
	 }
	else 
	if (model == 0)
	{
        displayLocalizedTopology();
	}
	else 
	{
		displayTransformedTopology();
	}
	
}
void mySpecialKeyFunc2D( int key, int x, int y ){;
switch ( key ) {
	case GLUT_KEY_UP:		
		ooffsety+=changeStep;
		break;
	case GLUT_KEY_DOWN:
		ooffsety-=changeStep;
		break;
	case GLUT_KEY_LEFT:
		ooffsetx-=changeStep;
		break;
	case GLUT_KEY_RIGHT:
		ooffsetx+=changeStep;
		break;
	case GLUT_KEY_PAGE_UP:
		scale=scale+1;
		break;
	case GLUT_KEY_PAGE_DOWN:
		scale=scale-1;
		break;
       }
       glutPostRedisplay();
}



void changDisplayMode2D(){
	model =(model+1)%3;

	if (model ==0) {
		printf("Localized result\n");
	}
	else if (model ==1) {
		printf("Original topology\n");
	}
	else if (model ==2 ) {
		printf("Transformed result\n");
	}
}

void displayOriginalTopology()
{ 
	for(int i=0; i < globalField->nSensors; i ++) {
		Sensor *s = globalField->sensorPool + i;
		if( s-> notchDegree > globalField->edgeLandmarks.size() * 0.1 ) {
			glEnable(GL_POINT_SMOOTH);
			double colorredTemp= s->notchDegree/20;
			if ( colorredTemp > 1)
				colorredTemp =1;
			glColor3f(colorredTemp,0,0);
			glPointSize(4);
			glBegin(GL_POINTS);
			glVertex2f( (s->location.x+ooffsetx)*displayScale,(s->location.y+ooffsety)*displayScale);
			glEnd();
		}
		if(s->initLandmark == true) 
		{
			glDisable(GL_POINT_SMOOTH);
			glColor3f(0,0,1);
			glPointSize(4);
			glBegin(GL_POINTS);
			glVertex2f((s->location.x+ooffsetx)*displayScale,(s->location.y+ooffsety)*displayScale);
			glEnd();
		}
		else
			if(s->onEdge == true) {
				glEnable(GL_POINT_SMOOTH);
				glPointSize(4);
				glColor3f(0,1,0);
				glBegin(GL_POINTS);
				glVertex2f((s->location.x+ooffsetx)*displayScale,(s->location.y+ooffsety)*displayScale);
				glEnd();
			}
			else
				if(1)
				{
					glEnable(GL_POINT_SMOOTH);
					glPointSize(1);
					glColor3f(1,1,1);
					glBegin(GL_POINTS);
					glVertex2f((s->location.x+ooffsetx)*displayScale,(s->location.y+ooffsety)*displayScale);
					glEnd();

					glEnable(GL_POINT_SMOOTH);
					glPointSize(3);
					glColor3f(0.9,0.9,0.9);
					glBegin(GL_POINTS);
					glVertex2f((s->location.x+ooffsetx)*displayScale,(s->location.y+ooffsety)*displayScale);
					glEnd();
				}
	}
}

void displayLocalizedTopology()
{
	for(int i=0; i < globalField->nSensors; i ++) {
		Sensor *s = globalField->sensorPool + i;
		if( s-> notchDegree > globalField->edgeLandmarks.size()*0.1 ) {
			glEnable(GL_POINT_SMOOTH);
			double colorredTemp= s->notchDegree/20;
			if ( colorredTemp > 1)
				colorredTemp =1;
			glColor3f(colorredTemp,0,0);
			glPointSize(4);
			glBegin(GL_POINTS);
			glVertex2f( (s->localizedPosition.x+ooffsetx),s->localizedPosition.y+ooffsety);
			glEnd();
		}
		if(s->initLandmark == true) 
		{
			glDisable(GL_POINT_SMOOTH);
			glColor3f(0,0,1);
			glPointSize(4);
			glBegin(GL_POINTS);
			glVertex2f(s->localizedPosition.x+ooffsetx,s->localizedPosition.y+ooffsety);
			glEnd();
		}
		else
			if(s->onEdge == true) {
				glEnable(GL_POINT_SMOOTH);
				glPointSize(4);
				glColor3f(0,1,0);
				glBegin(GL_POINTS);
				glVertex2f(s->localizedPosition.x+ooffsetx,s->localizedPosition.y+ooffsety);
				glEnd();
			}
			else
				if(1)
				{
					glEnable(GL_POINT_SMOOTH);
					glPointSize(1);
					glColor3f(1,1,1);
					glBegin(GL_POINTS);
					glVertex2f(s->localizedPosition.x+ooffsetx,s->localizedPosition.y+ooffsety);
					glEnd();
					glEnable(GL_POINT_SMOOTH);
					
					glPointSize(3);
					glColor3f(0.9,0.9,0.9);
					glBegin(GL_POINTS);
					glVertex2f(s->localizedPosition.x+ooffsetx,s->localizedPosition.y+ooffsety);
					glEnd();

					
				}
	}
}

void displayTransformedTopology()
{

	for(int i=0; i < globalField->nSensors; i ++) {
		Sensor *s = globalField->sensorPool + i;
		if( s-> notchDegree > globalField->edgeLandmarks.size()*0.1 ) {
			double colorredTemp= s->notchDegree/20;
			if ( colorredTemp > 1)
				colorredTemp =1;
			glColor3f(1,0,0);
			glPointSize(4);
			glBegin(GL_POINTS);
			glVertex2f( (s->finalLocalization.x+ooffsetx)*displayScale,(s->finalLocalization.y+ooffsety)*displayScale);
			glEnd();
		}
		if(s->initLandmark == true) 
		{
			glColor3f(1,0,1);
			glPointSize(4);
			glBegin(GL_POINTS);
			glVertex2f((s->finalLocalization.x+ooffsetx)*displayScale,(s->finalLocalization.y+ooffsety)*displayScale);
			glEnd();
		}
		else
			if(s->onEdge == true) {
				glPointSize(4);
				glColor3f(0,1,0);
				glBegin(GL_POINTS);
				glVertex2f((s->finalLocalization.x+ooffsetx)*displayScale,(s->finalLocalization.y+ooffsety)*displayScale);
				glEnd();
			}
			else
				{
					glPointSize(3);
					glColor3f(0,0,0);
					glBegin(GL_POINTS);
					glVertex2f((s->finalLocalization.x+ooffsetx)*displayScale,(s->finalLocalization.y+ooffsety)*displayScale);
					glEnd();
				}
	}
}