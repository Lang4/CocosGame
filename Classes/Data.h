#pragma once
#include "tinyxml2\tinyxml2.h"
#include <vector>
#include <map>
class DataBase{
public:
	virtual void parseXml(tinyxml2::XMLElement * ele) = 0;
	virtual int getUniqueID() = 0;
};

template<typename DATA>
class DataManager{
public:
	void loadFromXml(const char *fileName,const char *nodeName)
	{
		tinyxml2::XMLDocument doc;
		doc.LoadFile(fileName);
		tinyxml2::XMLElement * configNode = doc.FirstChildElement("Config");
		if (configNode)
		{
			tinyxml2::XMLElement * eleNode = configNode->FirstChildElement(nodeName);
			while (eleNode)
			{
				DATA * data = new DATA;
				data->parseXml(eleNode);
				datas[data->getUniqueID()] = data;
				eleNode = eleNode->NextSiblingElement(nodeName);
			}
		}
	}
	DATA * get(int id)
	{
		DATAS_ITER iter = datas.find(id);
		if (iter != datas.end()) return iter->second;
		return NULL;
	}
public:
	std::map<int,DATA*> datas;
	typedef typename std::map<int, DATA*>::iterator DATAS_ITER;
};

void initAllInfos();