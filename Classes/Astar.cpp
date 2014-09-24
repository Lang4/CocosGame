#include "Astar.h"
GridMap::GridMap()
{
	width = height = 0;
	minRadius = 12;
	initCircles();
}

void GridMap::setSize(int width, int height)
{
	this->width = width;
	this->height = height;
}
GridMap *GridMap::create(int width, int height)
{
	GridMap *map = new GridMap();
	map->setSize(width, height);
	return map;
}
GridIndex GridMap::findPath(const GridIndex & src, const GridIndex & dest, stCheckMoveAble*check)
{
	const int width = (2 * minRadius + 1);
	const int height = (2 * minRadius + 1);
	const int MaxNum = width * height;
	std::vector<int> pDisMap(MaxNum, MaxNum);
	std::vector<PathPoint> stack(MaxNum * 8 + 1);
	PathQueueHead queueHead;

	PathPoint *root = &stack[MaxNum * 8];
	root->pos = src;
	root->cc = 0;
	root->father = NULL;
	enter_queue(queueHead, root, root->cc + judge(root->pos, dest));

	int Count = 0;

	while (Count < MaxNum)
	{
		root = exit_queue(queueHead);
		if (NULL == root)
		{
			return GridIndex(-1, -1);
		}

		if (abs(root->pos.x - dest.x) <= 0 && abs(root->pos.y - dest.y) <= 0)
		{
			break;
		}
		//        std::random_shuffle(adjust.begin(),adjust.end());
		for (int i = 0; i < adjust.size(); i++)
		{
			bool bCanWalk = true;
			GridIndex tempPos = root->pos;
			tempPos.x += adjust[i].x;
			tempPos.y += adjust[i].y;
			if (moveable(tempPos, check))
			{
				PathPoint *p = root;
				while (p)
				{
					if (p->pos.x == tempPos.x && p->pos.y == tempPos.y)
					{
						bCanWalk = false;
						break;
					}
					p = p->father;
				}

				if (bCanWalk)
				{
					int cost = root->cc + 1;
					int index = (tempPos.y - dest.y + minRadius) * width + (tempPos.x - dest.x + minRadius);
					if (index >= 0
						&& index < MaxNum
						&& cost < pDisMap[index])
					{
						pDisMap[index] = cost;
						PathPoint *pNewEntry = &stack[Count * 8 + i];
						pNewEntry->pos = tempPos;
						pNewEntry->cc = cost;
						pNewEntry->father = root;
						enter_queue(queueHead, pNewEntry, pNewEntry->cc + judge(pNewEntry->pos, dest));
					}
				}
			}
		}

		Count++;
	}
	GridIndex out(-1, -1);
	if (Count < MaxNum)
	{
		while (root)
		{
			if (root->father != NULL
				&& root->father->father == NULL)
			{
				out = root->pos;
				return out;
			}
			root = root->father;
		}
	}

	return out;
}

void GridMap::initCircles()
{
	adjust.clear();
	adjust.push_back(GridIndex(0, -1));
	adjust.push_back(GridIndex(-1, 0));
	adjust.push_back(GridIndex(1, 0));
	adjust.push_back(GridIndex(0, 1));
	adjust.push_back(GridIndex(1, -1));
	adjust.push_back(GridIndex(-1, -1));
	adjust.push_back(GridIndex(-1, 1));
	adjust.push_back(GridIndex(1, 1));
}

int GridMap::judge(const GridIndex &midPos, const GridIndex &endPos)
{
	int distance = abs((long)(midPos.x - endPos.x)) + abs((long)(midPos.y - endPos.y));
	return distance;
}

void GridMap::enter_queue(PathQueueHead &queueHead, PathPoint *pPoint, int currentCost)
{
	PathQueue pNew(pPoint, currentCost);
	if (!queueHead.empty())
	{
		for (iterator it = queueHead.begin(); it != queueHead.end(); it++)
		{
			if ((*it).cost > currentCost)
			{
				queueHead.insert(it, pNew);
				return;
			}
		}
	}
	queueHead.push_back(pNew);
}

PathPoint *GridMap::exit_queue(PathQueueHead &queueHead)
{
	PathPoint *ret = NULL;
	if (!queueHead.empty())
	{
		reference ref = queueHead.front();
		ret = ref.node;
		queueHead.pop_front();
	}
	return ret;
}
bool GridMap::moveable(const GridIndex &dest, stCheckMoveAble*check)
{
	if (dest.x < 0 || dest.y < 0) return false;
	if (dest.x > width || dest.y >= this->height) return false;
	if (check && !check->exec(this, dest)) return false;
	return true;
}
