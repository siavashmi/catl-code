#ifndef DISPLAY3D_H_
#define DISPLAY3D_H_
// the display codes are from the openGL red book
void myKeyboardFunc3D( unsigned char key, int x, int y );// input from the keyboard
void mySpecialKeyFunc3D( int key, int x, int y );// input from the keyboard
void drawScene3D(void);
void initRendering3D();
void resizeWindow3D(int w, int h);
void changeDisplayMode3D();// choose to display the original topology or the localized result
void drawPointOf83D();
void drawPointOfSmile3D();
void drawPointOfTorus3D();
void drawPointOfHourglass();
void drawPointOfcrossRing();
void putVert3D(int i, int j);// draw the boundary lines in torus
void putVert3D1(int i,int j);// draw the boundary lines in torus
// draw 3D topologies
void showOriginalOf8();
void showLolizedOf8();
void showTrasferedOf8();
void showOriginalOfSmile();
void showLolizedOfSmile();
void showTrasferedOfSmile();
void showOriginalOfTorus();
void showLocalizedOfTorus();
void showTrasferedOfTorus();
void showOriginalOfHourglass();
void showlocalizedOfHourglass();
void showTrasferedOfHourglass();
void showOriginalOfcrossRing();
void showlocalizedOfcrossRing();
void showTrasferedOfcrossRing();
#endif