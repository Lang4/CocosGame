#include "PreLoader.h"
#include "UtilAnimation.h"
#include "HelloWorldScene.h"
#include "XmlSkills.h"
#include "editor-support\cocostudio\CCSGUIReader.h"
#include "NodeUI.h"
#include "ui/UIListView.h"
#include "ui/UIButton.h"
#include "MainUI.h"
#include "Game.h"
USING_NS_CC; 
void PreLoader::init()
{
	paths.push_back("Effects/0.effs");
	paths.push_back("Effects/1.effs");
	paths.push_back("Effects/2.effs");
	paths.push_back("Effects/3.effs");
	paths.push_back("Effects/4.effs");
	paths.push_back("Effects/5.effs");
	paths.push_back("Effects/10.effs");
	paths.push_back("Effects/11.effs");
	paths.push_back("Effects/15.effs");
	paths.push_back("Effects/16.effs");
	paths.push_back("Effects/17.effs");
}
/**
* 启动加载
*/
void PreLoader::go(PreLoaderScene *scene)
{
	init();
	this->scene = NULL;
	if (!ithread) ithread = new std::thread(std::bind(&PreLoader::loader, this));
	this->scene = scene;
	over = false;
}
/**
* 开始加载
*/
void PreLoader::loader()
{
	for (int i = 0; i < paths.size(); i++)
	{
		info = "Loading " + paths[i];
		nowSize = 0;
		UtilParser::getInstance()->preLoadPack(paths[i].c_str(), this);
	}
	nowSize = 0;
	SkillConfigs::getInstance()->parseFromPack("1.skills",this);
	SkillConfigs::getInstance()->parseFromPack("93.skills", this);
	over = true;
}

void PreLoader::backGet(int size, int allSize)
{
	if (scene && scene->tag)
	{
		nowSize += size;
		String info;
		info.initWithFormat("%s %d/%d", this->info.c_str(), nowSize, allSize);
		scene->info->setString(info.getCString());
		scene->bar->setPercent((nowSize * 1.0 / allSize) * 100);
	}
}

cocos2d::Scene* PreLoaderScene::createScene()
{
	auto scene = Scene::create();
	auto layer = PreLoaderScene::create();
	scene->addChild(layer);
	return scene;
}

bool PreLoaderScene::init()
{
	tag = false;
	costDt = 0;
	Size size = CCDirector::sharedDirector()->getVisibleSize();
#if (0)
	
	Sprite * back = Sprite::create("Uis/PreLoad/back.png");
	this->addChild(back);
	back->setPosition(size.width / 2, size.height / 2);

	info = LabelTTF::create("", "Arial", 16);
	back->addChild(info);
	info->retain();
	info->setPosition(back->getContentSize().width / 2, 160);
	
	bar = ui::LoadingBar::create();
	bar->loadTexture("Uis/PreLoad/010.png");
	bar->setPercent(0);
	bar->setPosition(ccp(back->getContentSize().width / 2, 135));
	bar->setVisible(true);
	bar->retain();
	back->addChild(bar);
#else
	UIPanel *panel = NodeUICreator::getInstance()->parseFromFile("Uis/PreLoad", "preload.xml");
	info = panel->getElementByID(1)->to<LabelTTF>();
	bar = panel->getElementByID(2)->to<ui::LoadingBar>();
	if (panel)
	{
		this->addChild(panel);
	}
	info->retain();
#endif
	auto layout = static_cast<ui::Widget*>(cocostudio::GUIReader::getInstance()->widgetFromJsonFile("Uis/UILoader/UILoader_1.ExportJson"));
	auto root = static_cast<ui::Widget*>(layout->getChildByTag(5));
	if (root)
	{
		bar = (ui::LoadingBar*)root;
		bar->retain();
		layout->setPosition(ccp(size.width / 2, size.height / 2));
	}
	this->addChild(layout);
	// 测试ccs 界面
	loader.go(this);
	this->scheduleUpdate();
	//testListView();

	this->setTouchEnabled(true);

	tag = true;
	return true;
}

void PreLoaderScene::update(float dt)
{
	costDt += dt;
	if (loader.over && costDt > 1) // 3秒预留
	{
		loader.over = false;
		info->release();
		bar->release();
		Director::getInstance()->replaceScene(CCTransitionFade::create(0.4,Game::createScene()));
	}
	
}

void PreLoaderScene::testListView()
{
	ui::ListView * listView = ui::ListView::create();

	listView->setDirection(ui::ScrollView::Direction::HORIZONTAL);
	listView->setBounceEnabled(true);
	listView->setBackGroundImage("003-BG.png"); 
	listView->setSize(Size(240, 130));
	//listView->setClippingEnabled(false);
	listView->setPosition(ccp(20, 60));

//	this->addChild(listView);

	ui::Button* default_button = ui::Button::create("004-list_normal.png", "004-list_normal.png");
	default_button->setName("Title Button");

	ui::Layout* default_item = ui::Layout::create();
	default_item->setTouchEnabled(true);
	default_item->setSize(default_button->getSize());
//	default_button->setPosition(Point(default_item->getSize().width / 2.0f,
//		default_item->getSize().height / 2.0f));
	default_item->addChild(default_button);

	// set model
	listView->setItemModel(default_item);
	for (int i = 0; i < 4; ++i)
	{
		listView->pushBackDefaultItem();
	}

	// 从ccs 中加载
	auto root = static_cast<ui::Widget*>(cocostudio::GUIReader::getInstance()->widgetFromJsonFile("Uis/SkillView/SkillView.ExportJson"));
	if (root)
	{
		ui::ListView* view = (ui::ListView*)(root->getChildByTag(7));
		if (view)
		{
			view->setClippingEnabled(false);
			auto item = static_cast<ui::Widget*>(cocostudio::GUIReader::getInstance()->widgetFromJsonFile("Uis/SkillView/DefaultItem.ExportJson"));
			if (item)
			{
				//view->setItemModel((ui::Widget*)item->getChildByTag(9));
			}
			for (int i = 0; i < 4; ++i)
			{
				view->pushBackCustomItem(item->clone());
			}
		}
	}
	this->addChild(root);
}