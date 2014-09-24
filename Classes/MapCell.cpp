#include "MapCell.h"

void MapCell::updateView(const char *mapName, const Size &gridScale)
{
	int x = ((int)gloabIndex.x < 0 ? 0 : (int)gloabIndex.x);
	int y = ((int)gloabIndex.y < 0 ? 0 : (int)gloabIndex.y);

	String urlMapName;
	urlMapName.initWithFormat("%s%s/", url.c_str(), mapName);
	request->setUrl(urlMapName.getCString(), NULL);
	// 根据全局索引设置sprite
	String miniName;
	miniName.initWithFormat("MiniMaps/%s.png", mapName);
	Texture2D * texture = TextureCache::getInstance()->addImage(miniName.getCString());
	Rect rect(gloabIndex.x * 32, texture->getContentSize().height - (gloabIndex.y + 1)* 32, 32, 32);

	int heightGird = texture->getContentSize().height * 8 / 256;
	int widthGird = texture->getContentSize().width * 8 / 256;
	
	CCString fileName;

	if (x >= 0 && y >= 0 && x <= widthGird - 1 && y <= heightGird - 1)
	{
		sprite = Sprite::createWithTexture(texture, rect);
		this->addChild(sprite);
		sprite->setAnchorPoint(ccp(0, 0));
		sprite->setScaleX(gridScale.width); // 宽
		sprite->setScaleY(gridScale.height);// 高
		CCString str;
		str.appendWithFormat("LOGING %u %u", y, x);
		fileName.appendWithFormat("%u_%u.png", x, y);
		int index = x + (heightGird - y - 1) * widthGird;
		fileName.initWithFormat("0000%d.png", index);
		char buffer[10] = { 0 };
		std::string temp = fileName.getCString();
		for (int i = 8; i >= 0; i--)
		{
			buffer[i] = temp.back();
			temp.pop_back();
		}
		LabelTTF * info = LabelTTF::create(buffer, "Arial", 13);
		info->setPosition(ccp(128, 128));
		this->addChild(info);
		//	log("load picture %s\n",fileName.getCString());
		request->addFile(buffer, new MapCellResourceCallback(this));
		request->go();
		request->release();
	}
	else request->release();
}

MapCellResourceCallback::MapCellResourceCallback(MapCell *cell) :cell(cell){
	cell->retain();
}
void MapCellResourceCallback::callback(const char *fileName, std::vector<char>* content)
{
	Image image;
	if (image.initWithImageData((unsigned char *)(&(*content)[0]), content->size()))
	{
		Texture2D * texture = new Texture2D();
		if (texture && texture->initWithImage(&image))
		{
			// 新建纹理
			if (cell)
			{
				if (cell->sprite)
					cell->sprite->removeFromParent();

				if (cell->getReferenceCount() >= 1)
				{
					Sprite *newSprite = Sprite::createWithTexture(texture);
					if (newSprite)
					{
						newSprite->setAnchorPoint(ccp(0, 0));
						cell->addChild(newSprite);
						//	log("add new sprite %s %p\n",newSprite,fileName);
						LabelTTF * info = LabelTTF::create(fileName, "Arial", 16);
						newSprite->addChild(info);
						info->setPosition(128, 128);
					}
					cell->sprite = newSprite;
				}
				cell->release();
				cell = NULL;
			}
			texture->release();
		}
		else
		{
			log("fileName %s create failed", fileName);
		}
	}
	else
	{
		log("not image %s", fileName);
	}
}
MapCellResourceCallback::~MapCellResourceCallback()
{
	if (cell)
	{
		cell->release();
	}
	cell = NULL;
}

/**
* 设置加载根目录
*/
void CXXUrlRequest::setUrl(const char *url, ICXXRequestCallback* allCallback)
{
	this->url = url;
	this->allCallback = allCallback;
}
/**
* 增加加载资源
*/
void CXXUrlRequest::addFile(const char *fileName, ICXXRequestCallback* callback)
{
	FILECALLBACKS_ITER iter = this->fileCallbacks.find(fileName);
	if (iter != this->fileCallbacks.end() && !iter->second)
	{
		return;
	}
	this->fileCallbacks[fileName] = callback;
}
/**
* 开始加载
*/
void CXXUrlRequest::go()
{
	for (FILECALLBACKS_ITER iter = fileCallbacks.begin(); iter != fileCallbacks.end(); ++iter)
	{
		HttpRequest* request = new HttpRequest();
		std::string filePath = this->url + iter->first;
		request->setUrl(filePath.c_str());
		request->setRequestType(HttpRequest::Type::GET);
		request->setResponseCallback(this, httpresponse_selector(CXXUrlRequest::onHttpRequestCompleted));

		request->setTag(iter->first.c_str());
		//	HttpClient::getInstance()->setTimeoutForRead(2);
		HttpClient::getInstance()->send(request);
		request->release();
		//		log("请求文件%s",filePath.c_str());
	}
}
/**
* http 回调
*/
void CXXUrlRequest::onHttpRequestCompleted(cocos2d::network::HttpClient *sender, cocos2d::network::HttpResponse *response)
{
	if (!response) {
		//	this->release();
		return;
	}
	FILECALLBACKS_ITER iter = fileCallbacks.end();
	if (0 != strlen(response->getHttpRequest()->getTag())) {
		//  log("%s completed",response->getHttpRequest()->getTag());
		iter = fileCallbacks.find(response->getHttpRequest()->getTag());
		if (iter == fileCallbacks.end() || iter->second == 0)
		{
			//		this->release();
			return;
		}
	}
	else
	{
		delete iter->second;
		iter->second = NULL;
		//	this->release();
		return;
	}
	long statusCode = response->getResponseCode();
	char statusString[64] = {};

	// sprintf(statusString, "HTTP Status Code: %ld, tag = %s",statusCode,response->getHttpRequest()->getTag());
	//  log("response code: %ld",statusCode);

	if (!response->isSucceed()) {
		//   log("response failed");
		log("%s error buffer: %s", response->getHttpRequest()->getTag(), response->getErrorBuffer());
		delete iter->second;
		iter->second = NULL;
		//	this->release();
		return;
	}

	std::vector<char>* buffer = response->getResponseData();
	if (buffer->size())
	{
		ICXXRequestCallback * call = iter->second;
		if (call)
		{
			call->callback(iter->first.c_str(), buffer);
			//		log("get file %s success",response->getHttpRequest()->getTag());
		}
	}
	if (iter->second)
	{
		delete iter->second;
		iter->second = NULL;
	}
	size++;
	if (size >= fileCallbacks.size())
	{
		if (this->allCallback)
		{
			this->allCallback->callback("", NULL);
			delete this->allCallback;
			this->allCallback = NULL;
		}
		//	this->release();
		return;
	}
	//printf("\n");
}

CXXUrlRequest::~CXXUrlRequest()
{
	if (allCallback)
		delete allCallback;
	for (FILECALLBACKS_ITER iter = fileCallbacks.begin(); iter != fileCallbacks.end(); ++iter)
	{
		if (iter->second) delete iter->second;
	}
	fileCallbacks.clear();
}