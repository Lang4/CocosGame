#include "NodeUI.h"
UIElement * UIPanel::getElementByID(int index)
{
	if (index < elements.size()) return &elements[index];
	return NULL;
}

void UIPanel::addElement(int id, UIElement ele)
{
	if (id >= elements.size()) elements.resize(id + 1);
	elements[id] = ele;
}
NodeUICreator * NodeUICreator::me = NULL;

NodeUICreator * NodeUICreator::getInstance()
{
	if (!me) me = new NodeUICreator;
	return me;
}
void NodeUICreator::bindCreateUIFunctions()
{
	if (elements.size()) return;
	elements["sprite"] = &NodeUICreator::createSprite;
	elements["loadingbar"] = &NodeUICreator::createLoadingBar;
	elements["labelttf"] = &NodeUICreator::createLabelTTF;
}

UIElement NodeUICreator::createElementByName(const std::string &name, tinyxml2::XMLElement *ele)
{
	UIElement temp;
	ELEMENTS_ITER iter = elements.find(name);
	if (iter != elements.end())
	{
		CREATE_FUNCTION createFunc = iter->second;
		return (this->*createFunc)(ele);
	}
	return temp;
}

/**
* 创建进度条
*/
UIElement NodeUICreator::createSprite(tinyxml2::XMLElement *ele)
{
	Sprite * back = Sprite::create(ele->Attribute("texture"));
	back->setPosition(ccp(ele->IntAttribute("x"), ele->IntAttribute("y")));
	UIElement uie;
	uie.value = back;
	uie.type = UIElement::SPRITE;
	return uie;
}

/**
* 创建精灵
*/
UIElement NodeUICreator::createLoadingBar(tinyxml2::XMLElement *ele)
{
	ui::LoadingBar * bar = ui::LoadingBar::create();
	bar->loadTexture(ele->Attribute("texture"));
	bar->setPercent(ele->IntAttribute("percent"));
	bar->setPosition(ccp(ele->IntAttribute("x"), ele->IntAttribute("y")));
	UIElement uie;
	uie.value = bar;
	uie.type = UIElement::LOADING_BAR;
	return uie;
}
/**
* 创建文本
*/
UIElement NodeUICreator::createLabelTTF(tinyxml2::XMLElement *ele)
{
	LabelTTF * info = LabelTTF::create("", "Arial", 16);
	info->setPosition(ccp(ele->IntAttribute("x"), ele->IntAttribute("y")));
	UIElement uie;
	uie.value = info;
	uie.type = UIElement::LABELTTF;
	return uie;
}
Node * NodeUICreator::createChilds(const char *dirName,UIPanel *panel, tinyxml2::XMLElement * nodeElement)
{
	UIElement element = createElementByName(nodeElement->Attribute("class"), nodeElement);
	Node *node = element.toNode();
	if (node)
	{
		panel->addElement(nodeElement->IntAttribute("id") ,element);
		Node *rootNode = Node::create();
		tinyxml2::XMLElement *childElement = nodeElement->FirstChildElement("node");
		while (childElement)
		{
			Node *node = createChilds(dirName,panel, childElement);
			/**
			 * 为组建赋值
			 */
			assignNodePosition(node, panel, childElement);
			rootNode->addChild(node);
			childElement = childElement->NextSiblingElement("node");
		}
		node->addChild(rootNode);
	}
	return node;
}
/**
* 从文件中创建UI
*/
UIPanel* NodeUICreator::parseFromFile(const char *dirName, const char *configName)
{
	UIPanel * panel = UIPanel::create();
	if (panel)
	{
		String configPath;
		configPath.initWithFormat("%s/%s", dirName, configName);
		// 加载界面信息
		tinyxml2::XMLDocument doc;
		doc.LoadFile(configPath.getCString());
		tinyxml2::XMLElement * nodeElement = doc.FirstChildElement("node");
		if (nodeElement)
		{
			Node *node = createChilds(dirName, panel, nodeElement);
			assignNodePosition(node, panel, nodeElement);
			panel->addChild(node);
		}
	}
	return panel;
}

void NodeUICreator::assignNodePosition(Node *node, UIPanel *panel, tinyxml2::XMLElement * nodeElement)
{
	if (node && panel && nodeElement)
	{
		Size size = CCDirector::sharedDirector()->getVisibleSize();
		const char *strX = nodeElement->Attribute("x");
		const char *strY = nodeElement->Attribute("y");

		if (strX)
		{
			if (!strcmp(strX, "screenmid")) node->setPositionX(size.width/2);
			else
			{
				node->setPositionX(atoi(strX));
			}
		}
		if (strY)
		{
			if (!strcmp(strY, "screenmid")) node->setPositionY(size.height/2);
			else
			{
				node->setPositionY(atoi(strY));
			}
		}
	}
}