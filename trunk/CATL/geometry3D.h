#ifndef GEOMETRY_H_
#define GEOMETRY_H_


#include <vector>
#include "common.h"
#include "math.h"

using namespace std;

class Point
{
public:
	Point() {x = 0; y = 0;}
	Point(double x_, double y_, double z_) : x(x_), y(y_), z(z_){};
	~Point(){};
	
	double X() {return x;}
	double Y() {return y;}
	double Z() {return z;}

	double x;
	double y;
	double z;

	double a, b, c, d;

	inline double distance(Point P) { 
		double xDist = pow(P.x - x, 2);
		double yDist = pow(P.y - y, 2);
		double zDist = pow(P.z - z, 2);
		return sqrt(xDist + yDist + zDist);
	};
	
	inline double distance(double X, double Y, double Z) { 		
		return distance(Point(X, Y, Z));
	};


	inline Point & operator = (const Point &right)
	{
		x = right.x;
		y = right.y;
		z = right.z;

		return *this;
	}

	inline bool operator == (const Point &right) const
	{
		return (x == right.x && y == right.y && z == right.z);
	}

	inline bool operator != (const Point &right) const
	{
		return (x != right.x || y != right.y || z != right.z);
	}

	inline bool approxInequal(const Point &right)
	{
		return (!DOUBLE_EQUAL(x, right.x) || !DOUBLE_EQUAL(y, right.y) || !DOUBLE_EQUAL(z, right.z));
	}

	inline bool approxEqual(const Point &right)
	{
		return (DOUBLE_EQUAL(x, right.x) && DOUBLE_EQUAL(y, right.y) && DOUBLE_EQUAL(z, right.z));
	}

	inline bool isValid()
	{
		return (x != IMPOSSIBLE_VALUE && y != IMPOSSIBLE_VALUE && z != IMPOSSIBLE_VALUE);
	}
};


class LineSegment
{
public:
	LineSegment(Point start_, Point end_);
	LineSegment(Point start_, double angle_);
	LineSegment();
	~LineSegment();
	
	Point start;
	Point end;
	
	double a;  //end.y - start.y
	double b;  //-(end.x - start.x)
	double c;  //-(start.x * end.y - end.x * start.y)

	void setEnds(Point start_, Point end_);

	double getX(double y);
	double getY(double x);
	double getZ(double z);

	Point getOnlinePoint(Point start, Point end, double dist);// find the points on the line, used as cleaning up 

	double distanceToPoint(Point pt);// not used

	bool containEnd(Point pt); //pt is one of the ends
	bool containPointClose(Point pt); //not used
	bool containPointExclusiveEnds(Point pt);// not used
	bool containPointOpen(Point pt);// not used
	
	bool operator == (const LineSegment &ls) 
	{
		return (a == ls.a && b == ls.b && c == ls.c);
	}
};

#endif

