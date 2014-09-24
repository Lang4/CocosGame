#pragma once
#include "cocos2d.h"
#include "ui/UILoadingBar.h"
#include "ui/UIButton.h"
#include <map>
#include <vector>
#include "tinyxml2\tinyxml2.h"
USING_NS_CC;
/**
 * 界面元素
 */
class UIElement{
public:
	int type; // 界面元素类型
	void * value;
	enum{
		LOADING_BAR = 1,
		SPRITE =2,
		LABELTTF = 3,
	};
	UIElement()
	{
		type = 0;
		value = NULL;
	}
	Node *toNode(){ return (Node*)value; }
	template<typename T>
	T *to(){ return (T*)value; }
};
/**
 * 界面承载器
 */
class UIPanel:public Node{
public:
	/**
	 * 
	 */
	CREATE_FUNC(UIPanel);
	/**
	 * 通过Index 查询界面元素
	 */
	UIElement * getElementByID(int index);

	std::vector<UIElement> elements; // 界面元素集合

	void addElement(int id, UIElement ele);
};
/**
 * 一个手工编写的 简易UI 解析器
 */
class NodeUICreator{
public:
	static NodeUICreator * me;
	static NodeUICreator * getInstance();
	NodeUICreator()
	{
		bindCreateUIFunctions();
	}
	/**
	 * 创建函数集合
	 */
	typedef UIElement(NodeUICreator::*CREATE_FUNCTION)(tinyxml2::XMLElement *ele);

	/**
	 * 从文件中创建UI
	 */
	UIPanel* parseFromFile(const char *dirName, const char *configName);

	/**
	 * 绑定创建函数
	 */
	void bindCreateUIFunctions();
	
	/**
	 * 创建进度条
	 */
	UIElement createLoadingBar(tinyxml2::XMLElement *ele);

	/**
	 * 创建精灵
	 */
	UIElement createSprite(tinyxml2::XMLElement *ele);

	/**
	 * 创建文本
	 */
	UIElement createLabelTTF(tinyxml2::XMLElement *ele);
	/**
	 * 通过名字创建元素
	 * \param name 名字
	 * \return 返回界面元素
	 */
	UIElement createElementByName(const std::string &name, tinyxml2::XMLElement *ele);

	/**
	 * 将子元素创建成Node
	 */
	Node * createChilds(const char *dirName,UIPanel *panel, tinyxml2::XMLElement * nodeElement);
	/*
	 * 为组建赋值
	 **/
	void assignNodePosition(Node *node, UIPanel *panel, tinyxml2::XMLElement * nodeElement);
private:
	std::map<std::string, CREATE_FUNCTION> elements; // 创建函数集合
	typedef std::map<std::string, CREATE_FUNCTION>::iterator ELEMENTS_ITER;
};
/**
 * 获取文件
 * 获取纹理
 */
class NodeUIPackLoader{
public:
		
private:

};