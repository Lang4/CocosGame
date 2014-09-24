#include "IRoleState.h"
#include "UtilAnimation.h"
#include "Scene.h"
#include "Astar.h"
/**
* 切换当前状态
*/
void RoleStateMangager::chageState(IRoleState *nowState)
{
	if (_states.empty())
	{
		_states.push_back(nowState);
		nowState->Enter();
	}
	else
	{
		IRoleState *state = _states.back();
		_states.pop_back();
		if (state) state->Exit();
		_states.push_back(nowState);
		nowState->Enter();
	}
	
}
/**
* 保存当前状态
*/
void RoleStateMangager::pushState(IRoleState *nowState)
{
	if (!_states.empty())
	{
		IRoleState *state = _states.back();
		if (state) state->Exit();
		_states.push_back(nowState);
		nowState->Enter();
		
	}
	else
	{
		_states.push_back(nowState);
		nowState->Enter();
	}
	
}
/**
* 恢复当前状态
*/
IRoleState *RoleStateMangager::popState()
{
	if (!_states.empty())
	{
		IRoleState *state = _states.back();
		if (state) state->Exit();
		_states.pop_back();
		if (_states.empty())
		{
			return NULL;
		}
		state = _states.back();
		if (state) state->Enter();
		return state;
	}
	else
	{
		return NULL;
	}
}

/**
* 获取当前状态
*/
IRoleState * RoleStateMangager::nowState()
{
	if (!_states.empty())
	{
		IRoleState *state = _states.back();
		return state;
	}
	else
	{
		return NULL;
	}
}

/**
* 处理消息
*/
void RoleStateMangager::PasreMsg(Msg *msg)
{
	IRoleState *roleState = nowState();
	if (roleState)
	{
		roleState->PasreMsg(msg);
	}
}
/*****************************************************************************************************************/
// 状态基类
/*****************************************************************************************************************/

void IRoleState::pushMsg(Msg *msg)
{
	Msg *newM = msg->clone();
	if (!newM) return;
	for (MSGBUFFER_ITER iter = msgBuffer.begin(); iter != msgBuffer.end();)
	{
		Msg * old = *iter;
		if (msg->MsgType == old->MsgType)
		{
			iter = msgBuffer.erase(iter);
		}
		else ++iter;
	}
	msgBuffer.push_back(newM);
}
void IRoleState::clearMsgBuffer()
{
	for (MSGBUFFER_ITER iter = msgBuffer.begin(); iter != msgBuffer.end(); ++iter)
	{
		Msg * msg = *iter;
		delete msg;
	}
	msgBuffer.clear();
}

void IRoleState::cartoonOver()
{
	CartoonOverMsg msg;
	PasreMsg(&msg);
}
/*****************************************************************************************************************/
// 静止状态
/*****************************************************************************************************************/
/**
* 进入该状态
* 开始播放IDLE动画
*/
void IdleRoleState::Enter()
{
	if (_sprite)
	{
		_sprite->stopAction(_action);
		_sprite->setVisible(true);
	}
	else
	{
		_sprite =Sprite::create();
		role->addChild(_sprite);
	}
	if (_sprite)
	{
		int dir = role->getDirection();
		if (dir > 4)
		{
			dir = 8 - dir;
			_sprite->setFlipX(true);
		}
		else
			_sprite->setFlipX(false);
		_action = Animate::create(UtilParser::getInstance()->getAnimation(role->getAnimationName(), "1", dir));
		if (_action)
		{
			Sequence* seq = Sequence::create((Animate*)_action,
				CallFunc::create(this, (SEL_CallFunc)(&IdleRoleState::cartoonOver)),
				NULL);
			_action = seq;
			_sprite->runAction(seq);
		}
		else throw "IdleRoleState::Enter";
	}
}

/**
* 退出时
* 保存改状态
*/
void  IdleRoleState::Exit()
{
	if (_sprite)
	{
		if (_action)
			_sprite->stopAction(_action);
		_sprite->setVisible(false);
		_action = NULL;
	}
}

/**
* 状态处理器 会响应点击事件 被击事件 动画结束事件
*/
void IdleRoleState::PasreMsg(Msg * msg)
{
	switch (msg->MsgType)
	{
		case Msg::ATTACK:{
			// 处理攻击 
			AttackMsg *atkMsg = (AttackMsg*)msg;
			role->attackState.target = atkMsg->target;
			role->attackState.targetPosition = atkMsg->point;
			role->getRSM()->pushState(&role->attackState);
		}break;
		case Msg::BE_ATTACK:
		{
			BeAttackMsg * beAttackMsg = (BeAttackMsg*)msg;
			role->beAttackState.beAttacker = beAttackMsg->from;
			role->getRSM()->pushState(&role->beAttackState);
		}break;
		case Msg::TOUCH_MSG:
		{
			TouchMsg *touchMsg = (TouchMsg*)msg;
			// 处理点击
			// 判断是否需要移动
#if (1)
			role->autoWalkState.destination = touchMsg->point;
			role->getRSM()->pushState(&role->autoWalkState); // 切换到自动寻路流程
#else
			role->getRSM()->pushState(&role->attackState);
#endif
		}break; 
		case Msg::CARTOON_OVER:
		{
			// 处理动画结束
			if (role->continueMove)
			{
				role->autoWalkState.destination = role->getNextPointByDir();
				role->getRSM()->pushState(&role->autoWalkState); // 切换到自动寻路流程
			}
			else Enter(); // 重新开始
		}break;
		case Msg::GOTO_MSG:
		{
			GoToMsg * gotoMsg = (GoToMsg*)msg;
			role->walkState.point = MapScene::transPixelPointToGird(gotoMsg->point);
			role->getRSM()->pushState(&role->walkState);
		}break;
		default:
			break;
	}
}

/***********************************************************************************************************/
// 自动寻路
/***********************************************************************************************************/
void AutoWalkRoleState::Enter()
{
	if (!mousePoint)
	{
		mousePoint = Sprite::create();
		role->getParent()->addChild(mousePoint,3);

	}
#if (1)
	if (mousePoint)
	{
		mousePoint->stopAllActions();

		action = Repeat::create(Animate::create(UtilParser::getInstance()->getAnimation("misc.anis", "MOUSEPOINT", 0, 1)), -1);
		if (action)
		{
			mousePoint->runAction(action);
		}
	}
#endif
	// 判断是否到点 否则切换到WalkState
	if (destination.equals(ccp(0, 0)))
	{
		role->getRSM()->popState();
		mousePoint->setVisible(false);
		mousePoint->stopAllActions();
	}
	else
	{
		mousePoint->setPosition(destination); // 在目的地展示点
		mousePoint->setVisible(true);
		/**
		 * 按Astar来做
		 */
		role->walkState.point = MapScene::transPixelPointToGird(destination);
		
		int dir = GameDirs::getDirection(role->getPositionX(), role->getPositionY(), destination.x, destination.y);
		// 计算方向
		role->setDirection(dir);
		// 或许应该 
		role->getRSM()->pushState(&role->walkState);
	}
}
/**
* 响应点击事件
*/
void AutoWalkRoleState::PasreMsg(Msg * msg)
{
	switch (msg->MsgType)
	{
		case Msg::ATTACK:{
			// 处理攻击 
			role->getRSM()->pushState(&role->attackState);
		}break;
	}
}
/***********************************************************************************************************/
// 走路
/***********************************************************************************************************/
#include "Protos\Command.h"
#include "Network\MyClient.h"
void WalkRoleState::Enter()
{
	// 播放走路动画
	if (!_sprite)
	{
		_sprite = Sprite::create();
		role->addChild(_sprite);
	}
	Point dest = MapScene::transGirdToPixelPoint(point);
	int dir = GameDirs::getDirection(role->getPositionX(), role->getPositionY(), dest.x, dest.y);
	// 计算方向
	role->setDirection(dir);
	if (_sprite)
	{
		_sprite->setVisible(true);
		int dir = role->getDirection();
		if (dir > 4)
		{
			dir = 8 - dir;
			_sprite->setFlipX(true);
		}
		else
		{
			_sprite->setFlipX(false);
		}
		_animation = UtilParser::getInstance()->getAnimation(role->getAnimationName(), "2", dir);
		if (!_animation)
		{
			throw "IdleRoleState::Enter";
		}
		_animation->retain();
		costDt = 0;
		index = 0;
		stepCostDT = 0;
	}
	// 更新移动相关的位置
	_previousPosition = _startPosition = role->getPosition();
	role->doSendMoveMsg(dest);
	_positionDelta = dest - role->getPosition();
}
void WalkRoleState::Exit()
{
	if (_sprite)
	{
		_sprite->setVisible(false);
		role->autoWalkState.destination = ccp(0, 0);
	}
	if (_animation) _animation->release();
	_animation = NULL;
}

void WalkRoleState::update(float dt)
{
	if (_animation && _sprite)
	{
		costDt += dt; // 帧时间积累
		float eachFrameTime = _animation->getDelayPerUnit()/2;
		const Vector<AnimationFrame*>& frames = _animation->getFrames();
		if (costDt >= eachFrameTime )
		{
			index = (index + 1) % frames.size();
			_sprite->setDisplayFrame(frames.at(index)->getSpriteFrame());
			costDt = 0;
		}
		// 更新位置
		float allNeedTime = eachFrameTime * frames.size();
		stepCostDT += dt; // 移动时间累积
		if (allNeedTime)
		{
			Vec2 currentPos = role->getPosition();
			Vec2 diff = currentPos - _previousPosition;
			_startPosition = _startPosition + diff;
			Vec2 newPos = _startPosition + (_positionDelta * stepCostDT);
			role->setPosition(newPos);
			_previousPosition = newPos;
		}
		if (index >= frames.size() - 1 && stepCostDT >= allNeedTime)
		{
			//role->getRSM()->popState();
			CartoonOverMsg msg;
			role->getRSM()->PasreMsg(&msg);
			return;
		}
	}
}
void WalkRoleState::PasreMsg(Msg *msg)
{
	switch (msg->MsgType)
	{
	case Msg::ATTACK:{
		// 处理攻击 
	//	pushMsg(msg);
		AttackMsg *atkMsg = (AttackMsg*)msg;
		role->attackState.target = atkMsg->target;
		role->attackState.targetPosition = atkMsg->point;
		role->getRSM()->pushState(&role->attackState);
	}break;
	case Msg::TOUCH_MSG:
	{
		// 更新
		//role->getRSM()->popState();
	}break;
	case Msg::FRAME_UPDATE:
	{
		UpdateFrameMsg *fMsg = (UpdateFrameMsg*)msg;
		update(fMsg->dt);
	}break;
	case Msg::BE_ATTACK:{
		// 被击 需要立即被执行
		BeAttackMsg *beAtkMsg = (BeAttackMsg*)msg;
		role->beAttackState.beAttacker = beAtkMsg->from;
		role->getRSM()->pushState(&role->beAttackState);
	}break;
	case Msg::GOTO_MSG:{
		GoToMsg *gotoMsg = (GoToMsg*)msg;
		if (role->continueMove)
		{
			role->walkState.point = MapScene::transPixelPointToGird(gotoMsg->point);
			role->autoWalkState.mousePoint->setPosition(role->autoWalkState.destination); // 在目的地展示点
			Enter();
		}
	}break;
	case Msg::CARTOON_OVER:
	{
		// 返回到上一个状态
		if (!role->continueMove)
			role->getRSM()->popState();

		// 处理消息滞留
		for (MSGBUFFER_ITER iter = msgBuffer.begin(); iter != msgBuffer.end(); ++iter)
		{
			Msg *msg = *iter;
			switch (msg->MsgType)
			{
				case Msg::ATTACK:
				{
					role->getRSM()->pushState(&role->attackState);
				}break;
			}
		}
		if (role->continueMove)
		{
			role->autoWalkState.destination = role->getNextPointByDir();
			role->walkState.point = MapScene::transPixelPointToGird(role->autoWalkState.destination);
			role->autoWalkState.mousePoint->setPosition(role->autoWalkState.destination); // 在目的地展示点
			Enter();
		}
		this->clearMsgBuffer();
	}break;
	default:
		break;
	}
}
/***********************************************************************************************************/
// 攻击
/***********************************************************************************************************/
void AttackRoleState::Enter()
{
	// 技能初始化
	skill.config = SkillConfigs::getInstance()->getSkillConfig(role->roleIndex.c_str(), role->getNowSkill()); // 设置技能配置文件
	skill.roleName = role->getAnimationName();
	skill.parent = role;
	skill.me = role;
	skill.target = target;
	skill.targetPosition = targetPosition;
	int dir = GameDirs::getDirection(role->getPositionX(), role->getPositionY(), targetPosition.x, targetPosition.y);
	role->setDirection(dir);
	skill.init();
	skill.setCurrectDir(role->getDirection());
	actionPlayFinish = false;
	if (!_sprite)
	{
		_sprite = Sprite::create();
		role->addChild(_sprite);
	}
	costDt = 0;
}
void AttackRoleState::Exit()
{
	if (_sprite)
	{
		role->removeChild(_sprite);
		_sprite = NULL;
	}
	skill.end();
}
void AttackRoleState::PasreMsg(Msg * msg)
{
	switch (msg->MsgType)
	{
		case Msg::FRAME_UPDATE:
		{
			UpdateFrameMsg *fMsg = (UpdateFrameMsg*)msg;
			update(fMsg->dt);
		}break;
		default:
			break;
	}
}
void AttackRoleState::update(float dt)
{
	// 技能刷新 
	if (costDt > 0.05)
	{
		skill.update();

		// 技能若结束 则切换状态
		SpriteFrame*frame = skill.getActionFrame();
		if (frame)
		{
			_sprite->setDisplayFrame(frame);
		}
		if (role->getDirection() > 4)
		{
			_sprite->setFlipX(true);
		}	
		costDt = 0;
	}
	if (skill.isOver())
	{
		role->getRSM()->popState();
	}
	costDt += dt; // 帧时间积累
}
/***********************************************************************************************************/
// 被击
/***********************************************************************************************************/
void BeAttackRoleState::Enter()
{
	// 根据受击方向 调整动画
	if (_sprite)
	{
		_sprite->stopAction(_action);
	}
	else
	{
		_sprite = Sprite::create();
		role->addChild(_sprite);
	}
	if (_sprite && beAttacker)
	{
		// 根据攻击者调整受击方向
		int beDir = GameDirs::getDirection(role->getPositionX(), role->getPositionY(),beAttacker->getPositionX(), beAttacker->getPositionY());
		role->setDirection(beDir);
		int dir = role->getDirection();
		if (dir > 4)
		{
			dir = 8 - dir;
			_sprite->setFlipX(true);
		}
		Animation *ani = UtilParser::getInstance()->getAnimation(role->getAnimationName(), "3", dir);
		if (ani)
		{
			_action = Animate::create(ani);
			if (_action)
			{
				Sequence* seq = Sequence::create((Animate*)_action,
					CallFunc::create(this, (SEL_CallFunc)(&BeAttackRoleState::cartoonOver)),
					NULL);
				_sprite->runAction(seq);
			}
			else throw "BeAttackRoleState::Enter";
		}
	}
}
/**
 * 处理外部事件
 */
void BeAttackRoleState::PasreMsg(Msg *msg)
{
	// 处理点击 有概率恢复状态
	switch (msg->MsgType)
	{
	case Msg::ATTACK:{
		// 处理攻击 
		pushMsg(msg);
	}break;
	case Msg::TOUCH_MSG:
	{
		// 更新
	}break;
	case Msg::BE_ATTACK:{
		
	}break;
	case Msg::CARTOON_OVER:
	{
		// 返回到上一个状态
		role->getRSM()->popState();

		// 处理消息滞留
	}break;
	default:
		break;
	}
}

void BeAttackRoleState::Exit()
{
	// 清除自身动画

	if (_sprite)
	{
		if (_action)
			_sprite->stopAction(_action);
		role->removeChild(_sprite);
		_sprite = NULL;
		_action = NULL;
	}
}


/***********************************************************************************************************/
// 角色
/***********************************************************************************************************/
bool IRole::init()
{
	idleState.role = this;
	autoWalkState.role = this;
	walkState.role = this;
	attackState.role = this;
	beAttackState.role = this;
	this->scheduleUpdate();
#if (0)
	Sprite * rolePoint = Sprite::create();
	Action *action = Repeat::create(Animate::create(UtilParser::getInstance()->getAnimation("misc.anis", "ROLEPOINT", 0,1)), -1);
	if (action)
	{
		rolePoint->runAction(action);
	}
	this->addChild(rolePoint); // 脚底光环
#endif
	
	return true;
}

void IRole::play()
{
	getRSM()->chageState(&idleState); // 初始时切花至静止状态
}
/**
* 动画结束回调
*/
void IRole::cartoonOver()
{
	CartoonOverMsg overMsg;
	getRSM()->PasreMsg(&overMsg);
}

int IRole::getDirection()
{
	return this->direction;
}

void IRole::setDirection(int direction)
{
	this->direction = direction;
}

void IRole::update(float dt)
{
	Node::update(dt);

	UpdateFrameMsg uMsg;
	uMsg.dt = dt;
	getRSM()->PasreMsg(&uMsg);
}

Sprite * IRole::getActionSprite()
{
	if (!_sprite) {
		_sprite = Sprite::create();
		this->addChild(_sprite);
	}
	return _sprite;
}


/**
* 获取动画名字
*/
const char * IRole::getAnimationName()
{
	return roleName.c_str();
}

void IRole::changeAnimation(const char *name)
{
	String info;
	roleIndex = name;
	info.initWithFormat("%s.%s", name, "anis");
	roleName = info.getCString();
}

float IRole::calcDistance(IRole *role)
{
	if (!role) return 0;
	float dist = ccpDistance(getPosition(), role->getPosition());
	return dist;
}

/**
* 获取当前技能
*/
const char * IRole::getNowSkill()
{
	return nowSkill.c_str();
}

Sprite * IRole::getStateSprite()
{
	return getRSM()->nowState()->getSprite();
}

void IRole::doContinueMove()
{
	continueMove = true;
}
void IRole::clearContinueMove()
{
	continueMove = false;
}

Point IRole::getNextPointByDir()
{
	Point dest;
	int dirValue = 212;
	int dValue = 176;
	switch (direction)
	{
		case GameDirs::BACK_DIR:
		{
			dest = ccp(0, dirValue);
		}break;
		case GameDirs::LEFT_DIR:
		{
			dest = ccp(-dirValue, 0);
		}break;
		case GameDirs::RIGHT_DIR:
		{
			dest = ccp(dirValue, 0);
		}break;
		case GameDirs::FRONT_DIR:
		{
			dest = ccp(0, -dirValue);
		}break;
		case GameDirs::BACK_LEFT_DIR:
		{
			dest = ccp(-dValue, dValue);
		}break;
		case GameDirs::FRONT_RIGHT_DIR:
		{
			dest = ccp(dValue, -dValue);
		}break;
		case GameDirs::FRONT_LEFT_DIR:
		{
			dest = ccp(-dValue, -dValue);
		}break;
		case GameDirs::BACK_RIGHT_DIR:
		{
			dest = ccp(dValue, dValue);
		}break;
	}
	return ccpAdd(dest,this->getPosition()); 
}

void IRole::doSendMoveMsg(const Point &point)
{
	Terrain * map = SceneManager::getInstance()->nowTerrain;
	if (!map) return;
	Cmd::stUserMoveMoveUserCmd send;
	send.x = point.x / 64;
	send.y = map->mapGridCountSize.height - point.y / 32;
	send.byDirect = getDirection();
	send.bySpeed = 100;
	if (tempID)
	{
		send.dwUserTempID = tempID;
		theClient.sendMsg(&send, sizeof(send));
	}
}

/**
* 直接移动到某点
*/
void IRole::goTo(const Point &point)
{
	GoToMsg msg;
	msg.point = point;
	getRSM()->PasreMsg(&msg);
}

/**
* 寻路移动到某点
*/
void IRole::moveTo(const Point &point)
{

}