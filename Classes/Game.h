/**
 * 创建Game
 */
#pragma once
#include "MainUI.h"
#include "Scene.h"

#include "cocos2d.h"
class IRole;
#include "Map.h"
class Game : public cocos2d::Layer
{
public:
	static cocos2d::Scene* createScene();
	virtual bool init();
	CREATE_FUNC(Game);
	void onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *event);
	bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event  *event);
	//通过点击结束区域 来设置 lable的位置
	void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event  *event);
	/**
	 *  更新自己
	 **/
	void update(float dt);

	/**
	 * 返回获取的网关
	 */
	void doGetGate(void*cmd, int len);

	/**
	 * 处理无档案的情况
	 */
	void doLoginError(void *cmd, int len);

	/**
	 * 处理登录成功
	 */
	void doLoginOk(void *cmd, int len);

	/*
	 * 处理地图信息 
	 **/
	void doGetMap(void *cmd, int len);

	/**
	 * 获取其他人的信息
	 */
	void doGetOthers(void *cmd, int len);
	/**
	 * 移动Npc
	 */
	void doMoveNpc(void *cmd, int len);
	/**
	 * 移动User
	 */
	void doMoveUser(void *cmd, int len);
	/**
	 * 获取人物主数据
	 */
	void doGetMainRoleData(void *cmd, int len);
	/**
	 * 增加其他玩家
	 */
	void doAddOther(void *cmd, int len);
	/**
	 * 增加NPC
	 */
	void doAddNpc(void *cmd, int len);
	/**
	 * 攻击对象
	 */
	void doAttackRole(void *cmd, int len);
	MoveOPUI *moveOPUI;
};