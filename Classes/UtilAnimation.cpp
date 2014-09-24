#include "UtilAnimation.h"

UtilParser * UtilParser::me = NULL;

UtilParser * UtilParser::getInstance()
{
	if (!me) me = new UtilParser;
	return me;
}
Animation * UtilParser::getAnimation(const char *name, const char *aniName, int dir,int dirs)
{
	CCString str;
	str.initWithFormat("%s/", aniName);
	ANISPACKS iter = anisPacks.find(name);
	MyPack *my = NULL;
	if (iter != anisPacks.end())
	{
		my = iter->second;
	}
	else
	{
		my = new MyPack();
		String  packFileName;
		packFileName.initWithFormat("Animations/%s", name);
		my->load(packFileName.getCString());
		anisPacks[name] = my;
	}
	CCString uniqueIDName;
	uniqueIDName.initWithFormat("%s_%s_%d", name, aniName, dir);
	ANIMATIONS_ITER iiter = animations.find(uniqueIDName.getCString());
	if (iiter != animations.end())
	{
		return iiter->second;
	}
	if (my)
	{
		struct CheckEffectSame :public ICheckSameGroup{
			bool check(const std::string &src, const std::string &dest)
			{
				int fileName = 0;
				int dirs = 0;
				UtilParser::getInstance()->getDirsByName(fileName, dirs, src);
				if (src == dest) return true;
				if (fileName == atoi(dest.c_str())) return true;
				return false;
			}
		}check;
		MyGroup * group = my->getGrpByPath(str.getCString(),&check);
		if (group)
		{
			Animation * animation = Animation::create();
			int eachFrameCount = group->getFileCount() / dirs;
			for (int i = dir * eachFrameCount; i < (eachFrameCount)*(1 + dir) && i < group->getFileCount(); i++)
			{
				MyFileBinary *bin = my->getFileContent(group,i);
				if (bin && bin->content.size())
				{
					Image image;
					if (image.initWithImageData((unsigned char *)(&(bin->content[0])), bin->content.size()))
					{
						Texture2D * texture = new Texture2D();
						if (texture && texture->initWithImage(&image))
						{
							Rect rect = CCRectMake(0, 0, texture->getContentSize().width, texture->getContentSize().height);
							SpriteFrame *frame = SpriteFrame::createWithTexture(texture, rect);
							// 创建动画帧
							animation->setDelayPerUnit(0.15f);
							animation->addSpriteFrame(frame);
							texture->release();
						}
						
					}
					else throw "error";
					bin->unload();
				}
			}
			if (animation)
			{
				animation->retain();
				animations[uniqueIDName.getCString()] = animation;
			}
			return animation;
		}
	}
	return NULL;
}
AnimationFrame * CacheAnimation::getFrame(int frame, int dir)
{
	if (dirs == 0) return NULL;
	int count = animation->getFrames().size();
	int eachFrameCount = count / dirs;
	int index = eachFrameCount * dir + frame;
	if (index < count)
		return animation->getFrames().at(index);
	return NULL;
}
/**
* 获取缓存动画 Animation
* \param name 文件名字
* \param aniName 动作名字
* \param 返回缓存对象
*/
CacheAnimation  UtilParser::getCacheAnimation(const char *name, const char *aniName)
{
	CCString index;
	index.initWithFormat("%s-%s", name, aniName);
	CACHEANIMATIONS_ITER pos = cacheAnimations.find(index.getCString());
	if (pos != cacheAnimations.end())
	{
		return pos->second;
	}
	CCString str;
	str.initWithFormat("%s/", aniName);
	ANISPACKS iter = anisPacks.find(name);
	MyPack *my = NULL;
	if (iter != anisPacks.end())
	{
		my = iter->second;
	}
	else
	{
		my = new MyPack();
		String  packFileName;
		packFileName.initWithFormat("Animations/%s", name);
		my->load(packFileName.getCString());
		anisPacks[name] = my;
	}
	CacheAnimation cacheAnimation;
	if (my)
	{
		struct CheckEffectSame :public ICheckSameGroup{
			bool check(const std::string &src, const std::string &dest)
			{
				int fileName = 0;
				dirs = 0;
				UtilParser::getInstance()->getDirsByName(fileName, dirs, src);
				if (fileName == atoi(dest.c_str())) return true;
				dirs = 0;
				return false;
			}
			CheckEffectSame()
			{
				dirs = 0;
			}
			int dirs;
		}check;

		MyGroup * group = my->getGrpByPath(str.getCString(), &check);
		cacheAnimation.dirs = check.dirs;
		if (group)
		{
			Animation * animation = Animation::create();

			for (int i = 0; i < group->getFileCount(); i++)
			{
				MyFileBinary *bin = my->getFileContent(group, i);
				if (bin && bin->content.size())
				{
					Image image;
					if (image.initWithImageData((unsigned char *)(&(bin->content[0])), bin->content.size()))
					{
						Texture2D * texture = new Texture2D();
						if (texture && texture->initWithImage(&image))
						{
							Rect rect = CCRectMake(0, 0, texture->getContentSize().width, texture->getContentSize().height);
							SpriteFrame *frame = SpriteFrame::createWithTexture(texture, rect);
							// 创建动画帧
							animation->setDelayPerUnit(0.15f);
							animation->addSpriteFrame(frame);
							texture->release();
						}

					}
					else throw "error";
					bin->unload();
				}
			}
			cacheAnimation.animation = animation;
			cacheAnimations[index.getCString()] = cacheAnimation;
			return cacheAnimation;
		}
	}
	return cacheAnimation;
}
CacheAnimation UtilParser::getCacheEffectAnimation(const char *name, const char *aniName)
{
	CCString index;
	index.initWithFormat("%s-%s", name, aniName);
	CACHEANIMATIONS_ITER pos = cacheAnimations.find(index.getCString());
	if (pos != cacheAnimations.end())
	{
		return pos->second;
	}

	int indexAni = atoi(aniName) / 20;
	String packName;
	packName.initWithFormat("Effects/%d.effs", indexAni);

	CacheAnimation cacheAnimation;
	ANISPACKS iter = anisPacks.find(packName.getCString());
	MyPack *my = NULL;
	if (iter != anisPacks.end())
	{
		my = iter->second;
	}
	else
	{
		my = new MyPack();
		my->load(packName.getCString());
		anisPacks[packName.getCString()] = my;
	}
	if (my)
	{
		CCString str;
		str.initWithFormat("%s/", aniName);
		struct CheckEffectSame :public ICheckSameGroup{
			bool check(const std::string &src, const std::string &dest)
			{
				int fileName = 0;
				dirs = 0;
				UtilParser::getInstance()->getDirsByName(fileName, dirs, src);
				if (src == dest) return true;
				if (fileName == atoi(dest.c_str())) return true;
				dirs = 0;
				return false;
			}
			CheckEffectSame()
			{
				dirs = 0;
			}
			int dirs;
		}check;
		CCString pList;
		pList.initWithFormat("%s/0.plist", aniName,&check);
		MyFileBinary * pListInfo = my->getFileContent(pList.getCString(),&check);
		if (pListInfo)
		{
			if (getCacheAnimationByPlist(my,aniName,pListInfo, cacheAnimation))
			{
				cacheAnimations[index.getCString()] = cacheAnimation;
				return cacheAnimation;
			}
			return cacheAnimation;
		}
		MyGroup * group = my->getGrpByPath(str.getCString(), &check);
		cacheAnimation.dirs = check.dirs;
		if (group)
		{
			Animation * animation = Animation::create();
			
			for (int i = 0; i < group->getFileCount(); i++)
			{
				MyFileBinary *bin = my->getFileContent(group, i);
				if (bin && bin->content.size())
				{
					Image image;
					if (image.initWithImageData((unsigned char *)(&(bin->content[0])), bin->content.size()))
					{
						Texture2D * texture = new Texture2D();
						if (texture && texture->initWithImage(&image))
						{
							Rect rect = CCRectMake(0, 0, texture->getContentSize().width, texture->getContentSize().height);
							SpriteFrame *frame = SpriteFrame::createWithTexture(texture, rect);
							// 创建动画帧
							animation->setDelayPerUnit(0.15f);
							animation->addSpriteFrame(frame);
							texture->release();
						}

					}
					else throw "error";
					bin->unload();
				}
			}
			cacheAnimation.animation = animation;
			cacheAnimations[index.getCString()] = cacheAnimation;
			return cacheAnimation;
		}
	}
	return cacheAnimation;
}
/**
* 预加载打包文件
*/
void UtilParser::preLoadPack(const char *fileName, IPackGetBack * iCall)
{
	ANISPACKS iter = anisPacks.find(fileName);
	MyPack *my = NULL;
	if (iter != anisPacks.end())
	{
		my = iter->second;
	}
	else
	{
		my = new MyPack();
		my->load(fileName, iCall);
		anisPacks[fileName] = my;
	}
}
Animation * UtilParser::getEffect(const char *fileName, const char *aniName, int dir, int &dirCount, IPackGetBack * iCall)
{
	CCString str;
	str.initWithFormat("%s/", aniName);
	int index = atoi(aniName) / 20;
	String packName;
	packName.initWithFormat("Effects/%d.effs", index);
	ANISPACKS iter = anisPacks.find(packName.getCString());
	MyPack *my = NULL;
	if (iter != anisPacks.end())
	{
		my = iter->second;
	}
	else
	{
		my = new MyPack();
		my->load(packName.getCString(), iCall);
		anisPacks[packName.getCString()] = my;
	}
	if (iCall) return NULL;
	if (my)
	{
		struct CheckEffectSame :public ICheckSameGroup{
			bool check(const std::string &src, const std::string &dest)
			{
				int fileName = 0;
				int dirs = 0;
				UtilParser::getInstance()->getDirsByName(fileName, dirs, src);
				if (fileName == atoi(dest.c_str())) return true;
				return false;
			}
		}check;
		MyGroup * group = my->getGrpByPath(str.getCString(),&check);
		if (group)
		{
			// 通过group的名字获取帧数
			int dirs = 0, fileName;
			getDirsByName(fileName,dirs,group->name);
			dirCount = dirs;
			Animation * animation = Animation::create();
			int eachFrameCount = group->getFileCount() / dirs;
			for (int i = dir * eachFrameCount; i < (eachFrameCount)*(1 + dir) && i < group->getFileCount(); i++)
			{
				MyFileBinary *bin = my->getFileContent(group, i);
				if (bin && bin->content.size())
				{
					Image image;
					if (image.initWithImageData((unsigned char *)(&(bin->content[0])), bin->content.size()))
					{
						Texture2D * texture = new Texture2D();
						if (texture && texture->initWithImage(&image))
						{
							Rect rect = CCRectMake(0, 0, texture->getContentSize().width, texture->getContentSize().height);
							SpriteFrame *frame = SpriteFrame::createWithTexture(texture, rect);
							// 创建动画帧
							animation->setDelayPerUnit(0.15f);
							animation->addSpriteFrame(frame);
							texture->release();
						}

					}
					else throw "error";
					bin->unload();
				}
			}
			return animation;
		}
	}
	return NULL;
}

/**
* 通过名字获取方向数1_XXX_123
*/
bool UtilParser::getDirsByName(int &fileName, int &dirs, const std::string &name)
{
	fileName = -1;
	dirs = -1;
	const char * pointer = name.c_str();
	std::string temp;
	while (*pointer != '\0')
	{
		if (fileName == -1)
		{
			if (*pointer >= '0' && *pointer <= '9')
			{
				temp.push_back(*pointer);
			}
			else
			{
				if (!temp.empty())
					fileName = atoi(temp.c_str());
				else return false;
			}
			if (*pointer == '_')
			{
				dirs = -2;
			}
		}
		else if (dirs == -1)
		{
			if (*pointer == '_') { dirs = -2; };
		}
		else if (dirs == -2)
		{
			if (*pointer == '_') { dirs = -3; temp.clear(); }
		}
		else if (dirs == -3)
		{
			if (*pointer >= '0' && *pointer <= '9')
			{
				temp.push_back(*pointer);
			}
			else
			{
				if (!temp.empty())
				{
					dirs = atoi(temp.c_str());
					return true;
				}
				else return false;
			}
		}
		pointer++;
	}
	if (!temp.empty())
	{
		dirs = atoi(temp.c_str());
		return true;
	}
	return false;
}


bool Tools::isClickTheRealSprite(Sprite* p_sprite, const Point& p_point){
	CCSize l_sizeInPixel = p_sprite->getContentSize();
	//得到相对，Sprite左下的偏移
	int l_iX = (int)(l_sizeInPixel.width / 2.0f + p_point.x);
	int l_iY = (int)(l_sizeInPixel.height / 2.0f + (int)p_point.y);
	CCPoint l_pointBL = ccp(l_iX, l_iY);
	CCRect l_rect = CCRect(0, 0, l_sizeInPixel.width, l_sizeInPixel.height);
	if (!(l_rect.containsPoint(l_pointBL))){
		return false;
	}

	//开辟出空内存buffer
	//unsigned char* l_buffer = (unsigned char*)malloc(l_sizeInPixel.width * l_sizeInPixel.height * 4);
	//memset(l_buffer, 0, l_sizeInPixel.width * l_sizeInPixel.height * 4);
	uint8_t l_buffer[4] = {0};
	Sprite * temp = Sprite::createWithTexture(p_sprite->getTexture());
	temp->setAnchorPoint(ccp(0, 0));
	//写内存
	CCSize size = p_sprite->getParent()->getContentSize();
	CCRenderTexture* l_renderTexture = CCRenderTexture::create(l_sizeInPixel.width, l_sizeInPixel.height, kCCTexture2DPixelFormat_RGBA8888);
	l_renderTexture->beginWithClear(0, 0, 0, 0);
	temp->visit();
	glReadPixels(l_pointBL.x, l_pointBL.y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, l_buffer);
	l_renderTexture->end();
	//透明度
	//int l_iValue_A = l_buffer[4*(int)(l_pointBL.y*l_sizeInPixel.width + l_pointBL.x)+3];
	int l_iValue_A = l_buffer[3];
	//free(l_buffer);
	if (l_iValue_A < 5){
		return false;
	}
	return true;
}
/*******************************************************************/
// 处理Dict 创建
/*******************************************************************/
class MyDictMaker : public SAXDelegator
{
public:
	typedef enum
	{
		SAX_NONE = 0,
		SAX_KEY,
		SAX_DICT,
		SAX_INT,
		SAX_REAL,
		SAX_STRING,
		SAX_ARRAY
	}SAXState;

	typedef enum
	{
		SAX_RESULT_NONE = 0,
		SAX_RESULT_DICT,
		SAX_RESULT_ARRAY
	}SAXResult;

	SAXResult _resultType;
	ValueMap _rootDict;
	ValueVector _rootArray;

	std::string _curKey;   ///< parsed key
	std::string _curValue; // parsed value
	SAXState _state;

	ValueMap*  _curDict;
	ValueVector* _curArray;

	std::stack<ValueMap*> _dictStack;
	std::stack<ValueVector*> _arrayStack;
	std::stack<SAXState>  _stateStack;

public:
	MyDictMaker()
		: _resultType(SAX_RESULT_NONE)
	{
	}

	~MyDictMaker()
	{
	}

	ValueMap dictionaryWithContentsOfString(const std::string& content)
	{
		_resultType = SAX_RESULT_DICT;
		SAXParser parser;

		CCASSERT(parser.init("UTF-8"), "The file format isn't UTF-8");
		parser.setDelegator(this);

		parser.parse(content.c_str(),content.size());
		return _rootDict;
	}

	ValueVector arrayWithContentsOfFile(const std::string& content)
	{
		_resultType = SAX_RESULT_ARRAY;
		SAXParser parser;

		CCASSERT(parser.init("UTF-8"), "The file format isn't UTF-8");
		parser.setDelegator(this);

		parser.parse(content.c_str(), content.size());
		return _rootArray;
	}

	void startElement(void *ctx, const char *name, const char **atts)
	{
		CC_UNUSED_PARAM(ctx);
		CC_UNUSED_PARAM(atts);
		const std::string sName(name);
		if (sName == "dict")
		{
			if (_resultType == SAX_RESULT_DICT && _rootDict.empty())
			{
				_curDict = &_rootDict;
			}

			_state = SAX_DICT;

			SAXState preState = SAX_NONE;
			if (!_stateStack.empty())
			{
				preState = _stateStack.top();
			}

			if (SAX_ARRAY == preState)
			{
				// add a new dictionary into the array
				_curArray->push_back(Value(ValueMap()));
				_curDict = &(_curArray->rbegin())->asValueMap();
			}
			else if (SAX_DICT == preState)
			{
				// add a new dictionary into the pre dictionary
				CCASSERT(!_dictStack.empty(), "The state is wrong!");
				ValueMap* preDict = _dictStack.top();
				(*preDict)[_curKey] = Value(ValueMap());
				_curDict = &(*preDict)[_curKey].asValueMap();
			}

			// record the dict state
			_stateStack.push(_state);
			_dictStack.push(_curDict);
		}
		else if (sName == "key")
		{
			_state = SAX_KEY;
		}
		else if (sName == "integer")
		{
			_state = SAX_INT;
		}
		else if (sName == "real")
		{
			_state = SAX_REAL;
		}
		else if (sName == "string")
		{
			_state = SAX_STRING;
		}
		else if (sName == "array")
		{
			_state = SAX_ARRAY;

			if (_resultType == SAX_RESULT_ARRAY && _rootArray.empty())
			{
				_curArray = &_rootArray;
			}
			SAXState preState = SAX_NONE;
			if (!_stateStack.empty())
			{
				preState = _stateStack.top();
			}

			if (preState == SAX_DICT)
			{
				(*_curDict)[_curKey] = Value(ValueVector());
				_curArray = &(*_curDict)[_curKey].asValueVector();
			}
			else if (preState == SAX_ARRAY)
			{
				CCASSERT(!_arrayStack.empty(), "The state is wrong!");
				ValueVector* preArray = _arrayStack.top();
				preArray->push_back(Value(ValueVector()));
				_curArray = &(_curArray->rbegin())->asValueVector();
			}
			// record the array state
			_stateStack.push(_state);
			_arrayStack.push(_curArray);
		}
		else
		{
			_state = SAX_NONE;
		}
	}

	void endElement(void *ctx, const char *name)
	{
		CC_UNUSED_PARAM(ctx);
		SAXState curState = _stateStack.empty() ? SAX_DICT : _stateStack.top();
		const std::string sName((char*)name);
		if (sName == "dict")
		{
			_stateStack.pop();
			_dictStack.pop();
			if (!_dictStack.empty())
			{
				_curDict = _dictStack.top();
			}
		}
		else if (sName == "array")
		{
			_stateStack.pop();
			_arrayStack.pop();
			if (!_arrayStack.empty())
			{
				_curArray = _arrayStack.top();
			}
		}
		else if (sName == "true")
		{
			if (SAX_ARRAY == curState)
			{
				_curArray->push_back(Value(true));
			}
			else if (SAX_DICT == curState)
			{
				(*_curDict)[_curKey] = Value(true);
			}
		}
		else if (sName == "false")
		{
			if (SAX_ARRAY == curState)
			{
				_curArray->push_back(Value(false));
			}
			else if (SAX_DICT == curState)
			{
				(*_curDict)[_curKey] = Value(false);
			}
		}
		else if (sName == "string" || sName == "integer" || sName == "real")
		{
			if (SAX_ARRAY == curState)
			{
				if (sName == "string")
					_curArray->push_back(Value(_curValue));
				else if (sName == "integer")
					_curArray->push_back(Value(atoi(_curValue.c_str())));
				else
					_curArray->push_back(Value(utils::atof(_curValue.c_str())));
			}
			else if (SAX_DICT == curState)
			{
				if (sName == "string")
					(*_curDict)[_curKey] = Value(_curValue);
				else if (sName == "integer")
					(*_curDict)[_curKey] = Value(atoi(_curValue.c_str()));
				else
					(*_curDict)[_curKey] = Value(utils::atof(_curValue.c_str()));
			}

			_curValue.clear();
		}

		_state = SAX_NONE;
	}

	void textHandler(void *ctx, const char *ch, int len)
	{
		CC_UNUSED_PARAM(ctx);
		if (_state == SAX_NONE)
		{
			return;
		}

		SAXState curState = _stateStack.empty() ? SAX_DICT : _stateStack.top();
		const std::string text = std::string((char*)ch, 0, len);

		switch (_state)
		{
		case SAX_KEY:
			_curKey = text;
			break;
		case SAX_INT:
		case SAX_REAL:
		case SAX_STRING:
		{
			if (curState == SAX_DICT)
			{
				CCASSERT(!_curKey.empty(), "key not found : <integer/real>");
			}

			_curValue.append(text);
		}
			break;
		default:
			break;
		}
	}
};
/**
* 通过pList 获取动画缓存
*/
bool UtilParser::getCacheAnimationByPlist(MyPack *my, const char *aniName, MyFileBinary * bin, CacheAnimation & animation)
{
	ReadFromPlistAnimation rpa;
	rpa.parseFromBin(my, aniName, bin);
	return rpa.getAnimtion(animation);
}

void ReadFromPlistAnimation::parseFromBin(MyPack *my, const char *aniName, MyFileBinary * bin)
{
	// 获取dic 和纹理
	MyDictMaker tMaker;
	ValueMap dict = tMaker.dictionaryWithContentsOfString(bin->content);
	std::string texturePath("");
	if (dict.find("metadata") != dict.end())
	{
		ValueMap& metadataDict = dict["metadata"].asValueMap();
		// try to read  texture file name from meta data
		texturePath = metadataDict["textureFileName"].asString();
	}
	String texturePathInPack;
	texturePathInPack.initWithFormat("%s/%s", aniName, texturePath.c_str());
	struct CheckEffectSame :public ICheckSameGroup{
		bool check(const std::string &src, const std::string &dest)
		{
			int fileName = 0;
			int dirs = 0;
			UtilParser::getInstance()->getDirsByName(fileName, dirs, src);
			if (fileName == atoi(dest.c_str())) return true;
			return false;
		}
	}check;
	MyFileBinary * co = my->getFileContent(texturePathInPack.getCString(),&check);
	if (co && co->content.size())
	{
		Image image;
		if (image.initWithImageData((unsigned char *)(&(co->content[0])), co->content.size()))
		{
			Texture2D * texture = new Texture2D();
			if (texture && texture->initWithImage(&image))
			{
				addSpriteFramesWithDictionary(dict, texture);
				texture->release();
			}
		}
		else throw "error";
		bin->unload();
	}
}
void ReadFromPlistAnimation::addSpriteFramesWithDictionary(ValueMap& dictionary, Texture2D* texture)
{
	ValueMap& framesDict = dictionary["frames"].asValueMap();
	int format = 0;

	// get the format
	if (dictionary.find("metadata") != dictionary.end())
	{
		ValueMap& metadataDict = dictionary["metadata"].asValueMap();
		format = metadataDict["format"].asInt();
	}

	// check the format
	CCASSERT(format >= 0 && format <= 3, "format is not supported for SpriteFrameCache addSpriteFramesWithDictionary:textureFilename:");

	for (auto iter = framesDict.begin(); iter != framesDict.end(); ++iter)
	{
		ValueMap& frameDict = iter->second.asValueMap();
		std::string spriteFrameName = iter->first;
		SpriteFrame* spriteFrame = NULL;
		if (spriteFrame)
		{
			continue;
		}

		if (format == 0)
		{
			float x = frameDict["x"].asFloat();
			float y = frameDict["y"].asFloat();
			float w = frameDict["width"].asFloat();
			float h = frameDict["height"].asFloat();
			float ox = frameDict["offsetX"].asFloat();
			float oy = frameDict["offsetY"].asFloat();
			int ow = frameDict["originalWidth"].asInt();
			int oh = frameDict["originalHeight"].asInt();
			// check ow/oh
			if (!ow || !oh)
			{
				CCLOGWARN("cocos2d: WARNING: originalWidth/Height not found on the SpriteFrame. AnchorPoint won't work as expected. Regenrate the .plist");
			}
			// abs ow/oh
			ow = abs(ow);
			oh = abs(oh);
			// create frame
			spriteFrame = SpriteFrame::createWithTexture(texture,
				Rect(x, y, w, h),
				false,
				Vec2(ox, oy),
				Size((float)ow, (float)oh)
				);
		}
		else if (format == 1 || format == 2)
		{
			Rect frame = RectFromString(frameDict["frame"].asString());
			bool rotated = false;

			// rotation
			if (format == 2)
			{
				rotated = frameDict["rotated"].asBool();
			}

			Vec2 offset = PointFromString(frameDict["offset"].asString());
			Size sourceSize = SizeFromString(frameDict["sourceSize"].asString());

			// create frame
			spriteFrame = SpriteFrame::createWithTexture(texture,
				frame,
				rotated,
				offset,
				sourceSize
				);
		}
		else if (format == 3)
		{
			// get values
			Size spriteSize = SizeFromString(frameDict["spriteSize"].asString());
			Vec2 spriteOffset = PointFromString(frameDict["spriteOffset"].asString());
			Size spriteSourceSize = SizeFromString(frameDict["spriteSourceSize"].asString());
			Rect textureRect = RectFromString(frameDict["textureRect"].asString());
			bool textureRotated = frameDict["textureRotated"].asBool();

			// get aliases
			ValueVector& aliases = frameDict["aliases"].asValueVector();

			// create frame
			spriteFrame = SpriteFrame::createWithTexture(texture,
				Rect(textureRect.origin.x, textureRect.origin.y, spriteSize.width, spriteSize.height),
				textureRotated,
				spriteOffset,
				spriteSourceSize);
		}

		// add sprite frame
		spriteFrame->retain();
		_frames.push_back(spriteFrame);
	}
}
bool ReadFromPlistAnimation::getAnimtion(CacheAnimation &animation)
{
	Animation * anis = Animation::create();
	anis->setDelayPerUnit(0.15f);
	for (int i = 0; i < _frames.size(); i++)
	{
		anis->addSpriteFrame(_frames[i]);
		_frames[i]->release();
	}
	animation.animation = anis;
	animation.dirs = 1;
	return true;
}