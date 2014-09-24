#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
class IRole;
#include "Map.h"
class HelloWorld : public cocos2d::Layer
{
public:
    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene();

    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();  
    
    // a selector callback
    void menuCloseCallback(cocos2d::Ref* pSender);
    
    // implement the "static create()" method manually
    CREATE_FUNC(HelloWorld);


	void onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *event); 
	bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event  *event);
	//通过点击结束区域 来设置 lable的位置
	void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event  *event);
	IRole *role;
	Terrain *map;
};

#endif // __HELLOWORLD_SCENE_H__
