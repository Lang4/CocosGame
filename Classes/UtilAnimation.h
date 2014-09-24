#pragma once
#include "cocos2d.h"
#include "MyPack.h"
#include <bitset>
USING_NS_CC;
class IRole;
/**
 * 检测点信息
 */
class CollideInfo{
public:
	/**
	 * 碰撞点检查信息 粒度为 4 * 4
	 */
	std::bitset<16> infos; 
	/**
	 * 检查改点是否有信息
	 */
	bool checkPoint(const Point &point);
};
/**
 * 从外面打包资源 FilePacker 打包好后 就可以直接读取了
 * 从文件中处理动画
 * PERSONNAME: // 人物名字
 *		_WALK_4_8/ 8 方向 4帧
 *		_IDLE_4_8/ 8方向 4帧	  
 */
class CacheAnimation{
public:
	Animation *animation; // 动画
	int dirs; // 方向数量
	CacheAnimation()
	{
		animation = 0;
		dirs = 0;
	}
	AnimationFrame * getFrame(int frame,int dir);
	/**
	 * 获取碰撞信息
	 */
	CollideInfo * getCollideInfo(int frame, int dir);
};
class UtilParser{
public:
	/**
	 * 获取唯一实例
	 */
	static UtilParser * getInstance();
	/**
	 * 获取动画
	 */
	Animation * getAnimation(const char *name, const char *aniName, int dir,int dirs = 5);

	/**
	 * 获取缓存动画 Effect
	 * \param name 文件名字
	 * \param aniName 动作名字
	 * \param 返回缓存对象
	 */
	CacheAnimation getCacheEffectAnimation(const char *name, const char *aniName);
	
	/**
	* 获取缓存动画 Animation
	* \param name 文件名字
	* \param aniName 动作名字
	* \param 返回缓存对象
	*/
	CacheAnimation getCacheAnimation(const char *name, const char *aniName);

	/**
	 * 获取特效 并out方向
	 * \param fileName 所在的打包文件
	 * \param aniName 特效的名字
	 * \param dir 获取指定的方向的帧
	 * \param out dir输出特效的方向数量
	 * \return 动画帧
	 */
	Animation * getEffect(const char *fileName, const char *aniName, int dir, int &dirCount, IPackGetBack * iCall = NULL);

	/**
	 * 通过名字获取方向数
	 */
	bool getDirsByName(int &fileName, int &dirs, const std::string &name);


	/**
	 * 预加载打包文件
	 */
	void preLoadPack(const char *fileName, IPackGetBack * iCall = NULL);

	/**
	 * 通过pList 获取动画缓存
	 */
	bool getCacheAnimationByPlist(MyPack *my, const char *aniName, MyFileBinary * bin, CacheAnimation & animation);
public:
	// 缓存动画等操作
	std::map<std::string, MyPack*> anisPacks; //动画打包文件集合
	typedef std::map<std::string, MyPack*>::iterator ANISPACKS;

	std::map<std::string, Animation*> animations;
	typedef std::map<std::string, Animation*>::iterator ANIMATIONS_ITER;
	std::map<std::string, CacheAnimation> cacheAnimations;// 动画缓存集合
	typedef std::map<std::string, CacheAnimation>::iterator CACHEANIMATIONS_ITER; // 迭代器
	static UtilParser * me;
};

class GameDirs{
public:
	enum{
		//前 6点方向
		FRONT_DIR = 0,

		//右前 5点方向
		FRONT_RIGHT_DIR  = 1,

		//右 3点方向
		RIGHT_DIR  =2,

		//右后 1点方向
		BACK_RIGHT_DIR = 3,

		//后 0点方向
		BACK_DIR  = 4,

		//左后 11点方向
		BACK_LEFT_DIR = 5,

		//左 9点方向
		LEFT_DIR  = 6,

		//左前 7点方向
		FRONT_LEFT_DIR = 7,
	};
	// 计算一个点在另一个点的方向,把方向划分成8个
	// 取值是SkillConst文件里面的方向(FRONT_DIR~~FRONT_LEFT_DIR)
	// @param centerx 角色站立的位置的X坐标 
	// @param centery 角色站立的位置的Y坐标
	// @param calcx 计算的点的X坐标
	// @param calcy 计算的点的Y坐标
	static int getDirection(int centerx, int centery, int calcx, int calcy)
	{
		//根据Math.atan2的计算结果
		//右边为0弧度,左边为PI弧度(也可看成是-PI弧度),上边(或后边)是-PI/2弧度,下边(或前面)是PI/2弧度
		//把整个圆分成16份(2*PI弧度分成16份),每个方向占用2份,根据atan2的结果判断方向
		const float RADIAN_1_16 = 3.1415926 / 8;
		const float RADIAN_RIGHT = 0;
		const float RADIAN_BACK = -3.1415926 * .5;
		const float RADIAN_LEFT = 3.1415926;
		const float RADIAN_FRONT = 3.1415926 * .5;
		int dir = -1;
		float dx = calcx - centerx;
		float dy = -calcy + centery;
		float radians = atan2(dy, dx);
		if (0 > radians)
		{
			if (RADIAN_BACK > radians)
			{
				//RADIAN_BACK是-PI/2,减去PI/16
				if ((RADIAN_BACK - RADIAN_1_16) < radians)
				{
					dir = BACK_DIR;
				}
				else if (-(RADIAN_LEFT - RADIAN_1_16) > radians)
				{
					dir = LEFT_DIR;
				}
				else
				{
					dir = BACK_LEFT_DIR;
				}
			}
			else
			{
				if ((RADIAN_BACK + RADIAN_1_16) > radians)
				{
					dir = BACK_DIR;
				}
				else if ((RADIAN_RIGHT - RADIAN_1_16) < radians)
				{
					dir = RIGHT_DIR;
				}
				else
				{
					dir = BACK_RIGHT_DIR;
				}
			}
		}
		else
		{
			//根据PI/2分出左右
			//RADIAN_FRONT < radians表示位于左边
			if (RADIAN_FRONT < radians)
			{
				//RADIAN_FRONT是PI/2,加上PI/16
				if ((RADIAN_FRONT + RADIAN_1_16) > radians)
				{
					dir = FRONT_DIR;
				}
				else if ((RADIAN_LEFT - RADIAN_1_16) < radians)
				{
					dir = LEFT_DIR;
				}
				else
				{
					dir = FRONT_LEFT_DIR;
				}
			}
			else
			{
				if ((RADIAN_FRONT - RADIAN_1_16) < radians)
				{
					dir = FRONT_DIR;
				}
				else if ((RADIAN_RIGHT + RADIAN_1_16) > radians)
				{
					dir = RIGHT_DIR;
				}
				else
				{
					dir = FRONT_RIGHT_DIR;
				}
			}
		}
		return dir;
	}
};

class Tools{
public:
	static bool isClickTheRealSprite(Sprite *p_sprite, const Point& point);
};

/**
 * 从bin中读取资源
 */
class ReadFromPlistAnimation{
public:
	void parseFromBin(MyPack *my, const char *aniName, MyFileBinary * bin);
	void addSpriteFramesWithDictionary(ValueMap& dictionary, Texture2D* texture);
	bool getAnimtion(CacheAnimation &animation);
private:
	std::vector<SpriteFrame*> _frames;
};