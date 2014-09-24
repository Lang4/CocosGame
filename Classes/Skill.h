#pragma once
/**
 * 技能的播放
 */
#include "XmlSkills.h"
#include "UtilAnimation.h"
/**
 * 技能操作获取相关信息接口
 */
class ISkill{
public:
	virtual Point GetAttackObjectPosition() = 0;// 获取被击者位置
	virtual Point GetGameObjectPosition() =0;//获得技能拥有者的位置
	virtual Node * getEffectLayer(int ID) { return NULL; }// 获取特效所在的层
	virtual std::string getRoleName(){ return ""; } // 获取角色的名字[就是角色的文件名]
	virtual bool checkHurts(std::vector<IRole*>& out, const Point &point, Sprite *src, const std::string & effectID){ return false; }
};
/**
* 技能音效
*/
class SkillSoundEffect{
public:
	void stopPlay();
	SoundConfig *config;
	void playSound();
	int soundId;
	SkillSoundEffect()
	{
		soundId = 0;
	}
};
class Skill;
/**
 * 技能子效果
 */
class SkillSubEffect:public Node{
public:
	ISkill *skill;
	CREATE_FUNC(SkillSubEffect);
	bool init(){ return true; }
	SubEffectConfig *config;
	SkillSoundEffect sound;
	Sprite *sprite; // 特效对应的精灵
	void startPlay();
	SkillSubEffect()
	{
		config = NULL;
		skill = NULL;
		sprite = NULL;
		picDir = 0;
		costDt = 0;
		reachX = reachY = false;
	}
	bool init(Skill *skill);
	void update(float dt);
	void updatePosition(float dt);
	int currFrame; // 当前帧
	float nowSpeed; // 当前速度
	float moveAngle; // 移动角度
	float currSpeedX; // 当前横坐标
	float currSpeedY; // 当前纵坐标
	float accelerate; // 加速度
	float rotateAngle; // 旋转脚本
	float rotateSpeed; // 旋转速度
	int interval;
	bool startPlayTag; // 是否开始标示
	bool playFinish; // 是否播放结束
	float connectDistance; // 两个对象见的距离
	float getPointAngle(float x, float y);

	void show(bool); // 展示 或 隐藏
	bool checkFinish(); // 检查是否结束
	void updateRotate(float dt); // 更新角度
	void updateSound(float dt); // 更新声音
	void changeFrame(); // 切换帧
	//检查下一帧的控制信息
	//延迟的记录延迟的帧数
	//跳过的当前帧加一
	void checkFrameCtrl(int frame);
	int frameIndex; // 当前帧
	int currDelayCount; // 当前延帧
	CacheAnimation cacheAnimation; // 帧数据
	int totalFrame; // 所有帧的总数
	void calcTotalFrame(); // 计算总帧数
	bool checkOutOfRange(); // 判断操作范围
	bool haveReach(); // 是否到达目标
	bool reachX; // 是否X轴到达
	bool reachY; // 是否Y轴到达
	int maxAtkDisttance; // 最大技能攻击距离
	int attackDistance; // 展示距离
	Point startPos; // 开始位置
	void calcStartPosition(); // 计算开始位置
	int currDir; // 当前方向
	void changeDir(int dir); // 当前方向
	int picDir; // 图片的方向
	float deg2Rad(float val){ return 3.1415926 /180 * val; }
	float getDirDegree(int val);
	float costDt; //  每帧间隔
	bool checkHurt(); // 检查伤害
};
/**
 * 技能效果
 */
class SkillEffect{
public:
	void init(Skill *skill);
	void startPlay();
	void changeDir(int dir); // 更改方向
	std::vector<SkillSubEffect*> subEffects;
	typedef std::vector<SkillSubEffect*>::iterator SUBEFFECTS_ITER;
	EffectConfig *config;
	SkillEffect()
	{
		config = NULL;
	}
};
class Skill;
/**
 * 技能动作
 */
class SkillAction{
public:
	/**
	 * 获取动作帧
	 */
	SpriteFrame * getActionFrame();
	/**
	 * 重置状态 获取新的动作集合
	 */
	void changeAction();

	/**
	 * 切换方向
	 */
	void changeDir(int dir);
	//检查下一帧的控制信息
	//延迟的记录延迟的帧数
	//跳过的当前帧加一
	bool checkFrameCtrl(int frame);

	/**
	 * 初始化
	 */
	void init(Skill *skill);
	CacheAnimation cacheAnimation;

	ActionConfig *config; // 配置的静态信息

	
	int currFrame; // 当前帧
	int currFrameInEle; //当前帧索引
	int currDelayCount; // 当前延时次数
	int dir; // 方向
	ISkill *skill; // 当前技能
	SkillAction()
	{
		config = NULL;
		currFrame = 0;
		currFrameInEle = 0;
		currDelayCount = 0;
		dir = 0;
		skill = NULL;
	}
	/**
	 * 结束
	 */
	void end();
	void startPlay();
};

class SkillFrame{
public:
	std::vector<SkillEffect> effects;
	typedef std::vector<SkillEffect>::iterator EFFECTS_ITER;
	FrameConfig* config;
	void init(Skill *skill);
	void startPlay();
	SkillFrame()
	{
		config = NULL;
	}
	SkillAction action; // 当前动作
	SkillSoundEffect sound; // 音效
	void changeDir(int dir); // 更改方向
};
/**
 * 1. 攻击时展示相关动作
 */
class Skill:public ISkill{
public:
	Skill()
	{
		currFrame = 0;
		dir = 0;
		config = NULL;
		parent = NULL;
		currDelayCount = 0;
		currFrameInEle = 0;
		nowAction = NULL;
		me = NULL;
		target = NULL;
	}
	IRole * me;
	IRole *target;
	Point targetPosition;
	int currFrame; //当前帧,该动作的当前帧
	int currFrameInEle; //当前元件内帧的索引
	int currDelayCount; //当前延迟的帧数
	int dir;//方向
	void setCurrectDir(int dir); // 设置方向
	SkillConfig *config; // 技能相关静态信息
	void init(); // 初始化
	void update(); // 更新
	void checkUpdate(int frame); // 检查当前帧更新
	void end(); // 结束
	bool actionPlayFinish;
	bool isOver(){ return actionPlayFinish;} // 是否结束
	SpriteFrame * getActionFrame(); // 获取当前动作帧
	virtual Point GetAttackObjectPosition();// 获取被击者位置
	virtual Point GetGameObjectPosition();//获得技能拥有者的位置
	SkillAction *nowAction;
	std::vector<SkillFrame> frames; // 每帧信息
	typedef std::vector<SkillFrame>::iterator FRAMES_ITER;
	void startPlay();
	Node *parent;
	Node * getEffectLayer(int ID);
	std::string roleName;
	std::string getRoleName(){ return roleName; }
	/*
	* 用作碰撞检查的
	**/
	CCRenderTexture *rtForC;

	bool checkHurts(std::vector<IRole*>& out, const Point &point, Sprite *src, const std::string & effectID);
	bool checkCollide(Sprite *src, Sprite *dest);
};