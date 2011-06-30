#include "geometry3D.h"
#include "random.h"
#include "common.h"

void myKeyboardFunc( unsigned char key, int x, int y );
void drawScene(void);
void initRendering();
void resizeWindow(int w, int h);
void drawPoint();
void initSensors(const char* topoFile);
int display(int argc, char** argv);
void mySpecialKeyFunc( int key, int x, int y );
void changeDisplayMode();

void drawPointOfSmile();


void drawPointOfS();
void drawPointOfTorus();

void putVert(int i, int j);
//void drawGluCylinderWithCaps( double height, double radius, int slices, int stacks );
//void drawGluSlantCylinderWithCaps( double height, double radiusBase, double radiusTop, int slices, int stacks );

// read the function, then let's do this, 
//just a few functions called by this program 

// nothing really big 


void showOriginalOf8();
void showLolizedOf8();
void showTrasferedOf8();

void showOriginalOfSmile();
void showLolizedOfSmile();
void showTrasferedOfSmile();

void showOriginalOfS();
void showLocalizedOfS();
void showTrasferedOfS();

void showOriginalOfTorus();
void showLocalizedOfTorus();
void showTrasferedOfTorus();
