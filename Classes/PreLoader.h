#pragma once
#include "cocos2d.h"
#include "MyPack.h"
#include "ui/UILoadingBar.h"
/**
 * 预加载机制
 */
class PreLoaderScene;
class PreLoader:public IPackGetBack{
public:
	std::thread * ithread;
	PreLoaderScene *scene;
	PreLoader()
	{
		ithread = NULL;
		nowSize = 0;
		over = false;
	}
	/**
	 * 启动加载
	 */
	void go(PreLoaderScene *scene);
	/**
	 * 开始加载
	 */
	void loader();
	~PreLoader()
	{
		if (ithread)
		{
			ithread->detach();
			delete ithread;
		}
	}
	void init();
	bool over;
	void backGet(int size, int allSize);
	std::string info;
	int nowSize;
	std::vector<std::string> paths;
};
/**
 * 预加载场景
 */
class PreLoaderScene : public cocos2d::Layer{
public:
	static cocos2d::Scene* createScene();
	PreLoaderScene()
	{
		costDt = 0;
	}
	virtual bool init();

	CREATE_FUNC(PreLoaderScene);

	void update(float dt);

	void testListView();
public:
	PreLoader loader;
	cocos2d::LabelTTF *info;
	cocos2d::ui::LoadingBar *bar;
	float costDt;
	bool tag;
};