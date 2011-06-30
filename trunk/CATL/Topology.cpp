#include "Topology.h"
#include <math.h>		// For math routines (such as sqrt & trig).
#include <stdio.h>
#include <GL/glut.h>	// OpenGL Graphics Utility Library
#include "common.h"
#include "assert.h"
#include <ctype.h>

#include  "sensor.h"

float xrange = 120;
float yrange = 200;
float zrange = 50;


vector<Point> sensors;
int nSensors = 30000;


// The next global variable controls the animation's state and speed.
float RotateAngle = 0.0f;		// Angle in degrees of rotation around y-axis
float Azimuth = 0.0;			// Rotated up or down by this amount
float scale = 15;
float scalestepsize = 0.3f;
float AngleStepSize = 3.0f;		// Step three degrees at a time
const float AngleStepMax = 10.0f;
const float AngleStepMin = 0.1f;

// The next global variable controls the animation's state and speed.
float CurrentAngle = 0.0f;			// Angle in degrees
float AnimateStep = 0.5f;			// Rotation step per update


int WireFrameOn = 1;			// == 1 for wire frame mode


int displayMode=0;
// glutKeyboardFunc is called below to set this function to handle
//		all "normal" key presses.
void myKeyboardFunc( unsigned char key, int x, int y )// what is the use of this function? 
{
	switch ( key ) {
	case 'w':// in the main loop, every time input the key 'w', it will change, for the parameters WireFramOn is changed, 
		WireFrameOn = 1-WireFrameOn;
		if ( WireFrameOn ) {
			glPolygonMode ( GL_FRONT_AND_BACK, GL_LINE );		// Just show wireframes
		}
		else {
			glPolygonMode ( GL_FRONT_AND_BACK, GL_FILL );		// Show solid polygons
		}// the glpolygonmode : wire solid and point ; front and back views,
		glutPostRedisplay();// window redisplayed,
		break;
	case 'R':
		AngleStepSize *= 1.5;
		if (AngleStepSize>AngleStepMax ) {
			AngleStepSize = AngleStepMax;// what is the use of these parameters, 
		}
		break;
	case 'r':
		AngleStepSize /= 1.5;
		if (AngleStepSize<AngleStepMin ) {
			AngleStepSize = AngleStepMin;
		}
		break;
	case 'n':
		changeDisplayMode();
		break;
	case 27:	// Escape key
		exit(1);
	}
}


////////////////////////////////////////////////////////////////////////// the display functions need to be changed
//I need to display the nodes 
void mySpecialKeyFunc( int key, int x, int y )// different views, with the arrow keys to control the angles to see the topology 
{
	switch ( key ) {
	case GLUT_KEY_UP:		
		Azimuth += AngleStepSize;
		if ( Azimuth>180.0f ) {
			Azimuth -= 360.0f;
		}
		break;
	case GLUT_KEY_DOWN:
		Azimuth -= AngleStepSize;
		if ( Azimuth < -180.0f ) {
			Azimuth += 360.0f;
		}
		break;
	case GLUT_KEY_LEFT:
		RotateAngle += AngleStepSize;
		if ( RotateAngle > 180.0f ) {
			RotateAngle -= 360.0f;
		}
		break;
	case GLUT_KEY_RIGHT:
		RotateAngle -= AngleStepSize;
		if ( RotateAngle < -180.0f ) {
			RotateAngle += 360.0f;
		}
		break;
	case GLUT_KEY_PAGE_UP:
		scale += scalestepsize;
		if(scale > 50) {
			scale = 50;
		}
		break;
	case GLUT_KEY_PAGE_DOWN:
		scale -= scalestepsize;
		if(scale < 0) {
			scale = 0;
		}
		break;
	}
	glutPostRedisplay();

}

/*
 * drawScene() handles the animation and the redrawing of the
 *		graphics window contents.
 */
void drawScene(void)
{
	if(topologyIndex==0)// 8topology
	drawPoint();
	else
		if(topologyIndex==1)
               drawPointOfSmile();
		else
			if(topologyIndex==2)
				drawPointOfS();

			else
			if(topologyIndex==3)
				drawPointOfTorus();
	
}

// Initialize OpenGL's rendering modes
void initRendering()
{
    glEnable( GL_DEPTH_TEST );	// Depth testing must be turned on
	glEnable( GL_NORMALIZE );
	glEnable( GL_BLEND );
	glPointSize(2);//change the size of the node sensors, 
	glPolygonMode ( GL_FRONT_AND_BACK, GL_LINE );
	// from the different views, 
	// Just show wireframes at first
}

// Called when the window is resized
//		w, h - width and height of the window in pixels.
// how can we tell the window is resized?
void resizeWindow(int w, int h)
{
	float aspectRatio;
	glViewport( 0, 0, w, h );	// View port uses whole window
	h = (h == 0) ? 1 : h;
	aspectRatio = (float)w/(float)h;

	// Set up the projection view matrix
    glMatrixMode( GL_PROJECTION );// this is projection, 
    glLoadIdentity();
    gluPerspective( 60.0, aspectRatio, 1.0, 30.0 );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	// Move system 8 units away to be able to view from the origin.
	glTranslatef(0.0, 0.0, -8.0);

	// Tilt system 15 degrees downward in order to view from above 
	//   the xy-plane.
	glRotatef(15.0, 1.0,0.0,0.0);	 

}



void initSensors(const char* topoFile)
{
	sensors.clear();

	if(topoFile) {
		FILE *fp;
		fp = fopen(topoFile, "r");
		char line[128];
		if (!fp)  {
			fprintf(stderr, "no topo file %s found\n", topoFile);
			exit(0);
		}
		do {
			memset(line, 0, sizeof(line));
			fgets(line, sizeof(line), fp);
			if(!isspace(*line) && (*line)) {
				Point s;
				sscanf(line, "%lf %lf %lf", &s.x, &s.y, &s.z);
				sensors.push_back(Point(s.x,s.y,s.z));
			}
		}while(!feof(fp));
		fclose(fp);
	}
}
///the initial is redundant

void drawPoint()
{
	switch(displayMode)
	{
	case 0:
		showOriginalOf8();
	break;
	case 1:
		showLolizedOf8();
		break;
	case 2:
	     showTrasferedOf8();
		break;
	}
}


void changeDisplayMode(){
	 displayMode=(displayMode+1)%3;
}






void drawPointOfSmile()
{
	if(displayMode==0)
	{
		showOriginalOfSmile();
	}
	if(displayMode==2)
	{
		showTrasferedOfSmile();
	}
}


void drawPointOfS()
{
	if(displayMode==0)
	{
		showOriginalOfS();
	}
	if(displayMode==1)
	{
		showTrasferedOfS();
	}
}

void showOriginalOf8()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// clear the color and the depth, 
	// Rotate the image
	glMatrixMode( GL_MODELVIEW );				// what matrix are you changing , there are three matrix in all, the view,projection and the texture,
	glLoadIdentity();							
	glTranslatef( 0, 0, -25);				// at which view are you looking at the points,
	//////////////////////////////////////////////////////////////////////////here add the parameter sent to this program,
	glScalef(scale/X_RANGE,scale/Y_RANGE,scale/Z_RANGE);// scale,
	glRotatef( RotateAngle, 0.0, 1.0, 0.0 );		// Rotate around y-axis
	glRotatef( Azimuth, 1.0, 0.0, 0.0 );			// Set Azimuth angle
	//rotate around the vector I just wrote.

	glPushMatrix();// push the matrix, prevent the change of the original point, 
	glTranslatef( -xrange/2, -yrange/2, -zrange/2 );
	//	glRotatef( -90.0, 1.0, 0.0, 0.0 );
	glColor4f( 1.0, 0.2, 0.2, 0.3 );					// Reddish color the last one is about the transparency 
	glBlendFunc(GL_SRC_ALPHA,GL_ONE); 
	glDisable( GL_DEPTH_TEST );

	glBegin( GL_QUAD_STRIP );//
	glVertex3f( 0.0, 0.0, 0.0 );
	glVertex3f( 0.0, 0.0, zrange );
	glVertex3f( 0.0, yrange, 0.0 );
	glVertex3f( 0.0, yrange, zrange );
	glVertex3f( xrange, yrange, 0.0 );
	glVertex3f( xrange, yrange, zrange );
	glVertex3f( xrange, 0.0, 0.0 );
	glVertex3f( xrange, 0.0, zrange );
	glVertex3f( 0.0, 0.0, 0.0 );
	glVertex3f( 0.0, 0.0, zrange );
	glEnd();// these form a closed graph~
	// a cube does have four 8 points, but we need to close it, with the specified order, 

	glColor4f( 0.2, 1.0, 0.2, 0.3 );					// Greenish color
	glBegin( GL_QUAD_STRIP );
	glVertex3f( xrange/3, yrange/5, 0.0 );
	glVertex3f( xrange/3, yrange/5, zrange );
	glVertex3f( xrange/3, yrange/5*2, 0.0 );
	glVertex3f( xrange/3, yrange/5*2, zrange );
	glVertex3f( xrange/3*2, yrange/5*2, 0.0 );
	glVertex3f( xrange/3*2, yrange/5*2, zrange );
	glVertex3f( xrange/3*2, yrange/5, 0.0 );
	glVertex3f( xrange/3*2, yrange/5, zrange );
	glVertex3f( xrange/3, yrange/5, 0.0 );
	glVertex3f( xrange/3, yrange/5, zrange );
	glEnd();
	glBegin( GL_QUAD_STRIP );
	glVertex3f( xrange/3, yrange/5*3, 0.0 );
	glVertex3f( xrange/3, yrange/5*3, zrange );
	glVertex3f( xrange/3, yrange/5*4, 0.0 );
	glVertex3f( xrange/3, yrange/5*4, zrange );
	glVertex3f( xrange/3*2, yrange/5*4, 0.0 );
	glVertex3f( xrange/3*2, yrange/5*4, zrange );
	glVertex3f( xrange/3*2, yrange/5*3, 0.0 );
	glVertex3f( xrange/3*2, yrange/5*3, zrange );
	glVertex3f( xrange/3, yrange/5*3, 0.0 );
	glVertex3f( xrange/3, yrange/5*3, zrange );
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glColor3f( 1.0, 1.0, 1.0 );
	glTranslatef( -xrange/2, -yrange/2, -zrange/2 );
	for(int i = 0; i < globalField->nSensors; i++)
	{   
		Sensor *s =globalField->sensorPool+i;	 
		if(s->seed==true)
		{
			glColor3f( 1.0, 0.0, 0.0 );
			glPointSize(5);
			glBegin( GL_POINTS );
			glVertex3f(s->location.x,s->location.y,s->location.z);
			glEnd();
		}
		else
			if(s->onEdge==true&&0)
		 {
			 glColor3f( 0.0, 1.0, 0.0 );
			 glPointSize(2);
			 glBegin( GL_POINTS );
			 glVertex3f(s->location.x,s->location.y,s->location.z);
			 glEnd();
		 }
			else
				if(s->initLandmark==true)
				{
					glColor3f( 0.0, 1.0, 0.0 );
					glPointSize(3);
					glBegin( GL_POINTS );
					glVertex3f(s->location.x,s->location.y,s->location.z);
					glEnd();
				}
				else
					if(s->notchDegree>notchThreshold)
					{  
						glColor3f( 1.0, 0.0, 0.0 );
						glPointSize(2);
						glBegin( GL_POINTS );
						glVertex3f(s->location.x,s->location.y,s->location.z);
						glEnd();

					}
					else
					{
						glColor3f( 1.0, 1.0, 1.0 );
						glPointSize(2);
						glBegin( GL_POINTS );
						glVertex3f(s->location.x,s->location.y,s->location.z);
						glEnd();
					}

	}
	glPopMatrix();
	glFlush();
	glutSwapBuffers();
}
void showLolizedOf8()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// clear the color and the depth, 
	// Rotate the image
	glMatrixMode( GL_MODELVIEW );				// what matrix are you changing , there are three matrix in all, the view,projection and the texture,
	glLoadIdentity();							
	glTranslatef( 0, 0, -7);				// at which view are you looking at the points,
	//////////////////////////////////////////////////////////////////////////here add the parameter sent to this program,
	glScalef(scale/X_RANGE,scale/Y_RANGE,scale/Z_RANGE);// scale,
	glRotatef( RotateAngle, 0.0, 1.0, 0.0 );		// Rotate around y-axis
	glRotatef( Azimuth, 1.0, 0.0, 0.0 );	


	glPushMatrix();
	glColor3f( 1.0, 1.0, 1.0 );
	glTranslatef( -xrange/10, -yrange/10, -zrange/10 );

	for(int i =0;i<globalField->nSensors;i++){
		Sensor *s=globalField->sensorPool+i;
		if(s->localizedPosition!=INVALID_POINT)
		{
			if(s->seed==true)
			{
				glColor3f( 1.0, 0, 0 );
				glPointSize(5);
				glBegin( GL_POINTS );
				glVertex3f(s->localizedPosition.x,s->localizedPosition.y,s->localizedPosition.z);
				glEnd();
			}
			else
				if(s->onEdge==true){
					glColor3f( 0.0, 1, 0 );
					glPointSize(2);
					glBegin( GL_POINTS );
					glVertex3f(s->localizedPosition.x,s->localizedPosition.y,s->localizedPosition.z);
					glEnd();
				}
				else if(s->initLandmark==true){
					glColor3f( 1.0, 1, 0 );
					glPointSize(3);
					glBegin( GL_POINTS );
					glVertex3f(s->localizedPosition.x,s->localizedPosition.y,s->localizedPosition.z);
					glEnd();
				}
				else{
					glColor3f( 1.0, 1, 1 );
					glPointSize(2);
					glBegin( GL_POINTS );
					glVertex3f(s->localizedPosition.x,s->localizedPosition.y,s->localizedPosition.z);
					glEnd();
				}
		}
	}
	glPopMatrix();
	glFlush();
	glutSwapBuffers();
}
void showTrasferedOf8()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// clear the color and the depth, 
	// Rotate the image
	glMatrixMode( GL_MODELVIEW );				// what matrix are you changing , there are three matrix in all, the view,projection and the texture,
	glLoadIdentity();							
	glTranslatef( 0, 0, -25);				// at which view are you looking at the points,
	//////////////////////////////////////////////////////////////////////////here add the parameter sent to this program,
	glScalef(scale/X_RANGE,scale/Y_RANGE,scale/Z_RANGE);// scale,
	glRotatef( RotateAngle, 0.0, 1.0, 0.0 );		// Rotate around y-axis
	glRotatef( Azimuth, 1.0, 0.0, 0.0 );			// Set Azimuth angle
	//rotate around the vector I just wrote.

	glPushMatrix();// push the matrix, prevent the change of the original point, 
	glTranslatef( -xrange/2, -yrange/2, -zrange/2 );
	//	glRotatef( -90.0, 1.0, 0.0, 0.0 );
	glColor4f( 1.0, 0.2, 0.2, 0.3 );					// Reddish color the last one is about the transparency 
	glBlendFunc(GL_SRC_ALPHA,GL_ONE); 
	glDisable( GL_DEPTH_TEST );

	glBegin( GL_QUAD_STRIP );//
	glVertex3f( 0.0, 0.0, 0.0 );
	glVertex3f( 0.0, 0.0, zrange );
	glVertex3f( 0.0, yrange, 0.0 );
	glVertex3f( 0.0, yrange, zrange );
	glVertex3f( xrange, yrange, 0.0 );
	glVertex3f( xrange, yrange, zrange );
	glVertex3f( xrange, 0.0, 0.0 );
	glVertex3f( xrange, 0.0, zrange );
	glVertex3f( 0.0, 0.0, 0.0 );
	glVertex3f( 0.0, 0.0, zrange );
	glEnd();// these form a closed graph~
	// a cube does have four 8 points, but we need to close it, with the specified order, 

	glColor4f( 0.2, 1.0, 0.2, 0.3 );					// Greenish color
	glBegin( GL_QUAD_STRIP );
	glVertex3f( xrange/3, yrange/5, 0.0 );
	glVertex3f( xrange/3, yrange/5, zrange );
	glVertex3f( xrange/3, yrange/5*2, 0.0 );
	glVertex3f( xrange/3, yrange/5*2, zrange );
	glVertex3f( xrange/3*2, yrange/5*2, 0.0 );
	glVertex3f( xrange/3*2, yrange/5*2, zrange );
	glVertex3f( xrange/3*2, yrange/5, 0.0 );
	glVertex3f( xrange/3*2, yrange/5, zrange );
	glVertex3f( xrange/3, yrange/5, 0.0 );
	glVertex3f( xrange/3, yrange/5, zrange );
	glEnd();
	glBegin( GL_QUAD_STRIP );
	glVertex3f( xrange/3, yrange/5*3, 0.0 );
	glVertex3f( xrange/3, yrange/5*3, zrange );
	glVertex3f( xrange/3, yrange/5*4, 0.0 );
	glVertex3f( xrange/3, yrange/5*4, zrange );
	glVertex3f( xrange/3*2, yrange/5*4, 0.0 );
	glVertex3f( xrange/3*2, yrange/5*4, zrange );
	glVertex3f( xrange/3*2, yrange/5*3, 0.0 );
	glVertex3f( xrange/3*2, yrange/5*3, zrange );
	glVertex3f( xrange/3, yrange/5*3, 0.0 );
	glVertex3f( xrange/3, yrange/5*3, zrange );
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glColor3f( 1.0, 1.0, 1.0 );
	glTranslatef( -xrange/2, -yrange/2, -zrange/2 );

	for(int i =0;i<globalField->nSensors;i++){
		Sensor *s=globalField->sensorPool+i;
		if(s->localizedPosition!=INVALID_POINT)
		{
			if(s->seed==true)
			{
				glColor3f( 1.0, 0, 0 );
				glPointSize(5);
				glBegin( GL_POINTS );
				glVertex3f(s->finalLocalization.x,s->finalLocalization.y,s->finalLocalization.z);
				glEnd();
			}
			else
				if(s->onEdge==true){
					glColor3f( 0.0, 1, 0 );
					glPointSize(2);
					glBegin( GL_POINTS );
					glVertex3f(s->finalLocalization.x,s->finalLocalization.y,s->finalLocalization.z);
					glEnd();
				}
				else if(s->initLandmark==true){
					glColor3f( 1.0, 1, 0 );
					glPointSize(3);
					glBegin( GL_POINTS );
					glVertex3f(s->finalLocalization.x,s->finalLocalization.y,s->finalLocalization.z);
					glEnd();
				}
				else{
					glColor3f( 1.0, 1, 1 );
					glPointSize(2);
					glBegin( GL_POINTS );
					glVertex3f(s->finalLocalization.x,s->finalLocalization.y,s->finalLocalization.z);
					glEnd();
				}
		}
	}
	glPopMatrix();
	glFlush();
	glutSwapBuffers();
}

void showOriginalOfSmile()
{
	GLUquadricObj* myReusableQuadric = NULL;
	xrange = 100;
	xrange = 100;
	yrange = 100;
	zrange = 100;
	float radius = xrange/2;
	float H = 1.8;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if ( ! myReusableQuadric ) {
		myReusableQuadric = gluNewQuadric();  
		// Should (but don't) check if pointer is still null --- to catch memory allocation errors.
		gluQuadricNormals( myReusableQuadric, GL_TRUE );
	}

	// Rotate the image
	glMatrixMode( GL_MODELVIEW );				// Current matrix affects objects positions
	glLoadIdentity();							// Initialize to the identity
	glTranslatef( 0.0, 0.0, -20.0 );				// Translate  from origin (in front of viewer)
	glScalef(scale/xrange,scale/yrange,scale/zrange);
	glRotatef( RotateAngle, 0.0, 1.0, 0.0 );		// Rotate around y-axis
	glRotatef( Azimuth, 1.0, 0.0, 0.0 );			// Set Azimuth angle
	//Draw face
	glPushMatrix();
	//	glTranslatef( -X_RANGE/2, -Y_RANGE/2, -Z_RANGE/2 );
	glColor4f( 1.0, 0.2, 0.2, 0.0 );					// Reddish color
	glBlendFunc(GL_SRC_ALPHA,GL_ONE); 
	glDisable( GL_DEPTH_TEST );
	if(WireFrameOn)
		glutWireSphere(radius, 20, 20);
	else
		glutSolidSphere(radius, 20, 20);
	glPopMatrix();
	//Draw left eye
	glPushMatrix();
	glColor4f( 0.2, 1.0, 0.2, 0.0 );					//Greenish color
	glTranslatef( -radius/3, radius/3, -radius*H/2 );
	gluCylinder(myReusableQuadric, radius/5, radius/5, radius*H, 20, 20);
	glPopMatrix();
	//Draw right eye
	glPushMatrix();
	glTranslatef( radius/3, radius/3, -radius*H/2 );
	gluCylinder(myReusableQuadric, radius/5, radius/5, radius*H, 20, 20);
	glPopMatrix();
	//Draw glasses
	glPushMatrix();
	glColor4f( 0.2, 1.0, 0.2, 0.0 );
	glTranslatef(radius/3, radius/3, radius*H/2);
	gluDisk( myReusableQuadric, 0.0, radius/5, 20, 20 );
	glPopMatrix();

	glPushMatrix();
	glTranslatef(radius/3, radius/3, -radius*H/2);
	gluDisk( myReusableQuadric, 0.0, radius/5, 20, 20 );
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-radius/3, radius/3, radius*H/2);
	gluDisk( myReusableQuadric, 0.0, radius/5, 20, 20 );
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-radius/3, radius/3, -radius*H/2);
	gluDisk( myReusableQuadric, 0.0, radius/5, 20, 20 );
	glPopMatrix();
	//Draw mouth
	glPushMatrix();
	glColor4f( 0.2, 1.0, 0.2, 0.0 );
	glTranslatef(-xrange/3/2, -yrange/6/2-radius/3, -zrange*0.9/2);
	glBegin( GL_QUAD_STRIP );
	glVertex3f( 0.0, 0.0, 0.0 );
	glVertex3f( xrange/3, 0.0, 0.0 );
	glVertex3f( 0.0, yrange/6, 0.0 );
	glVertex3f( xrange/3, yrange/6, 0.0 );

	glVertex3f( 0.0, yrange/6, zrange*0.9 );
	glVertex3f( xrange/3, yrange/6, zrange*0.9 );
	glVertex3f( 0.0, 0.0, zrange*0.9 );
	glVertex3f( xrange/3, 0.0, zrange*0.9 );
	glVertex3f( 0.0, 0.0, 0.0 );
	glVertex3f( xrange/3, 0.0, 0.0 );
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glColor3f( 1.0, 1.0, 1.0 );
	//	glTranslatef( -X_RANGE/2, -Y_RANGE/2, -Z_RANGE*0.75/2 );


   for(int i =0;i<globalField->nSensors;i++)
   {
	   Sensor *s=globalField->sensorPool+i;

	   if(s->seed==true)
	   {
		   glPointSize(9);
		   glColor3f(1,0,0);
		   glBegin( GL_POINTS );
		   glVertex3f(s->location.x-X_RANGE/2, s->location.y-Y_RANGE/2, s->location.z-Z_RANGE/2);
		   glEnd();
	   }
	   if(s->notchDegree>notchThreshold)
	   {
		 glPointSize(2);
		 glColor3f(1,0,0);
         glBegin( GL_POINTS );
		 glVertex3f(s->location.x-X_RANGE/2, s->location.y-Y_RANGE/2, s->location.z-Z_RANGE/2);
		 glEnd();
	   }
	   else{
		   glPointSize(2);
		   glColor3f(1,1,1);
		   glBegin( GL_POINTS );
		   glVertex3f(s->location.x-X_RANGE/2, s->location.y-Y_RANGE/2, s->location.z-Z_RANGE/2);
		   glEnd();
	   }

   }



	glPopMatrix();

	// Flush the pipeline, swap the buffers
	glFlush();
	glutSwapBuffers();
}
void showLolizedOfSmile()
{
	// null at this time
}
void showTrasferedOfSmile()
{
	xrange = 100;
	xrange = 100;
	yrange = 100;
	zrange = 100;
	float radius = xrange/2;
	float H = 1.8;
	GLUquadricObj* myReusableQuadric = NULL;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if ( ! myReusableQuadric ) {
		myReusableQuadric = gluNewQuadric();  
		// Should (but don't) check if pointer is still null --- to catch memory allocation errors.
		gluQuadricNormals( myReusableQuadric, GL_TRUE );
	}

	// Rotate the image
	glMatrixMode( GL_MODELVIEW );				// Current matrix affects objects positions
	glLoadIdentity();							// Initialize to the identity
	glTranslatef( 0.0, 0.0, -20.0 );				// Translate  from origin (in front of viewer)
	glScalef(scale/xrange,scale/yrange,scale/zrange);
	glRotatef( RotateAngle, 0.0, 1.0, 0.0 );		// Rotate around y-axis
	glRotatef( Azimuth, 1.0, 0.0, 0.0 );			// Set Azimuth angle
	//Draw face
	glPushMatrix();
	//	glTranslatef( -X_RANGE/2, -Y_RANGE/2, -Z_RANGE/2 );
	glColor4f( 1.0, 0.2, 0.2, 0.0 );					// Reddish color
	glBlendFunc(GL_SRC_ALPHA,GL_ONE); 
	glDisable( GL_DEPTH_TEST );
	if(WireFrameOn)
		glutWireSphere(radius, 20, 20);
	else
		glutSolidSphere(radius, 20, 20);
	glPopMatrix();
	//Draw left eye
	glPushMatrix();
	glColor4f( 0.2, 1.0, 0.2, 0.0 );					//Greenish color
	glTranslatef( -radius/3, radius/3, -radius*H/2 );
	gluCylinder(myReusableQuadric, radius/5, radius/5, radius*H, 20, 20);
	glPopMatrix();
	//Draw right eye
	glPushMatrix();
	glTranslatef( radius/3, radius/3, -radius*H/2 );
	gluCylinder(myReusableQuadric, radius/5, radius/5, radius*H, 20, 20);
	glPopMatrix();
	//Draw glasses
	glPushMatrix();
	glColor4f( 0.2, 1.0, 0.2, 0.0 );
	glTranslatef(radius/3, radius/3, radius*H/2);
	gluDisk( myReusableQuadric, 0.0, radius/5, 20, 20 );
	glPopMatrix();

	glPushMatrix();
	glTranslatef(radius/3, radius/3, -radius*H/2);
	gluDisk( myReusableQuadric, 0.0, radius/5, 20, 20 );
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-radius/3, radius/3, radius*H/2);
	gluDisk( myReusableQuadric, 0.0, radius/5, 20, 20 );
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-radius/3, radius/3, -radius*H/2);
	gluDisk( myReusableQuadric, 0.0, radius/5, 20, 20 );
	glPopMatrix();
	//Draw mouth
	glPushMatrix();
	glColor4f( 0.2, 1.0, 0.2, 0.0 );
	glTranslatef(-xrange/3/2, -yrange/6/2-radius/3, -zrange*0.9/2);
	glBegin( GL_QUAD_STRIP );
	glVertex3f( 0.0, 0.0, 0.0 );
	glVertex3f( xrange/3, 0.0, 0.0 );
	glVertex3f( 0.0, yrange/6, 0.0 );
	glVertex3f( xrange/3, yrange/6, 0.0 );

	glVertex3f( 0.0, yrange/6, zrange*0.9 );
	glVertex3f( xrange/3, yrange/6, zrange*0.9 );
	glVertex3f( 0.0, 0.0, zrange*0.9 );
	glVertex3f( xrange/3, 0.0, zrange*0.9 );
	glVertex3f( 0.0, 0.0, 0.0 );
	glVertex3f( xrange/3, 0.0, 0.0 );
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glColor3f( 1.0, 1.0, 1.0 );
	//	glTranslatef( -X_RANGE/2, -Y_RANGE/2, -Z_RANGE*0.75/2 );
	glBegin( GL_POINTS );

	for(int i = 0; i < globalField->nSensors; i++)
	{
		Sensor *s=globalField->sensorPool+i;
		glVertex3f(s->finalLocalization.x-X_RANGE/2, s->finalLocalization.y-Y_RANGE/2, s->finalLocalization.z-Z_RANGE/2);
	}
	glEnd();
	glPopMatrix();

	// Flush the pipeline, swap the buffers
	glFlush();
	glutSwapBuffers();
}

void showOriginalOfS()
{
	float xrange = 200;
	float yrange = 200;
	float zrange = 100;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	scale=10;
	// Rotate the image
	glMatrixMode( GL_MODELVIEW );				// Current matrix affects objects positions
	glLoadIdentity();							// Initialize to the identity
	glTranslatef( 0.0, 0.0, -20.0 );				// Translate  from origin (in front of viewer)
	glScalef(scale/X_RANGE,scale/Y_RANGE,scale/Z_RANGE);
	glRotatef( RotateAngle, 0.0, 1.0, 0.0 );		// Rotate around y-axis
	glRotatef( Azimuth, 1.0, 0.0, 0.0 );			// Set Azimuth angle

	glPushMatrix();
	glTranslatef( -xrange/2, -yrange/2, -zrange/2 );
	glColor4f( 1.0, 0.2, 0.2, 0.3 );					// Reddish color
	glBlendFunc(GL_SRC_ALPHA,GL_ONE); 
	glDisable( GL_DEPTH_TEST );

	glBegin( GL_QUAD_STRIP );
	glVertex3f( 0.0, 0.0, 0.0 );
	glVertex3f( 0.0, 0.0, zrange );
	glVertex3f( 0.0, yrange/5, 0.0 );
	glVertex3f( 0.0, yrange/5, zrange );

	glVertex3f( xrange/5*3, yrange/5, 0.0 );
	glVertex3f( xrange/5*3, yrange/5, zrange );
	glVertex3f( xrange/5*3, yrange/5*2, 0.0 );
	glVertex3f( xrange/5*3, yrange/5*2, zrange );

	glVertex3f( 0.0, yrange/5*2, 0.0 );
	glVertex3f( 0.0, yrange/5*2, zrange );
	glVertex3f( 0.0, yrange, 0.0 );
	glVertex3f( 0.0, yrange, zrange );

	glVertex3f( xrange, yrange, 0.0 );
	glVertex3f( xrange, yrange, zrange );
	glVertex3f( xrange, yrange/5*4, 0.0 );
	glVertex3f( xrange, yrange/5*4, zrange );

	glVertex3f( xrange/5*2, yrange/5*4, 0.0 );
	glVertex3f( xrange/5*2, yrange/5*4, zrange );
	glVertex3f( xrange/5*2, yrange/5*3, 0.0 );
	glVertex3f( xrange/5*2, yrange/5*3, zrange );

	glVertex3f( xrange, yrange/5*3, 0.0 );
	glVertex3f( xrange, yrange/5*3, zrange );
	glVertex3f( xrange, 0.0, 0.0 );
	glVertex3f( xrange, 0.0, zrange );

	glVertex3f( 0.0, 0.0, 0.0 );
	glVertex3f( 0.0, 0.0, zrange );
	glVertex3f( 0.0, yrange/5, 0.0 );
	glVertex3f( 0.0, yrange/5, zrange );
	glEnd();
	glPopMatrix();


	glPushMatrix();
	glColor3f( 1.0, 1.0, 1.0 );
	glTranslatef( -xrange/2, -yrange/2, -zrange/2 );
	for(int i =0;i<globalField->nSensors;i++)
	{
		Sensor *s=globalField->sensorPool+i;
		if(s->seed==true)
		{
			glPointSize(9);
			glColor3f(1,0,0);
			glBegin(GL_POINTS);
			glVertex3f(s->location.x,s->location.y,s->location.z);
			glEnd();
		}
		else
			if(s->notchDegree>notchThreshold){
				glPointSize(3);
				glColor3f(1,0,0);
				glBegin(GL_POINTS);
				glVertex3f(s->location.x,s->location.y,s->location.z);
				glEnd();
			}
			else
			{
				glPointSize(2);
				glColor3f(1,1,1);
				glBegin(GL_POINTS);
				glVertex3f(s->location.x,s->location.y,s->location.z);
				glEnd();
			}
	}


	glPopMatrix();

	// Flush the pipeline, swap the buffers
	glFlush();
	glutSwapBuffers();
}
void showTrasferedOfS()
{
	float xrange = 200;
	float yrange = 200;
	float zrange = 100;
    scale=10;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// Rotate the image
	glMatrixMode( GL_MODELVIEW );				// Current matrix affects objects positions
	glLoadIdentity();							// Initialize to the identity
	glTranslatef( 0.0, 0.0, -20.0 );				// Translate  from origin (in front of viewer)
	glScalef(scale/X_RANGE,scale/Y_RANGE,scale/Z_RANGE);
	glRotatef( RotateAngle, 0.0, 1.0, 0.0 );		// Rotate around y-axis
	glRotatef( Azimuth, 1.0, 0.0, 0.0 );			// Set Azimuth angle

	glPushMatrix();
	glTranslatef( -xrange/2, -yrange/2, -zrange/2 );
	glColor4f( 1.0, 0.2, 0.2, 0.3 );					// Reddish color
	glBlendFunc(GL_SRC_ALPHA,GL_ONE); 
	glDisable( GL_DEPTH_TEST );

	glBegin( GL_QUAD_STRIP );
	glVertex3f( 0.0, 0.0, 0.0 );
	glVertex3f( 0.0, 0.0, zrange );
	glVertex3f( 0.0, yrange/5, 0.0 );
	glVertex3f( 0.0, yrange/5, zrange );

	glVertex3f( xrange/5*3, yrange/5, 0.0 );
	glVertex3f( xrange/5*3, yrange/5, zrange );
	glVertex3f( xrange/5*3, yrange/5*2, 0.0 );
	glVertex3f( xrange/5*3, yrange/5*2, zrange );

	glVertex3f( 0.0, yrange/5*2, 0.0 );
	glVertex3f( 0.0, yrange/5*2, zrange );
	glVertex3f( 0.0, yrange, 0.0 );
	glVertex3f( 0.0, yrange, zrange );

	glVertex3f( xrange, yrange, 0.0 );
	glVertex3f( xrange, yrange, zrange );
	glVertex3f( xrange, yrange/5*4, 0.0 );
	glVertex3f( xrange, yrange/5*4, zrange );

	glVertex3f( xrange/5*2, yrange/5*4, 0.0 );
	glVertex3f( xrange/5*2, yrange/5*4, zrange );
	glVertex3f( xrange/5*2, yrange/5*3, 0.0 );
	glVertex3f( xrange/5*2, yrange/5*3, zrange );

	glVertex3f( xrange, yrange/5*3, 0.0 );
	glVertex3f( xrange, yrange/5*3, zrange );
	glVertex3f( xrange, 0.0, 0.0 );
	glVertex3f( xrange, 0.0, zrange );

	glVertex3f( 0.0, 0.0, 0.0 );
	glVertex3f( 0.0, 0.0, zrange );
	glVertex3f( 0.0, yrange/5, 0.0 );
	glVertex3f( 0.0, yrange/5, zrange );
	glEnd();
	glPopMatrix();


	glPushMatrix();
	glColor3f( 1.0, 1.0, 1.0 );
	glTranslatef( -xrange/2, -yrange/2, -zrange/2 );
	glBegin( GL_POINTS );
	for(int i = 0; i < globalField->nSensors; i++)
	{
		Sensor *s=globalField->sensorPool+i;
		glVertex3f(s->finalLocalization.x, s->finalLocalization.y, s->finalLocalization.z);
	}
	glEnd();
	glPopMatrix();

	// Flush the pipeline, swap the buffers
	glFlush();
	glutSwapBuffers();
}



void drawPointOfTorus()
{
	if(displayMode==0)
		showOriginalOfTorus();
	 if(displayMode==2)
		showTrasferedOfTorus();
		
}

void putVert(int i, int j) {
	float MajorRadius = 70;
	float MinorRadius = 30;
	int NumWraps = 40;
	int NumPerWrap = 32;
	float wrapFrac = (j%NumPerWrap)/(float)NumPerWrap;
	float phi = PI*2*wrapFrac;
	float theta = PI*2*(i%NumWraps+wrapFrac)/(float)NumWraps;
	float sinphi = sin(phi);
	float cosphi = cos(phi);
	float sintheta = sin(theta);
	float costheta = cos(theta);
	float y = MinorRadius*sinphi;
	float r = MajorRadius + MinorRadius*cosphi;
	float x = sintheta*r;
	float z = costheta*r;
	glNormal3f(sintheta*cosphi, sinphi, costheta*cosphi);
	glVertex3f(x,y,z);

}

void showOriginalOfTorus()
{
int i,j;
GLenum shadeModel = GL_SMOOTH;
GLenum polygonMode = GL_LINE;
GLenum LocalMode = GL_TRUE;
float scale = 8.0f;

float Noemit[4] = {0.0, 0.0, 0.0, 1.0};
float Matspec[4] = {1.0, 1.0, 1.0, 1.0};
float Matnonspec[4] = {0.8, 1.0, 1.0, 1.0};
float Matshiny = 16.0;
float RotX = 90.0f;					// Rotational position around x-axis
float RotY = 0.0f;	
int QuadMode = 1;	
int NumWraps = 40;
int NumPerWrap = 32;
float MajorRadius = 70;
float MinorRadius = 30;
float PNoemit[4] = {0.0, 0.0, 0.0, 1.0};
float PMatspec[4] = {1.0, 1.0, 1.0, 1.0};
float PMatnonspec[4] = {1.0, 0.0, 0.0, 1.0};
float PMatshiny = 10.0;


	// Clear the redering window
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glShadeModel( shadeModel );	// Set the shading to flat or smooth.
	glPolygonMode(GL_FRONT_AND_BACK, polygonMode);	// Set to be "wire" or "solid"
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, LocalMode); 
 


	// Torus Materials
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, Matnonspec);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, Matspec);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, Matshiny);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, Noemit);


	glMatrixMode( GL_MODELVIEW );				// Current matrix affects objects positions
	glLoadIdentity();	
	glTranslatef(0,0,-10);
    glRotatef( RotateAngle, 0.0, 1.0, 0.0 );
    glRotatef( Azimuth, 1.0, 0.0, 0.0 );

	glPushMatrix();		// Save to use again next time.

	
	glRotatef( RotX, 1.0, 0.0, 0.0);
	glRotatef( RotY, 0.0, 1.0, 0.0);
	
	// Draw the torus
	glScalef( scale/X_RANGE, scale/Y_RANGE, scale/Z_RANGE );

	glColor4f( 0.5, 1.0, 0.5, 1.0 );
//	glutWireSphere(X_RANGE/2, 30, 30);

	glBegin( QuadMode==1 ? GL_QUAD_STRIP : GL_TRIANGLE_STRIP );
	for (i=0; i<NumWraps; i++ ) {
		for (j=0; j<NumPerWrap; j++) {
			putVert(i,j);
			putVert(i+1,j);
		}
	}
	putVert(0,0);
	putVert(1,0);
	glEnd();

//	glPopMatrix();
	
//	glPushMatrix();
	// Draw the reference pyramid
	glTranslatef( -MajorRadius-MinorRadius-MajorRadius/10.0, 0.0, 0.0);
	glScalef( scale/4, scale/4, scale/4 );
	glColor4f( 1.0f, 1.0f, 0.0f, 1.0f );

	glBegin(GL_TRIANGLE_STRIP);
	glVertex3f( -0.5, 0.0, sqrt(3.0)*0.5 );
	glVertex3f( -0.5, 0.0, -sqrt(3.0)*0.5 );
	glVertex3f( 1.0, 0.0, 0.0);
	glVertex3f( 0.0, sqrt(2.0), 0.0);
	glVertex3f( -0.5, 0.0, sqrt(3.0)*0.5 );
	glVertex3f( -0.5, 0.0, -sqrt(3.0)*0.5 );
	glEnd();

	glPopMatrix();		// Restore to original matrix as set in resizeWindow()
	//Points material
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, PMatnonspec);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, PMatspec);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, PMatshiny);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, PNoemit);


	/*
	if ( runMode ) {
		RotY += RotIncrementY;
		if ( fabs(RotY)>360.0 ) {
			RotY -= 360.0*((int)(RotY/360.0));
		}
		RotX += RotIncrementX;
		if ( fabs(RotX)>360.0 ) {
			RotX -= 360.0*((int)(RotX/360.0));
		}
	}
	*/
	// Set the orientation.

	glRotatef( RotX, 1.0, 0.0, 0.0);
	glRotatef( RotY, 0.0, 1.0, 0.0);
	glColor4f( 0.0, 0.0, 0.0, 1.0 );

	glScalef( scale/X_RANGE, scale/Y_RANGE, scale/Z_RANGE );


  for(int i =0;i<globalField->nSensors;i++)
  {

	  Sensor *s=globalField->sensorPool+i;
	  if(s->notchDegree>notchThreshold)
	  {
      glPointSize(2);
	  glColor3f(1,0,0);
	  glBegin(GL_POINTS);
	  glVertex3f(s->location.x,s->location.y,s->location.z);
      glEnd();
	  }
	  if(s->seed==true)
	  {
		  glPointSize(5);
		  glColor3f(1,0,0);
		  glBegin(GL_POINTS);
		  glVertex3f(s->location.x,s->location.y,s->location.z);
		  glEnd();
	  }
	  else
	  {
		  glPointSize(2);
		  glColor3f(1,1,1);
		  glBegin(GL_POINTS);
		  glVertex3f(s->location.x,s->location.y,s->location.z);
		  glEnd();
	  }
  }

	
	/*
	//draw a ball
	glPushMatrix();
	glScalef( scale/X_RANGE, scale/Y_RANGE, scale/Z_RANGE );
	glColor4f(0.5, 0.8, 0.0, 1.0);
	glutWireSphere(X_RANGE/2, 20, 20);
	glPopMatrix();
	*/
	// Flush the pipeline, swap the buffers
    glFlush();
    glutSwapBuffers();
   


}


void showTrasferedOfTorus()
{

	int i,j;
GLenum shadeModel = GL_SMOOTH;
GLenum polygonMode = GL_LINE;
GLenum LocalMode = GL_TRUE;
float scale = 8.0f;

float Noemit[4] = {0.0, 0.0, 0.0, 1.0};
float Matspec[4] = {1.0, 1.0, 1.0, 1.0};
float Matnonspec[4] = {0.8, 1.0, 1.0, 1.0};
float Matshiny = 16.0;
float RotX = 90.0f;					// Rotational position around x-axis
float RotY = 0.0f;	
int QuadMode = 1;	
int NumWraps = 40;
int NumPerWrap = 32;
float MajorRadius = 70;
float MinorRadius = 30;
float PNoemit[4] = {0.0, 0.0, 0.0, 1.0};
float PMatspec[4] = {1.0, 1.0, 1.0, 1.0};
float PMatnonspec[4] = {1.0, 0.0, 0.0, 1.0};
float PMatshiny = 10.0;


	// Clear the redering window
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glShadeModel( shadeModel );	// Set the shading to flat or smooth.
	glPolygonMode(GL_FRONT_AND_BACK, polygonMode);	// Set to be "wire" or "solid"
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, LocalMode); 
 


	// Torus Materials
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, Matnonspec);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, Matspec);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, Matshiny);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, Noemit);


	glMatrixMode( GL_MODELVIEW );				// Current matrix affects objects positions
	glLoadIdentity();	
	glTranslatef(0,0,-10);
    glRotatef( RotateAngle, 0.0, 1.0, 0.0 );
    glRotatef( Azimuth, 1.0, 0.0, 0.0 );

	glPushMatrix();		// Save to use again next time.

	
	glRotatef( RotX, 1.0, 0.0, 0.0);
	glRotatef( RotY, 0.0, 1.0, 0.0);
	
	// Draw the torus
	glScalef( scale/X_RANGE, scale/Y_RANGE, scale/Z_RANGE );

	glColor4f( 0.5, 1.0, 0.5, 1.0 );
//	glutWireSphere(X_RANGE/2, 30, 30);

	/*
	glBegin( QuadMode==1 ? GL_QUAD_STRIP : GL_TRIANGLE_STRIP );
	for (i=0; i<NumWraps; i++ ) {
		for (j=0; j<NumPerWrap; j++) {
			putVert(i,j);
			putVert(i+1,j);
		}
	}
	putVert(0,0);
	putVert(1,0);
	glEnd();
*/

//	glPopMatrix();
	
//	glPushMatrix();
	// Draw the reference pyramid
	glTranslatef( -MajorRadius-MinorRadius-MajorRadius/10.0, 0.0, 0.0);
	glScalef( scale/4, scale/4, scale/4 );
	glColor4f( 1.0f, 1.0f, 0.0f, 1.0f );

	glBegin(GL_TRIANGLE_STRIP);
	glVertex3f( -0.5, 0.0, sqrt(3.0)*0.5 );
	glVertex3f( -0.5, 0.0, -sqrt(3.0)*0.5 );
	glVertex3f( 1.0, 0.0, 0.0);
	glVertex3f( 0.0, sqrt(2.0), 0.0);
	glVertex3f( -0.5, 0.0, sqrt(3.0)*0.5 );
	glVertex3f( -0.5, 0.0, -sqrt(3.0)*0.5 );
	glEnd();

	glPopMatrix();		// Restore to original matrix as set in resizeWindow()
	//Points material
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, PMatnonspec);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, PMatspec);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, PMatshiny);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, PNoemit);


	/*
	if ( runMode ) {
		RotY += RotIncrementY;
		if ( fabs(RotY)>360.0 ) {
			RotY -= 360.0*((int)(RotY/360.0));
		}
		RotX += RotIncrementX;
		if ( fabs(RotX)>360.0 ) {
			RotX -= 360.0*((int)(RotX/360.0));
		}
	}
	*/
	// Set the orientation.

	glRotatef( RotX, 1.0, 0.0, 0.0);
	glRotatef( RotY, 0.0, 1.0, 0.0);
	glColor4f( 0.0, 0.0, 0.0, 1.0 );

	glScalef( scale/X_RANGE, scale/Y_RANGE, scale/Z_RANGE );


  for(int i =0;i<globalField->nSensors;i++)
  {

	  Sensor *s=globalField->sensorPool+i;
	  if(s->notchDegree>notchThreshold)
	  {
      glPointSize(2);
	  glColor3f(1,0,0);
	  glBegin(GL_POINTS);
	  glVertex3f(s->finalLocalization.x,s->finalLocalization.y,s->finalLocalization.z);
      glEnd();
	  }
	  if(s->seed==true)
	  {
		  glPointSize(5);
		  glColor3f(1,0,0);
		  glBegin(GL_POINTS);
		  glVertex3f(s->finalLocalization.x,s->finalLocalization.y,s->finalLocalization.z);
		  glEnd();
	  }
	  else
	  {
		  glPointSize(2);
		  glColor3f(1,1,1);
		  glBegin(GL_POINTS);
		  glVertex3f(s->finalLocalization.x,s->finalLocalization.y,s->finalLocalization.z);
		  glEnd();
	  }
  }

	
	/*
	//draw a ball
	glPushMatrix();
	glScalef( scale/X_RANGE, scale/Y_RANGE, scale/Z_RANGE );
	glColor4f(0.5, 0.8, 0.0, 1.0);
	glutWireSphere(X_RANGE/2, 20, 20);
	glPopMatrix();
	*/
	// Flush the pipeline, swap the buffers
    glFlush();
    glutSwapBuffers();
   
}