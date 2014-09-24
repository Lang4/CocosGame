#pragma once

#include "cocos2d.h"
#include "network\HttpClient.h"

USING_NS_CC;
using namespace network;

class ICXXRequestCallback{
public:
	virtual void callback(const char *fileName, std::vector<char>* content) = 0;
	virtual ~ICXXRequestCallback(){}
};
class CXXUrlRequest :public Ref{
public:
	CXXUrlRequest()
	{
		size = 0;
		allCallback = NULL;
	}
	~CXXUrlRequest();
	/**
	* 设置加载根目录
	*/
	void setUrl(const char *url, ICXXRequestCallback* allCallback);
	/**
	* 增加加载资源
	*/
	void addFile(const char *fileName, ICXXRequestCallback* callback);
	/**
	* 开始加载
	*/
	void go();
private:
	std::map<std::string, ICXXRequestCallback*> fileCallbacks;
	typedef std::map<std::string, ICXXRequestCallback*>::iterator FILECALLBACKS_ITER;
	int size;
	std::string url;
	ICXXRequestCallback* allCallback;
	/**
	* http 回调
	*/
	void onHttpRequestCompleted(cocos2d::network::HttpClient *sender, cocos2d::network::HttpResponse *response);
};

class MapCell;
class MapCellResourceCallback:public ICXXRequestCallback{
public:
	void callback(const char *fileName,std::vector<char>* content);
	
	MapCell *cell;
	MapCellResourceCallback(MapCell *cell);
	~MapCellResourceCallback();
};

class MapCell : public Node{
public:
	Point gloabIndex; // 全局引索
	Sprite *sprite; // 精灵
	CXXUrlRequest* request;
	int nowIndex; // 当前视图索引
	std::string url; // 所在的url
	MapCell()
	{
		sprite = NULL;
		nowIndex = -1;
		request = new CXXUrlRequest();
	}
	/**
	 * 设置Url
	 */
	void setUrl(const char *url){ this->url = url; }
	/**
	 * 更新视图
	 */
	void updateView(const char *mapName, const Size &gridScale);
	
	/**
	 * \param point 为当前视图左上角位置
	 */
	void updatePoistion(const Size & gridPixelSize)
	{
		if (sprite)
		{
			int offsetx = gloabIndex.x * gridPixelSize.width;
			int offsety = gloabIndex.y * gridPixelSize.height;
			this->setPosition(offsetx,offsety);
		}
	}
	~MapCell()
	{

	}
};

