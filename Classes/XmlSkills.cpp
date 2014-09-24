#include "XmlSkills.h"
#include "MyPack.h"
SkillConfigs *SkillConfigs::me = NULL;
SkillConfigs* SkillConfigs::getInstance()
{
	if (!me) me = new SkillConfigs();
	return me;
}
 

SkillConfig * SkillConfigs::getSkillConfig(const std::string& cid, const std::string& sid)
{
	CCString uid;
	uid.initWithFormat("%s%s", cid.c_str(), sid.c_str());
	SKILLS_ITER iter = skills.find(uid.getCString());
	if (iter != skills.end())
	{
		return iter->second;
	}
	return NULL;
}
/**
* 从打包文件中解析技能
*/
void SkillConfigs::parseFromPack(const char *fileName, IPackGetBack *iBack)
{
	MyPack *my = NULL;
	String packName;
	packName.initWithFormat("Skills/%s", fileName);
	my = new MyPack();
	my->load(packName.getCString(),iBack);
	if (my)
	{
		MyGroup * group = my->getGrpByPath("SKILLS");
		if (group)
		{
			int skillCount = group->getFileCount() ;
			for (int i = 0; i < skillCount; i++)
			{
				MyFileBinary *bin = my->getFileContent(group, i);
				if (bin && bin->content.size())
				{
					parseFromString(bin->content.c_str());
					bin->unload();
				}
			}
			return ;
		}
	}
	return ;
}