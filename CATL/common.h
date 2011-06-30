
#ifndef COMMON_H_
#define COMMON_H_

#define PI 3.1415926
#define myabs(a) ((a) > 0 ? (a) : (-(a)))
#define between(a, asmall, abig) ((a) >= (asmall) && (a) <= (abig))
#define EQUAL_DOUBLE_ERROR double(0.0001)
#define DOUBLE_EQUAL(a, b) (myabs((a)-(b)) < EQUAL_DOUBLE_ERROR)
#define DOUBLE_EQUAL_PT(a, b) (myabs((a.x)-(b.x)) < EQUAL_DOUBLE_ERROR && myabs((a.y)-(b.y)) < EQUAL_DOUBLE_ERROR)
#define CEILING(a) ((a>0)?((int)(a)==(a)?(a):((a)+1)):((int)(a)==(a)?((a)+1):(a))) //ceil
#define mymax(a,b)    (((a) > (b)) ? (a) : (b))
#define mymin(a,b)    (((a) < (b)) ? (a) : (b))
#define INVALID_POINT (Point(IMPOSSIBLE_VALUE, IMPOSSIBLE_VALUE, IMPOSSIBLE_VALUE))
#define BIG_DISTANCE (double)0xffffffff
#define IMPOSSIBLE_VALUE double(0xffffff)
#define BIG_VALUE (double)0xffffffff
#define MAXNUM_LEVELS 256//max levels in this code   
extern double COMM_RANGE;//communication range
extern double notchThreshold;// Node beyond this level will be considered as the notch
extern int globalRound;//round in the iterative localization
extern int LANDMARK_SPACING;//Controlling the landmark density
extern int QUASI_UBG;//quasi-ubg communication model
extern double QUASI_UBG_ALPHA;//

extern double X_RANGE;
extern double Y_RANGE;
extern double Z_RANGE;

extern int topologyIndex;// the index of the topology
extern bool threeDimension;//three dimension or two dimension
extern bool connectivityInformation;// Input the connectivity file or the location file
extern int algorithmIndex;//CATL or the dv-hop method
#endif
