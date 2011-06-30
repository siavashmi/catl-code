 
#include "sensor.h"
#include "random.h"
#include "assert.h"
#include "common.h"
#include "geometry3D.h"
#include "field.h"

Sensor::Sensor()// construction function of the class Sensor, 
{	
	index = -1;							//½ÚµãµÄID
	location = INVALID_POINT;
	neighbors.clear();				
	
	children.clear();
	parent = NULL;

	broadcastCovered = 0;
	isUnderNotch = false;
	notchDegree = 0;
	isNotchPoint = false;

	onEdge = false;
	nearEdge = -1;
	initLandmark = false;

	level = -1;
	oldConfidence = 0;
	
	localizedPosition = INVALID_POINT;
	localizationConfidence = 0;
	tempLocation = INVALID_POINT;
	nextRound = int(IMPOSSIBLE_VALUE);
	lastScheduledRound = 0;
	
	seed = false;
}

Sensor::~Sensor()
{
}


void Sensor::clear()// some parameters to null. 
{
	index = -1;							//ID
	location = INVALID_POINT;
	tempLocation = INVALID_POINT;
	neighbors.clear();				
	twoHopNeighbors.clear();
	children.clear();
	parent = NULL;
	field = NULL;
	candidateParents.clear();
	
	broadcastCovered = 0;
	isUnderNotch = false;
	notchDegree = 0;
	isNotchPoint = false;
	
	onEdge = false;
	nearEdge = -1;
	initLandmark = false;
	
	level = -1;
	oldConfidence = 0;
	
	localizedPosition = INVALID_POINT;
	localizationConfidence = 0;
	tempLocation = INVALID_POINT;
	nextRound = int(IMPOSSIBLE_VALUE);
	lastScheduledRound = 0;
	
	seed = false;
}

void Sensor::updateNeighbors()
{
	neighbors.clear();
	
	for(int i = 0; i < field->nSensors; i++) {
		Sensor & s = field->sensorPool[i];
		if (i != index && location.distance(s.location) < COMM_RANGE ){
			neighbors.push_back(&s);
		}
	}
	return;
}


void Sensor::updateNeighborsQuasiUBG()// Quasi unit-disk model the parameters need to be input from the user or not?
{
	
	int i;
	neighbors.clear();

	UniformRandVar rand;
	double prob = (2 - QUASI_UBG_ALPHA) / 3.1;

	for(i = 0; i < field->nSensors; i++) {
		Sensor & s = field->sensorPool[i];
		if (i == index) 
			continue;

		double d = location.distance(s.location);

		if (d < (1 - QUASI_UBG_ALPHA) * COMM_RANGE ){
			neighbors.push_back(&s);
		}
		else if (d < (1 + QUASI_UBG_ALPHA) * COMM_RANGE) {
			if (rand.value() < prob) {
				neighbors.push_back(&s);
			}
		}
	}
}


void Sensor::init(int index_, Point location_)//the index and location, 
{
	index = index_;
	location = location_;
}

void Sensor::init(int index_)
{
	index =index_;
}



void Sensor::getMultiHopNeighborhood(int maxHopCount, set<Sensor*> & nHopNeighbors)
{	
	int hopCount = 0;

	if (maxHopCount == 2 && !twoHopNeighbors.empty()) {
		nHopNeighbors = twoHopNeighbors;
	}
	else if (maxHopCount == 1) {
		nHopNeighbors.insert(neighbors.begin(), neighbors.end());
	}

	set<Sensor*> *frontier = new set<Sensor*>;
	frontier->insert(this);

	while (hopCount < maxHopCount && !frontier->empty()) {
		
		set<Sensor*> *newFrontier = new set<Sensor*>;

		for(SCITER iter = frontier->begin(); iter != frontier->end(); ++iter) {
			Sensor *s = (*iter);
			
			for(int j = 0; j < s->neighbors.size(); j++) {
				Sensor *n = s->neighbors[j];	
				if (n != this) {
					nHopNeighbors.insert(n);
					newFrontier->insert(n);
				}					
			}
		}

		delete frontier;		
		frontier = newFrontier;			
		hopCount++;
	}

	delete frontier;// deal with the pointers, initialize and delete.

	if (maxHopCount == 2 && twoHopNeighbors.empty()) {
		twoHopNeighbors = nHopNeighbors;
	}
}

int Sensor::getMultiHopNeighborhoodSize(int hopCount)// find the size, 
{
	set<Sensor*> nghs;
	if (hopCount == 1) {
		return neighbors.size();
	}
	else if (hopCount == 2 && !twoHopNeighbors.empty()) {
		return twoHopNeighbors.size();
	}
	else
	{
		getMultiHopNeighborhood(hopCount, nghs);
		return nghs.size();
	}
}


void Sensor::localizationFlood(bool updatePathQuality, int hopnum)// the bool updatePathQuality is not used...
{	
	int i, j;

	for(i = 0; i < field->nSensors; i++) {
		field->sensorPool[i].broadcastCovered = 0;
		field->sensorPool[i].candidateParents.clear();
		field->sensorPool[i].parent = NULL;
		field->sensorPool[i].children.clear();
	}


	int depth = 0;// depth of the nodes from the root landmark
	broadcastCovered = 1;// identify whether the node is broadcast covered or not, 
	set<Sensor*> *frontier = new set<Sensor*>;// pointer , with the pointer new pointer
	frontier->insert(this);// insert the current sensor(the landmark calling the function)
		
	while (!frontier->empty()) {// the pointer frontier and the newfrontier is changed at the end of this part.
		
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
		}// there are still nodes not covered,so the depth of the tree needs to be higher

		//let each node in the new level choose a parent
		for(SCITER iNewFron = newFrontier->begin(); iNewFron != newFrontier->end(); ++iNewFron) {
				
			Sensor* c = *iNewFron;
			Sensor *parentChosen = NULL;

			//choose a random parent from the candidate set
			int randidx = rand() % c->candidateParents.size();
			int count = 0;

			for(SCITER iCan = c->candidateParents.begin(); iCan != c->candidateParents.end(); ++iCan, ++count) {
				Sensor* can = *iCan;
				
				if (count == randidx) {
					parentChosen = can;
					break;
				}
			}
        
			c->broadcastCovered = 1;// the nodes in the pointer newfrontier, 

			//path quality estimating
			pathInfo pathI;
			pathI.hopcount = depth;
			pathI.nexthop = parentChosen->index;
			pathI.quality = 0;

			//if (updatePathQuality) {
				Sensor* curNode = c;
				vector<Sensor*> path;
				
				path.push_back(c);
				
				while (curNode != this){
					Sensor *curParent;
					
					if (curNode == c) {
						curParent = parentChosen;
					}
					else {
						curParent = field->sensorPool + (curNode->spMap[index]).nexthop;
					}
					
					path.push_back(curParent);
					curNode = curParent;
				};
				
				pathI.quality = field->estimatePathQuality(path);
			//}

			c->spMap.insert(DEST_2_PATH_INFO::value_type(index, pathI));
		}
		
		frontier = newFrontier;

		if (depth > hopnum)
			break;
	}

	delete frontier;


	
	for(i = 0; i < field->nSensors; i++) {
		field->sensorPool[i].broadcastCovered = 0;
	}// for further use, 
	
}
void Sensor::updateSpMap()// with the confidence test, from the map read the shortest map , then get the quality 
{

	DEST_2_PATH_INFO::iterator mIter;

	//printf("update spmap %d ", index);

	for(mIter = spMap.begin(); mIter != spMap.end(); ++mIter){
		int destid = (*mIter).first;
		pathInfo & pInfo = (*mIter).second; 
		
		vector<Sensor*> sPath;
		getShortestPath(field->sensorPool + destid, sPath);
		sPath.insert(sPath.begin(), this);

		pInfo.quality = field->estimatePathQuality(sPath);
	}
}


double Sensor::localizationError()//just get the error , note here the error is related with the communication range, 
{
	if (localizedPosition == INVALID_POINT) {
		return field->diameter;
		// this better be changed to the 2*diameter. a little bigger than the real distance, 
	}
	else {
		double d = finalLocalization.distance(location);
		return d / COMM_RANGE;
	}
}



//NOTE: the path does not include the caller
void Sensor::getShortestPath(Sensor* dest, vector<Sensor*> & path)
{
	if (spMap.find(dest->index) == spMap.end()) {
		fprintf(stderr, "sp not found in cache!\n");
		return;
	}
	if ( this->isNeighbor(dest->index))
	{
       path.clear();
       return;
	}	
	Sensor* curNode = field->sensorPool + spMap[dest->index].nexthop;
	
	while (curNode != dest){
		path.push_back(curNode);
		curNode = field->sensorPool + (curNode->spMap[dest->index]).nexthop;
	};
}

void Sensor ::findNotches(char *dumbfile) // Finding notch flood from the node
{
	int i, j;
	int depth = 0;
	int maxDepth = 0;
	set<Sensor*>*  nodeLevels[MAXNUM_LEVELS] = {0};  

	for(i = 0; i < field->nSensors; i++) { // Initialize the state for the nodes
		field->sensorPool[i].broadcastCovered = (i == index ? 1 : 0);
		field->sensorPool[i].isNotchPoint = false;
		field->sensorPool[i].isUnderNotch = false;
		field->sensorPool[i].level = -1;
		field->sensorPool[i].parent = NULL;
		field->sensorPool[i].children.clear();
		field->sensorPool[i].candidateParents.clear();
		memset(field->sensorPool[i].subtreeSizes, 0, sizeof(field->sensorPool[i].subtreeSizes));
	}

	set<Sensor*> *frontier = new set<Sensor*>;
	frontier->insert(this);

	while (!frontier->empty()) {

		nodeLevels[depth] = frontier;

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
			if (depth > MAXNUM_LEVELS) {
				fprintf(stderr, "flooding beyond pre-allocated level!\n");
				exit(0);
			}
		}

		// Each node in this new level chooses a parent by random number
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
		}

		frontier = newFrontier;
	}

	delete frontier;


	maxDepth = depth;
	if (maxDepth > field->diameter) { // Change the network diameter
		field->diameter = maxDepth;
	}

	// Record the data in the file
	if (dumbfile) {

		FILE *fp = fopen(dumbfile, "w");

		if (fp) {
			fprintf(fp, "#%d\n", index);
			for(i = 0; i < field->nSensors; i++) {
				Sensor & s = field->sensorPool[i];
				if (s.index != index) {
					fprintf(fp, "%d\t%d\n", s.index, s.parent->index);
				}
			}
		}
		fclose(fp);
	}	

	//initialize the subtree size vector for each node
	for(i = 0; i < field->nSensors; i++) {
		for(j = 0; j < MAXNUM_LEVELS; j++)
			field->sensorPool[i].subtreeSizes[j] = 1;
	}

	for(i = 0; i < field->nSensors; i++) { 
		Sensor *curNode = &field->sensorPool[i];
		if (curNode->broadcastCovered && curNode->index != index) {
			depth = 0; // Initialize the depth for the nodes
			while (curNode != this){
				Sensor *curParent = curNode->parent;
				depth++;

				for(j = depth; j < maxDepth; j++) { 
					curParent->subtreeSizes[j]++; // Calculate the depth from flooding initlandmark to this node
				}

				curNode = curParent;
			};
		}
	}

	// From the bottom level
	for(i = maxDepth; i >= 0; i--) {
		set<Sensor*> *nodes = nodeLevels[i];

		for(set<Sensor*>::const_iterator niter = nodes->begin(); niter != nodes->end(); ++niter) {
			Sensor* s = (*niter);

			if (s->index == index) continue;

			// Large subtree
			for(int k = 3; k < maxDepth - s->level; k++) {

				int subtrsize = s->subtreeSizes[k];
				int judgeSize=0;
				if( threeDimension == false) { // different scale for 2D and 3D
					judgeSize=15;
				}
				else {
					judgeSize=10;
				}
			
				if (subtrsize > judgeSize * k) { 

					set<Sensor*> vincinity;
					s->getMultiHopNeighborhood(2, vincinity);

					for(SCITER iN = vincinity.begin(); iN != vincinity.end(); ++iN) {
						Sensor* sn = *iN;
						if (sn->onEdge) {    // 
							s->nearEdge = 1;
							break;
						}
					}

					if (s->nearEdge != 1)  // If the node is far from the boundary, it is definitely not an notch 
						break;

					s->isNotchPoint = true;
					s->notchDegree += 1; // Add the notch degree

					// Propagate notch effect to s's neighbors
					for(SCITER iNgh = vincinity.begin(); iNgh != vincinity.end(); ++iNgh) {
						Sensor *ngh = (*iNgh);
						if (ngh->isNeighbor(s->index)) { // One hop ngh
							if( threeDimension == false)
							ngh->notchDegree += 0.4;
							else 
								ngh->notchDegree +=0.2;
						}
						else {
							if ( threeDimension == false )
							ngh->notchDegree += 0.1;
							else
								;
						}
					}

					break;
				}
			}

			if (s->isNotchPoint) {
				// Mark this notch 
				set<Sensor*> curLevel = s->children;
				int totalNumLevels = 0;
				while (!curLevel.empty()) {
					totalNumLevels++;
					SCITER iS;
					for(iS = curLevel.begin(); iS != curLevel.end(); ++iS) {
						((Sensor*)(*iS))->isUnderNotch = true;
					}

					set<Sensor*> nextLevel;
					for(iS = curLevel.begin(); iS != curLevel.end(); ++iS) {
						Sensor *cs = (*iS);
						for(SCITER cIter = cs->children.begin();
							cIter != cs->children.end(); ++cIter){
								Sensor *sTemp = *cIter;
								if (sTemp->level > cs->level) {
									nextLevel.insert(*cIter);
								}
						}
					}
					curLevel.clear();
					curLevel = nextLevel;
				}

				// Delete this for the upper level nodes
				Sensor* notchParent = s->parent;
				int ancestorLevel = 0;
				while (notchParent != this) {
					ancestorLevel++;

					for(j = 0; j <= totalNumLevels; j++) {
						notchParent->subtreeSizes[j + ancestorLevel] -= s->subtreeSizes[j];
					}

					for(; j < maxDepth; j++) {
						notchParent->subtreeSizes[j + ancestorLevel] -= s->subtreeSizes[totalNumLevels];
					}

					notchParent = notchParent->parent;

				}
			}
		}
	}

	for(i = 0; i < maxDepth; i++) {
		if (nodeLevels[i]) {
			nodeLevels[i]->clear();
			delete nodeLevels[i];
		}
	}
}





bool Sensor::isEdgeLandmark()
{
	if (initLandmark && onEdge)
		return true;
	else return false;
}



void Sensor::setSeed()
{
	localizationConfidence = 1;
	oldConfidence = 0;
	initLandmark = true;
	nextRound = 0;
	seed = true;
}



void Sensor::multilateration()
{

	int i;
	vector<int> lms;
	double lmConf = 0;

	selectBestLandmarks(lms, lmConf);
    if(lmConf==0)
		return;


	DEST_2_PATH_INFO lmMap;
	
	for(i = 0; i < lms.size(); i++) {		//just deal with the best landmarks
		lmMap[lms[i]] = spMap[lms[i]];
	}

	bool succeed = false;//the bool 
	double solutionGap = 0;
	Point pt=Point(0,0,0);

	if( threeDimension == false)
	 pt = multilaterationLocalization2D(lmMap, 0.02, 5, succeed);
	else 
		pt=multilaterationLocalization3D(lmMap, 0.02, 5, succeed);
	


	double maxDistance = 0;
	int m;
	
	
	//do some extra correctness check 
	if (succeed)  {
		
		if(lms.size() >= 6) {

			vector<Point> results;
			
			for(i = 0; i < 50; i++) {		//
				
				lmMap.clear();
				random_shuffle(lms.begin(), lms.end());
				int randomnum = mymax(lms.size() / 2, 4);
				for(int count = 0; count < randomnum; count++) {
					int idx = lms[count];
					lmMap[idx] = spMap[idx];		//pathInfo
				}
				Point lc=Point (0,0,0);
				if( threeDimension == false)
				lc = multilaterationLocalization2D(lmMap, 0.02, 5, succeed);
				else 
					lc=multilaterationLocalization3D(lmMap, 0.02, 5, succeed);
				if (succeed) {
					results.push_back(lc);
				}
			}
			
			if (results.size() != 0) {
				for(m = 0; m < results.size() - 1; m++) {
					for(int n = 1; n < results.size(); n++) {
						double d = results[m].distance(results[n]);
						if (d > maxDistance) {
							maxDistance = d;							
							if (maxDistance > 2*field->diameter) break;
							// this also be changed to the diameter*2
						}
					}
					
					if (maxDistance > 2*field->diameter)	break;
					// this also be changed to the diameter*2
				}
			}
			
			if (maxDistance > 2*field->diameter) {	// this also be changed to the diameter*2				
			}
			else {					
				localizedPosition = pt;
				oldConfidence = localizationConfidence;
				localizationConfidence = lmConf;
			}
		}
		else {				
			localizedPosition = pt;
			oldConfidence = localizationConfidence;
			localizationConfidence = lmConf;
			
		}
		
	}
}

//for 3D
Point Sensor::multilaterationLocalization3D(map<int, pathInfo> &dst, double precise, 
											  int multisolver, bool & locationsucceed)// the references can be changed according to problem.
{//the final function, 
	int temprounds = 0;
	int endlesslooprounds = 1000;
	double avg_hop_dist = field->avgNodeDistance;
	UniformRandVar rvalue;
	locationsucceed = true;
	Point estimation;			//this is the estimate position
	Point s0;
	vector<Point> basepoints;
	basepoints.clear();
	double offset = 3.8;
	vector<Point> estimationset;
	estimationset.clear();
	map<int, pathInfo>::iterator pos;
	double testdist = 0;
	double virtualx = 0;
	double virtualy = 0;
	double virtualz = 0;
//	int step = 4;
	int iterator_num = 1000;
	int sign = iterator_num;			//the tag of the end of computing
	int solvertag = multisolver;	
	bool s0tag = 0;
	double A = 0, B = 0, C = 0, D = 0, E = 0, F = 0, G = 0, H = 0, I = 0, J = 0;
	double ES = 0;
	double ES0 = 0;
	map<double, Point>::iterator col;
	int i = 0, j = 0;
	pos = dst.begin();
	int m = dst.size();
	for(pos = dst.begin(); pos != dst.end(); pos++)
	{
		i = pos->first;		//index
		s0.x += field->sensorPool[i].localizedPosition.x/m;
		s0.y += field->sensorPool[i].localizedPosition.y/m;
		s0.z += field->sensorPool[i].localizedPosition.z/m;//average of the landmarks
	}
	Point basepoint;//estimation? 
	basepoints.push_back(s0);
	basepoint.x = s0.x;
	basepoint.y = s0.y - offset;
	basepoints.push_back(basepoint);
	basepoint.y = s0.y + offset;
	basepoints.push_back(basepoint);
	basepoint.x = s0.x - offset;
	basepoint.y = s0.y;
	basepoints.push_back(basepoint);
	basepoint.x = s0.x + offset;
	basepoints.push_back(basepoint);
	basepoint.x = s0.x;
	basepoint.y = s0.y;
	basepoint.z = s0.z + offset;
	basepoints.push_back(basepoint);
	basepoint.z = s0.z - offset;
	basepoints.push_back(basepoint);

	basepoint.z = s0.z;
	basepoint.x = s0.x + offset;
	basepoint.y = s0.y + offset;
	basepoints.push_back(basepoint);
	basepoint.y = s0.y - offset;
	basepoints.push_back(basepoint);
	basepoint.x = s0.x - offset;
	basepoints.push_back(basepoint);
	basepoint.y = s0.y + offset;
	basepoints.push_back(basepoint);

	basepoint.y = s0.y;
	basepoint.z = s0.z + offset;
	basepoints.push_back(basepoint);
	basepoint.z = s0.z - offset;
	basepoints.push_back(basepoint);
	basepoint.x = s0.x + offset;
	basepoints.push_back(basepoint);
	basepoint.z = s0.z + offset;
	basepoints.push_back(basepoint);
	basepoint.x = s0.x;
	basepoint.y = s0.y + offset;
	basepoints.push_back(basepoint);
	basepoint.z = s0.z - offset;
	basepoints.push_back(basepoint);
	basepoint.y = s0.y - offset;
	basepoints.push_back(basepoint);
	basepoint.z = s0.z + offset;
	basepoints.push_back(basepoint);
	basepoint.x = s0.x + offset;
	basepoints.push_back(basepoint);
	basepoint.x = s0.x - offset;
	basepoints.push_back(basepoint);
	basepoint.z = s0.z - offset;
	basepoints.push_back(basepoint);
	basepoint.y = s0.y + offset;
	basepoints.push_back(basepoint);
	basepoint.z = s0.z + offset;
	basepoints.push_back(basepoint);
	basepoint.x = s0.x + offset;
	basepoints.push_back(basepoint);
	basepoint.z = s0.z - offset;
	basepoints.push_back(basepoint);
	basepoint.y = s0.y - offset;
	basepoints.push_back(basepoint);// what is the choice of the offset? compared with the hop-euclidean ?
	                                //  27 in all ?

	while(solvertag)// what is the use of the multisolver ?   
	{
		sign = iterator_num;
		s0 = basepoints[multisolver - solvertag];//s0 is in the basepoint, used for what?
		while(sign)
		{
			s0tag = 0;
			ES0 = 0;
			ES = 0;
			A = 0;
			B = 0;
			C = 0;
			D = 0;
			E = 0;
			F = 0;
			G = 0;
			H = 0;
			I = 0;
			J = 0;
			for(pos = dst.begin(); pos != dst.end(); pos++)
			{
				double temp0 = s0.distance(field->sensorPool[pos->first].localizedPosition) - pos->second.hopcount*avg_hop_dist;
				ES0 += pow(temp0, 2);//the sum of all the errors from the reference points, 
			}
			for(pos = dst.begin(); pos != dst.end(); pos++)
			{
				double d = s0.distance(field->sensorPool[pos->first].localizedPosition);
				double x1 = field->sensorPool[pos->first].localizedPosition.x;
				double y1 = field->sensorPool[pos->first].localizedPosition.y;
				double z1 = field->sensorPool[pos->first].localizedPosition.z;
				double A1 = (s0.x - x1)/d;
				double B1 = (s0.y - y1)/d;
				double C1 = (s0.z - z1)/d;
				double testd = pos->second.hopcount*avg_hop_dist;
				double D1 = d - pos->second.hopcount*avg_hop_dist - (s0.x*(s0.x-x1) + s0.y*(s0.y-y1) + s0.z*(s0.z-z1))/d;
				A += A1*A1;
				B += 2*A1*B1;
				C += B1*B1;
				D += 2*A1*C1;
				E += 2*B1*C1;
				F += 2*A1*D1;
				G += 2*B1*D1;
				H += C1*C1;
				I += 2*C1*D1;
				J += D1*D1;
			}
			if((8*A*C*H+2*B*E*D-2*C*D*D-2*A*E*E-2*H*B*B) > 0 && (4*A*C-B*B) > 0 && A > 0 )
			{
				virtualx = (2*C*D*I+E*E*F+2*B*G*H-4*C*F*H-B*E*I-D*E*G)/(8*A*C*H+2*B*E*D-2*C*D*D-2*A*E*E-2*H*B*B);
				virtualy = (D*D*G+2*A*E*I+2*B*F*H-4*A*G*H-D*E*F-B*D*I)/(8*A*C*H+2*B*E*D-2*C*D*D-2*A*E*E-2*H*B*B);
				virtualz = (2*C*D*F+2*A*E*G+B*B*I-4*A*C*I-B*D*G-B*E*F)/(8*A*C*H+2*B*E*D-2*C*D*D-2*A*E*E-2*H*B*B);
				estimation = Point(virtualx, virtualy, virtualz);
				s0tag = 1;
			}			
			else
			{
				s0.x = estimation.x + rvalue.value()*10;
				s0.y = estimation.y + rvalue.value()*10;
				s0.z = estimation.z + rvalue.value()*10;
			}
			if(s0tag)
			{
	 			for(pos = dst.begin(); pos != dst.end(); pos++)
				{
					double temp0 = estimation.distance(field->sensorPool[pos->first].localizedPosition) - pos->second.hopcount*avg_hop_dist;
						ES += pow(temp0, 2);
				}
//				printf("The contrast of ES0 and ES is %f\n", fabs(ES0 - ES));
				if(fabs(ES0 - ES) < precise)
				{
					estimationset.push_back(estimation);
					break;
				}
				else
					s0 = estimation;
//			printf("The value of sign is %d\n", sign);
				sign--;
			}
			else {
				temprounds ++;
				if ( temprounds > endlesslooprounds) {
					locationsucceed = false;
					localizationConfidence =0;
					localizedPosition =INVALID_POINT;
					return INVALID_POINT;
				}
			}
		}
		solvertag--;
		if(sign == 0)
		{
			//printf("Failed!\n");
			locationsucceed = false;
			estimationset.push_back(estimation);
			break;
		}
	}
	if(locationsucceed)
	{
		for(i = 0; i < multisolver-1; i++)
		{
			for(j = i+1; j < multisolver; j++)
			{
				testdist = estimationset[i].distance(estimationset[j]);
				if(testdist > 0.5)
				{
//					printf("The multisolver distance between %d and %d is %lf\n", i, j, testdist);
					locationsucceed = false;
					break;
				}
			}
			if(!locationsucceed)
				break;
		}
	}
	estimation = estimationset[0];

	return estimation;
}
	

double Sensor::getLandmarkSetConfidence(vector<int> & landmarks)	// get the confidence of the landmark set	

{
	double conf = 0;
	int i, j;
	vector<Sensor*> lms;
	

	if( threeDimension == false) {
		if (landmarks.size() <3)
			return 0;
	}
	else
	{
	if(landmarks.size() < 4) {		
		return 0;
	}
	}

	double avgPathQuality = 0;
	double totalQuality = 0;

	for(i = 0; i < landmarks.size(); i++) {
		totalQuality += spMap[landmarks[i]].quality;
	}

	avgPathQuality = totalQuality / landmarks.size();

	conf = avgPathQuality;

	for(i = 0; i < landmarks.size(); i++) 
		lms.push_back(field->sensorPool + landmarks[i]);

	//inter-landmark statistics
	int maxHops = 0;
	int totalHops = 0;
	double avgHops;

	int num = lms.size();		//at first num = 4

	//node to landmark distance
	int minDist = 0xffff;

	assert(lms.size() > 1);		//true passed

	for(i = 0; i < num - 1; i++) {		//find maxhop between landmarks
		for(j = i + 1; j < num; j++) {
			int hops = lms[i]->spMap[lms[j]->index].hopcount;
			if (hops > maxHops) 
				maxHops = hops;

			totalHops += hops;
		}
	}

	avgHops = double(totalHops) * 2 / (num * num - num);
	
	for(i = 0; i < num; i++) {		//find the mindist between s and landmarks
		int dist = spMap[lms[i]->index].hopcount;

		if (dist < minDist) 
			minDist = dist;
	}

	double temp = double(minDist) / maxHops;

	if (temp > 1.5) {		//judge whether the result can be trusted or not , the distance (sensor to the landmarks and the landmark themselves)
		conf = 0;
	}

	return conf;
}

void Sensor::selectBestLandmarks(vector<int> & lms, double & confidence)
{

	int i;	

	vector<int> goodCandidates, selectedLms, bestSelection;		//save index
	DEST_2_PATH_INFO::iterator iter;

	for(iter = spMap.begin(); iter != spMap.end(); ++iter) {		
		Sensor *lm = field->sensorPool + (*iter).first;
		double qual = (*iter).second.quality;

		if (qual > 0) {
			
			bool closeLm = false;
			for(i = 0; i < goodCandidates.size(); i++) {		//at first candidate.size() = 0
				Sensor *gc = field->sensorPool + goodCandidates[i];
				if (lm->localizedPosition.distance(gc->localizedPosition) < 2 * field->avgNodeDistance) {		//landmark can't be close
					closeLm = true;// model, 
					break;
				}
			}
			
			if (closeLm) 
				continue;// omit the nearby landmarks, 
			
			if ((*iter).second.hopcount > field->diameter) // also the diameter
				continue;

			goodCandidates.push_back(lm->index);		//lm->index is a landmark index
		}
	}



	if( threeDimension == false) {
		if(goodCandidates.size()<3) {
			localizationConfidence=0;
			return;
		}
	}

	else{
	if (goodCandidates.size() < 4) {			//at least need 4 landmarks to localize
		localizationConfidence = 0;
		return;
	     }
	}
	int count = 10;// chose the 10 landmarks for calculation

	if (goodCandidates.size() <= count) {
		lms = goodCandidates;
		confidence = getLandmarkSetConfidence(lms);		//avgPathQuality or 0
	}
	else {
		double maxConf = 0;
		for(i = 0; i < 20; i++) {
			randomlyPickLandmarkSet(goodCandidates, count, selectedLms);// random , random , random 
			double tempConf = getLandmarkSetConfidence(selectedLms);

			if (tempConf > maxConf) {
				maxConf = tempConf;
				bestSelection = selectedLms;
			}
		}

		lms = bestSelection;
		confidence = maxConf;
	}
}

void Sensor::randomlyPickLandmarkSet(const vector<int> & candidates, int numPicks, vector<int> & selection)
{
	vector<int> nodeset = candidates;
	assert(numPicks <= candidates.size());
	selection.clear();
	random_shuffle(nodeset.begin(), nodeset.end());
	
	for(int i = 0; i < numPicks; i++) {	
		selection.push_back(nodeset[i]);
	}
}



int Sensor::shortestPathLength(Sensor* dest)
{
	if (spMap.find(dest->index) == spMap.end()) {		
		return 0;
	}
	Sensor* curNode = field->sensorPool + spMap[dest->index].nexthop;
	int total_hop=1;
	while (curNode != dest){
		total_hop++;
		curNode = field->sensorPool + (curNode->spMap[dest->index]).nexthop;
	};
	return total_hop;
}






void Sensor::edgeNeighborSize() // If the number of  edge nodes in the neighborhood is small, it is not an edge node
{
    int num=0;
	for(int i=0;i<neighbors.size();i++) {
		Sensor *s= neighbors[i];
		if( s->onEdge == true) {
			num++;
		}
	}
	if(num<1)
		onEdge=false;
}

Point Sensor::multilaterationLocalization2D(map<int, pathInfo> &dst, double precise, 
								 int multisolver, bool & locationsucceed)
{
	double avg_hop_dist = field->avgNodeDistance;

	avg_hop_dist=1;
	int temprounds=0;
	int endlessLoopRounds=1000;

	
	UniformRandVar rvalue;
	locationsucceed = true;
	Point estimation(0,0,0);			//this is the estimate position
	Point s0;
	vector<Point> basepoints;
	basepoints.clear();
	double offset = 10;
	vector<Point> estimationset;
	estimationset.clear();
	map<int, pathInfo>::iterator pos;
	double testdist = 0;
	double virtualx = 0;
	double virtualy = 0;
	int iterator_num = 100;
	int sign = iterator_num;			//the tag of the end of computing
	int solvertag = multisolver;	
	double A = 0, B = 0, C = 0, D = 0, E = 0, F = 0;
	double ES = 0;
	double ES0 = 0;
	double result = 0;
	double minresult = 0xffffffff;
	map<double, Point>::iterator col;
	bool s0tag = 0;
	int i = 0, j = 0;
	pos = dst.begin();
	int m = dst.size();
	for(pos = dst.begin(); pos != dst.end(); pos++)
	{
		i = pos->first;		//index
		s0.x += field->sensorPool[i].localizedPosition.x/m;
		s0.y += field->sensorPool[i].localizedPosition.y/m;
		s0.z=0;
	}
	Point basepoint;
	basepoint.z=0;
	basepoints.push_back(s0);
	basepoint.x = s0.x;
	basepoint.y = s0.y - offset;
	basepoints.push_back(basepoint);
	basepoint.y = s0.y + offset;
	basepoints.push_back(basepoint);
	basepoint.x = s0.x - offset;
	basepoint.y = s0.y;
	basepoints.push_back(basepoint);
	basepoint.x = s0.x + offset;
	basepoints.push_back(basepoint);
	basepoint.y = s0.y + offset;
	basepoints.push_back(basepoint);
	basepoint.y = s0.y - offset;
	basepoints.push_back(basepoint);
	basepoint.x = s0.x - offset;
	basepoints.push_back(basepoint);
	basepoint.y = s0.y + offset;
	basepoints.push_back(basepoint);


	while(solvertag)
	{
		sign = iterator_num;
		s0 = basepoints[multisolver - solvertag];
		while(sign)
		{
			s0tag = 0;
			ES0 = 0;
			ES = 0;
			A = 0;
			B = 0;
			C = 0;
			D = 0;
			E = 0;
			F = 0;
			for(pos = dst.begin(); pos != dst.end(); pos++)
			{
				double temp0 = s0.distance(field->sensorPool[pos->first].localizedPosition) - pos->second.hopcount*avg_hop_dist;
				ES0 += pow(temp0, 2);
			}
			for(pos = dst.begin(); pos != dst.end(); pos++)
			{
				double d = s0.distance(field->sensorPool[pos->first].localizedPosition);
				double x1 = field->sensorPool[pos->first].localizedPosition.x;
				double y1 = field->sensorPool[pos->first].localizedPosition.y;
				double A1 = (s0.x - x1)/d;
				double B1 = (s0.y - y1)/d;
				double landmarkdist = pos->second.hopcount*avg_hop_dist;
				Point landmark = field->sensorPool[pos->first].localizedPosition;
				double C1 = d - pos->second.hopcount*avg_hop_dist - (s0.x*(s0.x-x1) + s0.y*(s0.y-y1))/d;
				A += A1*A1;
				B += 2*A1*B1;
				C += B1*B1;
				D += 2*A1*C1;
				E += 2*B1*C1;
				F += C1*C1;
			}
			if((4*A*C - B*B) > 0)
			{
				virtualy = (B*D - 2*A*E)/(4*A*C - B*B);
				virtualx = (B*E - 2*C*D)/(4*A*C - B*B);
				estimation = Point(virtualx, virtualy,0);
				s0tag = 1;
			}
			else
			{
				s0.x = estimation.x + rvalue.value()*1;
				s0.y = estimation.y + rvalue.value()*1;
			}
			if(s0tag)
			{
	 			for(pos = dst.begin(); pos != dst.end(); pos++)
				{
					double temp0 = estimation.distance(field->sensorPool[pos->first].localizedPosition) - pos->second.hopcount*avg_hop_dist;
					ES += pow(temp0, 2);
				}

				if(fabs(ES0 - ES) < precise)
				{
					estimationset.push_back(estimation);
					break;
				}
				else
					s0 = estimation;
				sign--;
			}
			else {
				temprounds++;
				if(temprounds > endlessLoopRounds) {
					printf(" The current maximum rounds : %d \n" , temprounds);
                    locationsucceed = false;
					localizedPosition = INVALID_POINT;
					return INVALID_POINT;
					
				}
			}
		}
		solvertag--;
		if(sign == 0)
		{
			locationsucceed = false;
			estimationset.push_back(estimation);
			break;
		}
	}
	if(locationsucceed)
	{
		for(i = 0; i < multisolver-1; i++)
		{
			for(j = i+1; j < multisolver; j++)
			{
				testdist = estimationset[i].distance(estimationset[j]);
				if(testdist > 0.5)
				{
					locationsucceed = false;
					break;
				}
			}
			if(!locationsucceed)
				break;
		}
	}
	estimation = estimationset[0];

	  return estimation;

}


bool Sensor::isNeighbor(int index_)
{

   
	for (int i =0;i <neighbors.size(); i++) {
		Sensor *sNeighbor = neighbors[i];
		if(sNeighbor->index == index_)
			return true;
	}

	return false;
}


Point Sensor ::multiateration2DDvHop(map<int, pathInfo> &dst, double precise)
{

	double avg_hop_dist = field->avgNodeDistanceDvHop;


	UniformRandVar rvalue;


	Point s0=Point(0,0,0);


	map<int, pathInfo>::iterator pos;
	Point estimation;

	double virtualx = 0;
	double virtualy = 0;

	int iterator_num = 100;
	int sign = iterator_num;			//the tag of the end of computing


	double A = 0, B = 0, C = 0, D = 0, E = 0, F = 0;
	double ES = 0;
	double ES0 = 0;
	double result = 0;


	bool s0tag = 0;
	int i = 0, j = 0;
	pos = dst.begin();
	int m = dst.size();
	for(pos = dst.begin(); pos != dst.end(); pos++)
	{
		i = pos->first;		//index
		s0.x += field->sensorPool[i].localizedPointDvHop.x/m;
		s0.y += field->sensorPool[i].localizedPointDvHop.y/m;
		s0.z=0;
	}


	while(sign)
	{
		s0tag = 0;
		ES0 = 0;
		ES = 0;
		A = 0;
		B = 0;
		C = 0;
		D = 0;
		E = 0;
		F = 0;
		for(pos = dst.begin(); pos != dst.end(); pos++)
		{
			double temp0 = s0.distance(field->sensorPool[pos->first].localizedPointDvHop) - pos->second.hopcount*avg_hop_dist;
			ES0 += pow(temp0, 2);
		}
		for(pos = dst.begin(); pos != dst.end(); pos++)
		{
			double d = s0.distance(field->sensorPool[pos->first].localizedPointDvHop);
			double x1 = field->sensorPool[pos->first].localizedPointDvHop.x;
			double y1 = field->sensorPool[pos->first].localizedPointDvHop.y;
			double A1 = (s0.x - x1)/d;
			double B1 = (s0.y - y1)/d;

			double C1 = d - pos->second.hopcount*avg_hop_dist - (s0.x*(s0.x-x1) + s0.y*(s0.y-y1))/d;
			A += A1*A1;
			B += 2*A1*B1;
			C += B1*B1;
			D += 2*A1*C1;
			E += 2*B1*C1;
			F += C1*C1;
		}
		if((4*A*C - B*B) > 0)
		{
			virtualy = (B*D - 2*A*E)/(4*A*C - B*B);
			virtualx = (B*E - 2*C*D)/(4*A*C - B*B);
			estimation = Point(virtualx, virtualy,0);
			s0tag = 1;
		}
		else
		{
			s0.x = estimation.x + rvalue.value()*10;
			s0.y = estimation.y + rvalue.value()*10;
			s0.z=0;
		}
		if(s0tag)
		{
			for(pos = dst.begin(); pos != dst.end(); pos++)
			{
				double temp0 = estimation.distance(field->sensorPool[pos->first].localizedPointDvHop) - pos->second.hopcount*avg_hop_dist;
				ES += pow(temp0, 2);
			}
			//				printf("The contrast of ES0 and ES is %f\n", fabs(ES0 - ES));
			if(fabs(ES0 - ES) < precise)
			{
				return estimation;

			}
			else
				s0 = estimation;
			//				printf("The value of sign is %d\n", sign);
			sign--;
		}
	}
	if(sign == 0)
	{
		return estimation;
	}

	return estimation;

}


Point Sensor:: multiateration3DDvHop( map<int, pathInfo> &dst, double precise) 
{
	double avg_hop_dist = field->avgNodeDistanceDvHop;

	UniformRandVar rvalue;

	Point s0 = Point(0,0,0);

	map <int ,pathInfo> :: iterator pos;
	Point estimation;

    bool s0tag = 0;
	double virtualx=0;
	double virtualy=0;
	double virtualz=0;

	int iterator_num=100;
	int sign = iterator_num;

	double A = 0, B = 0, C = 0, D = 0, E = 0, F = 0, G = 0, H = 0, I = 0, J = 0;
	double ES = 0;
	double ES0 = 0;

	int i =0,j=0;

	pos=dst.begin();
	int m = dst.size();

	for ( pos = dst.begin(); pos!= dst.end(); pos++) {
		i = pos->first;		//index
		s0.x += field->sensorPool[i].localizedPointDvHop.x/m;
		s0.y += field->sensorPool[i].localizedPointDvHop.y/m;
		s0.z += field->sensorPool[i].localizedPointDvHop.z/m;//average of the landmarks
	} 

	while(sign)
	{
		s0tag = 0;
		ES0 = 0;
		ES = 0;
		A = 0;
		B = 0;
		C = 0;
		D = 0;
		E = 0;
		F = 0;
		G = 0;
		H = 0;
		I = 0;
		J = 0;
		for(pos = dst.begin(); pos != dst.end(); pos++)
		{
			double temp0 = s0.distance(field->sensorPool[pos->first].localizedPointDvHop) - pos->second.hopcount*avg_hop_dist;
			ES0 += pow(temp0, 2);//the sum of all the errors from the reference points, 
		}
		for(pos = dst.begin(); pos != dst.end(); pos++)
		{
			double d = s0.distance(field->sensorPool[pos->first].localizedPointDvHop);
			double x1 = field->sensorPool[pos->first].localizedPointDvHop.x;
			double y1 = field->sensorPool[pos->first].localizedPointDvHop.y;
			double z1 = field->sensorPool[pos->first].localizedPointDvHop.z;
			double A1 = (s0.x - x1)/d;
			double B1 = (s0.y - y1)/d;
			double C1 = (s0.z - z1)/d;
			double testd = pos->second.hopcount*avg_hop_dist;
			double D1 = d - pos->second.hopcount*avg_hop_dist - (s0.x*(s0.x-x1) + s0.y*(s0.y-y1) + s0.z*(s0.z-z1))/d;
			A += A1*A1;
			B += 2*A1*B1;
			C += B1*B1;
			D += 2*A1*C1;
			E += 2*B1*C1;
			F += 2*A1*D1;
			G += 2*B1*D1;
			H += C1*C1;
			I += 2*C1*D1;
			J += D1*D1;
		}
		if((8*A*C*H+2*B*E*D-2*C*D*D-2*A*E*E-2*H*B*B) > 0 && (4*A*C-B*B) > 0 && A > 0 )
		{
			virtualx = (2*C*D*I+E*E*F+2*B*G*H-4*C*F*H-B*E*I-D*E*G)/(8*A*C*H+2*B*E*D-2*C*D*D-2*A*E*E-2*H*B*B);
			virtualy = (D*D*G+2*A*E*I+2*B*F*H-4*A*G*H-D*E*F-B*D*I)/(8*A*C*H+2*B*E*D-2*C*D*D-2*A*E*E-2*H*B*B);
			virtualz = (2*C*D*F+2*A*E*G+B*B*I-4*A*C*I-B*D*G-B*E*F)/(8*A*C*H+2*B*E*D-2*C*D*D-2*A*E*E-2*H*B*B);
			estimation = Point(virtualx, virtualy, virtualz);
			s0tag = 1;
		}			
		else
		{
			s0.x = estimation.x + rvalue.value()*10;
			s0.y = estimation.y + rvalue.value()*10;
			s0.z = estimation.z + rvalue.value()*10;
		}
		if(s0tag)
		{
			for(pos = dst.begin(); pos != dst.end(); pos++)
			{
				double temp0 = estimation.distance(field->sensorPool[pos->first].localizedPointDvHop) - pos->second.hopcount*avg_hop_dist;
				ES += pow(temp0, 2);
			}

			if(fabs(ES0 - ES) < precise)
			{
				return estimation;
			}
			else
				s0 = estimation;

			sign--;
		}
	}

	if(sign == 0)
	{
		return estimation;
	}

	return estimation;
}


void Sensor::areaFlooding(int hopcount)// the bool updatePathQuality is not used...
{	

	int i, j;

	for(i = 0; i < field->nSensors; i++) {
		field->sensorPool[i].broadcastCovered = 0;
		field->sensorPool[i].candidateParents.clear();
		field->sensorPool[i].parent = NULL;
		field->sensorPool[i].children.clear();
	}





	int depth = 0;// depth of the nodes from the root landmark
	broadcastCovered = 1;// identify whether the node is broadcast covered or not, 
	set<Sensor*> *frontier = new set<Sensor*>;// pointer , with the pointer new pointer
	frontier->insert(this);// insert the current sensor(the landmark calling the function)

	while (!frontier->empty()) {// the pointer frontier and the newfrontier is changed at the end of this part.

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
		}// there are still nodes not covered,so the depth of the tree needs to be higher

		//let each node in the new level choose a parent
		for(SCITER iNewFron = newFrontier->begin(); iNewFron != newFrontier->end(); ++iNewFron) {

			Sensor* c = *iNewFron;
			Sensor *parentChosen = NULL;

			//choose a random parent from the candidate set
			int randidx = rand() % c->candidateParents.size();
			int count = 0;

			for(SCITER iCan = c->candidateParents.begin(); iCan != c->candidateParents.end(); ++iCan, ++count) {
				Sensor* can = *iCan;

				if (count == randidx) {
					parentChosen = can;
					break;
				}
			}

			c->broadcastCovered = 1;// the nodes in the pointer newfrontier, 

			//path quality estimating
			pathInfo pathI;
			pathI.hopcount = depth;
			pathI.nexthop = parentChosen->index;
			pathI.quality = 0;

			Sensor* curNode = c;
			vector<Sensor*> path;

			path.push_back(c);

			while (curNode != this){
				Sensor *curParent;

				if (curNode == c) {
					curParent = parentChosen;
				}
				else {
					curParent = field->sensorPool + (curNode->areaspMap[index]).nexthop;
				}

				path.push_back(curParent);
				curNode = curParent;
			};

			pathI.quality = field->estimatePathQuality(path);


			c->areaspMap.insert(DEST_2_PATH_INFO::value_type(index, pathI));
		}

		frontier = newFrontier;

		if (depth > hopcount){
			delete frontier;
		return;
		}
			
	}


	delete frontier;



	for(i = 0; i < field->nSensors; i++) {
		field->sensorPool[i].broadcastCovered = 0;
	} 

}


