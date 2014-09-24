#include "MainUI.h"
#include "Scene.h"
#include "UtilAnimation.h"
#include "editor-support\cocostudio\CCSGUIReader.h"
#include "IRoleState.h"
bool CDShowItem::init()
{
	// 创建底图
	_sprite = Sprite::create();
	this->addChild(_sprite);
	info = LabelTTF::create("1", "Arial", 16);
	this->addChild(info);
	auto listener = EventListenerTouchOneByOne::create();
	listener->onTouchBegan = [](Touch * touch, Event *event)
	{
		CDShowItem* target = (CDShowItem*)(event->getCurrentTarget());
		
		Point locationInNode = target->convertToNodeSpace(touch->getLocation());
		Size s = target->_sprite->getContentSize();
		Rect rect = Rect(-s.width / 2, -s.height/2, s.width, s.height);
		if (rect.containsPoint(locationInNode))
		{
			if (1 || Tools::isClickTheRealSprite(target->_sprite, locationInNode))
			{
				target->_canMove = true;
				if (target->_valid)
					target->doClickMe();
					
				target->show();
				return true;
			}
		}
		return false;
	};
	listener->onTouchMoved = [](Touch * touch, Event *event)
	{
		CDShowItem* target = (CDShowItem*)(event->getCurrentTarget());
		if (target->_canMove)
		{
		//	target->setPosition(target->getPosition() + touch->getDelta());
		}
	};
	listener->onTouchEnded = [=](Touch* touch, Event* event){
		CDShowItem* target = (CDShowItem*)(event->getCurrentTarget());
		target->_canMove = false;
	};
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this); 

	auto back = Sprite::create("Icons/2.png");
	cdShow = ProgressTimer::create(back);
	this->addChild(cdShow);
	cdShow->setType(ProgressTimer::Type::RADIAL);
	return true;
}
/**
* 展示 cd 时间
*/
void CDShowItem::show()
{
	_valid = false;
	ProgressTo * t = ProgressTo::create(2, 100);
	Action *action = Sequence::create(t, CallFunc::create(this, (SEL_CallFunc)(&CDShowItem::cartoonOver)), NULL);
	cdShow->setPercentage(0);
	cdShow->runAction(action);
}
void CDShowItem::cartoonOver()
{
	_valid = true;
	cdShow->setPercentage(0);
}

void SkillItem::initWithInfo(const char *pngName, int skillId){	
	Texture2D * t2d = TextureCache::getInstance()->addImage(pngName);
	setTexture(t2d);
	this->skillId = skillId;
}
/**
* 设置纹理
*/
void CDShowItem::setTexture(Texture2D *texture)
{
	if (_sprite)
	{
		Rect rect = CCRectMake(0, 0, texture->getContentSize().width, texture->getContentSize().height);
		SpriteFrame *frame = SpriteFrame::createWithTexture(texture, rect);
		_sprite->setSpriteFrame(frame);
	}
}
void SkillItem::doClickMe()
{
	String skillName;
	skillName.initWithFormat("%d", skillId);
	IRole * role = SceneManager::getInstance()->mainRole;
	if (role)
	{
		role->nowSkill = skillName.getCString();
		AttackMsg msg;
		role->getRSM()->PasreMsg(&msg);
	}
}


void SkillShowPanel::push(CDShowItem *item)
{
	items.push_back(item);
	this->addChild(item);
}
void SkillShowPanel::show(){
	Size size = CCDirector::sharedDirector()->getVisibleSize();
	setPosition(size.width - 64, 72);
	const float PI = 3.1415926;
	static float angles[9] = {0,90,135,180,90,120,150,180,170};
	static float radius[9] = { 0, 120, 120, 120, 190, 190, 190, 190, 190 };
	for (int i = 1; i < items.size(); i++){
		float angle = angles[i];
		CDShowItem * item = items[i];
		CCPoint dest =
			ccp(
			cos(angle * PI / 180) * radius[i],
			sin(angle * PI / 180) * radius[i]
			);
		
		item->setPosition(dest);
	}
}
/**
* 通过名字 和 信息初始化聊天框
*/
void TaskTalkUI::initWithInfo(const char *name, const char *info)
{
	auto layout = static_cast<ui::Widget*>(cocostudio::GUIReader::getInstance()->widgetFromJsonFile("Uis/TaskTalk/TaskTalk.ExportJson"));
	this->addChild(layout);
	this->setVisible(false);
}

/**
* 说话
*/
void TaskTalkUI::talk()
{
	this->setVisible(true);
}

bool ChoiceHeroUI::init()
{
	auto layout = static_cast<ui::Widget*>(cocostudio::GUIReader::getInstance()->widgetFromJsonFile("Uis/ChoiceHeroUI/ChoiceHeroUI.ExportJson"));
	this->addChild(layout);
	return true;
}

bool HeroHeadUI::init()
{
	auto layout = static_cast<ui::Widget*>(cocostudio::GUIReader::getInstance()->widgetFromJsonFile("Uis/HeroHeadUI/HeroHeadUI.ExportJson"));
	this->addChild(layout);
	return true;
}

bool ChoiceBossUI::init()
{
	auto layout = static_cast<ui::Widget*>(cocostudio::GUIReader::getInstance()->widgetFromJsonFile("Uis/ChoiceHeroUI/ChoiceBossUI.ExportJson"));
	this->addChild(layout);
	return true;
}

bool QuickUI::init()
{
	auto layout = static_cast<ui::Widget*>(cocostudio::GUIReader::getInstance()->widgetFromJsonFile("Uis/QuickUI/QuickUI.ExportJson"));
	this->addChild(layout);
	return true;
}
bool MainUI::init()
{
	Size size = CCDirector::sharedDirector()->getVisibleSize();
#if (0)
	{
		CDShowItem * ssi = CDShowItem::create();
		Texture2D * t2d = TextureCache::getInstance()->addImage("Icons/1.png");
		ssi->setTexture(t2d);
	//	this->addChild(ssi, 2);
		ssi->setPosition(ccp(400, 400));
		ssi->skillName = "5007";
	}
	
	{
		CDShowItem * ssi = CDShowItem::create();
		Texture2D * t2d = TextureCache::getInstance()->addImage("Icons/3.png");
		ssi->setTexture(t2d);
		ssi->skillName = "5006";
	//	this->addChild(ssi, 2);
		ssi->setPosition(ccp(500, 500));
	}
#endif
	// 创建技能条
	SkillShowPanel *ssp = SkillShowPanel::create();
	{
		SkillItem * item = SkillItem::create();
		if (item)
		{
			item->initWithInfo("Uis/Icons/putskill_up.png", 5000);
			ssp->push(item);
		}
	}
	{
		SkillItem * item = SkillItem::create();
		if (item)
		{
			item->initWithInfo("Uis/Icons/putskill_baoji.png", 5001);
			ssp->push(item);
		}
	}
	{
		SkillItem * item = SkillItem::create();
		if (item)
		{
			item->initWithInfo("Uis/Icons/putskill_baoji.png", 5002);
			ssp->push(item);
		}
	}
	{
		SkillItem * item = SkillItem::create();
		if (item)
		{
			item->initWithInfo("Uis/Icons/putskill_baoji.png", 5003);
			ssp->push(item);
		}
	}
	{
		SkillItem * item = SkillItem::create();
		if (item)
		{
			item->initWithInfo("Uis/Icons/putskill_baoji.png", 5004);
			ssp->push(item);
		}
	}
	{
		SkillItem * item = SkillItem::create();
		if (item)
		{
			item->initWithInfo("Uis/Icons/speed_up_skill.png", 5007);
			ssp->push(item);
		}
	}
	this->addChild(ssp,2);
	ssp->show();

	TaskTalkUI *taskTalk = TaskTalkUI::create();
	taskTalk->setPosition(ccp(size.width/2, size.height / 2 - 100));
//	this->addChild(taskTalk);
	taskTalk->initWithInfo("", "");
	taskTalk->talk();

	ChoiceHeroUI *heroUI = ChoiceHeroUI::create();
	this->addChild(heroUI);
	heroUI->setPosition(ccp(350, size.height-45));

	HeroHeadUI *headUI = HeroHeadUI::create();
	this->addChild(headUI);
	headUI->setPosition(ccp(120, size.height - 60));

	ChoiceBossUI *bossUI = ChoiceBossUI::create();
//	this->addChild(bossUI);
	bossUI->setPosition(ccp(size.width - 300, size.height - 145));


	QuickUI *qucikUI = QuickUI::create();
	//this->addChild(qucikUI);
	qucikUI->setPosition(ccp(size.width /2, 68));

	return true;
}

/************************************************************************************/
// 移动盘
/************************************************************************************/

/////////////////////////移动操作盘////////////////////////////////////////
MoveOPUI* MoveOPUI::create(IRole *player)
{
	MoveOPUI * pRet = new MoveOPUI();
	if (pRet && pRet->init(player))
	{
		pRet->autorelease();
		return pRet;
	}
	CC_SAFE_DELETE(pRet);
	return NULL;
}
bool MoveOPUI::init(IRole *player)
{
	this->player = player;
	CCSprite *opmove = CCSprite::create("Uis/Icons/moveop.png");
	if (opmove)
	{
		this->addChild(opmove);
		opMoveCenter = ccp(136, 136);
		this->setPosition(opMoveCenter);
	}
	return true;
}
/**
* 处理点击
* \parma touchType 点击类型
* \param touchPoint 点击点
*/
bool MoveOPUI::doTouch(int touchType, Touch *touch)
{
	if (!player) return false;

	int opMoveTap = 32;
	int OP_SIZE = 256;
	CCPoint dest = touch->getLocation();
	switch (touchType)
	{
	case MY_TOUCH_DOWN:
	{
		if (nowTouch) return false;
		if (ccpDistance(dest, opMoveCenter) > OP_SIZE / 2) // 只在圆盘内操作有效
		{
			player->clearContinueMove();
			return false;
		}
		CCPoint tsrc = ccp((int)(opMoveCenter.x / opMoveTap), (int)(opMoveCenter.y / opMoveTap));
		CCPoint tdest = ccp((int)(dest.x / opMoveTap), (int)(dest.y / opMoveTap));
		int nowDir = GameDirs::getDirection(tsrc.x, tsrc.y, tdest.x, tdest.y);
		player->setDirection(nowDir);
		player->doContinueMove();
		showTouch(nowDir);
		nowTouchIn = true;
		nowTouch = touch;
		return true;
	}break;
	case MY_TOUCH_MOVE:
	{
		if (nowTouch != touch) return false;
		if (nowTouchIn)
		{
			CCPoint tsrc = ccp((int)(opMoveCenter.x / opMoveTap), (int)(opMoveCenter.y / opMoveTap));
			CCPoint tdest = ccp((int)(dest.x / opMoveTap), (int)(dest.y / opMoveTap));
			int nowDir = GameDirs::getDirection(tsrc.x, tsrc.y, tdest.x, tdest.y);
			player->setDirection(nowDir);
			player->doContinueMove();
			showTouch(nowDir);
			return true;
		}
	}
		break;
	case MY_TOUCH_END:
	{
		if (nowTouch != touch) return false;
		nowTouch = NULL;
		player->clearContinueMove();
		nowTouchIn = false;
		showTouch(-1);
	}break;
	}
	return false;
}
void MoveOPUI::showTouch(int dirType)
{
	if (!tagSprite)
	{
		tagSprite = CCSprite::create("Uis/Icons/movetag.png");
		this->addChild(tagSprite);
	}
	CCPoint dest = ccp(0, 0);
	int dirValue = 112;
	int dValue = 76;
	switch (dirType)
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
			dest = ccp(dValue,dValue);
		}break;
	}
	tagSprite->setPosition(dest);
}