#pragma once
#include "NpcBase.h"

DataManager<NpcBase> npcbm;

void initAllInfos()
{
	npcbm.loadFromXml("Datas/npcbase.xml","npcbase");
}