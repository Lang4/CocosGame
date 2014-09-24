#pragma once
#include "cocos2d.h"
USING_NS_CC;
class GridIndex{
public:
	int x;
	int y;
	GridIndex()
	{
		x = y = 0;
	}
	GridIndex(int x, int y) :x(x), y(y)
	{
	}
	static GridIndex make(int x, int y)
	{
		return GridIndex(x, y);
	}
};
struct PathPoint{
	GridIndex pos;
	int cc;
	PathPoint * father;
	PathPoint()
	{
		cc = 0;
		father = NULL;
	}
};
struct PathQueue{
	PathPoint *node;
	int cost;
	PathQueue(PathPoint *node, int cost)
	{
		this->node = node;
		this->cost = cost;
	}
	PathQueue(const PathQueue &queue)
	{
		node = queue.node;
		cost = queue.cost;
	}
	PathQueue & operator= (const PathQueue &queue)
	{
		node = queue.node;
		cost = queue.cost;
		return *this;
	}
};
class GridMap;
struct stCheckMoveAble{
	virtual bool exec(GridMap * map, const GridIndex &index){ return true; }
};
class GridMap{
public:
	/**
	* …Ë∂®±ﬂΩÁ
	*/
	static GridMap *create(int width, int height);
	GridMap();
	int width;
	int height;
	void setSize(int width, int height);
	GridIndex findPath(const GridIndex & src, const GridIndex & dest, stCheckMoveAble*check= NULL);
	void visit();
private:
	std::vector<GridIndex> adjust;
	int minRadius;
	typedef std::list<PathQueue> PathQueueHead;
	typedef  PathQueueHead::iterator iterator;
	typedef  PathQueueHead::reference reference;
	virtual void initCircles();
	int judge(const GridIndex &midPos, const GridIndex &endPos);
	void enter_queue(PathQueueHead &queueHead, PathPoint *pPoint, int currentCost);
	virtual bool moveable(const GridIndex &dest, stCheckMoveAble*check = NULL);
	PathPoint *exit_queue(PathQueueHead &queueHead);
};