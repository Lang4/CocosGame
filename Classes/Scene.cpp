#include "Scene.h"

/**
* 将像素点转化为网格
*/
GridIndex MapScene::transPixelPointToGird(const Point &point)
{
	return GridIndex(point.x / 64, point.y / 32);
}
/**
* 将网格转化为像素点
*/
Point MapScene::transGirdToPixelPoint(const GridIndex &point)
{
	return Point(point.x * 64, point.y * 32);
}
SceneManager * SceneManager::me = NULL;
SceneManager * SceneManager::getInstance()
{
	if (!me) me = new SceneManager();
	return me;
}
void SceneManager::addTo(int layerID, Node *node)
{
	if (nowTerrain)
	{
		nowTerrain->addChild(node, layerID);
	}
}

void SceneManager::remove(Node *node)
{
	if (nowTerrain)
	{
		nowTerrain->removeChild(node);
	}
}

void SceneManager::addOtherRole(IRole *role)
{
	if (role == mainRole) return;
	addTo(ROLE_LAYER, role);
	_roles.push_back(role);
}

void SceneManager::doTick(float dt)
{
	Size size = CCDirector::sharedDirector()->getVisibleSize();
	for (ROLES_ITER iter = _roles.begin(); iter != _roles.end(); ++iter)
	{
		IRole * role = *iter;
		if (role->calcDistance(mainRole) < size.width/2)
		{
			role->setVisible(true);
		}
		else
			role->setVisible(false);
	}
}

/**
* 获取攻击对象
*/
Point SceneManager::getAttackObjectPosition(Node *role)
{
	Point point;
	float distance = 0;
	for (ROLES_ITER iter = _roles.begin(); iter != _roles.end(); ++iter)
	{
		IRole * target = *iter;
		if (target){
			float temp = ccpDistance(role->getPosition(), target->getPosition());
			if (distance == 0)
			{
				distance = temp;
				point = target->getPosition();
			}
			else if (distance >= temp)
			{
				distance = temp;
				point = target->getPosition();
			}
		}
	}
	return point;
}
#include "CollisionDetection.h"
bool SceneManager::checkHurts(std::vector<IRole*>& out, Sprite *src)
{
	for (ROLES_ITER iter = _roles.begin(); iter != _roles.end(); ++iter)
	{
		IRole * role = *iter;
		if (role)
		{
			Sprite *sprite = role->getStateSprite();
			if (!sprite)
			{
				// 计算普通碰撞
			}
			else
			{
				Sprite * temp = Sprite::createWithTexture(sprite->getTexture(),sprite->getTextureRect());
				temp->setPosition(role->getPosition());
				if (ccpDistance(src->getPosition(), role->getPosition()) < 100)
				{
					out.push_back(role);
				}
				/*
				Size visibleSize = CCDirector::sharedDirector()->getVisibleSize();
				if (!rtForC)
				{
					rtForC = CCRenderTexture::create(visibleSize.width * 2, visibleSize.height * 2);
					rtForC->setPosition(ccp(visibleSize.width, visibleSize.height));
					rtForC->setVisible(false);
					rtForC->retain();
				}
				bool bb = CollisionDetection::GetInstance()->areTheSpritesColliding(src, temp, true, rtForC);
				if (bb)
				{
					return true;
				}*/
			}
		
		}
	}
	return out.size();
}


void SceneManager::addNpcByTempId(int tempid, IRole * role)
{
	_npcs[tempid] = role;
}
IRole* SceneManager::getNpcByTempId(int tempid)
{
	NPCS_ITER iter = _npcs.find(tempid);
	if (iter != _npcs.end())
	{
		return iter->second;
	}
	return NULL;
}

void SceneManager::addUserByTempId(int tempid, IRole * role)
{
	_users[tempid] = role;
}
IRole* SceneManager::getUserByTempId(int tempid)
{
	USERS_ITER iter = _users.find(tempid);
	if (iter != _users.end())
	{
		return iter->second;
	}
	return NULL;
}