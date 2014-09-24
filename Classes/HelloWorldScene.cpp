#include "HelloWorldScene.h"
#include "IRoleState.h"
#include "XmlSkills.h"
#include "UtilAnimation.h"
#include "RpgFollow.h"
USING_NS_CC;

Scene* HelloWorld::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));
    
	closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2 ,
                                origin.y + closeItem->getContentSize().height/2));

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    /////////////////////////////
    // 3. add your codes below...

    // add a label shows "Hello World"
    // create and initialize a label
    
    auto label = LabelTTF::create("Hello World", "Arial", 24);
    
    // position the label on the center of the screen
    label->setPosition(Vec2(origin.x + visibleSize.width/2,
                            origin.y + visibleSize.height - label->getContentSize().height));

    // add the label as a child to this layer
    this->addChild(label, 1);

    // add "HelloWorld" splash screen"
    auto sprite = Sprite::create("HelloWorld.png");

    // position the sprite on the center of the screen
    sprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));

    // add the sprite as a child to this layer
  //  this->addChild(sprite, 0);
    
	role = IRole::create();
	role->changeAnimation("1");
	role->play();
	//this->addChild(role, 1);
	role->setPosition(ccp(1900, 1600));

	this->setTouchEnabled(true);


	auto listener = EventListenerTouchOneByOne::create();  //创建一个单点触摸事件 :EventListenerTouchAllAtOnce 为多点
	//设置这些事件的的回调方法
	listener->onTouchBegan = CC_CALLBACK_2(HelloWorld::onTouchBegan, this);
	listener->onTouchEnded = CC_CALLBACK_2(HelloWorld::onTouchEnded, this);
	listener->onTouchMoved = CC_CALLBACK_2(HelloWorld::onTouchMoved, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this); //事件调度器

	

	SkillConfig * skillConfig = SkillConfigs::getInstance()->getSkillConfig("1", "5000");
	
	map = Terrain::create();
//	map->setMapName("127.0.0.1/JLGame/data/maps/100/");
	map->setMapName("http://www.1ceyou.com/RpgGame/map/100/");
	map->setGridCount(100, 100);
	map->setViewGridCount(8, 5);
	map->setGridSize(256, 256);
	map->addToScene(this);
	map->setPosition(0,0);

	map->runAction(RpgFollow::create(CCSizeMake(76800, 6144), role));

	map->addChild(role,2);

	for (int i = 1; i <= 3; i++)
	{
		IRole * role = IRole::create();
		String info;
		info.initWithFormat("%d", i);
		role->changeAnimation(info.getCString());
		role->setPosition(ccp(1900 + i * 100, 1600 + i * 30));
		map->addChild(role, 2);
		role->setDirection(i);
		role->play();
	}
    return true;
}


void HelloWorld::menuCloseCallback(Ref* pSender)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
	MessageBox("You pressed the close button. Windows Store Apps do not implement a close button.","Alert");
    return;
#endif

    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}


void HelloWorld::onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *event){

	
}
bool HelloWorld::onTouchBegan(Touch* touch, Event  *event)
{
	if (role)
	{
		TouchMsg moveMsg;
		moveMsg.point = map->convertTouchToNodeSpace(touch);// ->getLocation();
		role->getRSM()->PasreMsg(&moveMsg);
	}
	return true;
}
//通过点击结束区域 来设置 lable的位置
void HelloWorld::onTouchEnded(Touch* touch, Event  *event)
{
	
}