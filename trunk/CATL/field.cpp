#include "sensor.h"
#include "random.h"
#include "assert.h"
#include "common.h"
#include "geometry3D.h"
#include <GL/glut.h>
#include "display2D.h"
#include "Topology.h"
#include "display3D.h"
#include "field.h"


Field::Field()
{	
	diameter = 0;
	totalNumFloodings = 0;
}

Field::~Field()
{
	delete [] sensorPool;
}

//read file
void Field::readSensorLocation3D(const char* topoFile)//read the location of the nodes, 
{
	vector<Point> validPoints;
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
				validPoints.push_back(s);
			}
		}while(!feof(fp));
		fclose(fp);
     	nSensors = validPoints.size();
		printf("nSensor = %d\n", nSensors);
	}
	
	sensorPool = new Sensor[nSensors];
	for(int i = 0; i < validPoints.size(); i++){
		sensorPool[i].init(i, validPoints[i]);
		sensorPool[i].field = this;
	}

}



void Field::connectivityTest()// Test the connectivity of the network
{
	printf("Test connectivity\n");
	int j;
	int totalNumCovered =0;
	vector<Sensor> tempsPool;
	vector<Sensor> tempsensorPool;
	set<Sensor*> *frontier = new set<Sensor*>;
       
		tempsPool.clear();
		int depth = 0;
		totalNumCovered = 0;
		frontier->clear();
		frontier->insert(&sensorPool[0]);
		tempsPool.push_back(sensorPool[0]);

       for(int i =0;i<globalField->nSensors;i++){
		   globalField->sensorPool[i].broadcastCovered=0;
	   }
	  
		while (!frontier->empty()) {
			set<Sensor*> *newFrontier = new set<Sensor*>;
			for(set<Sensor*>::const_iterator iFron = frontier->begin(); iFron != frontier->end(); ++iFron) {
				Sensor *s = *iFron;
				for(j = 0; j < s->neighbors.size(); j++) {		
					Sensor *n = s->neighbors[j];
					if (n->broadcastCovered == 0) {
						newFrontier->insert(n);
						n->candidateParents.insert(s);
					}
				}
			}
			
			if (!newFrontier->empty()) {
				depth++;
				if (depth > MAXNUM_LEVELS) { // Too many levels in the network.
					fprintf(stderr, "flooding beyond pre-allocated level!\n");
					exit(0);
				}
			}
			
			//let each node in the new level choose a parent
			for(SCITER iNewFron = newFrontier->begin(); iNewFron != newFrontier->end(); ++iNewFron) {
				
				Sensor* c = *iNewFron;
				Sensor* parentChosen = NULL;
				
				int randidx = rand() % c->candidateParents.size();
				int count = 0;
				
				for(SCITER iCan = c->candidateParents.begin(); iCan != c->candidateParents.end(); ++iCan, ++count) {
					Sensor* can = *iCan;
					
					if (count == randidx) {
						parentChosen = can;
						break;
					}
				}
				
				c->level = depth;
				c->broadcastCovered = 1;
				c->parent = parentChosen;
				parentChosen->children.insert(c);
				tempsPool.push_back(*c);
				
				totalNumCovered++;
			}
			frontier = newFrontier;
		}


		if ( totalNumCovered < nSensors) { // Not all sensors broadcast covered in this function. So the network is disconnected.
			printf(" this network is dis-connected\n");
			exit(0);
		}

	delete frontier;
}

void Field::determineInitLandmarks()// Choose the evenly distributed initlandmarks                                              
{          
	for(int i = 0; i < nSensors; i++) {
		sensorPool[i].broadcastCovered = 0;
	}
		
	//First, select some edge nodes to be init landmarks
	for(int i = 0; i < nSensors; i++) {
			
		Sensor* s = sensorPool + i;

		if (!s->onEdge || s->broadcastCovered) continue;

		set<Sensor*> vincinity;

		s->getMultiHopNeighborhood(LANDMARK_SPACING, vincinity); //Test if there is initlandmark in the neighborhood of LANDMARK_SPACING hop

		bool found = false;

		for(set<Sensor*>::const_iterator iter = vincinity.begin(); iter != vincinity.end(); ++iter) {
			Sensor* s1 = *iter;
			s1->broadcastCovered = 1;

			if (s1->initLandmark) {
				found = true;
				break;
			}
		}

		if (!found) {
			s->initLandmark = true;
			s->broadcastCovered = 1;
		}
	}
		
	//then, select some interior nodes to be init landmarks
	for(int i = 0; i < nSensors; i++) {
			
		Sensor* s = sensorPool + i;

		if (s->onEdge || s->broadcastCovered) continue;

		set<Sensor*> vincinity;

		s->getMultiHopNeighborhood(LANDMARK_SPACING, vincinity); //Same with the above

		bool found = false;

		for(set<Sensor*>::const_iterator iter = vincinity.begin(); iter != vincinity.end(); ++iter) {
			Sensor* s1 = *iter;
			s1->broadcastCovered = 1;

			if (s1->initLandmark) {
				found = true;
				break;
			}
		}

		if (!found) {
			s->initLandmark = true;
			s->broadcastCovered = 1;
		}
	}
	
	for(int i = 0; i < nSensors; i++) {
		sensorPool[i].broadcastCovered = 0;
		if (sensorPool[i].initLandmark) {
			initLandmarks.insert(sensorPool + i);
		}
	}


	for (int i =0; i < nSensors; i++) {
		sensorPool->broadcastCovered=0;
		if (sensorPool[i].isEdgeLandmark()) {
			edgeLandmarks.insert(sensorPool+i);
		}
	}
	printf("the current size of the initLandmark:%d",initLandmarks.size());
}


void Field::determineNotches() // Determine notches
{

	int i;
	for(i = 0; i < nSensors; i++) {
		Sensor* s = sensorPool + i;
		if (s->isEdgeLandmark()) { // Flooding from edge initlandmark
			printf("(%dth) flooding ", totalNumFloodings, i);
			s->findNotches(); // Flooding function
			totalNumFloodings++;
		}
	}
	printf("flooding frequency: %lf\n", double(totalNumFloodings)/nSensors);
	findNotchFloodings = totalNumFloodings;
	int num = 0;
	for(i = 0; i < nSensors; i++) {			
		Sensor* s = sensorPool + i;
		if (s->notchDegree > notchThreshold) { // The notchdegree is higher than the notchThreshold, so it is an notch
			num++;
		}
	}
	printf("total number of init notch nodes %d, network diameter (approx): %d\n", num, diameter); 
	// The diameter is to measure the size of the network
}

void Field::determineEdgeNodes() // Determine edge nodes 
{
	
	for(int i = 0; i < nSensors; i++) {
		Sensor* s = sensorPool + i;
		double percentageScale=0;
		if( threeDimension ==false) // Different scale for 2D and 3D network
			percentageScale =0.8;
		else 
			percentageScale=0.75;
		if (s->getMultiHopNeighborhoodSize(2) < percentageScale * Field::avg2HopNeighborhoodSize) { // The neighbor size is too small
			s->onEdge = true;
		}
		else {
			s->onEdge = false;
		}
	}

	// Refine the edge nodes by the number of edge nodes in the edge's neighborhood
	for(int round=0;round<1;round++) { 
		for (int i =0;i<nSensors;i++) {
			Sensor *s=sensorPool+i;
			if(s->onEdge==true) {
				s->edgeNeighborSize();
				  }
				}
			}		
}


void Field::determineLandmarkNeighborhoods()
{
	int i;
	SCITER iter;

	for ( i =0; i<nSensors; i++) {
		Sensor *s=sensorPool+i;
		s->spMap.clear();
	}

	for(iter = initLandmarks.begin(); iter != initLandmarks.end(); ++iter) {			
		(*iter)->areaFlooding(5*LANDMARK_SPACING);
	}


   // Find initlandmarks in the neighborhood  

	for (iter = initLandmarks.begin(); iter !=initLandmarks.end(); ++iter) {
		Sensor *s =*iter;

		for(SCITER iter1 = initLandmarks.begin(); iter1 != initLandmarks.end(); ++iter1) {	
			Sensor *ngh = *iter1;

			if (ngh->areaspMap.find(s->index)!=ngh->areaspMap.end())
			if (ngh != s && s->areaspMap[ngh->index].hopcount <= 5 * LANDMARK_SPACING) 
			{ 
				if (s->areaspMap[ngh->index].quality == 1 || ngh->areaspMap[s->index].quality == 1) { // Have the node if the quality is good
					s->landmarkNeighbors.push_back(ngh);
				}
				else if (ngh !=s&&s->areaspMap[ngh->index].hopcount <= 2 * LANDMARK_SPACING) { // Have the node if the two initlandmarks are near
					s->landmarkNeighbors.push_back(ngh);
				}
			}
		}
		
	}
	
	for(iter = initLandmarks.begin(); iter != initLandmarks.end(); ++iter) {	
			
		Sensor* s = *iter;
		
		for(SCITER iter1 = initLandmarks.begin(); iter1 != initLandmarks.end(); ++iter1) {	
			Sensor *ngh = *iter1;

            if (ngh->areaspMap.find(s->index)!=ngh->areaspMap.end())
			if (ngh != s && s->areaspMap[ngh->index].hopcount <= 3 * LANDMARK_SPACING) { 
				if (s->areaspMap[ngh->index].quality == 1 || ngh->areaspMap[s->index].quality == 1) { // Have the node if the quality is good
					s->landmarkNeighbors.push_back(ngh);
				}
				else if (ngh !=s&&s->areaspMap[ngh->index].hopcount <= 2 * LANDMARK_SPACING) { // Have the node if the two initlandmarks are near
					s->landmarkNeighbors.push_back(ngh);
				}
			}
		}
	}

	for(iter = initLandmarks.begin(); iter != initLandmarks.end(); ++iter) {		
		Sensor* s = *iter;
		if (s->landmarkNeighbors.size() > 6)// For nodes with fewer initlandmark neighbors
			continue;
		
		for(SCITER iter1 = initLandmarks.begin(); iter1 != initLandmarks.end(); ++iter1) {	
			Sensor *ngh = *iter1;

			if(s->areaspMap[ngh->index].hopcount <= 2 * LANDMARK_SPACING)
				continue;

			if (ngh != s && s->areaspMap[ngh->index].hopcount <= 3 * LANDMARK_SPACING) {
				if (s->areaspMap[ngh->index].quality == 1 || ngh->areaspMap[s->index].quality == 1) {
					
					s->landmarkNeighbors.push_back(ngh);

					if (find(ngh->landmarkNeighbors.begin(), ngh->landmarkNeighbors.end(), s)
						== ngh->landmarkNeighbors.end()) {
						ngh->landmarkNeighbors.push_back(s);
					}
				}
			}	
		}
	}
}


void Field::determineSeeds3D()// four seeds are needed here
{	
	for(SCITER iter = initLandmarks.begin(); iter != initLandmarks.end(); ++iter) {			
		(*iter)->localizationFlood(true);
	}

	int j;
	map<int, int> notchFreeNumNeighborsMap;
	map<int, int> notchFreeRadiusMap;

	int *levelSize = new int[nSensors];

	for(SCITER si = initLandmarks.begin(); si != initLandmarks.end(); ++si) { // choose the landmark with the largest notch-free area
		Sensor * s = *si;
		
		int scope = nSensors;
		int totalSize = 0;
		memset(levelSize, 0, sizeof(int) * nSensors);
		
		for(j = 0; j < nSensors; j++) {
			
			if(j == s->index) continue;
			
			int d = sensorPool[j].spMap[s->index].hopcount;
			levelSize[d]++;

			if (sensorPool[j].notchDegree >= notchThreshold && d < scope) {
				scope = d;// the scope is changed to the minimum number of all the notch-free distance
			}				
		}
		
		for(j = 0; j < scope; j++) {
			totalSize += levelSize[j];
		}
		
		notchFreeNumNeighborsMap[s->index] = totalSize;		//the total number of sensors in the scope
		notchFreeRadiusMap[s->index] = scope;		
	}
	//notchFreeNumNeighborsMap.size() = initLandmarks.size()
	delete [] levelSize;

	int freestNodeIdx = -1;
	int maxArea = 0;
	for(map<int, int>::const_iterator iter = notchFreeNumNeighborsMap.begin(); 
								iter != notchFreeNumNeighborsMap.end(); ++iter) {

		int nidx = (*iter).first;
		int area = (*iter).second;

		if (area > maxArea) {
			maxArea = area;
			freestNodeIdx = nidx;
		}
	}

	assert(freestNodeIdx >= 0);// assert ? 

	set<Sensor*> notchFreeNeighbors;	
	int freeDistance = notchFreeRadiusMap[freestNodeIdx]-1;
	sensorPool[freestNodeIdx].getMultiHopNeighborhood(freeDistance, notchFreeNeighbors);

	Sensor* seed0 = sensorPool + freestNodeIdx;

	for(int i = 0; i < nSensors; i++) {			
		Sensor* s = sensorPool + i;
		s->spMap.clear();
	}
    seed0->localizationFlood();
	printf("the seed node seed0:");
	printf("%d\n",seed0->index);
	Sensor* seed1 = NULL, *seed2 = NULL, *seed3 = NULL;
	double maxDistance = 0;

	SCITER nfni;
	double dist_a,dist_b,dist_c,dist_d,dist_e,dist_f;
	for(nfni = notchFreeNeighbors.begin(); nfni != notchFreeNeighbors.end(); ++nfni) {
		Sensor* s = *nfni;
		double d = s->shortestPathLength(seed0);
		if (d > maxDistance) {
			maxDistance = d;
			seed1 = s;
			dist_a=d;
		}
	}
	printf("the seed node seed1:" );
	printf("%d",seed1->index);
    seed1->localizationFlood();

	maxDistance = 0;

	for(nfni = notchFreeNeighbors.begin(); nfni != notchFreeNeighbors.end(); ++nfni) {
		Sensor* s = *nfni;
		if (s != seed0 && s!= seed1) {	
			double d20 = s->shortestPathLength(seed0);
			double d21 = s->shortestPathLength(seed1);
			double d = d20 + d21;
			double diff = fabs(d20 - d21);// guarantee that the two edges are almost the same length, 
			if (diff < 2 * avgNodeDistance && d > maxDistance) {
				maxDistance = d;
				seed2 = s;
				dist_b=d20;
				dist_c=d21;
			}
		}	
	}
	printf("the seed node seed2:");
	printf("%d\n",seed2->index);

    seed2->localizationFlood();

	for(nfni = notchFreeNeighbors.begin(); nfni != notchFreeNeighbors.end(); ++nfni) {
		Sensor* s = *nfni;
		if (s != seed0 && s!= seed1 && s != seed2) {	
			double d30 = s->shortestPathLength(seed0) ;
			double d31 = s->shortestPathLength(seed1);
			double d32 = s->shortestPathLength(seed2);
			double d = d30 + d31 + d32;	
			
			double diff = mymax(fabs(d32 - d31), fabs(d31 - d30));
			diff = mymax(diff, fabs(d30 - d32));

			if (diff < 3 * avgNodeDistance && d > maxDistance) {
				maxDistance = d;
				seed3 = s;
				dist_d=d30;
				dist_e=d31;
				dist_f=d32;
			}
		}	
	}
	printf("the seed node seed3:");
    printf("%d",seed3->index);


//////////////////////////////////////////////////////////////////////////then comes the assignment offour seed nodes
//first have the four seed characteristics determined

	seed0->setSeed();
	seed1->setSeed();
	seed2->setSeed();
    seed3->setSeed();

	printf(" the seeds: \n:%d %d %d %d\n",seed0->index,seed1->index,seed2->index,seed3->index);
	printf(" the initial seed->: %d",seed0->index);



}


void Field::checkLoclizedResult()// check if the node is too far away from its neighbor
{
	
	set<Sensor*> nearNghs;

	for(int i = 0; i < nSensors; i++) {
		Sensor* s = sensorPool + i;
		
		nearNghs.clear();
		nearNghs.insert(s->neighbors.begin(), s->neighbors.end());
		int numLocalized = 0;
		double accumX = 0;
		double accumY = 0;
		double accumZ = 0;
		
		for(set<Sensor*>::iterator iter = nearNghs.begin(); iter != nearNghs.end(); ++iter) {
			Sensor *ngh = *iter;
			if (ngh == s)		//impossible how could the neighbor contain himself
				continue;
			
			if (ngh->localizedPosition != INVALID_POINT) { // only consider localized node
				numLocalized++;
				accumX += ngh->localizedPosition.x;
				accumY += ngh->localizedPosition.y;				
				accumZ += ngh->localizedPosition.z;
			}
		}
		
		if (numLocalized) {  // check if the node is too fat away from the neighbor
			double avgX = accumX / numLocalized;
			double avgY = accumY / numLocalized;
			double avgZ = accumZ / numLocalized;
			
			double xdiff = fabs(s->localizedPosition.x - avgX);
			double ydiff = fabs(s->localizedPosition.y - avgY);
			double zdiff = fabs(s->localizedPosition.z - avgZ);
						
			if (xdiff > 2 * avgNodeDistance || ydiff > 2 * avgNodeDistance || zdiff > 2 * avgNodeDistance) {
				s->localizedPosition = INVALID_POINT;
				s->localizationConfidence = 0;
			}
		}		
	}
}


bool Field::initLandmarksLocalized()// all landmarksare localized
{

	int num = 0;
	
	for(SCITER iter = initLandmarks.begin(); iter != initLandmarks.end(); ++iter) {
		Sensor *s = *iter;
		if (s->localizedPosition != INVALID_POINT) {
			num++;
		}
	}

	if (num == initLandmarks.size()) {
		printf("all init landmarks are localized!......\n");
		return true;
	}
	else {
		printf("only %d init landmarks are localized! (should be %d)******** \n", num, initLandmarks.size());
		return false;
	}
}

void Field::springMassAdjustLandmarks(int numIterations)// spring-mass on the landmark level
{
	int i, j;
	printf("spring mass iter...\n");
	for(i = 0; i < numIterations; i++) {
		
		printf("%d ", i);

		for(j = 0; j < nSensors; j++) {
			Sensor *s = sensorPool + j;

			if (s->localizedPosition ==	INVALID_POINT) 
				continue;

			if (!s->initLandmark) 
				continue;
			vector<double> displacement(3);
			getDisplacementByVirtualForceFromLandmarks(s, s->landmarkNeighbors, displacement); // Force by the landmark neighbors
						
			s->localizedPosition.x += displacement[0];
			s->localizedPosition.y += displacement[1];	
			s->localizedPosition.z += displacement[2];
		}
	}
	printf("spring mass iterations end\n");
}


void Field::getDisplacementByVirtualForceFromNeighbors(Sensor* sensor, vector<Sensor*> & neighbors, vector<double> & displacement)
{		
	Point curCenter = sensor->localizedPosition;
	vector<double> force(3);

    for(vector<Sensor*>::iterator iter = neighbors.begin(); iter != neighbors.end(); ++iter) {
        
		Sensor* s = *iter;
		Point sCenter = s->localizedPosition;

		if (sCenter == INVALID_POINT) 
			continue;

		double distance = curCenter.distance(sCenter);
		if(distance == 0)
			distance = 0.01;

		double sprintConst = 0.5;

		double restLength;
		
		if (sensor->spMap.find(s->index) != sensor->spMap.end()) {
			restLength = sensor->spMap[s->index].hopcount * sensor->field->avgNodeDistance;
		}
		else {
			restLength = sensor->field->avgNodeDistance;
		}
		
		
		double scalar = (restLength - distance) * sprintConst;

		force[0] += scalar * (curCenter.x - sCenter.x) / distance;
		force[1] += scalar * (curCenter.y - sCenter.y) / distance;
		force[2] += scalar * (curCenter.z - sCenter.z) / distance;
	}
		
	double forceScalar = sqrt(pow(force[0],2) + pow(force[1],2) + pow(force[2],2));

	UniformRandVar randv;
	UniformRandVar randpos(0, avgNodeDistance/2);

	if (randv.value() < 0.05) { //perturbation with a small probability
		
		force[0] = randpos.value() - avgNodeDistance / 4;
		force[1] = randpos.value() - avgNodeDistance / 4;
		force[2] = randpos.value() - avgNodeDistance / 4;
	}
	else {
		double cap = avgNodeDistance/2;
		if (forceScalar > cap) {
			force[0] *= (cap / forceScalar);
			force[1] *= (cap / forceScalar);
			force[2] *= (cap / forceScalar);
		}
		else if (forceScalar < 0.1) {
			force[0] = 0;
			force[1] = 0;
			force[2] = 0;
		}	
	}	
	displacement[0] = force[0];
	displacement[1] = force[1];
	displacement[2] = force[2];
	return;
}

void Field::getDisplacementByVirtualForceFromLandmarks(Sensor* sensor, vector<Sensor*> & neighbors, vector<double> & displacement)
{
			
	Point curCenter = sensor->localizedPosition;
	vector<double> force(3);

	//printf("\n");

    for(vector<Sensor*>::iterator iter = neighbors.begin(); iter != neighbors.end(); ++iter) {	//all pf these neighbors are initlandmarks
        
		Sensor* s = *iter;
		Point sCenter = s->localizedPosition;

		if (sCenter == INVALID_POINT) 
			continue;

		double distance = curCenter.distance(sCenter);
		if(distance == 0)
			distance = 0.01;// there may be two points in the same place, 

		double sprintConst = 0.5;		//coefficient of spring

		double restLength;
			
		
		
		if (sensor->areaspMap.find(s->index) != sensor->areaspMap.end()) {
			restLength = sensor->areaspMap[s->index].hopcount * avgNodeDistance;
		}
		else if (sensor->isNeighbor(s->index)) { //one-hop neighbor
			restLength = avgNodeDistance;
		}
		else {
			fprintf(stderr, "spring mass, unknown distance %d! in the level of landmark\n", s->index);
			continue;
		}
		
		//printf("rest len %g, orginal len %g\n", restLength, originlength);

		double scalar = (restLength - distance) * sprintConst;		//stand for the force

		force[0] += scalar * (curCenter.x - sCenter.x) / distance;	//x component of force(scalar)
		force[1] += scalar * (curCenter.y - sCenter.y) / distance;	//y component of force(scalar)
		force[2] += scalar * (curCenter.z - sCenter.z) / distance;	//z component of force(scalar)
	}
		
	double forceScalar = sqrt(pow(force[0],2) + pow(force[1],2) + pow(force[2],2));
	
	double cap = avgNodeDistance/2;
	if (forceScalar > cap) {
		force[0] *= (cap / forceScalar);
		force[1] *= (cap / forceScalar);
		force[2] *= (cap / forceScalar);
	}
	else if (forceScalar <0.1) {
		force[0] = 0;
		force[1] = 0;
		force[2] = 0;
	}	
	
	displacement[0] = force[0];
	displacement[1] = force[1];
	displacement[2] = force[2];

	return;
}


void Field::springMassAdjustNonLandmarks(int numIterations)// spring-mass on the common nodes level
{
	
	int i, j;

	printf("spring mass final...\n");

	for(i = 0; i < numIterations; i++) {

		printf("%d ", i);
		
		for(j = 0; j < nSensors; j++) {
			Sensor *s = sensorPool + j;

			if (s->seed || s->initLandmark || s->localizedPosition == INVALID_POINT || s->tempLocation != INVALID_POINT) 
				continue;
			
			vector<double> displacement(3);
			getDisplacementByVirtualForceFromNeighbors(s, s->neighbors, displacement);  // Get force by the immediate neighbors
						
			s->localizedPosition.x += displacement[0];
			s->localizedPosition.y += displacement[1];	
			s->localizedPosition.z += displacement[2];

		}
	}
}

void Field::landmarkLinkAligning() // Use the links between the initlandmarks to help the common nodes adjustment
{
	printf("beginning of link aligning\n");
	int i, k;
	
	for(i = 0; i < nSensors; i++) {
		Sensor * s = sensorPool + i;
		s->numContainingSpPaths = 0;
	}

	for(i = 0; i < nSensors; i++) {
		Sensor * s = sensorPool + i;
		if (!s->initLandmark)		
			continue;
       
		for(vector<Sensor*>::iterator vIter = s->landmarkNeighbors.begin(); vIter != s->landmarkNeighbors.end(); ++vIter) {
			
			Sensor* v = *vIter;

			if (s->index > v->index) {
				vector<Sensor*> path;
				
				if (s->spMap.find(v->index) != s->spMap.end() || v->spMap.find(s->index) != v->spMap.end()) 
				s->getShortestPath(v, path);
				else continue;
				
				if (path.size() == 0) 
					continue;
				
				LineSegment edge(s->localizedPosition, v->localizedPosition);					
				double len = edge.start.distance(edge.end);
				double interval = len / (path.size()+1);		
				
				for(k = 0; k < path.size(); k++) {
					Point pt = edge.getOnlinePoint(edge.start, edge.end, interval * (k+1));
					
					if(path[k]->tempLocation != INVALID_POINT) {						
						path[k]->tempLocation.x = path[k]->tempLocation.x + pt.x;
						path[k]->tempLocation.y = path[k]->tempLocation.y + pt.y;
						path[k]->tempLocation.z = path[k]->tempLocation.z + pt.z;
					}
					else {
						path[k]->tempLocation = pt;
					}
					path[k]->numContainingSpPaths++;

				}
			}
		}
	}

	for(i = 0; i < nSensors; i++) {
		Sensor * s = sensorPool + i;

		if (s->tempLocation != INVALID_POINT) {
			s->localizedPosition.x = s->tempLocation.x / s->numContainingSpPaths;
			s->localizedPosition.y = s->tempLocation.y / s->numContainingSpPaths;
			s->localizedPosition.z = s->tempLocation.z / s->numContainingSpPaths;
		}
	}

	for(i = 0; i < nSensors; i++) {
		Sensor * s = sensorPool + i;

		if (!s->onEdge) {
			s->tempLocation = INVALID_POINT;
		}
	}

	printf(" end of aligning\n");
}

void Field::scheduleFlooding()
{
	for(int i = 0; i < nSensors; i++) {
		Sensor* s = sensorPool + i;
	
		if (s->isEdgeLandmark()){

			//well localized nodes do not flood again
			if (s->oldConfidence >= 0.7) {
				continue;
			}

			//location-improved nodes should update other nodes
			//with their coordinates
			if (s->localizationConfidence > s->oldConfidence + 0.3) {
			
				s->nextRound = globalRound + 1;
				s->lastScheduledRound = globalRound;
			}			
		}
	}
}





double Field::estimatePathQuality(vector<Sensor*> & path)
{
	int i;
	double quality;
	double maxNotchDegree = 0;
	double avgNotchDegree = 0;
	double totalNotchDegree = 0;
	int numNotches = 0;

	int firstNotchIndex = -1;	
	int lastNotchIndex = -1;

	int hopsUntilFirstNotch = 0;
	int hopsAfterLastNotch = 0;
	int hopsBetweenFirstAndLastNotches = 0;
	int affectedLength = 0;

	Sensor* maxNotchPoint = NULL;

	if (path.size() <= 2) {
		return 1;
	}

	for(i = 0; i < path.size(); i++) {
		Sensor * s = path[i];

		if (s->notchDegree > notchThreshold) {
			numNotches++;
			totalNotchDegree += s->notchDegree;
		}

		if (s->notchDegree > notchThreshold && firstNotchIndex == -1) {
			firstNotchIndex = i;
		}

		if (s->notchDegree > notchThreshold) 
			lastNotchIndex = i;

		if (firstNotchIndex == -1) 
			hopsUntilFirstNotch++;

		if (lastNotchIndex >= 0)
			hopsAfterLastNotch = path.size() - lastNotchIndex;

		if (s->notchDegree > maxNotchDegree) {
			maxNotchDegree = s->notchDegree;
			maxNotchPoint = s;
		}
	}

	if (numNotches == 0) {
		affectedLength = 0;
	}
	else {
		hopsBetweenFirstAndLastNotches = path.size() - hopsUntilFirstNotch - hopsAfterLastNotch - 1;
		if (hopsBetweenFirstAndLastNotches < 0)  hopsBetweenFirstAndLastNotches = 0;
		
		int affectedLength = mymin(hopsUntilFirstNotch, hopsAfterLastNotch);
		affectedLength = mymax(affectedLength, hopsBetweenFirstAndLastNotches);

		if (affectedLength >= 2) 
			return 0;
		
		avgNotchDegree = totalNotchDegree/numNotches;
	}
	
	quality = 1;
	quality = quality / (1 + affectedLength);
	//quality = quality / sqrt(1 + avgNotchDegree);
	
	return quality;
}


//////////////////////////////////////////////////////////////////////////


void Field::initSensors8(const char* topoFile)// generate the sensors in the field, store them in the file 
{
	//////////////////////////////////////////////////////////////////////////   8 topology 
	float xrange = 120;
	float yrange = 200;
	float zrange = 50;
    int nSensors = 30000;
	vector<Point> sensors;
	FILE *fp;
	fp = fopen(topoFile, "w+");
	int i, j = 0;
	int nRow;
	int nCol;
	int nWid;
	nRow = nCol = nWid = CEILING(pow(nSensors, 1.0/3.0));
	double interval = double(Z_RANGE)/double(nRow+1);


	NormalRandVar randv(0, interval*0.25);
	for(i = 0; i < nSensors; i++) 
	{
		int idxRow = i / nWid / nWid;
		int idxCol = i / nWid - (i / nWid / nWid) * nWid;
		int idxWid = i - (i / nWid) * nWid;
		double gridX = (idxRow + 1) * interval;
		double gridY = (idxCol + 1) * interval;
		double gridZ = (idxWid + 1) * interval;
		double x = gridX + randv.value();
		double y = gridY + randv.value();
		double z = gridZ + randv.value();
		if (x < 0) x = 0;
		if (x > X_RANGE) x = X_RANGE;
		if (y < 0) y = 0;
		if (y > Y_RANGE) y = Y_RANGE;
		if (z < 0) z = 0;
		if (z > Z_RANGE) z = Z_RANGE;
		if(!((x>xrange/3 && x<xrange/3*2 && y>yrange/5 && y<yrange/5*2 ) 
			|| (x>xrange/3 && x<xrange/3*2 && y>yrange/5*3 && y<yrange/5*4 )) && x>0 && x<xrange && y>0 && y<yrange && z>0 && z<zrange)
		{
			sensors.push_back(Point(x,y,z));
			fprintf(fp, "%lf %lf %lf\n", x, y, z);
		}

	}
	printf("end of the init Sensors\n");
	printf("the total num of sensors we have%d\n",sensors.size());
	fclose(fp);
}

void Field::calNeighbor()// calculate neighbors for the sensors
{
    printf(" Calculate neighbor information\n");
	for( int i = 0; i < nSensors; i++) {
		if (!QUASI_UBG) {
			sensorPool[i].updateNeighbors();// UBG communication model
		}
		else {
			sensorPool[i].updateNeighborsQuasiUBG();// QUASI_UBG communication model
		}
	}	
}



void Field::cleanup()// use the neighbor to localize the unlocalized nodes
{
	bool complete;
	do {

		complete = true;
 
		for(int i = 0; i < nSensors; i++) {
			Sensor* s = sensorPool + i;

			if (s->localizedPosition == INVALID_POINT) {

				complete = false;

				set<Sensor*> nearNghs;
				s->getMultiHopNeighborhood(1, nearNghs);

				int numLocalized = 0;
				double accumX = 0;
				double accumY = 0;
				double accumZ = 0;

				for(set<Sensor*>::iterator iter = nearNghs.begin(); iter != nearNghs.end(); ++iter) {
					Sensor *ngh = *iter;
					if (ngh == s) 
						continue;

					if (ngh->localizedPosition != INVALID_POINT ) {
						numLocalized++;
						accumX += ngh->localizedPosition.x;
						accumY += ngh->localizedPosition.y;
						accumZ += ngh->localizedPosition.z;
					}
				}
				// The average of the localized neighbors
				if (numLocalized) {
					double avgX = accumX / numLocalized;
					double avgY = accumY / numLocalized;
					double avgZ = accumZ / numLocalized;

					

					s->localizedPosition = Point(avgX, avgY, avgZ);
					s->localizationConfidence = 0.9;
				}
				else {
					s->localizedPosition = INVALID_POINT;
					s->localizationConfidence = 0;
				}
			}			
		}
	}while (!complete);


	if(initLandmarksLocalized()){
		printf("The program is finished\n");
	}
}







void Field::inputParameters3D(int model)
{
	switch(model)
	{
	case 0:
		COMM_RANGE=8.75;
		QUASI_UBG=0;
		LANDMARK_SPACING=3;
		break;
	case 1:
		COMM_RANGE=5.8;
		QUASI_UBG=0;
		LANDMARK_SPACING=3;
		break;
	case 2:
		COMM_RANGE=8.2;// average node degree 11.3
       // COMM_RANGE=7.5;//   average node degree 8.56
		//COMM_RANGE=7.7;//average node degree 9.32
       // COMM_RANGE = 9;//average node degree 14.94
		//COMM_RANGE=9.5;
		QUASI_UBG=0;
		LANDMARK_SPACING=4;
		break;
	case 3: 
		COMM_RANGE=8;
		QUASI_UBG=0;
		LANDMARK_SPACING=2;
		break;
	case 4:
		COMM_RANGE = 8.8;
		QUASI_UBG=0;
		LANDMARK_SPACING=3;
		break;
	}
}


void  Field::initSensorsSmile(const char* topoFile)// topology smile
{
	float xrange = 100;
	float yrange = 100;
	float zrange = 100;
	float radius = xrange/2;
	vector<Point> sensors;
	FILE *fp;
	fp = fopen(topoFile, "w+");

	int totalSensors = 120000;
	sensors.clear();
	int i, j = 0;
	int nRow;
	int nCol;
	int nWid;
	nRow = nCol = nWid = CEILING(pow(totalSensors, 1.0/3.0));
	double interval = double(Z_RANGE)/double(nRow+1);

	NormalRandVar randv(0, interval*0.25);

	for(i = 0; i < totalSensors; i++) 
	{
		int idxRow = i / nWid / nWid;
		int idxCol = i / nWid - (i / nWid / nWid) * nWid;
		int idxWid = i - (i / nWid) * nWid;
		double gridX = (idxRow + 1) * interval;
		double gridY = (idxCol + 1) * interval;
		double gridZ = (idxWid + 1) * interval;
		double x = gridX + randv.value();
		double y = gridY + randv.value();
		double z = gridZ + randv.value();

		if (x < 0) x = 0;
		if (x > X_RANGE) x = X_RANGE;
		if (y < 0) y = 0;
		if (y > Y_RANGE) y = Y_RANGE;
		if (z < 0) z = 0;
		if (z > Z_RANGE) z = Z_RANGE;
		double x0 = x-X_RANGE/2;
		double y0 = y-Y_RANGE/2;
		double z0 = z-Z_RANGE/2;
		double d = sqrt(x0*x0+y0*y0+z0*z0);
		double d1 = sqrt((x0+radius/3)*(x0+radius/3) + (y0-radius/3)*(y0-radius/3));
		double d2 = sqrt((x0-radius/3)*(x0-radius/3) + (y0-radius/3)*(y0-radius/3));
		if(d1>=radius/5 && d2>=radius/5 && d<radius && !(x0>-xrange/6 && x0<xrange/6 && y0>-yrange/12-radius/3 && y0<yrange/12-radius/3))
		{
			sensors.push_back(Point(x,y,z));
			fprintf(fp, "%lf %lf %lf\n", x, y, z);
		}
	}
	fclose(fp);
}




void Field::initSensorsTorus(const char*topoFile)// topology torus
{
	float MajorRadius = 70;
	float MinorRadius = 30;
	vector <Point> sensorlocation;
	FILE *fp;
	fp = fopen(topoFile, "w+");
	nSensors=42000;
	sensorlocation.clear();
	int i, j = 0;
	int nRow;
	int nCol;
	int nWid;
	nRow = nCol = nWid = CEILING(pow(nSensors, 1.0/3.0));
	double interval = double(Y_RANGE)/double(nRow+1);
	COMM_RANGE = 2.2 * double(Y_RANGE)/double(nRow+1);
	NormalRandVar randv(0, interval * 0.25);

for(i = 0; i < nSensors; i++) 
	{
		int idxRow = i / nWid / nWid;
		int idxCol = i / nWid - (i / nWid / nWid) * nWid;
		int idxWid = i - (i / nWid) * nWid;
		double gridX = (idxRow + 1) * interval;
		double gridY = (idxCol + 1) * interval;
		double gridZ = (idxWid + 1) * interval;
		double x = gridX + randv.value() - X_RANGE/2;
		double y = gridY + randv.value() - Y_RANGE/2;
		double z = gridZ + randv.value() - Z_RANGE/2;
		
		
		float d = sqrt(x*x + z*z);
		float phi1 = asin(y/MinorRadius);
		float phi2 = PI - phi1;
		float r1 = MajorRadius + MinorRadius*cos(phi1);
		float r2 = MajorRadius + MinorRadius*cos(phi2);
		float d1 = (r1>r2?r1:r2);
		float d2 = (r1<r2?r1:r2);
		if(d < d1 && d > d2)
		{
			sensorlocation.push_back(Point(x,y,z));
			fprintf(fp, "%lf %lf %lf\n", x, y, z);
		}
	}
	fclose(fp);

}

void Field::calAvgNeighborsize() // Calculate the average neighbor size
{
	double totalDeg = 0;
	double totalDeg2 = 0;
	double totalOverallNodeDegree = 0;

	vector<int> degrees;
	vector<int> degrees2;

	int idx1 = 0, idx2 = 0;

	for( int i = 0; i < nSensors; i++) {
		Sensor &s = sensorPool[i];
		totalOverallNodeDegree += s.neighbors.size();
	}



	UniformRandVar  samplerand;
	for(int i = 0; i < nSensors; i++) {
		if (samplerand.value() < 0.1) {
			degrees.push_back(sensorPool[i].neighbors.size());
			degrees2.push_back(sensorPool[i].getMultiHopNeighborhoodSize(2));
		}
	}

	make_heap(degrees.begin(), degrees.end());
	double avg1;
	double avg2;


	totalDeg=0;
	totalDeg2=0;
	for(int i = 0; i < degrees.size(); i++) {
		totalDeg += degrees[i];
	}

	avg1 = totalDeg / (double)degrees.size();

	for(int i = 0; i < degrees2.size(); i++) {
		totalDeg2 += degrees2[i];
	}

	avg2 = totalDeg2 / (double)degrees2.size();

	avg1HopNeighborhoodSize = avg1;
	avg2HopNeighborhoodSize = avg2;
	printf("the avg1hop is ;%f\n",avg1);
	printf("the avg2hop is :%f\n",avg2);
}


void Field::generateSensors3D(const char* topoFile)
{

	switch(topologyIndex)
	{
	case 0:
		globalField->initSensors8(topoFile);
		break;
	case 1:
		globalField->initSensorsSmile(topoFile);
		break;
	case 2:
		globalField->initSensorsTorus(topoFile);
		break;
	case 3:
		globalField->initSensorsHourGlass(topoFile);
		break;
	case 4:
		globalField->initSensorscrossRing(topoFile);
		break;
	}
}

void Field::readSensorLocation2D(const char * topoFile)// read the sensor location 
{
	vector<Point> validPoints;

	if(topoFile) {
		FILE *fp;
		fp = fopen(topoFile, "r");
		char line[128];
	
		if (!fp)  {
			fprintf(stderr, "no topo file %s found\n", topoFile);
			exit(0);
		}
		memset(line,0,sizeof(line));
		fgets(line,sizeof(line),fp);
		sscanf(line,"%d",&nSensors);
		do {
			memset(line, 0, sizeof(line));
			fgets(line, sizeof(line), fp);
			if(!isspace(*line) && (*line)) {
				Point s;
				sscanf(line, "%lf %lf %lf", &s.x, &s.y, &s.z);
				validPoints.push_back(s);
			}
		}while(!feof(fp));
		fclose(fp);
		nSensors = validPoints.size();
		printf("nSensor = %d\n", nSensors);
	}

	sensorPool = new Sensor[nSensors];
	for(int i = 0; i < validPoints.size(); i++){
		sensorPool[i].init(i, validPoints[i]);
		sensorPool[i].field = this;
	}
	printf("The original sensor number:%d\n",nSensors);
}



void Field::display2DResult() 
{
	printf("display mode\n");
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH );
	
	// Window position (from top corner), and size (width and height)
	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( 1024, 728 );
	glutCreateWindow( "music" );
	// Initialize OpenGL rendering modes
	initRendering2D();
	printf("1\n");
	// Set up callback functions for key presses
	glutKeyboardFunc( myKeyboardFunc2D );
	glutSpecialFunc( mySpecialKeyFunc2D );
	// Set up the callback function for resizing the window
	glutReshapeFunc( resizeWindow2D);
	printf("2\n");
	// Call this for background processing
	glutIdleFunc( drawScene2D );
	// Call this whenever the window needs redrawing
	glutDisplayFunc( drawScene2D );
	fprintf(stdout, "Arrow keys control viewpoint.\n");
	// Start the main loop.  glutMainLoop never returns.
	glutMainLoop(  );
}

void Field::display3DResult()
{
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
	// Window position (from top corner), and size (width and height)
	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( 1024, 728 );
	glutCreateWindow( "3D" );
	// Initialize OpenGL rendering modes
	printf("1\n");
	initRendering3D();
	// Set up callback functions for key presses
	glutKeyboardFunc( myKeyboardFunc3D );
	glutSpecialFunc( mySpecialKeyFunc3D );
	// Set up the callback function for resizing the window
	glutReshapeFunc( resizeWindow3D );
	printf("2\n");
	// Call this for background processing
	glutIdleFunc( drawScene3D );
	// Call this whenever the window needs redrawing
	glutDisplayFunc( drawScene3D );
	fprintf(stdout, "Arrow keys control viewpoint.\n");
	// Start the main loop.  glutMainLoop never returns.
	glutMainLoop(  );
}


void Field::inputParameters2D(int index)
{
	switch(index) {
		case 0:
			COMM_RANGE=11;
			LANDMARK_SPACING=3;
			break;
		case 1:
			COMM_RANGE=12;
			LANDMARK_SPACING=3;
			break;
		case 2:
			COMM_RANGE=10;
			LANDMARK_SPACING=3;
			break;
			break;
		case 3:
			//COMM_RANGE=9.5;
		    COMM_RANGE=9.5;
			LANDMARK_SPACING=4;

			break;
	}

}

void Field::ouPutConnectivity()
{
	FILE *fp=fopen("connectivityInformation.txt","w");
	fprintf(fp,"#%d\n",nSensors);
	for(int i =0; i < nSensors; i ++) {
		Sensor *s=sensorPool+i;
		fprintf(fp,"%d\n",s->index);
		for(int j=0; j<s->neighbors.size();j++) {
			Sensor *s1= s->neighbors[j];
			fprintf(fp,"~%d\n",s1->index);
		}
	}
	fclose(fp);
}



void Field::iterativeLocalization()// iterative localization
{
	    int notchPush=0;
		globalRound = 0; // Round for iterative localization
		int totoalNumLocalized = 0;
		int *floodedFlags = new int[nSensors];
		memset(floodedFlags, 0, sizeof(int) * nSensors);

		bool complete = true;

		do {
			printf("*** current round %d **** \n", globalRound);
			complete = true;

			for(int i = 0; i < nSensors; i++) {
				Sensor* s = sensorPool + i;
				if (s->nextRound == globalRound && s->initLandmark) { 

					if (!floodedFlags[s->index]) {
						s->localizationFlood(true);
					}

					totalNumFloodings++;
					floodedFlags[s->index] = 1;
					s->oldConfidence = s->localizationConfidence;
					complete = false;     
				}
			}

			if (complete) {
				if (initLandmarksLocalized()) { // All the initlandmarks localized or not
					break;
				}
				else {
					complete = false;
					notchThreshold += edgeLandmarks.size() * 0.05; // Increase the initlandmark notchThreshold
					notchPush++;
					printf("current notch threshold is %g =================== \n", notchThreshold);


					if (notchThreshold >= edgeLandmarks.size() *THRESHOLD) {
						break;
					}

					for(int i = 0; i < nSensors; i++) {
						Sensor* s = sensorPool + i;
						if (s->localizationConfidence < 0.7) {
							s->updateSpMap();// All the nodes estimate the paths quality
						}
					}
				}
			}

			int numLocalized = 0;
			for(int i = 0; i < nSensors; i++) {
				Sensor* s = sensorPool + i;

				if (s->localizationConfidence >= 0.7) continue;	// If the node is well localized

				s->multilateration();// Localization for this node

				if (s->localizationConfidence > s->oldConfidence)  	{ 
					numLocalized++;
					totoalNumLocalized++;
					if (!s->isEdgeLandmark())  { // If this node is an initlandmark and is also on the edge
						s->oldConfidence = s->localizationConfidence;
					}
				}
			}

			printf("%d nodes have been localized in this round:###########\n", numLocalized);


			checkLoclizedResult();// Check the temporary localized results

			scheduleFlooding(); // Flooding for newly localized near edge initlandmarks in the next round


			if (initLandmarksLocalized()) {
				printf("all init landmarks localized --------------------\n");
				break;
			}

			if (globalRound >= 100) { // If there are too many rounds
				break;
			}
			globalRound++;

		} while(!complete);

		// The possible end of this iterative process: 1) All initlandmarks localized, 2) The current notchThreshold is too high,3) Too many rounds




		delete [] floodedFlags;
		iterativeLocalizationFloodings = totalNumFloodings - findNotchFloodings;//
		printf("the total flooding in the iterative process:%d\n",iterativeLocalizationFloodings);
        // Output the unlocalized initlandmark
		for(SCITER iter = initLandmarks.begin(); iter != initLandmarks.end(); ++iter) {
			Sensor *s = *iter;
			if (s->localizedPosition == INVALID_POINT) {
				printf("it is a invalid position\n");
				printf("%d,%d",s->index,s->spMap.size());
			}
		}

		FILE *fp=fopen("localizedresult.txt","w");
		for ( int i =0 ; i<nSensors; i++) {
			Sensor *s=sensorPool +i;
			fprintf(fp,"%f %f %f\n",s->localizedPosition.x,s->localizedPosition.y,s->localizedPosition.z);
		}
	   fclose(fp);
}


void Field::determineSeeds2D()//choose three seeds for 2D
{
	for(SCITER iter = initLandmarks.begin(); iter != initLandmarks.end(); ++iter) {			
		(*iter)->localizationFlood(true);
	}

	int j;
	map<int, int> notchFreeNumNeighborsMap;
	map<int, int> notchFreeRadiusMap;

	double tempNumber=0;
	double tempradius=0;
	int *levelSize = new int[nSensors];

	for(SCITER si = initLandmarks.begin(); si != initLandmarks.end(); ++si) {
		Sensor * s = *si;

		int scope = nSensors;
		int totalSize = 0;
		memset(levelSize, 0, sizeof(int) * nSensors);

		for(j = 0; j < nSensors; j++) {

			if(j == s->index) continue;

			int d = sensorPool[j].spMap[s->index].hopcount;
			levelSize[d]++;

			if (sensorPool[j].notchDegree >= notchThreshold && d < scope) {
				scope = d;
			}				
		}

		for(j = 0; j < scope; j++) {
			totalSize += levelSize[j];
		}

		notchFreeNumNeighborsMap[s->index] = totalSize;
		notchFreeRadiusMap[s->index] = scope;		
	}
	delete [] levelSize;

	int freestNodeIdx = -1;
	int maxArea = 0;

	for(map<int, int>::const_iterator iter = notchFreeNumNeighborsMap.begin(); 
		iter != notchFreeNumNeighborsMap.end(); ++iter) {

			int nidx = (*iter).first;
			int area = (*iter).second;

			Sensor *sSeed=sensorPool+nidx;
			
			{
					if (area > maxArea) 
					{
						maxArea = area;
						freestNodeIdx = nidx;
					}
			}
	}
	Sensor* seed0 = sensorPool+freestNodeIdx;
	seed0->localizationFlood(true);


	set<Sensor*> notchFreeNeighbors;

	int freeDistance = notchFreeRadiusMap[freestNodeIdx]-1;
	sensorPool[freestNodeIdx].getMultiHopNeighborhood(freeDistance, notchFreeNeighbors);
	printf("The size of the notchfreeNeighbors:%d\n",notchFreeNeighbors.size());

	SCITER nfni;
	//first calculate avgNodeDistance
	double totalEuclideanDist = 0;
	int totalHopCount = 0;
    
	Sensor* seed1 = NULL, *seed2 = NULL, *seed3 = NULL;
    double maxDistance = 0;
	for(nfni = notchFreeNeighbors.begin(); nfni != notchFreeNeighbors.end(); ++nfni) {
		Sensor* s = *nfni;
		double d = s->shortestPathLength(seed0);
		if (d > maxDistance) {
			bool temp=true;
			if(temp) {
				maxDistance = d;
				seed1 = s;
			}
		}
	}
	seed1->localizationFlood(true);

	maxDistance = 0;

	for(nfni = notchFreeNeighbors.begin(); nfni != notchFreeNeighbors.end(); ++nfni) {
		Sensor* s = *nfni;
		if (s != seed0 && s!= seed1) {	
			double d20 = s->shortestPathLength(seed0);
			double d21 = s->shortestPathLength(seed1);
			double d = d20 + d21;
			double diff = fabs(d20 *1.5- d21);
			if (diff < 2 * 1 && d > maxDistance) {
				bool temp=true;
				if(temp)
				{
					maxDistance = d;
					seed2 = s;
				}
			}
		}	
	}
	seed2->localizationFlood(true);



	maxDistance=0;
	for(nfni =notchFreeNeighbors.begin();nfni!=notchFreeNeighbors.end();++nfni){
		Sensor *s=*nfni;
		if(s!=seed1&&s!=seed2){
			double d30 = s->shortestPathLength(seed1);
			double d31 = s->shortestPathLength(seed2);
			double d = d30 + d31;
			double diff=fabs(d30-d31);
			if(diff<2*1&&d>maxDistance){
				bool temp=true;
				if(temp)
				{
					maxDistance = d;
					seed3 = s;
				}
			}
		}
	}
	seed3->localizationFlood(true);

	seed3->setSeed();
	seed1->setSeed();
	seed2->setSeed();

	printf(" the index of the seeds\n");
	printf("%d<->%d<->%d\n",seed1->index,seed2->index,seed3->index);
}


void Field::seedCoordinates2D()// Seed coordinates for 2D
{

	double a,b,c,d,e,f,g;
	vector<int > seedIndex;
	for( int i =0; i < nSensors; i++) {
		Sensor *s=sensorPool+i;
		if(s->seed==true)
			seedIndex.push_back(s->index);//record the seeds in the vector
	}

	Sensor *s0=NULL,*s1=NULL,*s2=NULL;
	if(seedIndex.size()==3){
		s0=sensorPool+seedIndex[0];
		s1=sensorPool+seedIndex[1];
		s2=sensorPool+seedIndex[2];
	}

	a=s1->shortestPathLength(s0);// The hop count between the seeds
	b=s2->shortestPathLength(s0);
	c=s2->shortestPathLength(s1);

	averageDistance=0;

	double totalDistance=0;
	double totalCount=0;

	totalCount +=a;
	totalCount +=b;
	totalCount +=c;

	totalDistance +=s0->location.distance(s1->location);
	totalDistance +=s0->location.distance(s2->location);
	totalDistance +=s1->location.distance(s2->location);
	
	averageDistance=totalDistance/totalCount; // The distance one-hop represents

	s0->localizedPosition=Point(s0->location.x/averageDistance,s0->location.y/averageDistance,s0->location.z/averageDistance);
	s1->localizedPosition=Point(s1->location.x/averageDistance,s1->location.y/averageDistance,s1->location.z/averageDistance);
	s2->localizedPosition=Point(s2->location.x/averageDistance,s2->location.y/averageDistance,s2->location.z/averageDistance);

	for(int i = 0; i < nSensors; i++) {			
		Sensor* s = sensorPool + i;
		s->spMap.clear();
	}
	printf("the co:%f<->%f<->%f\n",s0->localizedPosition.x,s0->localizedPosition.y);
	printf("the co:%f<->%f<->%f\n",s1->localizedPosition.x,s1->localizedPosition.y);
	printf("the co:%f<->%f<->%f\n",s2->localizedPosition.x,s2->localizedPosition.y);

}




void Field::coordinatesTransform2D()// transform localized results
{
	for ( int i =0; i<nSensors; i++) {
		Sensor *s=sensorPool +i;
		s->finalLocalization=Point(s->localizedPosition.x*averageDistance,s->localizedPosition.y*averageDistance,s->localizedPosition.z*averageDistance);
	}

}




void Field::errorRrport()// calculate the error
{
    double error=0;
	for ( int i =0; i< nSensors ; i++) {
		Sensor *s = sensorPool +i;
		if (s->localizedPosition != INVALID_POINT) {
           error += s->location.distance(s->finalLocalization)/COMM_RANGE;
		}

		else {
			error += diameter;
		}
	}

	printf (" the total average error : %f\n",error/nSensors);

	double totalErr = 0;
	vector<double> errs;

	for( int i = 0; i < nSensors; i++) {
		double err = sensorPool[i].localizationError();
		//		totalDegree += sensorPool[i].neighbors.size();
		totalErr += err;
		errs.push_back(err);
	}

	sort(errs.begin(), errs.end());
	double avgErr = totalErr/ nSensors;
	//	double avg_node_degree = totalDegree/nSensors;
	int idx1 = int(0.95 * nSensors);
	int idx2 = int(0.05 * nSensors);
	int idxMedian = int(0.5 * nSensors);

	
	printf("\n --------------------------------------------------\n"
		"avgErr %lf, 5-prtl Err %lf, median %lf, 95-prtl Err %lf,, total rounds %d\n", avgErr, 
		errs[idx2], errs[idxMedian], errs[idx1], globalRound);

}


void Field::dvError()
{
	double error=0;
	for ( int i =0; i< nSensors ; i++) {
		Sensor *s = sensorPool +i;
		{
			error += s->localizedPointDvHop.distance( s->location )/COMM_RANGE;
		}
	}

	printf (" the total error : %f\n",error/nSensors);




	double totalErr = 0;
	//	double totalDegree = 0;
	vector<double> errs;

	for( int i = 0; i < nSensors; i++) {
		Sensor *s=sensorPool+i;
		double err = s->localizedPointDvHop.distance(s->location)/COMM_RANGE;
		//		totalDegree += sensorPool[i].neighbors.size();
		totalErr += err;
		errs.push_back(err);
	}

	sort(errs.begin(), errs.end());
	double avgErr = totalErr/ nSensors;
	//	double avg_node_degree = totalDegree/nSensors;
	int idx1 = int(0.95 * nSensors);
	int idx2 = int(0.05 * nSensors);
	int idxMedian = int(0.5 * nSensors);

	printf("\n --------------------------------------------------\n"
		"avgErr %lf, 5-prtl Err %lf, median %lf, 95-prtl Err %lf, total rounds %d\n", avgErr, 
		errs[idx2], errs[idxMedian], errs[idx1], globalRound);


	int failnumber=0;
	for ( int i =0 ; i < nSensors ; i++) {
		Sensor *s =sensorPool +i;
		if ( s-> finalLocalization == INVALID_POINT)
			failnumber++;
	} 
	printf(" the total fail number is : %d\n",failnumber);

}
void Field::inputConnectivity()
{

	FILE *fp = fopen("connectivityInformation.txt", "r");

	char line[1024] = {0};
	if (!fp)  {
		printf(" no file of the connectvityInformation.txt\n");
		exit(0);
	}



	memset(line, 0, sizeof(line));
	fgets(line, sizeof(line), fp);	
	if (*line == '#') {
		sscanf(&line[1], "%d", &nSensors);
		printf(" the total number of sensors :%d",nSensors);
		sensorPool = new Sensor[nSensors];
		memset(line, 0, sizeof(line));
	}

	fgets(line,sizeof(line),fp);
	do {
		if(!isspace(*line)&&(*line)) {
			Sensor s;
			sscanf(line,"%d",&s.index);
			sensorPool[s.index].init(s.index);
			sensorPool[s.index].field=this;

			if(feof(fp))
				break;

			do {
				fgets(line,sizeof(line),fp);
				if(*line !='~')
					break;
				else {
					if( line[1]!='~') {
						int coord = atoi(line +1);
						sensorPool[s.index].neighborIndex.push_back(coord);
					}
				}
			}while(!feof(fp));
		}
	}while(!feof(fp));


	fclose(fp);


	FILE *fp1=fopen("connectivityInformation.txt","w");


	for(int i =0; i < nSensors ;i++) {
		Sensor *s=sensorPool+i;
		for(int j =0;j<s->neighborIndex.size();j++) {
			Sensor *s1=sensorPool+s->neighborIndex[j];
			s->neighbors.push_back(s1);
			fprintf(fp1,"%d/",s1->index);
		}
		fprintf(fp1,"\n");
	}
}

void Field::inputParametersForConnectivityFile()
{
	if( threeDimension == false) {
	  LANDMARK_SPACING = 4;
	}
	else {
          LANDMARK_SPACING =3;
	}
}


void Field::outputResult()
{
	printf(" the total notch flooding:%d\n",findNotchFloodings);
	printf(" the total localization flooding:%d\n",iterativeLocalizationFloodings);
}


void Field::localizationDvHop2D()
{

	vector <int > seedIndex;
	for ( int i =0; i< nSensors; i++) {
		Sensor *s = sensorPool +i;
		if (s->seed ==true) {
			s->localizationFlood();
			s->localizedPointDvHop=s->location;
			seedIndex.push_back(s->index);
		}
	}

	Sensor *seed0=sensorPool+seedIndex[0];
	Sensor *seed1=sensorPool+seedIndex[1];
	Sensor *seed2=sensorPool+seedIndex[2];

	printf("The seed ID : %d %d %d\n",seed0->index,seed1->index,seed2->index);


	double totalDistance=0;
	totalDistance += seed0->location.distance(seed1->location);
	totalDistance += seed0->location.distance(seed2->location);
	totalDistance += seed1->location.distance(seed2->location);
	int totalHop=0;
	totalHop += seed0->shortestPathLength(seed1);
	totalHop += seed0->shortestPathLength(seed2);
	totalHop += seed1->shortestPathLength(seed2);

	avgNodeDistanceDvHop =totalDistance/totalHop;

	printf("The average hop distance:%f\n",avgNodeDistanceDvHop); 

	for ( int i =0 ;i< nSensors ; i++) {
		Sensor *s =sensorPool +i;
		if( s->seed ==false)
			s->localizedPointDvHop=s->multiateration2DDvHop(s->spMap, 0.02);
		printf("ID %d: %f %f %f\n",s->index,s->localizedPointDvHop.x,s->localizedPointDvHop.y,s->localizedPointDvHop.z);
	}
	FILE *fp=fopen("dvhopResult2D.txt","w");
	for ( int i =0; i< nSensors ; i++ ) {
		Sensor *s =sensorPool +i;
		fprintf( fp, "%f %f %f\n",s->localizedPointDvHop.x,s->localizedPointDvHop.y,s->localizedPointDvHop.z);
	}
}


void Field::localizationDvHop3D()
{
	vector <int > seedIndex;
	for ( int i =0; i< nSensors; i++) {
		Sensor *s = sensorPool +i;
		if (s->seed ==true) {
			s->localizationFlood();
			s->localizedPointDvHop=s->location;
			seedIndex.push_back(s->index);
		}
	}

	if (seedIndex.size() <4) {
		printf( " not enough seed number\n");
		exit(1);
	}
 
	Sensor *seed0=sensorPool+seedIndex[0];
	Sensor *seed1=sensorPool+seedIndex[1];
	Sensor *seed2=sensorPool+seedIndex[2];
	Sensor *seed3=sensorPool+seedIndex[3];


	double totalDistance=0;
	totalDistance += seed0->location.distance(seed1->location);
	totalDistance += seed0->location.distance(seed2->location);
	totalDistance += seed1->location.distance(seed2->location);
	totalDistance += seed0->location.distance(seed3->location);
	totalDistance += seed1->location.distance(seed3->location);
	totalDistance += seed2->location.distance(seed3->location);

	int totalHop=0;
	totalHop += seed0->shortestPathLength(seed1);
	totalHop += seed0->shortestPathLength(seed2);
	totalHop += seed1->shortestPathLength(seed2);
    totalHop += seed0->shortestPathLength(seed3);
	totalHop += seed1->shortestPathLength(seed3);
	totalHop += seed2->shortestPathLength(seed3);

    avgNodeDistanceDvHop =totalDistance/totalHop;


	for ( int i =0 ;i< nSensors ; i++) {
		Sensor *s =sensorPool +i;
		if( s->seed ==false)
			s->localizedPointDvHop=s->multiateration3DDvHop(s->spMap, 0.02);
		printf("ID %d: %f %f %f\n",s->index,s->localizedPointDvHop.x,s->localizedPointDvHop.y,s->localizedPointDvHop.z);
	}
	FILE *fp=fopen("dvhopResult3D.txt","w");
	for ( int i =0; i< nSensors ; i++ ) {
		Sensor *s =sensorPool +i;
		fprintf( fp, "%f %f %f\n",s->localizedPointDvHop.x,s->localizedPointDvHop.y,s->localizedPointDvHop.z);
	}
}





void Field::initSensorsHourGlass(const char *topoFile)
{

	vector<Point> validPoints;
	vector <Point> newPoints1;
	double interval =5;
    NormalRandVar randv(0,0.2*interval);

	for ( int i =-15; i<15; i++) {
		for ( int j =-15; j<15; j++) {
			for ( int k =0; k < 10; k++){
                 double x = i *interval + randv.value();
				 double z = j *interval +randv.value();
				 double y = k * interval + randv.value();
				if ( sqrt ( x*x + z*z)<= 15*interval-y)
					validPoints.push_back(Point(x,y,z));
			}
		}
	}


	for ( int i =0; i< validPoints.size(); i++) {
		Point a= validPoints[i];
		double tempx=a.x;
		double tempy=-a.y+19*interval;
		double tempz=a.z;
		newPoints1.push_back(Point(tempx,tempy,tempz));
	}


	FILE *fp=fopen(topoFile,"w");
	for ( int i =0; i<validPoints.size(); i ++) {
		Point a =validPoints[i];
		fprintf(fp,"%f %f %f\n",a.x,a.y,a.z);
		Point b =newPoints1[i];
		fprintf(fp,"%f %f %f\n",b.x,b.y,b.z);
	}

  

	fclose(fp);
}


void Field::initSensorscrossRing(const char * topoFile)
{


	float MajorRadius = 70;
	float MinorRadius = 30;
	vector <Point> sensorlocation;
	vector < Point> validPoints;
	FILE *fp=fopen(topoFile, "w+");
	nSensors=40000;
	/*sensorlocation.clear();*/
	int i, j = 0;
	int nRow;
	int nCol;
	int nWid;
	nRow = nCol = nWid = CEILING(pow(nSensors, 1.0/3.0));
	double interval = double(Y_RANGE)/double(nRow+1);
	COMM_RANGE = 2.2 * double(Y_RANGE)/double(nRow+1);
	NormalRandVar randv(0, interval * 0.25);

for(i = 0; i < nSensors; i++) 
	{
		int idxRow = i / nWid / nWid;
		int idxCol = i / nWid - (i / nWid / nWid) * nWid;
		int idxWid = i - (i / nWid) * nWid;
		double gridX = (idxRow + 1) * interval;
		double gridY = (idxCol + 1) * interval;
		double gridZ = (idxWid + 1) * interval;
		double x = gridX + randv.value() - X_RANGE/2;
		double y = gridY + randv.value() - Y_RANGE/2;
		double z = gridZ + randv.value() - Z_RANGE/2;
		
		
		float d = sqrt(x*x + z*z);
		float phi1 = asin(y/MinorRadius);
		float phi2 = PI - phi1;
		float r1 = MajorRadius + MinorRadius*cos(phi1);
		float r2 = MajorRadius + MinorRadius*cos(phi2);
		float d1 = (r1>r2?r1:r2);
		float d2 = (r1<r2?r1:r2);
		if(d < d1 && d > d2)
		{			
			fprintf(fp, "%f %f %f\n", x, y, z);
			if (!(z >=-20 && z <=20))
			{
				double a =x;
				double b =z;
				double c =y;
				fprintf(fp,"%f %f %f\n",a,b,c);
			}
		}
	}

	fclose(fp);
}

void Field::seedCoordinates3D() // Seed coordinates for 3D
{
	vector < int> seedIndex;
	for ( int i =0; i<nSensors; i++) {
		Sensor *s=sensorPool+i;
		if ( s->seed == true)
			seedIndex.push_back(s->index);
	}


	 averageDistance=0;
	double totalcount=0;
	double totaldistance=0;

	Sensor *s0=sensorPool+seedIndex[0];
	Sensor *s1=sensorPool+seedIndex[1];
	Sensor *s2=sensorPool+seedIndex[2];
	Sensor *s3=sensorPool+seedIndex[3];

    s0->localizationFlood();
	s1->localizationFlood();
	s2->localizationFlood();
	s3->localizationFlood();

	totaldistance += s0->location.distance(s1->location);
	totaldistance += s0->location.distance(s2->location);
	totaldistance += s0->location.distance(s3->location);
	totaldistance += s1->location.distance(s2->location);
	totaldistance += s1->location.distance(s3->location);
	totaldistance += s2->location.distance(s3->location);


	totalcount +=s0->shortestPathLength(s1);
	totalcount +=s0->shortestPathLength(s2);
	totalcount +=s0->shortestPathLength(s3);
	totalcount +=s1->shortestPathLength(s2);
	totalcount +=s1->shortestPathLength(s3);
	totalcount +=s2->shortestPathLength(s3);

	averageDistance =totaldistance/totalcount;


	s0->localizedPosition=Point(s0->location.x/averageDistance,s0->location.y/averageDistance,s0->location.z/averageDistance);
	s1->localizedPosition=Point(s1->location.x/averageDistance,s1->location.y/averageDistance,s1->location.z/averageDistance);
	s2->localizedPosition=Point(s2->location.x/averageDistance,s2->location.y/averageDistance,s2->location.z/averageDistance);
	s3->localizedPosition=Point(s3->location.x/averageDistance,s3->location.y/averageDistance,s3->location.z/averageDistance);

	printf("The localized result\n");
	printf("%f %f %f\n",s0->localizedPosition.x,s0->localizedPosition.y,s0->localizedPosition.z);
	printf("%f %f %f\n",s1->localizedPosition.x,s1->localizedPosition.y,s1->localizedPosition.z);
	printf("%f %f %f\n",s2->localizedPosition.x,s2->localizedPosition.y,s2->localizedPosition.z);
	printf("%f %f %f\n",s3->localizedPosition.x,s3->localizedPosition.y,s3->localizedPosition.z);



	for ( int i =0 ; i<nSensors; i++) {
		Sensor *s=sensorPool+i;
		s->spMap.clear();
	}

}


void Field::coordinatesTransform3D()
{
	for ( int i =0; i<nSensors; i++) {
		Sensor *s=sensorPool +i;
		s->finalLocalization=Point(s->localizedPosition.x*averageDistance,s->localizedPosition.y*averageDistance,s->localizedPosition.z*averageDistance);
	}
}





