/**
* 从配置文件中加载技能信息
* xxx.skills 对应技能的打包文件 从中解析出技能信息
*/
#pragma once
#include "cocos2d.h"
#include <map>
#include <vector>
#include "tinyxml2\tinyxml2.h"
USING_NS_CC;

/**
* 帧控制信息
*/
class FrameCtrlInfo{
public:
	int frameID; // 帧编号
	int ctrlType; // 控制类型
	int delayCount; // 延迟帧数
	enum{
		JUMP = 0, // 跳跃
		DELAY = 1,  // 延时
	};
	void parseXml(tinyxml2::XMLElement *element)
	{
		frameID = element->IntAttribute("Frame");
		ctrlType = element->IntAttribute("CtrlType");
		delayCount = element->IntAttribute("DelayCount");
	}
};
/**
 * 音效配置
 */
class SoundConfig{
public:
	//声音ID
	std::string strID;

	//
	bool withSound;

	//
	int playFrame;

	//
	int fadeIn;

	//
	int fadeOut ;

	//音量
	int volume;

	//重复
	bool repeat;

	SoundConfig()
	{
		withSound = false;
		playFrame = -1;
		fadeIn = 0;
		fadeOut = 0;
		volume = 0;
		repeat = false;
	}
	void parseXml(tinyxml2::XMLElement *element)
	{
		const char * ID = element->Attribute("SoundID"); 
		if (!ID)
		{
			ID = element->Attribute("ID");
		}
		if (ID) strID = ID;
		withSound = element->BoolAttribute("WithSound");
		const char *PlayFrame = element->Attribute("PlayFrame");
		if (!PlayFrame)  playFrame = -1;
		else playFrame = atoi(PlayFrame);
		fadeIn = element->IntAttribute("FadeIn");
		fadeOut = element->IntAttribute("FadeOut");
		volume = element->IntAttribute("Volume");
		repeat = element->BoolAttribute("Repeat");
	}
};
/**
 * 效果配置
 */
class SubEffectConfig{
public:
	/**
	* 和GameObject里面的一样，为了排序的需要
	* 顺带用来表示资源id
	*/
	int useForSort;
	struct OffsetConfig{
		/**
		* 位置基准,true:本人;false:目标
		*/
		bool baseOnSelf;

		/**
		* 高度偏移
		*/
		int heightOffset;

		/**
		* 位置偏移类型,false:和本人方向无关;true:和本人方向有关
		*/
		bool relatedWithSelf;

		/**
		* 和本人方向无关的偏移值
		*/
		struct OffsetNoDirection{
			int offsetX;
			int offsetY;
			Point offset;
			void parseXml(tinyxml2::XMLElement *element)
			{
				offsetX = element->IntAttribute("OffsetX");
				offsetY = element->IntAttribute("OffsetY");
				offset.x = offsetX;
				offset.y = offsetY;
			}
		}offsetNoDir;

		/**
		* 和本人方向有关的偏移值
		*/
		struct OffsetInDirection{
			Point currVal;
			std::map<int, Point> offsets; // 五个方向上的偏移
			void parseXml(tinyxml2::XMLElement *element)
			{
				tinyxml2::XMLElement * offsetElement = element->FirstChildElement("Offset");
				while (offsetElement)
				{
					int dir = offsetElement->IntAttribute("Direction");
					Point point;
					point.x = offsetElement->IntAttribute("OffsetX");
					point.y = offsetElement->IntAttribute("OffsetY");
					offsets[dir] = point;
					offsetElement = offsetElement->NextSiblingElement("Offset");
				}
			}
			Point getOffsetWithDir(int dir)
			{
				Point obj;
				if (4 >= dir)
				{
					obj = offsets[dir];
				}
				else
				{
					obj = offsets[8 - dir];
				}
				return obj;
			}
		}offsetInDir;
		void parseXml(tinyxml2::XMLElement *element)
		{
			baseOnSelf = element->BoolAttribute("BaseOnSelf");
			heightOffset = element->IntAttribute("HeightOffset");
			relatedWithSelf = element->BoolAttribute("PosOffsetWithDirection");
			tinyxml2::XMLElement * offsetNoDirection = element->FirstChildElement("OffsetNoDirection");
			if (offsetNoDirection)
			{
				offsetNoDir.parseXml(offsetNoDirection);
			}
			tinyxml2::XMLElement * OffsetInDirection = element->FirstChildElement("OffsetInDirection");
			if (OffsetInDirection)
			{
				offsetInDir.parseXml(OffsetInDirection);
			}
		}
		Point getOffsetWithDir(int dir)
		{
			if (relatedWithSelf)
			{
				return offsetInDir.getOffsetWithDir(dir);
			}
			return offsetNoDir.offset;
		}
	}offsetConfig;
	struct MoveStyleConifg{
		enum{
			MOVE_TYPE_STAY = 0, // 固定不动
			MOVE_TYPE_TOTARGET = 1, // 向目标移动
			MOVE_TYPE_TODIRECTION = 2, // 向固定方向移动
			MOVE_TYPE_CONNECT = 3, // 连接目标与自身
		};
		int moveType; // 移动类型
		//以攻击方向为角度基准，否则以向右为基准
		bool faceToDirection;
		//夹角
		float angle;
		//运动速度
		float moveSpeed;

		//运动加速度
		float moveAcceleration;

		//根据运动方向旋转
		bool rotateWithMove;

		/**
		* 从xml 中解析内容
		*/
		void parseXml(tinyxml2::XMLElement *element)
		{
			moveType = element->IntAttribute("MoveType"); 
			faceToDirection = element->BoolAttribute("FaceToDirection");
			angle = element->FloatAttribute("Angle");
			moveSpeed = element->FloatAttribute("MoveSpeed");
			moveAcceleration = element->FloatAttribute("MoveAcceleration");

			moveSpeed = moveSpeed * .001;
			moveAcceleration = moveAcceleration * .000001;

			rotateWithMove = element->BoolAttribute("RotateWithMove");
		}
	}moveStyleConfig;
	struct PlayOrderConfig{
		bool playIndependent;
		void parseXml(tinyxml2::XMLElement *element)
		{
			playIndependent = element->BoolAttribute("PlayIndependent");
		}
	}playOrderConfig; // 
	struct LayerConfig{
		enum{
			LAYER_FOOT = 0, // 特效层级
			LAYER_CHARA = 1,
		};
		int layer; // 特效所在层级
		bool notTrrigerOnVoid; // 空放时不触发特效
		void parseXml(tinyxml2::XMLElement *element)
		{
			layer = element->IntAttribute("Layer");
			notTrrigerOnVoid = element->BoolAttribute("NotTrrigerOnVoid");
		}
	}layerConfig; // 层配置
	struct DeadTypeConfig{
		enum{
			DEAD_TYPE_AFTERDONE = 0, // 播放完毕后死亡
			DEAD_TYPE_ATTACK = 1, // 打到目标后死亡
			DEAD_TYPE_FRAMES = 2, // 固定帧后死亡
			DEAD_TYPE_ATTACKNUM = 3, // 打击固定次数后死亡
		};
		int deadType;  //结束方式
		int deadFrames;   //固定帧数
		int attackNum; //固定打击次数
		void parseXml(tinyxml2::XMLElement *element)
		{
			deadType = element->IntAttribute("DeadType");
			deadFrames = element->IntAttribute("DeadFrames");
			attackNum = element->IntAttribute("AttackNum");
		}
	}deadTypeConfig; // 死亡配置

	struct HurtTypeConfig{
		enum{
			GENERATE_WHEN_ATTACK = 0, //碰撞时产生
			GENERATE_WHEN_FRAMES = 1,  //在固定帧产生
		};
		bool genHurt;//是否产生伤害
		std::string hurtEffectID;//伤害特效ID
		int generateType; // 产生方式
		int hurtFrame; //产生伤害的帧
		bool shakeScreen;//伤害时震屏
		void parseXml(tinyxml2::XMLElement *element)
		{
			genHurt = element->BoolAttribute("GenerateHurt");
			hurtEffectID = element->Attribute("HurtEffectID") ? element->Attribute("HurtEffectID") : "";
			generateType = element->IntAttribute("GenerateType");
			hurtFrame = element->IntAttribute("HurtFrame");
			shakeScreen = element->BoolAttribute("ShakeScreen");
		}
	}hurtTypeConfig; // 伤害形式
	struct RotateConfig{
		//绕特效中心旋转
		bool isEffectCenter ;

		//旋转中心水平偏移
		int rotateOffsetX ;

		//旋转中心垂直偏移
		int rotateOffsetY;

		//旋转角速度
		float rotateSpeed ;

		//旋转加速度
		float rotateAcceleration;

		void parseXml(tinyxml2::XMLElement *element)
		{
			isEffectCenter = element->BoolAttribute("IsEffectCenter");
			rotateOffsetX = element->IntAttribute("RotateOffsetX");
			rotateOffsetY = element->BoolAttribute("RotateOffsetY");
			rotateSpeed = element->FloatAttribute("RotateSpeed");
			rotateAcceleration = element->FloatAttribute("RotateAcceleration");
		}
	}rotateConfig; // 旋转配置
	struct GradualChangeConfig{
		struct ChangeValue{
			int x;
			int y;
			int alpha;
		} start,mid,end;
		void parseXml(tinyxml2::XMLElement *element)
		{
			tinyxml2::XMLElement *changeConfig = element->FirstChildElement("Start");
			parseChange(start,changeConfig);
			changeConfig = element->FirstChildElement("Mid");
			parseChange(mid, changeConfig);
			changeConfig = element->FirstChildElement("End");
			parseChange(end, changeConfig);
		}
		void parseChange(ChangeValue &value, tinyxml2::XMLElement *changeConfig)
		{
			value.x = changeConfig->IntAttribute("SizeX");
			value.y = changeConfig->IntAttribute("SizeY");
			value.alpha = changeConfig->IntAttribute("Alpha");
		}
	}gradualChangeConfig; // 渐进配置
	struct FrameControl{
		int preDirFrames; // 每个方向上的帧数
		std::map<int, FrameCtrlInfo> ctrlInfos;
		void parseXml(tinyxml2::XMLElement *element)
		{
			preDirFrames = element->IntAttribute("PerDirFrames");
			tinyxml2::XMLElement * frameCtrlElement = element->FirstChildElement("FrameControlInfo");
			while (frameCtrlElement)
			{
				FrameCtrlInfo info;
				info.parseXml(frameCtrlElement);
				ctrlInfos[info.frameID] = info;
				frameCtrlElement = frameCtrlElement->NextSiblingElement("FrameControlInfo");
			}
		}
		FrameCtrlInfo * getCtrlInfo(int frame)
		{
			std::map<int, FrameCtrlInfo>::iterator it = ctrlInfos.find(frame);
			if (it != ctrlInfos.end()) return &it->second;
			return NULL;
		}
	}frameControl; // 帧控制信息
	SoundConfig soundConfig; // 音效配置

	void parseXml(tinyxml2::XMLElement *element)
	{
		useForSort = element->IntAttribute("ID");
		tinyxml2::XMLElement* offsetConfigElement = element->FirstChildElement("OffsetConfig");
		if (offsetConfigElement)
		{
			offsetConfig.parseXml(offsetConfigElement);
		}
		tinyxml2::XMLElement* MoveStyleElement = element->FirstChildElement("MoveStyleConfig");
		if (MoveStyleElement)
		{
			moveStyleConfig.parseXml(MoveStyleElement);
		}
		tinyxml2::XMLElement* PlayOrderElement = element->FirstChildElement("PlayOrderConfig");
		if (PlayOrderElement)
		{
			playOrderConfig.parseXml(PlayOrderElement);
		}
		tinyxml2::XMLElement* LayerConfigElement = element->FirstChildElement("LayerConfig");
		if (LayerConfigElement)
		{
			layerConfig.parseXml(LayerConfigElement);
		}
		tinyxml2::XMLElement* DeadTypeElement = element->FirstChildElement("DeadTypeConfig");
		if (DeadTypeElement)
		{
			deadTypeConfig.parseXml(DeadTypeElement);
		}
		tinyxml2::XMLElement* HurtTypeElement = element->FirstChildElement("HurtTypeConfig");
		if (HurtTypeElement)
		{
			hurtTypeConfig.parseXml(HurtTypeElement);
		}
		tinyxml2::XMLElement* RotateConfigElement = element->FirstChildElement("RotateConfig");
		if (RotateConfigElement)
		{
			rotateConfig.parseXml(RotateConfigElement);
		}
		tinyxml2::XMLElement* GradualChangeElement = element->FirstChildElement("GradualChangeConfig");
		if (GradualChangeElement)
		{
			gradualChangeConfig.parseXml(GradualChangeElement);
		}
		tinyxml2::XMLElement* FrameControlElement = element->FirstChildElement("FrameControl");
		if (FrameControlElement)
		{
			frameControl.parseXml(FrameControlElement);
		}
		tinyxml2::XMLElement* SoundConfigElement = element->FirstChildElement("SoundConfig");
		if (SoundConfigElement)
		{
			soundConfig.parseXml(SoundConfigElement);
		}
	}
};
/**
* 特效配置
*/
class EffectConfig{
public:
	std::vector<SubEffectConfig> subEffects; // 子特效集合
	void parseXml(tinyxml2::XMLElement *element)
	{
		tinyxml2::XMLElement * subEffectElement = element->FirstChildElement("SubEffectConfig");
		while (subEffectElement)
		{
			SubEffectConfig effect;
			effect.parseXml(subEffectElement);
			subEffects.push_back(effect);
			subEffectElement = subEffectElement->NextSiblingElement("SubEffectConfig");
		}
	}
};
/**
 * 动作配置
 */
struct ActionConfig{
public:
	std::string actionID; // 动作编号
	bool keepImage;
	int totalFrames;
	int perDirFrames;
	std::map<int, FrameCtrlInfo> ctrlInfos;
	void parseXml(tinyxml2::XMLElement *element)
	{
		actionID = element->Attribute("ID") ? element->Attribute("ID"):"";
		keepImage = true;
		const char * keepImageStr = element->Attribute("KeepImage");
		if (keepImageStr && !strcmp(keepImageStr, "false"))
		{
			keepImage = false;
		}
		totalFrames = element->IntAttribute("TotalFrames");
		perDirFrames = element->IntAttribute("PerDirFrames");
		tinyxml2::XMLElement * frameCtrlElement = element->FirstChildElement("FrameControlInfo");
		while (frameCtrlElement)
		{
			FrameCtrlInfo info;
			info.parseXml(frameCtrlElement);
			ctrlInfos[info.frameID] = info;
			frameCtrlElement = frameCtrlElement->NextSiblingElement("FrameControlInfo");
		}
	}
	FrameCtrlInfo * getCtrlInfo(int frame)
	{
		std::map<int, FrameCtrlInfo>::iterator it = ctrlInfos.find(frame);
		if (it != ctrlInfos.end()) return &it->second;
		return NULL;
	}
};
/**
 * 每帧控制信息
 */
class FrameConfig{
public:
	int frameID; // 帧编号
	ActionConfig action; // 动作
	SoundConfig soundConfig; // 音效配置
	std::vector<EffectConfig> effects; // 特效配置
	void parseXml(tinyxml2::XMLElement *element)
	{
		frameID = element->IntAttribute("Frame");
		// 加载动作配置
		tinyxml2::XMLElement*actionElement = element->FirstChildElement("ActConfig");
		if (actionElement)
		{
			action.parseXml(actionElement);
		}
		// 音效配置
		tinyxml2::XMLElement*soundElement = element->FirstChildElement("SoundConfig");
		if (soundElement)
		{
			soundConfig.parseXml(soundElement);
		}
		
		// 特效配置
		tinyxml2::XMLElement * effectElement = element->FirstChildElement("EffectConfig");
		while (effectElement)
		{
			EffectConfig effect;
			effect.parseXml(effectElement);
			effects.push_back(effect);
			effectElement = effectElement->NextSiblingElement("EffectConfig");
		}
	}
};
/**
* 技能配置信息
*/
class SkillConfig{
public:
	int charactorID; // 角色编号
	int configID; // 技能编号
	struct Attr{
		//瞬移起始帧
		int teleportFrame;

		//瞬移总帧数
		int teleportTotalFrame;

		//瞬移加速度
		int teleportAccelerate;

		//当前瞬移的帧数
		int currTeleportFrame;

		//当前瞬移的X方向的速度
		float teleportSpeedX;

		//当前瞬移的Y方向的速度
		float teleportSpeedY;

		//跳跃起始帧
		int jumpFrame;

		//跳跃总帧数
		int jumpTotalFrame;

		//跳跃高度
		int jumpHeight;

		//跳跃加速度
		int jumpAccelerate;

		//当前跳跃的帧数
		int currJumpFrame;

		//当前跳跃的加速度
		int currJumpAcce;

		//当前跳跃的初速度
		int jumpStartSpeed;

		//总帧数
		int skillTotalFrames;
		
		void parseXml(tinyxml2::XMLElement *element)
		{
			teleportFrame = element->IntAttribute("TeleportFrame");

			teleportTotalFrame = element->IntAttribute("TeleportTotalFrame");

			teleportAccelerate = element->IntAttribute("TeleportAccelerate");

			jumpFrame = element->IntAttribute("JumpFrame");

			jumpTotalFrame = element->IntAttribute("JumpTotalFrame");

			jumpHeight = element->IntAttribute("JumpHeight");

			jumpAccelerate = element->IntAttribute("JumpAccelerate");

			skillTotalFrames = element->IntAttribute("TotalFrames");
		}
	}attrs;
	std::vector<FrameConfig> configLists;
	void parseXml(tinyxml2::XMLElement *element)
	{
		configID = element->IntAttribute("ID");
			
		attrs.parseXml(element);

		tinyxml2::XMLElement * configElement = element->FirstChildElement("ConfigList");
		while (configElement)
		{
			FrameConfig config;
			config.parseXml(configElement);
			configLists.push_back(config);
			configElement = configElement->NextSiblingElement("ConfigList");
		}
	}
};

/**
 * 技能信息管理器
 */
class IPackGetBack;
class SkillConfigs{
public:
	std::map<std::string, SkillConfig*> skills;
	typedef std::map<std::string, SkillConfig*>::iterator SKILLS_ITER;
	/**
	 * 从字符串中解析配置文件
	 */
	void parseFromString(const char *content){
		tinyxml2::XMLDocument doc;
		doc.Parse(content, strlen(content));
		tinyxml2::XMLElement * charactorElement = doc.FirstChildElement("Charactor");
		const char *ID = charactorElement->Attribute("ID");
		if (!ID) return;
		tinyxml2::XMLElement *skillConfigElemet = charactorElement->FirstChildElement("SkillConfig");
		if (skillConfigElemet)
		{
			SkillConfig * skillConfig = new SkillConfig();
			skillConfig->parseXml(skillConfigElemet);
			CCString uid;
			uid.initWithFormat("%s%d", ID, skillConfig->configID);
			skills[uid.getCString()] = skillConfig;
		}
	}
	SkillConfig * getSkillConfig(const std::string& cid, const std::string& sid);
	/**
	 * 从打包文件中解析技能
	 * 每个Pack对应于人物
	 */
	void parseFromPack(const char *fileName, IPackGetBack *iBack = NULL);

	static SkillConfigs* getInstance(); // 获取技能管理器的单例
	static SkillConfigs *me; // 技能管理器的单例
};