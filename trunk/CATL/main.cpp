#include <stdio.h>
#include <string.h>
#include "sensor.h"
#include "common.h"
#include "random.h"
#include "geometry3D.h"
#include "Topology.h"
#include <GL/glut.h>
#include "field.h"
//////////////////////////////////////////////////////////////////////////
double X_RANGE = 200;
double Y_RANGE = 200;
double Z_RANGE = 200;
//////////////////////////////////////////////////////////////////////////
double COMM_RANGE = 0; //Communication range for the sensors
double notchThreshold = 0;// Current notch threshold
int LANDMARK_SPACING = 0;// Control the density of the initlandmarks
int QUASI_UBG = 0;// Quasi-unit ball/disk graph communication model
double QUASI_UBG_ALPHA = 0.0;//alpha in the QUASI_UBG
Field *globalField = NULL;
int globalRound = 0;//The round in the iterative localization
double THRESHOLD = 0.5;// Final threshold to terminate the iterative localization 
//////////////////////////////////////////////////////////////////////////
int algorithmIndex=0;//0 for CATL and 1 for Dv-Hop
int topologyIndex=3;//index for the topology
bool threeDimension=false;// 2D or 3D
bool connectivityInformation=false;// Input connectivity information or the location information
const char* importFile3D[] = {"8.txt",  "smile.txt","torus.txt","hourglass.txt","crossRing.txt"};// 3D File name 
const char* importFile2D[]={"windows.off", "sun.off", "flower5.off", "music.off"};//2D File name

int main(int argc, char** argv)
{
	globalField = new Field();// Establish a globalField	
	if( connectivityInformation == false) {
		if( threeDimension == false) { 
        printf("This is 2D scenario\n");
        globalField->readSensorLocation2D(importFile2D[topologyIndex]); // Read the 2D sensor location file 
		globalField->inputParameters2D(topologyIndex);// Input parameters 
		}
		else {
			printf("This is 3D\n");
			globalField->generateSensors3D(importFile3D[topologyIndex]);// Generate 3D sensor location file
            globalField->readSensorLocation3D(importFile3D[topologyIndex]);// Read the 3D sensor location file
			globalField->inputParameters3D(topologyIndex);// Input parameters;
		}
            globalField->calNeighbor();// calculate the neighbor
            globalField->ouPutConnectivity();// Output connectivity information
	}
	else {	
		printf("input connectivity information\n");
		globalField->inputConnectivity();// Read the connectivity file.The file needs be related form
		globalField->inputParametersForConnectivityFile();//Input parameters for the algorithm.
	}

	globalField->connectivityTest(); // Check the network connectivity.If the network is not connected, terminate this algorithm
	globalField->calAvgNeighborsize();// Calculate the average size of one-hop neighbors and two-hop neighbors
    // edge & initlandmarks
    globalField->determineEdgeNodes();// Determine the edge nodes in the network. 
    globalField->determineInitLandmarks();// Choose the initlandmarks
	notchThreshold = globalField->edgeLandmarks.size() * 0.1;// Initial notchThreshold
 	printf("the current notchthreshold :%lf\n",notchThreshold);

    globalField->determineNotches();// Determine notches
	globalField->avgNodeDistance=1;// One unit length for one hop-count
    printf("The average node distance :%f\n",globalField->avgNodeDistance);
	if( threeDimension == false) {
    globalField->determineSeeds2D();// Determine the seeds in 2D
	globalField->seedCoordinates2D();// Assign coordinates to the seeds according to their real locations and the hop counts
	}
	else { // determine seeds for 3D
		globalField->determineSeeds3D(); // Determine the seeds in 3D
		globalField->seedCoordinates3D();
	}
	//////////////////////////////////////////////////////////////////////////choose to run the CATL or the Dv-Hop 
	if ( algorithmIndex == 0) {//CATL
		globalField->iterativeLocalization();// Iterative localization 
        globalField->cleanup();// For the unlocalized nodes, use the neighbor information to apply the centroid method
	 globalField->determineLandmarkNeighborhoods();// Get the initlandmarks in the near neighborhood
	 globalField->springMassAdjustLandmarks(50); // Adjust the initlandmarks
	 globalField->landmarkLinkAligning(); // 
	 globalField->springMassAdjustNonLandmarks(50); // For non-initlandmark nodes

	if (connectivityInformation == false)
	{
	 if( threeDimension == false)
	 globalField->coordinatesTransform2D();     // Transform the result to the absolute system
	 else 
		globalField->coordinatesTransform3D();  // Transform the result to the absolute system
	 globalField->errorRrport();     // Report the error
	}
	}
	else { // Use the same seeds in the Dv-Hop
		if ( threeDimension == false)
		    globalField->localizationDvHop2D();
		else
			globalField->localizationDvHop3D();
		globalField->dvError();
		exit(0);
	}

	globalField->outputResult();

	if(!threeDimension) 
    globalField->display2DResult(); //For 2D
	else
	globalField->display3DResult(); //For 3D
	return 0;
}


