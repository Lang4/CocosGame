#include "ui/UIPacker.h"
#include "MyPack.h"
class MyUIPacker :public UIPacker{
public:
	static MyUIPacker * getInstance();
	/**
	* 获取纹理
	*/
	virtual Texture2D * getTexutre(const char *name);
	/**
	* 获取Plist
	*/
	virtual bool getContent(std::string &content);
	

private:
	std::vector<MyPack*> packs;
};