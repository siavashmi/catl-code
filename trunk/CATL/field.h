#ifndef FIELD_H
#define FIELD_H

#include <iostream>
#include <set>
#include <map>
#include <vector>
#include <algorithm>
#include "math.h"
#include "geometry3D.h"
#include "common.h"
#include "sensor.h"

using namespace std;


class Sensor;
class Field;


class Field
{
public:
	Field();
	~Field();					
	void readSensorLocation3D(const char* topoFile);// read the sensor location file from 3D topology
	void readSensorLocation2D(const char * topoFile);// read the sneosr location file from 2D topology
	void connectivityTest();// test the connectivity   if the network is not connected, the algorithm will not go on 

	void determineEdgeNodes(); //find nodes on the edge
	void determineNotches();  //find notches
	void determineInitLandmarks(); //find landmarks
	void determineLandmarkNeighborhoods(); // establish links between nearby initLandmarks
	void determineSeeds3D(); //determine seed nodes in 3D
	void determineSeeds2D();// determine seed nodes in 2D
	void seedCoordinates2D();// seed coordinates for 2D
	void seedCoordinates3D();// seed coordinates for 3D

	double estimatePathQuality(vector<Sensor*> & path); //calculate the path quality
	void iterativeLocalization(); //iterative localization process
	void checkLoclizedResult();//check if the node is far from its neighbors
	void springMassAdjustLandmarks(int numIterations);// spring mass on the landmark level
	void springMassAdjustNonLandmarks(int numIterations);// spring mass on the common node level
	void landmarkLinkAligning();// adjust the nodes on the link between the landmarks
	bool initLandmarksLocalized();// check if all the landmarks are localized
	void getDisplacementByVirtualForceFromNeighbors(Sensor* sensor, vector<Sensor*> & neighbors, vector<double> & results);// commmon node level
	void getDisplacementByVirtualForceFromLandmarks(Sensor* sensor, vector<Sensor*> & neighbors, vector<double> & results);// landmark level
	void scheduleFlooding();// newly localized nodes will flood in the next round
	void cleanup();// use the neighbor information to clean up the unlocalized nodes

	int nSensors;// total number of sensors

	Sensor *sensorPool;// pointer to the sensor
	set<Sensor*> initLandmarks;// store the landmarks
	set<Sensor*> edgeLandmarks;// store the landmarks on the edge
	int diameter; // measure the size of the network
	double avg1HopNeighborhoodSize;// average 1 hop degree
	double avg2HopNeighborhoodSize;// average 2 hop degree
	double avgNodeDistance;// one in the algorithm to represent one hop
    double averageDistance;// distance one hop represents

	int totalNumFloodings;// total floodings
	int findNotchFloodings;// notch floodings
	int iterativeLocalizationFloodings;// localization floodings
	//////////////////////////////////////////////////////////////////////////
    void generateSensors3D(const char*topoFile);// generate the sensors for 3D
	void initSensors8(const char* topoFile);// generating the topology 8
	void initSensorsSmile(const char* topoFile);//generating the topology smile
	void initSensorsTorus(const char*topoFile);// generating the topology torus
	void initSensorsHourGlass(const char *topoFile);
	void initSensorscrossRing(const char *topoFile);


	void calNeighbor();// calculate the neighbor for the sensors
	void ouPutConnectivity();// output the connectivity 
	void inputConnectivity();// input the connectivity 

	void inputParameters3D(int ind);// input the parameters for 3D
	void inputParameters2D(int index);// input the parameters for 2D

	void coordinatesTransform2D();// transform the localized coordinates to the real system
	void coordinatesTransform3D();// transform the localized coordinates to the real system

	void calAvgNeighborsize();// calculate the average neighbor size
	void display2DResult();// display 2D result
	void display3DResult();// display 3D result

	void errorRrport();// report the error 
	void inputParametersForConnectivityFile();// input parameters for connectivity file

	void outputResult();// output the flooding
	//////////////////////////////////////////////////////////////////////////Dv-Hop
	double avgNodeDistanceDvHop;
	void localizationDvHop2D();
	void localizationDvHop3D();
	void dvError();
};

#endif