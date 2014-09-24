#pragma once
#include "cocos2d.h"
#include "IRoleState.h"
#include "Astar.h"
#include "Map.h"
USING_NS_CC; 

/**
 * 场景
 */
class MapScene{
public:
	enum{
		_BLOCK = 1 << 0,
		_CAN_WALK = 1 << 1,
	};
	/**
	 * 获取阻挡点信息
	 */
	int getBlock(const Point &point);
	/**
	 * 从文件中加载阻挡点信息
	 */
	void readBlockInfoFromFile(const char *fileName);

	/**
	 * 将像素点转化为网格
	 */
	static GridIndex transPixelPointToGird(const Point &point);
	/**
	 * 将网格转化为像素点
	 */
	static Point transGirdToPixelPoint(const GridIndex &point);
private:
	std::vector<int> _blockInfos;
};

/**
 * 场景管理器
 */
class SceneManager{
public:
	static SceneManager *me;
	static SceneManager * getInstance();
	SceneManager()
	{
		mainRole = NULL;
		uiLayer = NULL;
		nowTerrain = NULL;
		rtForC = NULL;
	}
	// 主角
	IRole * mainRole;
	enum{
		UI_LAYER = 4,
		MAP_LAYER = 0,
		ROLE_LAYER = 1,
		EFFECT_LAYER = 2,
		POINTER_LAYER = 3,
	};
	void addTo(int layerID, Node *node);
	void remove(Node *node);
	// 地图
	Terrain * nowTerrain;
	// 增加角色
	void addOtherRole(IRole *role);

	Node *uiLayer;// UI 层

	void doTick(float dt); // 定时器

	bool checkHurts(std::vector<IRole*>& out, Sprite *src);

	CCRenderTexture * rtForC;
	/**
	 * 获取攻击对象
	 */
	Point getAttackObjectPosition(Node *role);

	void addNpcByTempId(int tempid, IRole * role);
	IRole* getNpcByTempId(int tempid);

	void addUserByTempId(int tempid, IRole * role);
	IRole* getUserByTempId(int tempid);
private:
	std::vector<IRole* > _roles; // 角色集合
	typedef std::vector<IRole*>::iterator ROLES_ITER;

	std::map<int, IRole *> _npcs; // npc集合
	typedef std::map<int, IRole*>::iterator NPCS_ITER;

	std::map<int, IRole *> _users; // user集合
	typedef std::map<int, IRole*>::iterator USERS_ITER;
};