#include "Skill.h"
#include "UtilAnimation.h"
#include "SimpleAudioEngine.h"
/*********************************************************************************************/
// 音效
/*********************************************************************************************/
void SkillSoundEffect::stopPlay()
{
//	CocosDenshion::SimpleAudioEngine::getInstance()->stopEffect(soundId);
}
void SkillSoundEffect::playSound()
{
	CCString soundPath;
	soundPath.initWithFormat("Sounds/%s%s", config->strID.c_str(), ".mp3");
	soundId = CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(soundPath.getCString(), config->repeat);
}
/**********************************************************************************************/
// 子技能效果
/**********************************************************************************************/
void SkillSubEffect::startPlay()
{
	interval = 0;
	playFinish = false;
	frameIndex = 0;
	currFrame = 1;
	calcStartPosition();
	currDelayCount = 0;
	startPos = getPosition();
	maxAtkDisttance = 1;
	attackDistance = 300;
	changeFrame();
	show(true);
	costDt = 0;
}
bool SkillSubEffect::init(Skill *skill)
{
	this->skill = skill;
	if (!sprite)
	{
		sprite = CCSprite::create();
		this->addChild(sprite);
	}
	cacheAnimation.animation = NULL;
	CCString ID;
	ID.initWithFormat("%d", config->useForSort);
	cacheAnimation = UtilParser::getInstance()->getCacheEffectAnimation("effect.effs", ID.getCString());
	if (cacheAnimation.animation)
	cacheAnimation.animation->retain();
	this->scheduleUpdate();
	calcTotalFrame();
	currDir = 0;
	sound.config = &config->soundConfig;
	return true;
}
void SkillSubEffect::update(float dt)
{
	checkHurt(); // 检查伤害
	if (checkFinish())
	{
		playFinish = true;
		sound.stopPlay();

		show(false);
		return;
	}
	updatePosition(dt);
	updateRotate(dt);
	updateSound(dt);
	if (!costDt || costDt > 0.05)
	{
		currFrame++;
		changeFrame();
		costDt = 0;
	}
	costDt += dt;
}
bool SkillSubEffect::checkHurt()
{
	if (!config->hurtTypeConfig.genHurt) return false;

	switch (config->hurtTypeConfig.generateType)
	{
		case SubEffectConfig::HurtTypeConfig::GENERATE_WHEN_ATTACK:
		{
			std::vector<IRole*> roles;
			if (skill->checkHurts(roles,this->getPosition(), this->sprite, config->hurtTypeConfig.hurtEffectID))
			{
				if (config->deadTypeConfig.deadType == SubEffectConfig::DeadTypeConfig::DEAD_TYPE_ATTACK)
				{
					playFinish = true;
					show(false);
				}
				interval = 1;
				return true;
			}
		}break;
		case SubEffectConfig::HurtTypeConfig::GENERATE_WHEN_FRAMES:
		{
			std::vector<IRole*> roles;
			if (skill->checkHurts(roles, this->getPosition(), this->sprite, config->hurtTypeConfig.hurtEffectID))
			{
				interval = 1;
				return true;
			}
		}break;
		default:
			break;
	}
	return false;
}
void SkillSubEffect::updatePosition(float dt)
{
	if (config->moveStyleConfig.moveType == SubEffectConfig::MoveStyleConifg::MOVE_TYPE_STAY)
	{
		return; // 原地播放
	}
	//位置基准
	if (false == config->offsetConfig.baseOnSelf)
	{
		if (1 == currFrame)
		{
			// 获取攻击者的位置
			this->setPosition( skill->GetAttackObjectPosition());
		}
	}
	Point targetpos;
	if (config->moveStyleConfig.moveType == SubEffectConfig::MoveStyleConifg::MOVE_TYPE_TOTARGET)
	{
		// 获取攻击者位置 
		targetpos = skill->GetAttackObjectPosition();
		moveAngle = getPointAngle(targetpos.x - this->getPositionX(), targetpos.y - this->getPositionY());
		if (config->moveStyleConfig.rotateWithMove == true)
		{
			rotateAngle = getPointAngle(cos(moveAngle), sin(moveAngle));
			rotateAngle = rotateAngle * 180 / 3.1415926;
		}
	}
	else if (config->moveStyleConfig.moveType == SubEffectConfig::MoveStyleConifg::MOVE_TYPE_CONNECT)
	{
		targetpos = skill->GetAttackObjectPosition();
		Point objpos = skill->GetGameObjectPosition();
		int valx = targetpos.x - objpos.x;
		int valy = targetpos.y - objpos.y;
		connectDistance = sqrt(valx * valx + valy * valy);

		rotateAngle = getPointAngle((targetpos.x - getPositionX()), (targetpos.y - getPositionY()));
		rotateAngle = rotateAngle * 180 /3.1415926;
	}
	float speed = nowSpeed;
	if (interval > 0)
	{
		speed = 0;
		interval--;
	}
	currSpeedX = speed * cos(2 * 3.14 - moveAngle) * dt * 1000;
	currSpeedY = speed * sin(2 * 3.14  - moveAngle) * dt * 1000;

	nowSpeed += accelerate * dt;
	setPositionX(getPositionX() + currSpeedX);
	setPositionY(getPositionY() - currSpeedY);
}

void SkillSubEffect::updateRotate(float dt)
{
	rotateSpeed = config->rotateConfig.rotateAcceleration;
	rotateAngle += rotateSpeed;
}

void SkillSubEffect::updateSound(float dt)
{
	if (config->soundConfig.withSound)
	{
		if (currFrame == config->soundConfig.playFrame)
		{
			sound.playSound();
		}
	}
}

void SkillSubEffect::changeFrame()
{
	if (currDelayCount)
	{
		currDelayCount--;
	}
	else
	{
		if (cacheAnimation.animation)
		{
			checkFrameCtrl(frameIndex + 1);
			// 做镜像
			
			int nowDir = picDir;
			if (cacheAnimation.dirs < 4) nowDir = 0;
			AnimationFrame *aFrame = cacheAnimation.getFrame(frameIndex, nowDir);
			if (aFrame)
			{
				SpriteFrame * frame = aFrame->getSpriteFrame();
				if (frame)
				{
					sprite->setDisplayFrame(frame);
				}
			}
			
			if (currDir > 4 && cacheAnimation.dirs > 4)
			{
				sprite->setFlipX(true);
			}
			else
			{
				sprite->setFlipX(false);
			}
			if (frameIndex + 1 < cacheAnimation.animation->getFrames().size())
			{
				frameIndex++;
			}
			else
			{
				frameIndex = 0;
			}
		}
		else
		{
			checkFrameCtrl(frameIndex + 1);
		}
	}
	if (config->moveStyleConfig.moveType == SubEffectConfig::MoveStyleConifg::MOVE_TYPE_CONNECT)
	{
		if (sprite)
		{
			connectDistance = connectDistance / sprite->getBoundingBox().size.width;
			sprite->setScaleX(connectDistance);
		}
	}

	if (rotateAngle)
	{
		sprite->setRotation(-rotateAngle);
	}
}
void SkillSubEffect::checkFrameCtrl(int frame)
{
	currDelayCount = 0;
	for (int i = frame; i < config->frameControl.preDirFrames; i++)
	{
		FrameCtrlInfo *info = config->frameControl.getCtrlInfo(i);
		if (info)
		{
			if (info->ctrlType == FrameCtrlInfo::DELAY)
			{
				currDelayCount = info->delayCount;
			}
			else
			{
				if (info->ctrlType == FrameCtrlInfo::JUMP)
				{
					frameIndex++;
				}
				else
				{
					break;
				}
			}
		}
		else
			break;
	}
}
void SkillSubEffect::show(bool visible)
{
	if (this->isVisible() == visible)
	{
//		return;
	}
	setVisible(visible);
	// 加入到指定的层
	Node *layer = skill->getEffectLayer(config->layerConfig.layer);
	if (!layer)
	{
		return;
	}
	if (isVisible())
	{
		layer->addChild(this,2);
		startPlayTag = true;
	}
	else
	{
		layer->removeChild(this);
		startPlayTag = false;
	}
}
float SkillSubEffect::getPointAngle(float x, float y)
{
	float dis = sqrt(x *x + y*y);
	float angle = acos(x / dis);
	if (y < 0)
		angle = - angle;
	return angle;
}

void  SkillSubEffect::calcTotalFrame()
{
	totalFrame = 0;
	for (int i = 0; i < config->frameControl.preDirFrames; i++)
	{
		FrameCtrlInfo *info = config->frameControl.getCtrlInfo(i);
		if (info)
		{
			if (info->ctrlType == FrameCtrlInfo::JUMP)
				continue;
			else if (info->ctrlType == FrameCtrlInfo::DELAY)
				totalFrame += info->delayCount + 1;
		}
		else
		{
			totalFrame++;
		}
	}
	
	if (0 == totalFrame)
	{
		totalFrame == config->frameControl.preDirFrames;
	}
}
bool SkillSubEffect::checkFinish()
{
	bool bFinish = false;
	if (checkOutOfRange()) return true;
	switch (config->deadTypeConfig.deadType)
	{
		case SubEffectConfig::DeadTypeConfig::DEAD_TYPE_AFTERDONE:
		{
			startPlayTag = false;
			if (currFrame >= totalFrame)
			{
				bFinish = true;
			}
		}break;
		case SubEffectConfig::DeadTypeConfig::DEAD_TYPE_ATTACK:
		{
			if (1 < currFrame && haveReach())
				bFinish = true;
		}break;
		case SubEffectConfig::DeadTypeConfig::DEAD_TYPE_FRAMES:
		{
			if (config->deadTypeConfig.deadFrames <= currFrame)
			{
				bFinish = true;
			}
		}break;
		case SubEffectConfig::DeadTypeConfig::DEAD_TYPE_ATTACKNUM:
		{
			if (bFinish || currFrame >= totalFrame)
				bFinish = true;
		}break;
		default:
			bFinish = true;
			break;
	}
	return bFinish;
}
bool SkillSubEffect::haveReach()
{
	int valx = this->getPositionX();
	int valy = this->getPositionY();
	Point targetpos = skill->GetAttackObjectPosition();
	float speedX = currSpeedX;
	float speedY = currSpeedY;


	int dx = fabs(targetpos.x - valx);
	int dy = fabs(targetpos.y - valy);
	if (dx < fabs(speedX))
	{
		reachX = true;
	}
	if (dy < fabs(speedY))
	{
		reachY = true;
	}
	if (reachX && reachY)
	{
		reachX = reachY = false;
		return true;
	}
	return false;
}

bool SkillSubEffect:: checkOutOfRange()
{
	return false;
	if (maxAtkDisttance)
	{
		float dltX = fabs(this->getPositionX() - startPos.x);
		float dltY = fabs(this->getPositionY() - startPos.y);
		float dist = dltX * dltX + dltY * dltY;
		if (dist < attackDistance)
			return true;
	}
	return false;
}

void SkillSubEffect::calcStartPosition()
{
	if (config->offsetConfig.baseOnSelf)
	{
		this->setPosition(skill->GetGameObjectPosition());
	}
	else
	{
		this->setPosition(skill->GetAttackObjectPosition());
	}
	rotateAngle = 0;
	moveAngle = 0;
	Point targetpos = skill->GetAttackObjectPosition();
	Point objpos = skill->GetGameObjectPosition();
	if (targetpos.equals(objpos))
	{
		// 根据方向计算 targetpos
		switch (currDir)
		{
			case GameDirs::FRONT_DIR:
			{
				targetpos.y -= 1;
				break;
			}
			case GameDirs::FRONT_RIGHT_DIR:
			{
				targetpos.x += 1;
				targetpos.y -= 1;
				break;
			}
			case GameDirs::RIGHT_DIR:
			{
				targetpos.x += 1;
				break;
			}
			case GameDirs::BACK_RIGHT_DIR:
			{
				targetpos.x += 1;
				targetpos.y += 1;
				break;
			}
			case GameDirs::BACK_DIR:
			{
				targetpos.y += 1;
				break;
			}
			case GameDirs::BACK_LEFT_DIR:
			{
				targetpos.x -= 1;
				targetpos.y += 1;
				break;
			}
			case GameDirs::LEFT_DIR:
			{
				targetpos.x -= 1;
				break;
			}
			case GameDirs::FRONT_LEFT_DIR:
			{
				targetpos.x -= 1;
				targetpos.y -= 1;
				break;
			}
		}
	}
	//旋转角度,移动角度
	switch (config->moveStyleConfig.moveType)
	{
		case SubEffectConfig::MoveStyleConifg::MOVE_TYPE_STAY:
		{}break;
		case SubEffectConfig::MoveStyleConifg::MOVE_TYPE_TOTARGET:
		{
			moveAngle = getPointAngle(targetpos.x - objpos.x, targetpos.y - objpos.y);
			if (config->moveStyleConfig.rotateWithMove)
			{
				rotateAngle = getPointAngle(cos(moveAngle), sin(moveAngle));
			}
		}break;
		case SubEffectConfig::MoveStyleConifg::MOVE_TYPE_TODIRECTION:
		{
			float dstAngle = 0;
			if (targetpos.equals(objpos))
			{
				dstAngle = deg2Rad(getDirDegree(currDir));
			}
			else
			{
				dstAngle = getPointAngle(targetpos.x - objpos.x, targetpos.y - objpos.y);
			}
			float tempMoveAngle = deg2Rad(config->moveStyleConfig.angle);
			moveAngle = config->moveStyleConfig.faceToDirection ? dstAngle + tempMoveAngle : tempMoveAngle;
		
			if (config->moveStyleConfig.rotateWithMove)
			{
				rotateAngle = getPointAngle(cos(moveAngle), sin(moveAngle));
			}
		}break;
		case SubEffectConfig::MoveStyleConifg::MOVE_TYPE_CONNECT:
		{
			int valx = targetpos.x - objpos.x;
			int valy = targetpos.y - objpos.y;
			connectDistance = sqrt(valx * valx + valy * valy);
			rotateAngle = getPointAngle(targetpos.x - objpos.x, targetpos.y - objpos.y);
		}break;
	}
	if (false == config->offsetConfig.relatedWithSelf && config->offsetConfig.baseOnSelf && config->moveStyleConfig.moveType
		!= SubEffectConfig::MoveStyleConifg::MOVE_TYPE_STAY)
	{
		Point pp = config->offsetConfig.getOffsetWithDir(2);
		float tLen = sqrt(pp.x* pp.x + pp.y * pp.y);
		float offang = 0;
		if (tLen > 0)
		{
			offang = getPointAngle(pp.x, pp.y);
		}
		int value = moveAngle * 180 / 3.1415926;
		offang += moveAngle;
		float dy = tLen * sin(offang);
		float dx = tLen * cos(offang);
		setPositionY(getPositionY() + dy);
		setPositionX(getPositionX() + dx);
	}
	else
	{
		Point offset = config->offsetConfig.getOffsetWithDir(currDir);
		setPosition(ccpAdd(getPosition(),offset));
	}

	if (config->moveStyleConfig.moveType != SubEffectConfig::MoveStyleConifg::MOVE_TYPE_STAY)
	{
		nowSpeed = config->moveStyleConfig.moveSpeed;
		accelerate = config->moveStyleConfig.moveAcceleration;
	}
	else
	{
		nowSpeed = 0;
		accelerate = 0;
	}
	rotateSpeed = config->rotateConfig.rotateSpeed;
	sprite->setPositionY(-config->offsetConfig.heightOffset);
	rotateAngle = rotateAngle * 180 / 3.1415926;
}
void SkillSubEffect::changeDir(int dir)
{
	currDir = dir;
	if (dir <= 4)
	{
		picDir = dir;
	}
	else
		picDir = 8 - dir;
}
float SkillSubEffect::getDirDegree(int val)
{
	//float arr[8] = {90.0, 26.5, 0, -26.5, -90.0, -153.5, 180.0, 153.5 };
	float arr[8] = {-90.0, -26.5, 0, 26.5, 90.0, 153.5, -180.0, -153.5 };
	if (val < 0)
	{
		val = 0;
	}
	val = val % 8;
	return arr[val];
}
/**********************************************************************************************/
// 技能效果
/**********************************************************************************************/
void SkillEffect::startPlay()
{
	for (SUBEFFECTS_ITER iter = subEffects.begin(); iter != subEffects.end(); ++iter)
	{
		SkillSubEffect*effect = *iter;
		if (effect)
		{
			if (effect->config->playOrderConfig.playIndependent)
			{
				effect->startPlay();
				break;
			}
			else effect->startPlay();
		}
	}
}

void SkillEffect::init(Skill *skill)
{
	if (!config) return;
	subEffects.clear();
	for (int i = 0; i < config->subEffects.size(); i++)
	{
		SubEffectConfig *sConfig = &config->subEffects[i];
		SkillSubEffect* subEffect = SkillSubEffect::create();
		subEffect->config = sConfig;
		subEffect->init(skill);
		subEffect->retain();
		subEffects.push_back(subEffect);
	}
}

void SkillEffect::changeDir(int dir)
{
	for (SUBEFFECTS_ITER iter = subEffects.begin(); iter != subEffects.end(); ++iter)
	{
		SkillSubEffect*effect = *iter;
		if (effect)
		{
			effect->changeDir(dir);
		}
	}
}
/**********************************************************************************************/
// 技能动作
/**********************************************************************************************/
/**
* 获取动作帧
*/
SpriteFrame * SkillAction::getActionFrame()
{
	// 根据方向 帧信息 获取当前帧
	AnimationFrame * aFrame = cacheAnimation.getFrame(currFrameInEle, dir);
	SpriteFrame * frame = NULL;
	if (aFrame) frame = aFrame->getSpriteFrame();
	if (0 < currDelayCount)
	{
		currDelayCount--;
	}
	else
	{
		currFrameInEle++;
		this->checkFrameCtrl(currFrameInEle + 1);
	}
	currFrame++;
	return frame;
}
/**
* 重置状态 获取新的动作集合
*/
void SkillAction::changeAction()
{
	currFrame = 0;
	currFrameInEle = 0;
	cacheAnimation = UtilParser::getInstance()->getCacheAnimation(skill->getRoleName().c_str(), config->actionID.c_str());
	//根据strID 获取Animation
	this->checkFrameCtrl(currFrameInEle + 1);
	if (cacheAnimation.animation)
		cacheAnimation.animation->retain();
}

/**
* 切换方向
*/
void SkillAction::changeDir(int dir)
{
	this->dir = dir;
	if (dir > 4) this->dir = 8 - dir;
}
//检查下一帧的控制信息
//延迟的记录延迟的帧数
//跳过的当前帧加一
bool SkillAction::checkFrameCtrl(int frame)
{
	for (int i = frame; i < config->perDirFrames; i++){
		FrameCtrlInfo * ctrlInfo = config->getCtrlInfo(i);
		if (ctrlInfo)
		{
			if (ctrlInfo->ctrlType == FrameCtrlInfo::JUMP)
			{
				currFrameInEle++;
			}
			else if (ctrlInfo->ctrlType == FrameCtrlInfo::DELAY)
			{
				currDelayCount = ctrlInfo->delayCount;
				break; 
			}
		}
		else
		{
			break;
		}
	}
	return true;
}

void SkillAction::startPlay()
{
	
}

void SkillAction::init(Skill *skill)
{
	this->skill = skill;
}

void SkillAction::end()
{
	if (cacheAnimation.animation)
		cacheAnimation.animation->release();
	cacheAnimation.animation = NULL;
}
/***********************************************************************************************/
// 每帧控制信息
/***********************************************************************************************/
void SkillFrame::init(Skill *skill)
{
	action.config = &config->action; // 配置赋值
	action.init(skill);
	sound.config = &config->soundConfig; // 声音赋值
	for (int i = 0; i < config->effects.size(); i++)
	{
		SkillEffect effect;
		effect.config = &config->effects[i];
		effect.init(skill);
		effects.push_back(effect);
	}
}
void SkillFrame::startPlay()
{
	for (EFFECTS_ITER iter = effects.begin(); iter != effects.end(); ++iter)
	{
		SkillEffect &effect = *iter;
		effect.startPlay();
	}
	if (sound.config && sound.config->strID != "")
	{
		sound.stopPlay();
		sound.playSound();
	}
}
void SkillFrame::changeDir(int dir)
{
	action.changeDir(dir);
	for (EFFECTS_ITER iter = effects.begin(); iter != effects.end(); ++iter)
	{
		SkillEffect &effect = *iter;
		effect.changeDir(dir);
	}
}
/***********************************************************************************************/
// 技能系统
/***********************************************************************************************/
void Skill::init()
{
	actionPlayFinish = false;
	currFrame = 1;
	nowAction = NULL;
	if (!config) return;
	// 创建效果
	for (int i = 0; i < config->configLists.size(); i++)
	{
		SkillFrame frame;
		frame.config = &config->configLists[i];
		frame.init(this);
		frames.push_back(frame);
	}
	Size visibleSize = CCDirector::sharedDirector()->getVisibleSize();
	rtForC = CCRenderTexture::create(visibleSize.width * 2, visibleSize.height * 2);
	rtForC->setPosition(ccp(visibleSize.width, visibleSize.height));
	rtForC->retain();
	rtForC->setVisible(false);
}

void Skill::startPlay()
{
	if (!config) return;
	for (FRAMES_ITER iter = frames.begin(); iter != frames.end(); ++iter)
	{
		SkillFrame *frame = &(*iter);
		frame->startPlay();
	}
}
void Skill::update()
{
	if (!config) return;
	checkUpdate(currFrame);
	currFrame++;
	if (currFrame > config->attrs.skillTotalFrames)
	{
		actionPlayFinish = true;
	}
}
void Skill::end()
{
	frames.clear();
	nowAction = NULL;
}
void Skill::checkUpdate(int frame)
{
	if (!config) return;
	for (int i = 0; i < frames.size(); i++)
	{
		SkillFrame &sFrame = frames[i];
		if (sFrame.config->frameID == frame)
		{
			sFrame.action.changeAction();
			if (sFrame.action.cacheAnimation.animation)
			{
				nowAction = &sFrame.action;
			}
			
			sFrame.startPlay();
		}
	}
}
#include "Scene.h"
Point Skill::GetAttackObjectPosition()
{
	if (target) return target->getPosition();
	if (!targetPosition.equals(ccp(0,0)))return targetPosition;
	return SceneManager::getInstance()->getAttackObjectPosition(parent);
}
Point Skill::GetGameObjectPosition()
{
	return parent->getPosition();
}

SpriteFrame * Skill::getActionFrame()
{
	if (nowAction)
		return nowAction->getActionFrame();
	return NULL;
}

void Skill::setCurrectDir(int dir)
{
	dir = GameDirs::getDirection(GetGameObjectPosition().x, GetGameObjectPosition().y, GetAttackObjectPosition().x, GetAttackObjectPosition().y);
	this->dir = dir;
	for (FRAMES_ITER iter = frames.begin(); iter != frames.end(); ++iter)
	{
		iter->changeDir(dir);
	}
}

Node * Skill::getEffectLayer(int ID)
{
	return parent->getParent();
}
#include "CollisionDetection.h"
bool Skill::checkCollide(Sprite *src, Sprite *dest)
{
	if (CollisionDetection::GetInstance()->areTheSpritesColliding(src, dest, true, rtForC)) {
		return true;
	}
	return false;
}
#include "Scene.h"
bool Skill::checkHurts(std::vector<IRole*>& out, const Point &point, Sprite *src, const std::string & effectID)
{
	Sprite *temp = Sprite::createWithTexture(src->getTexture());
	temp->setPosition(point);
	if (SceneManager::getInstance()->checkHurts(out, temp))
	{
		for (std::vector<IRole*>::iterator iter = out.begin(); iter != out.end(); ++iter)
		{
			IRole * role = *iter;
			BeAttackMsg msg;
			msg.from = this->me;
			role->getRSM()->PasreMsg(&msg);
		}
		
		return false;
	}
	return false;
}