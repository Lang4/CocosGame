#include "cocos2d.h"
#include "Map.h"
USING_NS_CC;

class RpgFollow :public Follow{
public:
	static RpgFollow *create(const CCSize & mapSize, CCNode *player)
	{
		RpgFollow *pRet = new RpgFollow();
		if (pRet && pRet->initWithTarget(player, CCRectZero))
		{
			pRet->mapSize = mapSize;
			pRet->autorelease();
			return pRet;
		}
		CC_SAFE_DELETE(pRet);
		return NULL;
	}
	virtual void step(float dt)
	{
		CC_UNUSED_PARAM(dt);
		CCSize visibleSize = CCDirector::sharedDirector()->getVisibleSize();
		CCPoint nowPos;
		if (_boundarySet)
		{
			// whole map fits inside a single screen, no need to modify the position - unless map boundaries are increased
			if (_boundaryFullyCovered)
				return;
			CCPoint tempPos = ccpSub(_halfScreenSize, _followedNode->getPosition());
			nowPos = ccp(clampf(tempPos.x, _leftBoundary, _rightBoundary),
				clampf(tempPos.y, _bottomBoundary, _topBoundary));
		}
		else
		{
			nowPos = ccpSub(_halfScreenSize, _followedNode->getPosition());
		}
		Terrain * map = (Terrain*)_target;
		float x = _target->getPositionX();
		float y = _target->getPositionY();
		if (!(nowPos.x >= 0 || nowPos.x < visibleSize.width - mapSize.width))
		{
			x = nowPos.x;
		}

		if (!(nowPos.y >= 0 || nowPos.y < visibleSize.height - mapSize.height))
		{
			y = nowPos.y;
		}
		map->setPosition(x, y);
	}
	CCSize mapSize;
	RpgFollow()
	{
	}
};
