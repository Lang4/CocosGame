#pragma once
#include "Data.h"
class NpcBase :public DataBase{
public:
	int id;
	int pic;
	void parseXml(tinyxml2::XMLElement * ele)
	{
		id = ele->IntAttribute("id");
		pic = ele->IntAttribute("pic");
	}
	virtual int getUniqueID(){
		return id;
	}
};

extern DataManager<NpcBase> npcbm;