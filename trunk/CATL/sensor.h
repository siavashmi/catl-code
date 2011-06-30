#ifndef SENSOR_H
#define SENSOR_H

#pragma warning(disable: 4786)

#include <iostream>
#include <set>
#include <map>
#include <vector>
#include <algorithm>
#include "math.h"
#include "geometry3D.h"
#include "common.h"

using namespace std;
class Sensor;
class Field;
class pathInfo // path information
{
public:
	pathInfo() {memset(this, 0, sizeof(*this)); nexthop = -1;}

	enum {MAX_QUALITY = 1,};

	int hopcount;
	int nexthop;
	double quality; 
};
typedef map<int, pathInfo > DEST_2_PATH_INFO ;
typedef set<Sensor*>::const_iterator SCITER;

class Sensor
{
public:
	Sensor();
	~Sensor();

	int index;// index of the sensor
	Field *field;

	Point location; //real location
	Point localizedPosition; //localization result
	Point finalLocalization;// transfered localization result

	Point tempLocation;// the temporary localization, used in the spring-mass
    int numContainingSpPaths;// used in spring-mass

	vector<Sensor*> neighbors;//one hop neighbors
	set<Sensor*> twoHopNeighbors; //two hop neighbors
	DEST_2_PATH_INFO spMap; // store the information in the iterative localization
	DEST_2_PATH_INFO areaspMap;// store the information in the spring-mass
	vector<Sensor*> landmarkNeighbors;// the landmark neighbors

	bool isNotchPoint;// is identified as an notch point
	bool isUnderNotch; //is near a notch point, so it will be affected by the notch 
	double notchDegree;// notch degree
	bool onEdge; //edge point
	int nearEdge; //point is near edge or not
	bool initLandmark; // is landmark or not
	bool seed; //is seed or not

	int subtreeSizes[MAXNUM_LEVELS]; //subtree size of different depths (m)


	int broadcastCovered; 
	Sensor* parent;
	set<Sensor*> children;
	set<Sensor*> candidateParents;
	int level;

   
	int nextRound;// in iterative localization
	int lastScheduledRound;// in iterative localization
	
	double oldConfidence;// old confidence
    double localizationConfidence; //confidence of the localization


	void clear();
	void init(int index_, Point location_);// read sensor location and index
	void init(int index_);// read sensor index
	void updateNeighbors();// UBG communication model
	void updateNeighborsQuasiUBG();// quasi_ubg communication model


	void setSeed(); //seed nodes
	void localizationFlood(bool updatePathQuality = true, int hopnum = 0xffff); //flooding function

	void updateSpMap();// update the shortest path map
	void multilateration(); // multiateration 

	Point multilaterationLocalization3D(map<int, pathInfo> &dst, double precise, 
											int multisolver, bool & locationsucceed);// in 3D
	Point multilaterationLocalization2D(map<int, pathInfo> &dst, double precise, 
		int multisolver, bool & locationsucceed);//in 2D


	double localizationError();// calculate the localization error

	double getLandmarkSetConfidence(vector<int> & lms);// estimate the landmarks set confidence
	void selectBestLandmarks(vector<int> & lms, double & confidence);// choose landmarks with more confidence
	void randomlyPickLandmarkSet(const vector<int> & candidates, int numPicks, vector<int> & selection);//randomly choose the landmarks
	void getShortestPath(Sensor* dest, vector<Sensor*> &path); //shortest path
	void getMultiHopNeighborhood(int hopCount, set<Sensor*> & nHopNeighbors); // get the multihop neighbor
	int  getMultiHopNeighborhoodSize(int hopCount);// get the multihop neighbor size
    int shortestPathLength(Sensor* dest);// length of the shortest path
	void edgeNeighborSize();// the edge neighbor size
	void findNotches(char *dumbfile=NULL);// find notch flooding
	vector <int> neighborIndex;//
	bool isNeighbor(int index_);// the two nodes are neighbors to each other
	bool isEdgeLandmark();// is edge landmark
	Point displayPoint;
	void areaFlooding(int hopcount);// area flooding used in spring-mass

	//////////////////////////////////////////////////////////////////////////Dv-Hop
	Point localizedPointDvHop;
	bool seedDvHop;
	Point multiateration2DDvHop(map<int, pathInfo> &dst, double precise);
	Point multiateration3DDvHop(map<int, pathInfo> &dst, double precise);
};


extern Field *globalField;
extern double THRESHOLD;

#endif

