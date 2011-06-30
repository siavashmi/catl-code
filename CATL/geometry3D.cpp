#include "geometry3D.h"
#include "common.h"
#include <algorithm>
#include "assert.h"



/////////////////////////////// Line segments /////////////////////////
LineSegment::LineSegment(Point start_, Point end_)
{
	start = start_;
	end = end_;
	a = end.y - start.y;
	b = -(end.x - start.x);
	c = -(start.x * end.y - end.x * start.y);
}

LineSegment::LineSegment(Point start_, double angle_)
{
	start = start_;
	
	end.x = start.x + 10 * cos(angle_);
	end.y = start.y + 10 * sin(angle_);
	
	a = end.y - start.y;
	b = -(end.x - start.x);
	c = -(start.x * end.y - end.x * start.y);
}

LineSegment::LineSegment()
{
	start = INVALID_POINT;
	end = INVALID_POINT;
}

LineSegment::~LineSegment()
{
}

void LineSegment::setEnds(Point start_, Point end_)
{
	start = start_;
	end = end_;
	a = end.y - start.y;
	b = -(end.x - start.x);
	c = -(start.x * end.y - end.x * start.y);
}

bool LineSegment::containPointOpen(Point pt)// judge whether the point is no the line 
{
	double zero = a * pt.x + b * pt.y + c;

	double tmp = DOUBLE_EQUAL(zero, 0);
	return (DOUBLE_EQUAL(zero, 0));
}

double LineSegment::distanceToPoint(Point pt)
{

	double x1 = start.x;
	double y1 = start.y;
	double x2 = end.x;
	double y2 = end.y;
	double x0 = pt.x;
	double y0 = pt.y;

	double res = (x2-x1)*(y1-y0) - (x1-x0)*(y2-y1);
	res = myabs(res);

	res /= sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));

	return res;

	return 0;
}

bool LineSegment::containEnd(Point pt)
{
	return (start.approxEqual(pt) || end.approxEqual(pt));
}

bool LineSegment::containPointClose(Point pt)
{
	double minx = mymin(start.x, end.x);
	double maxx = mymax(start.x, end.x);
	double miny = mymin(start.y, end.y);
	double maxy = mymax(start.y, end.y);

	if (containPointOpen(pt) && 
		(pt.x > minx || DOUBLE_EQUAL(pt.x, minx)) && 
		(pt.x < maxx || DOUBLE_EQUAL(pt.x, maxx)) &&
		(pt.y > miny || DOUBLE_EQUAL(pt.y, miny)) && 
		(pt.y < maxy || DOUBLE_EQUAL(pt.y, maxy))) 
		return true;
	else
		return false;
}


bool LineSegment::containPointExclusiveEnds(Point pt)
{
	double minx = mymin(start.x, end.x);
	double maxx = mymax(start.x, end.x);
	double miny = mymin(start.y, end.y);
	double maxy = mymax(start.y, end.y);

	if (containPointOpen(pt) && 
		(pt.x > minx || DOUBLE_EQUAL(pt.x, minx)) && 
		(pt.x < maxx || DOUBLE_EQUAL(pt.x, maxx)) && 
		(pt.y > miny || DOUBLE_EQUAL(pt.y, miny)) && 
		(pt.y < maxy || DOUBLE_EQUAL(pt.y, maxy)) && 
		pt.approxInequal(start) && 
		pt.approxInequal(end)) 
		return true;
	else
		return false;
}


double LineSegment::getX(double y)
{
	assert(a != 0);
	return - (b * y + c) / a;
}

double LineSegment::getY(double x)
{
	if (!b) {
		int stop = 1;
	}
	assert(b);

	return - (a * x + c) / b;
}


Point LineSegment::getOnlinePoint(Point start, Point end, double dist)
{
	double d = start.distance(end);

	double px = start.x + (end.x - start.x) * dist / d;
	double py = start.y + (end.y - start.y) * dist / d;
	double pz = start.z + (end.z - start.z) * dist / d;

	return Point(px, py, pz);
}


