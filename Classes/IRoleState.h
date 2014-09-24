#pragma once
/**
 * 人物动作类设计
 * 走路 静止 攻击 死亡
 */
#include <vector>
#include <cocos2d.h>
#include "Astar.h"
#include "Skill.h"
USING_NS_CC;
class IRole;
class Msg{
public:
	enum{
		TOUCH_MSG = 1, // 点击事件
		CARTOON_OVER = 2, // 动画结束
		BE_ATTACK = 3, // 被击
		ATTACK = 4, // 攻击
		FRAME_UPDATE = 5, // 帧更新时间
		GOTO_MSG = 6, // 直接移动
	};
	int MsgType;
	Msg()
	{
		MsgType = 0;
	}
	Msg(int msg)
	{
		MsgType = msg;
	}
	virtual Msg *clone(){ return NULL; }
};
/**
* 点击事件
*/
class TouchMsg :public Msg{
public:
	Point point;
	TouchMsg() :Msg(TOUCH_MSG){}
	virtual Msg *clone()
	{ 
		TouchMsg *msg = new TouchMsg();
		msg->point = point;
		return msg;
	}
};
class GoToMsg :public Msg{
public:
	Point point;
	GoToMsg() :Msg(GOTO_MSG){}
	virtual Msg *clone()
	{
		GoToMsg *msg = new GoToMsg();
		msg->point = point;
		return msg;
	}
};
/**
* 动画结束事件
*/
class CartoonOverMsg :public Msg{
public:
	CartoonOverMsg() :Msg(CARTOON_OVER){}
	virtual Msg *clone()
	{
		CartoonOverMsg *msg = new CartoonOverMsg();
		return msg;
	}
};
/**
* 被击事件
*/
class BeAttackMsg :public Msg{
public:
	IRole *from; // 发起者 
	BeAttackMsg() :Msg(BE_ATTACK){
		from = NULL;
	}
	virtual Msg *clone()
	{
		BeAttackMsg *msg = new BeAttackMsg();
		msg->from = from;
		return msg;
	}
};
/**
* 攻击事件
*/
class AttackMsg :public Msg{
public:
	int skillId; // 使用的技能编号
	Point point; // 攻击的点
	IRole *target; // 攻击的对象
	AttackMsg() :Msg(ATTACK){
		skillId = 0; target = NULL;
	}
	virtual Msg *clone()
	{
		AttackMsg *msg = new AttackMsg();
		msg->point = point;
		msg->target = target;
		msg->skillId = skillId;
		return msg;
	}
};
/**
 * 帧事件
 */
class UpdateFrameMsg :public Msg{
public:
	float dt;
	UpdateFrameMsg() :Msg(FRAME_UPDATE)
	{
		dt = 0;
	}
};
class RoleStateMangager;
class IRoleState:public Ref{
public:
	/**
	* 进入该状态
	*/
	virtual void Enter() {}
	/**
	* 退出该状态
	*/
	virtual void Exit() {}

	/**
	* 状态处理器
	*/
	virtual void PasreMsg(Msg * msg) = 0;

	/**
	* 状态管理器
	*/
	RoleStateMangager *rsm;
	/**
	* 攻击者
	*/
	IRole *role;
	IRoleState(){ rsm = NULL; role = NULL; }

	virtual Sprite * getSprite(){ return NULL;}

	void cartoonOver();
public:
	/**
	 * 相同类型的消息只留最新的
	 */
	void pushMsg(Msg *msg);
	/**
	 * 消息缓存
	 */
	std::list<Msg*> msgBuffer;
	typedef std::list<Msg*>::iterator MSGBUFFER_ITER;
	/**
	 * 清除消息Buffer
	 */
	void clearMsgBuffer();
};

class RoleStateMangager{
public:
	/**
	* 切换当前状态
	*/
	void chageState(IRoleState *nowState);
	/**
	* 保存当前状态
	*/
	void pushState(IRoleState *state);
	/**
	* 恢复当前状态
	*/
	IRoleState *popState();

	/**
	* 获取当前状态
	*/
	IRoleState * nowState();

	/**
	* 处理消息
	*/
	void PasreMsg(Msg *msg);


private:
	std::vector<IRoleState*> _states;
};
/**
* 空闲状态
*/
class IdleRoleState :public IRoleState{
public:
	IdleRoleState(){
		_sprite = NULL;
		_action = NULL;
	}
	/**
	* 进入该状态
	* 开始播放IDLE动画
	*/
	virtual void Enter();
	/**
	* 退出时
	* 保存改状态
	*/
	virtual void Exit();

	/**
	* 状态处理器 会响应点击事件 被击事件 动画结束事件
	*/
	virtual void PasreMsg(Msg * msg);
	Sprite * getSprite(){ return _sprite; }
private:
	Action *_action; // 当前动作
	Sprite *_sprite; // 当前精灵
};
/**
* 走路状态
*/
class WalkRoleState :public IRoleState{
public:
	WalkRoleState()
	{
		_animation = NULL;
		_sprite = NULL;
		index = 0;
		costDt = 0;
		stepCostDT = 0;
	}
	/**
	 * 播放走路动画
	 */
	virtual void Enter();
	/**
	* 响应被击事件 动画结束事件 响应点击事件
	*/
	virtual void PasreMsg(Msg * msg);
	/**
	 * 清除动画
	 */
	virtual void Exit();
	/**
	 * 更新事件
	 */
	void update(float dt);

	GridIndex point; // 当前要走的网格点

	Sprite * getSprite(){ return _sprite; }
private:
	Animation *_animation; // 当前动作
	Sprite *_sprite; // 当前精灵
	int index;
	float costDt; // 当前帧播放时间累积
	float stepCostDT; // 移动时间累积
private:
	Vec2 _positionDelta;
	Vec2 _startPosition;
	Vec2 _previousPosition;
};
/**
* 自动寻路状态
*/
class AutoWalkRoleState :public IRoleState{
public:
	/**
	* 进入该状态时 判断是否到达目标点 若到达则 切换至上次状态
	*/
	virtual void Enter();
	/**
	* 响应点击事件
	*/
	virtual void PasreMsg(Msg * msg);
	/**
	 * 目的地
	 */
	Point destination;

	Sprite *mousePoint;

	Action *action;
	AutoWalkRoleState()
	{
		mousePoint = NULL;
		action = NULL;
	}
};
/**
* 攻击状态
*/
class AttackRoleState :public IRoleState{
public:
	/**
	 * 进入该状态
	 */
	virtual void Enter();

	/**
	 * 离开
	 */
	virtual void Exit();
	/**
	 * 处理消息
	 */
	virtual void PasreMsg(Msg * msg);

	/**
	 * 更新
	 */
	virtual void update(float dt);
	AttackRoleState()
	{
		currentFrame = 1;
		actionPlayFinish = false;
		_sprite = NULL;
		costDt = 0;
		target = NULL;
	}
	int currentFrame;
	Skill skill;
	float costDt;
	bool actionPlayFinish;

	Sprite * getSprite(){ return _sprite; }

	IRole *target;
	Point targetPosition;
private:
	Sprite *_sprite; // 当前精灵
};

/**
 * 被击状态
 */
class BeAttackRoleState:public IRoleState{
public:
	virtual void Enter();
	virtual void Exit();
	virtual void PasreMsg(Msg * msg);
	Sprite * getSprite(){ return _sprite; }
	BeAttackRoleState()
	{
		_action = NULL;
		_sprite = NULL;
		retain();
		beAttacker = NULL;
	}
	IRole * beAttacker;
private:
	Action *_action; // 当前动作
	Sprite *_sprite; // 当前精灵
};
/**
* 场景角色
*/
class IRole :public Node{
public:
	CREATE_FUNC(IRole);
	/**
	* 播放动画
	*/
	void play();
	/**
	* 动画结束回调
	*/
	void cartoonOver();

	IRole()
	{
		direction = 1;
		_sprite = 0;
		continueMove = false;
		tempID = 0;
	}
	/**
	* 初始化
	*/
	bool init();
	/**
	* 点击
	*/
	void onTouch();
	/**
	 * 更改自身外面
	 */
	void changeAnimation(const char *aniName);
	/**
	 * 动画名字
	 */
	std::string roleName;
	std::string roleIndex;
	/**
	 * 获取动画名字
	 */
	const char * getAnimationName();

	std::string nowSkill;
	/**
	 * 获取当前技能
	 */
	const char * getNowSkill();
	/**
	 * 获取角色方向
	 */
	int getDirection();

	RoleStateMangager * getRSM(){ return &rsm; }
	/**
	 * 设置当前方向
	 */
	void setDirection(int direction);

	void update(float dt);

	Sprite * getActionSprite();
	/**
	 * 计算与目标Role的距离
	 */
	float calcDistance(IRole *role);
	/**
	 * 移动盘控制的持续移动
	 */
	void doContinueMove();
	/**
	 * 不再持续移动
	 */
	void clearContinueMove();

	bool continueMove;

	/**
	 * 获取方向上下一个点
	 */
	Point getNextPointByDir();

	/**
	 * 发送同步消息到客户算
	 */
	void doSendMoveMsg(const Point &point);

	int tempID; // 临时编号
	/**
	 * 直接移动到某点
	 */
	void goTo(const Point &point);

	/**
	 * 寻路移动到某点
	 */
	void moveTo(const Point &point);
private:
	int direction;

	Sprite *_sprite; // 当前精灵
public:
	IdleRoleState idleState;	// 空闲状态

	WalkRoleState walkState; 	// 走路状态

	AutoWalkRoleState autoWalkState; // 自动寻路状态

	AttackRoleState attackState; // 攻击状态

	BeAttackRoleState beAttackState; // 被击状态
	/**
	* 角色状态管理器
	*/
	RoleStateMangager rsm;
	/**
	 * 获取当前状态对应的精灵
	 */
	Sprite * getStateSprite();
};