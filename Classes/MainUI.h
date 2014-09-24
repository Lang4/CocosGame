/**
 * 主界面相关
 */
#pragma once
#include "cocos2d.h"

USING_NS_CC;
class IRole;
/**
 * 技能快捷栏
 */
class CDShowItem:public Node{
public:
	CREATE_FUNC(CDShowItem);
	bool init();
	/**
	 * 展示
	 */
	void show();
	/**
	 * 设置纹理
	 */
	void setTexture(Texture2D *texture);

	CDShowItem()
	{
		_valid = true;
		_sprite = NULL;
		_canMove = false;
		info = NULL;
	}
	std::string skillName;

	virtual void doClickMe() {}
private:
	bool _canMove;
	bool _valid;
	void cartoonOver();
	Sprite *_sprite; // 展示精灵
	ProgressTimer *cdShow;
	LabelTTF *info;
};
/**
 * 技能条
 */
class SkillItem :public CDShowItem{
public:
	CREATE_FUNC(SkillItem);
	void initWithInfo(const char *pngName,int skillId);
	int skillId; // 技能
	void doClickMe();
};
/**
 * 切换英雄条
 */
class ChangeHeroItem :public CDShowItem{
public:
	CREATE_FUNC(ChangeHeroItem);
	void initWithInfo(const char*pngName, int heroId);
	int heroId;
	void doClickMe();
};
/**
 * 技能盘
 */
class SkillShowPanel :public Node{
public:
	CREATE_FUNC(SkillShowPanel);
	void push(CDShowItem *item);
	std::vector<CDShowItem*> items;
	void show();
};
class TaskTalkUI :public Node{
public:
	CREATE_FUNC(TaskTalkUI);
	bool init(){ return true; }
	/**
	 * 通过名字 和 信息初始化聊天框
	 */
	void initWithInfo(const char *name, const char *info);
	/**
	 * 说话
	 */
	void talk();
};

class ChoiceHeroUI :public Node{
public:
	CREATE_FUNC(ChoiceHeroUI);
	bool init();
};

class HeroHeadUI :public Node{
public:
	CREATE_FUNC(HeroHeadUI);
	bool init();
};

class ChoiceBossUI :public Node{
public:
	CREATE_FUNC(ChoiceBossUI);
	bool init();
};

class QuickUI :public Node{
public:
	CREATE_FUNC(QuickUI);
	bool init();
};
enum{
	MY_TOUCH_DOWN,
	MY_TOUCH_MOVE,
	MY_TOUCH_END,
};
/**
 * 移动操作盘
 */
class MoveOPUI :public Node{
public:
	CCPoint opMoveCenter;
	// 可以操作角色移动 
	static MoveOPUI* create(IRole *player);
	bool init(IRole *player);
	IRole *player;
	Touch *nowTouch;
	/**
	* 处理点击
	* \parma touchType 点击类型
	* \param touchPoint 点击点
	*/
	bool doTouch(int touchType, Touch* touch);
	MoveOPUI()
	{
		nowTouch = NULL;
		player = NULL;
		nowTouchIn = false;
		tagSprite = NULL;
	}
	bool nowTouchIn; // 当前是否在其上操作

	void showTouch(int dirType);
	Sprite * tagSprite;
}; // 移动盘 该盘一直在Game上

class MainUI:public Node{
public:
	CREATE_FUNC(MainUI);
	bool init();
};