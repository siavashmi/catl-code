#ifndef DISPLAY2D_H_
#define DISPLAY2D_H_
// The display codes are from the code in the openGL red book
void myKeyboardFunc2D(unsigned char key, int x, int y);// input key from the keyboard
void mySpecialKeyFunc2D( int key, int x, int y );//input from the keyboard
void drawScene2D(void);// drawing in 2D
void initRendering2D();
void resizeWindow2D(int w, int h);
void  drawPoint2D();
void changDisplayMode2D();// choose to display the original topology or the localized topology
void displayOriginalTopology();// display the original topology
void displayLocalizedTopology();// display the localized result
void displayTransformedTopology();// display the final result
#endif