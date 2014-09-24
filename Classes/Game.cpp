#include "Game.h"
#include "IRoleState.h"
#include "XmlSkills.h"
#include "UtilAnimation.h"
#include "RpgFollow.h"
#include "MyClient.h"
#include "Protos/Command.h"
#include "Data.h"
USING_NS_CC;

Scene* Game::createScene()
{
	auto scene = Scene::create();
	auto layer = Game::create();
	scene->addChild(layer);
	return scene;
}

bool Game::init()
{
	if (!Layer::init())
	{
		return false;
	}

	Size visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	IRole * role = IRole::create();
	role->changeAnimation("1");
	role->play();
	//role->setPosition(ccp(1900, 1600));
	SceneManager::getInstance()->mainRole = role;
	this->setTouchEnabled(true);
	moveOPUI = MoveOPUI::create(role);
	this->addChild(moveOPUI,4);
	
	auto listener = EventListenerTouchOneByOne::create();  //创建一个单点触摸事件 :EventListenerTouchAllAtOnce 为多点
	//设置这些事件的的回调方法
	listener->onTouchBegan = CC_CALLBACK_2(Game::onTouchBegan, this);
	listener->onTouchEnded = CC_CALLBACK_2(Game::onTouchEnded, this);
	listener->onTouchMoved = CC_CALLBACK_2(Game::onTouchMoved, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this); //事件调度器

	Terrain * map = Terrain::create();
	
	SceneManager::getInstance()->nowTerrain = map;
	map->retain();
#if (0)
	map->setUrl("127.0.0.1/Maps/");
	map->setMapName("100");
	map->setGridCount(100, 100);
	map->setViewGridCount(8, 5);
	map->setGridSize(256, 256);
	map->addToScene(this);
	map->setPosition(0, 0);
#endif
	

	SceneManager::getInstance()->addTo(SceneManager::ROLE_LAYER, role);

	for (int i = 1; i <= 3; i++)
	{
		IRole * role = IRole::create();
		String info;
		info.initWithFormat("%d", i);
		role->changeAnimation(info.getCString());
		role->setPosition(ccp(1900 + i * 100, 1600 + i * 30));
		SceneManager::getInstance()->addOtherRole(role);
		role->setDirection(i);
		role->play();
	}

	MainUI * mainUI = MainUI::create();
	this->addChild(mainUI, 4);
	SceneManager::getInstance()->uiLayer = mainUI;
	this->scheduleUpdate();

	MyNetNode::getInstance()->addTo(this); // 调用啦

	MyClient *client = new MyClient();
	if (client)
	{
		client->decodeType = 0;
		client->init("192.168.11.158", 16004);
		MyNetNode::getInstance()->addClient(0, client);
		client->get(Cmd::LOGON_USERCMD, Cmd::SERVER_RETURN_LOGIN_OK, this, std::bind(&Game::doGetGate, (Game*) this,std::placeholders::_1,std::placeholders::_2)); // 请求当前网关
	
		Cmd::stSessionKeyLoginCmd send;
		strncpy(send.pstrName, "tet13", MAX_NAMESIZE);
		send.zone = 1;
		send.sessionType = 1;
		client->sendMsg(&send, sizeof(send));
	}

	BIND_MSG(Cmd::LOGON_USERCMD, Cmd::SERVER_RETURN_LOGIN_FAILED, Game, doLoginError);
	BIND_MSG(Cmd::SELECT_USERCMD, Cmd::USERINFO_SELECT_USERCMD_PARA, Game, doLoginOk);
	BIND_MSG(Cmd::DATA_USERCMD, Cmd::MAPSCREENSIZE_DATA_USERCMD_PARA, Game, doGetMap);
	BIND_MSG(Cmd::MAPSCREEN_USERCMD, Cmd::MAPDATA_MAPSCREEN_USERCMD_PARA, Game, doGetOthers);
	BIND_MSG(Cmd::MOVE_USERCMD, Cmd::NPCMOVE_MOVE_USERCMD_PARA, Game, doMoveNpc);
	BIND_MSG(Cmd::MOVE_USERCMD, Cmd::USERMOVE_MOVE_USERCMD_PARA, Game,doMoveUser);
	BIND_MSG(Cmd::DATA_USERCMD, Cmd::MAIN_USER_DATA_USERCMD_PARA, Game, doGetMainRoleData);
	BIND_MSG(Cmd::MAPSCREEN_USERCMD, Cmd::ADDUSER_AND_POS_MAPSCREEN_STATE_USERCMD_PARA, Game, doAddOther);
	BIND_MSG(Cmd::MAPSCREEN_USERCMD, Cmd::ADDMAPNPC_AND_POS_MAPSCREEN_STATE_USERCMD_PARA, Game, doAddNpc);
	BIND_MSG(Cmd::MAGIC_USERCMD, Cmd::MAGIC_USERCMD_PARA, Game, doAttackRole);//攻击对象
	initAllInfos();
	return true;
}


void Game::onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *event){
	if (moveOPUI) moveOPUI->doTouch(MY_TOUCH_MOVE, touch);
}
bool Game::onTouchBegan(Touch* touch, Event  *event)
{
	if (moveOPUI && moveOPUI->doTouch(MY_TOUCH_DOWN, touch)) return true;
	if (SceneManager::getInstance()->mainRole)
	{
		TouchMsg moveMsg;
		moveMsg.point = SceneManager::getInstance()->nowTerrain->convertTouchToNodeSpace(touch);// ->getLocation();
		SceneManager::getInstance()->mainRole->getRSM()->PasreMsg(&moveMsg);
	}
	return true;
}
//通过点击结束区域 来设置 lable的位置
void Game::onTouchEnded(Touch* touch, Event  *event)
{
	if (moveOPUI) moveOPUI->doTouch(MY_TOUCH_END, touch);
}

/**
*  更新自己
**/
void Game::update(float dt)
{
	SceneManager::getInstance()->doTick(dt);
}

/**
* 返回获取的网关
*/
void Game::doGetGate(void*cmd, int len)
{
	// 连接网关
	Cmd::stServerReturnLoginSuccessCmd *ret = (Cmd::stServerReturnLoginSuccessCmd*)cmd;
	if (ret)
	{
		theClient.init(ret->pstrIP, ret->wdPort);
		theClient.decodeType = 1;
		DES_cblock block;
		memcpy(block, ret->key, 8);
		theClient.setDesKey(&block);
		MyNetNode::getInstance()->addClient(1, &theClient);

		Cmd::stPasswdLogonUserCmd send; // 登录到网关
		send.loginTempID = ret->loginTempID;
		send.dwUserID = ret->dwUserID;
		strncpy(send.pstrName, "tet13", MAX_NAMESIZE);
		send.zone = 1;
		theClient.sendMsg(&send, sizeof(send));
	}
}

/**
* 处理无档案的情况
*/
void Game::doLoginError(void *cmd, int len)
{
	Cmd::stCreateSelectUserCmd send;
	strncpy(send.strUserName, "tet13", MAX_NAMESIZE);
	send.profession = 1;
	send.charType = 1;
	send.country = 6;
	send.sex = 1;
	theClient.sendMsg(&send, sizeof(send));
}

void Game::doLoginOk(void *cmd, int len)
{
	Cmd::stUserInfoUserCmd *ret = (Cmd::stUserInfoUserCmd *) cmd;
	if (ret->charInfo[0].id == 0)
	{
		Cmd::stCreateSelectUserCmd send;
		strncpy(send.strUserName, "tet13", MAX_NAMESIZE);
		send.profession = 1;
		send.charType = 1;
		send.country = 6;
		send.sex = 1;
		theClient.sendMsg(&send, sizeof(send));
	}
	else
	{
		Cmd::stLoginSelectUserCmd send;
		send.charNo = 0;
		theClient.sendMsg(&send, sizeof(send));
	}
}

/*
* 处理地图信息
**/
void  Game::doGetMap(void *cmd, int len)
{
	Cmd::stMapScreenSizeDataUserCmd * ret = (Cmd::stMapScreenSizeDataUserCmd*) cmd;
	printf("%d,%d", ret->width, ret->height);

	Terrain * map = SceneManager::getInstance()->nowTerrain;
	map->setUrl("127.0.0.1/Maps/");
	map->setMapName("100");
	
	map->setGridCount(100, 100);
	map->setViewGridCount(8, 5);
	map->setGridSize(256, 256);
	map->addToScene(this);
	map->setPosition(0, 0);
	map->release();

	IRole * role = SceneManager::getInstance()->mainRole;
	role->setPosition(ret->mainRoleX * 64, (ret->height- ret->mainRoleY) * 32);
	
	map->runAction(RpgFollow::create(CCSizeMake(ret->width * 64, ret->height * 32), role));

	for (int index = 0; index < ret->npc_count; index++)
	{
		map->setGridCount(ret->width, ret->height);
		int id = ret->npc_list[index].id;
		int x = ret->npc_list[index].x;
		int y = ret->npc_list[index].y;
#if 0
		IRole * role = IRole::create();
		String info;
		info.initWithFormat("%d", 219);
		role->changeAnimation(info.getCString());
		role->setPosition(ccp(x * 64, (ret->height -  y) * 32));
		SceneManager::getInstance()->addOtherRole(role);
		role->setDirection(1);
		role->play();
#endif
	}
	Cmd::stLoadMapOKDataUserCmd loadMapOk;
	theClient.sendMsg(&loadMapOk, sizeof(loadMapOk));
}
#include "NpcBase.h"
void Game::doGetOthers(void *cmd, int len)
{
	Terrain * map = SceneManager::getInstance()->nowTerrain;
	IRole * mainRole = SceneManager::getInstance()->mainRole;
	if (!map) return;
	Cmd::stMapDataMapScreenUserCmd* ret = (Cmd::stMapDataMapScreenUserCmd*) cmd;
	if (ret)
	{
		if (ret->mdih.type == Cmd::MAPDATATYPE_NPC)
		{
			for (int i = 0; i < ret->mdih.size; i++)
			{
				Cmd::t_MapNpcDataPos * data = &ret->mnd[i];
				if (data)
				{
					int tempid = data->dwMapNpcDataPosition;
					int x = data->x;
					int y = data->y;
					IRole * role = IRole::create();
					String info;
					int pic = 0;
					NpcBase *base = npcbm.get(data->dwNpcDataID);
					if (base)
					{
						pic = base->pic;
					}
					info.initWithFormat("%d", pic);
					role->changeAnimation(info.getCString());
					role->setPosition(ccp(x * 64, (map->mapGridCountSize.height - y) * 32));
					SceneManager::getInstance()->addOtherRole(role);
					SceneManager::getInstance()->addNpcByTempId(tempid, role);
					role->setDirection(data->byDir);
					role->play();
				}
			}
		}
		if (ret->mdih.type == Cmd::MAPDATATYPE_USER)
		{
			for (int i = 0; i < ret->mdih.size; i++)
			{
				Cmd::t_MapUserDataPos * data = &ret->mud[i];
				if (data && data->dwUserTempID != mainRole->tempID)
				{
					int tempid = data->dwUserTempID;
					int x = data->x;
					int y = data->y;
					IRole * role = IRole::create();
					String info;
					info.initWithFormat("%d", 3);
					role->changeAnimation(info.getCString());
					role->setPosition(ccp(x * 64, (map->mapGridCountSize.height - y) * 32));
					SceneManager::getInstance()->addOtherRole(role);
					SceneManager::getInstance()->addUserByTempId(tempid, role);
					role->setDirection(data->byDir);
					role->play();
				}
			}
		}
	}
}

/**
* 移动Npc
*/
void Game::doMoveNpc(void *cmd, int len)
{
	Terrain * map = SceneManager::getInstance()->nowTerrain;
	if (!map) return;
	Cmd::stNpcMoveMoveUserCmd * ret = (Cmd::stNpcMoveMoveUserCmd*) cmd;
	IRole *npc = SceneManager::getInstance()->getNpcByTempId(ret->dwNpcTempID);
	if (npc)
	{
		npc->goTo( ccp(ret->x * 64, (map->mapGridCountSize.height - ret->y) * 32));
	}
}

/**
* 移动User
*/
void Game::doMoveUser(void *cmd, int len)
{
	Terrain * map = SceneManager::getInstance()->nowTerrain;
	if (!map) return;
	Cmd::stUserMoveMoveUserCmd * ret = (Cmd::stUserMoveMoveUserCmd*) cmd;
	IRole *user = SceneManager::getInstance()->getUserByTempId(ret->dwUserTempID);
	IRole * mainRole = SceneManager::getInstance()->mainRole;
	if (user)
	{
		if (user != mainRole)
		{
			user->goTo(ccp(ret->x * 64, (map->mapGridCountSize.height - ret->y) * 32));
		}
	}
}

void Game::doGetMainRoleData(void *cmd, int len)
{
	Cmd::stMainUserDataUserCmd * ret = (Cmd::stMainUserDataUserCmd*)cmd;
	if (ret)
	{
		IRole * role = SceneManager::getInstance()->mainRole;
		if (role)
		{
			role->tempID = ret->data.dwUserTempID;
			IRole * role = SceneManager::getInstance()->getUserByTempId(ret->data.dwUserTempID);
			if (!role)
				SceneManager::getInstance()->addUserByTempId(role->tempID, role);
		}
		else
		{
			IRole * role = SceneManager::getInstance()->getUserByTempId(ret->data.dwUserTempID);
			//SceneManager::getInstance()->mainRole = role;
		}
	}
}

void Game::doAddOther(void *cmd, int len)
{
	Terrain * map = SceneManager::getInstance()->nowTerrain;
	if (!map) return;
	Cmd::stAddUserAndPosMapScreenStateUserCmd *ret = (Cmd::stAddUserAndPosMapScreenStateUserCmd *)cmd;
	if (ret->data.dwUserTempID)
	{
		int tempid = ret->data.dwUserTempID;
		int x = ret->data.x;
		int y = ret->data.y;
		IRole * role = SceneManager::getInstance()->getUserByTempId(ret->data.dwUserTempID);
		if (role)
		{
		//	role->setPosition(ccp(x * 64, (map->mapGridCountSize.height - y) * 32));
		//	role->setDirection(ret->data.byDir);
		}
		else
		{
			
			IRole * role = IRole::create();
			String info;
			info.initWithFormat("%d", 3);
			role->changeAnimation(info.getCString());
			role->setPosition(ccp(x * 64, (map->mapGridCountSize.height - y) * 32));
			SceneManager::getInstance()->addOtherRole(role);
			SceneManager::getInstance()->addUserByTempId(tempid, role);
			role->setDirection(ret->data.byDir);
			role->play();
		}
	}

}
void Game::doAddNpc(void *cmd, int len)
{
	Terrain * map = SceneManager::getInstance()->nowTerrain;
	if (!map) return;
	Cmd::stAddMapNpcAndPosMapScreenStateUserCmd *ret = (Cmd::stAddMapNpcAndPosMapScreenStateUserCmd *)cmd;
	if (ret->data.dwMapNpcDataPosition)
	{
		int tempid = ret->data.dwMapNpcDataPosition;
		int x = ret->data.x;
		int y = ret->data.y;
		IRole * role = SceneManager::getInstance()->getNpcByTempId(ret->data.dwMapNpcDataPosition);
		if (role)
		{
			//	role->setPosition(ccp(x * 64, (map->mapGridCountSize.height - y) * 32));
			//	role->setDirection(ret->data.byDir);
		}
		else
		{

			IRole * role = IRole::create();
			String info;
			info.initWithFormat("%d", 0);
			role->changeAnimation(info.getCString());
			role->setPosition(ccp(x * 64, (map->mapGridCountSize.height - y) * 32));
			SceneManager::getInstance()->addOtherRole(role);
			SceneManager::getInstance()->addNpcByTempId(tempid, role);
			role->setDirection(ret->data.byDir);
			role->play();
		}
	}
}
/**
 * 攻击对象
 */
void Game::doAttackRole(void *cmd, int len)
{
	Terrain * map = SceneManager::getInstance()->nowTerrain;
	if (!map) return;
	Cmd::stAttackMagicUserCmd * ret = (Cmd::stAttackMagicUserCmd*) cmd;
	if (ret)
	{
		AttackMsg attackMsg;
		attackMsg.point = ccp(ret->xDes * 64, (map->mapGridCountSize.height - ret->yDes) * 32);
		attackMsg.skillId = ret->wdMagicType;
		IRole * attacker = NULL;
		IRole *defender = NULL;
		switch (ret->byAttackType)
		{
			case Cmd::ATTACKTYPE_N2U:
			{
				attacker = SceneManager::getInstance()->getNpcByTempId(ret->dwUserTempID);
				defender = SceneManager::getInstance()->getUserByTempId(ret->dwDefenceTempID);
			}break;
			case Cmd::ATTACKTYPE_N2N:
			{
				attacker = SceneManager::getInstance()->getNpcByTempId(ret->dwUserTempID);
				defender = SceneManager::getInstance()->getNpcByTempId(ret->dwDefenceTempID);
			}break;
			default:
				break;
		}
		if (attacker && attacker->isVisible())
		{
			String info;
			info.initWithFormat("%d", attackMsg.skillId);
			attacker->nowSkill = info.getCString(); // 测试用 按理应该是AttackState 记录
			attacker->getRSM()->PasreMsg(&attackMsg); // 攻击
		}
	}
}