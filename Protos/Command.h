#pragma once
/**
 * 定义相关类型
 */
typedef unsigned char uint8;
typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned long long uint64;


/**
 * 相关宏定义
 */
#ifndef MAX_PASSWORD
#define MAX_PASSWORD	16
#endif

#ifndef MAX_NAMESIZE
#define MAX_NAMESIZE	32
#endif

#ifndef MAX_IP_LENGTH
#define MAX_IP_LENGTH	16
#endif
#pragma pack(1)
/**
 * 登录指令
 **/
namespace Cmd{
	//////////////////////////////////////////////////////////////
	// 空指令定义开始
	//////////////////////////////////////////////////////////////
	const uint8 NULL_USERCMD_PARA = 0;
	struct stNullUserCmd{
		stNullUserCmd()
		{
			dwTimestamp = 0;
		}
		union{
			struct {
				uint8	byCmd;
				uint8	byParam;
			};
			struct {
				uint8	byCmdType;
				uint8	byParameterType;
			};
		};
		uint32	dwTimestamp;
	};
	////////////////////////////////////////////////////////
	// 登陆指令定义开始
	////////////////////////////////////////////////////////
	const uint8 LOGON_USERCMD = 1;
	struct stLogonUserCmd : public stNullUserCmd
	{
		stLogonUserCmd()
		{
			byCmd = LOGON_USERCMD;
		}
	};
	const uint8 SESSION_KEY_LOGIN = 13;
	struct stSessionKeyLoginCmd : public stLogonUserCmd
	{
		stSessionKeyLoginCmd()
		{
			byParam = SESSION_KEY_LOGIN;
			sessionType = zone = wangtong = 0;
		}
		char pstrName[MAX_NAMESIZE];        /**< 帐号 */
		uint16 sessionType;               //平台类型 0内部平台
		uint16 zone;                          /**< 游戏区编号 */
		uint8 wangtong;                   //0电信 1网通
		uint8 isChild;                    //是否未成年
	};
	/// 登陆失败后返回的信息
	const uint8 SERVER_RETURN_LOGIN_FAILED = 3;
	struct stServerReturnLoginFailedCmd : public stLogonUserCmd
	{
		stServerReturnLoginFailedCmd()
		{
			byParam = SERVER_RETURN_LOGIN_FAILED;
		}
		uint8 byReturnCode;           /**< 返回的子参数 */
	};
	/// 登陆成功，返回网关服务器地址端口以及密钥等信息
	const uint8 SERVER_RETURN_LOGIN_OK = 4;
	struct stServerReturnLoginSuccessCmd : public stLogonUserCmd
	{
		stServerReturnLoginSuccessCmd()
		{
			byParam = SERVER_RETURN_LOGIN_OK;
		}
		uint32 dwUserID;
		uint32 loginTempID;
		char pstrIP[64];
		uint16 wdPort;
		char key[8];
	};

	/// 客户登陆网关服务器发送账号和密码
	const uint8_t PASSWD_LOGON_USERCMD_PARA = 5;
	struct stPasswdLogonUserCmd : public stLogonUserCmd
	{
		stPasswdLogonUserCmd()
		{
			byParam = PASSWD_LOGON_USERCMD_PARA;
		}

		uint32 loginTempID;
		uint32 dwUserID;
		char pstrName[MAX_NAMESIZE];        /**< 帐号 */
		uint16 zone;
	};
	//////////////////////////////////////////////////////////////
	// 选择指令定义开始
	//////////////////////////////////////////////////////////////
	const uint8 SELECT_USERCMD = 24;
	struct stSelectUserCmd : public stNullUserCmd
	{
		stSelectUserCmd()
		{
			byCmd = SELECT_USERCMD;
		}
	};
	/// 最大角色信息个数
	const uint16 MAX_CHARINFO = 1;
	/// 角色信息
	struct SelectUserInfo
	{
		uint32 id;                        /// 角色编号
		char  name[MAX_NAMESIZE + 1];     /// 角色名称
		uint8 profession;                 //  职业
		uint8 type;                       /// 角色类型
		uint8 sex;                        /// 性别
		uint16 level;                     /// 角色等级
		uint32 mapid;                 /// 角色所在地图编号
		char  mapName[MAX_NAMESIZE + 1];  /// 角色所在地图名称
		uint16 country;                   /// 国家ID
		uint32 bitmask;                   /// 角色掩码
		char countryName[MAX_NAMESIZE + 1];  /// 国家名称
	};
	const uint8 USERINFO_SELECT_USERCMD_PARA = 1;
	struct stUserInfoUserCmd : public stSelectUserCmd
	{
		stUserInfoUserCmd()
		{
			byParam = USERINFO_SELECT_USERCMD_PARA;
			memset(charInfo,0,sizeof(charInfo));
			size = 0;
		}
		SelectUserInfo charInfo[MAX_CHARINFO];
		uint16 size;
		uint8 data[0];
		uint16 getLen() { return sizeof(*this) + size*sizeof(data[0]); }
	};
	/// 请求创建用户档案
	const uint8 CREATE_SELECT_USERCMD_PARA = 2;
	struct stCreateSelectUserCmd : public stSelectUserCmd
	{
		stCreateSelectUserCmd()
		{
			byParam = CREATE_SELECT_USERCMD_PARA;
		}

		char strUserName[MAX_NAMESIZE]; /**< 用户名字  */
		uint8 profession;             //职业
		uint8 charType;               //avater类型
		uint16 country;               /**< 国家ID */
		uint8 sex;                    //性别
	};
	/// 请求登陆
	const uint8 LOGIN_SELECT_USERCMD_PARA = 3;
	struct stLoginSelectUserCmd : public stSelectUserCmd
	{
		stLoginSelectUserCmd()
		{
			byParam = LOGIN_SELECT_USERCMD_PARA;
			asOrClient = 0;
			platformLevel = 0;
		}
		uint8 asOrClient;// as 0 , Client 1
		uint32 charNo;
		uint32 platformLevel;
		char jpegPassport[7];               /**< 图形验证码 */
		uint8 browserType;                   //浏览器类型
		char flashVersion[32];                  //flash版本
	};
	/// 数据指令
	const uint8 DATA_USERCMD = 3;
	struct stDataUserCmd : public stNullUserCmd
	{
		stDataUserCmd()
		{
			byCmd = DATA_USERCMD;
		}
	};
	//客户端加载地图完成
	const uint8 LOADMAPOK_DATA_USERCMD_PARA = 29;
	struct stLoadMapOKDataUserCmd : public stDataUserCmd{
		stLoadMapOKDataUserCmd()
		{
			byParam = LOADMAPOK_DATA_USERCMD_PARA;
		}
	};
	//////////////////////////////////////////////////////////////
	/// 用户移动指令定义开始
	//////////////////////////////////////////////////////////////
	const uint8 MOVE_USERCMD = 6;
	struct stMoveUserCmd : public stNullUserCmd{
		stMoveUserCmd()
		{
			byCmd = MOVE_USERCMD;
		}
	};
	/// 用户移动
	const uint8_t USERMOVE_MOVE_USERCMD_PARA = 2;
	struct stUserMoveMoveUserCmd : public stMoveUserCmd {
		stUserMoveMoveUserCmd()
		{
			byParam = USERMOVE_MOVE_USERCMD_PARA;
		}

		uint32 dwUserTempID;			/**< 用户临时编号 */

		uint8 byDirect;				/**< 移动方向 */
		uint8 bySpeed;				/**< 移动速度 */
		uint16 x;					/**< 目的坐标 */
		uint16 y;
	};
	/// Npc移动指令
	const uint8 NPCMOVE_MOVE_USERCMD_PARA = 20;
	struct stNpcMoveMoveUserCmd : public stMoveUserCmd{
		stNpcMoveMoveUserCmd()
		{
			byParam = NPCMOVE_MOVE_USERCMD_PARA;
		}

		uint32 dwNpcTempID;			/**< Npc临时编号 */

		uint8 byDirect;				/**< 移动方向 */
		uint8 bySpeed;				/**< 移动的速度 */
		uint16 x;					/**< 目的坐标 */
		uint16 y;
	};

	/// 地图信息
	const uint8 MAPSCREENSIZE_DATA_USERCMD_PARA = 28;
	struct stMapScreenSizeDataUserCmd : public stDataUserCmd{
		stMapScreenSizeDataUserCmd()
		{
			byParam = MAPSCREENSIZE_DATA_USERCMD_PARA;
		}

		uint32 width;     /**< 场景宽 */
		uint32 height;        /**< 场景高 */
		char pstrMapName[MAX_NAMESIZE]; /**< 大陆名称 */
		uint16 mapID;                 //地图id
		uint32 setting;               /**< 大陆标志 */
		uint32 rgb;                   /**< 颜色标志0x00rrggbb */
		char pstrGroupName[MAX_NAMESIZE];   /**< 服务器组名称 */
		char pstrCountryName[MAX_NAMESIZE]; /**< 国家名称 */
		char pstrCityName[MAX_NAMESIZE];    /**< 城市名称 */
		int mainRoleX;
		int mainRoleY;
		uint32 copy;                  //正在进行的副本
		uint16 forceCountryMap;       //强制显示地图
		short npc_count;
		struct {
			uint32 id, x, y;
		} npc_list[0];
		uint16 getLen() { return sizeof(*this) + npc_count*sizeof(npc_list[0]); }
	};
	
	struct t_MainUserData
	{
		uint32 dwUserID;					/// 用户id
		uint32 dwUserTempID;				/// 用户临时ID
		uint16 level;						/// 角色等级
		uint32 hp;						/// 当前生命值
		uint32 maxhp;						/// 最大生命值
		uint32 resumehp;					/// 生命值恢复
		uint32 mp;						/// 当前法术值
		uint32 maxmp;						/// 最大法术值
		uint32 resumemp;					/// 法术值恢复
		uint32 sp;						/// 当前体力值
		uint32 maxsp;						/// 最大体力值
		uint32 resumesp;					/// 体力值恢复
		uint32 pdamage;					/// 最小物理攻击力
		uint32 maxpdamage;				/// 最大物理攻击力
		uint32 mdamage;					/// 最小法术攻击力
		uint32 maxmdamage;				/// 最大法术攻击力
		uint32 pdefence;					/// 物理防御力
		uint32 mdefence;					/// 法术防御力
		uint64 exp;						/// 当前经验值
		uint64 nextexp;					/// 升级经验值
		uint16 attackrating;				/// 攻击命中
		uint16 attackdodge;				/// 攻击躲避
		uint16 bang;						/// 暴击率
		uint16 bangdamage;				/// 暴击伤害
		uint16 bangdef;					/// 抗暴击
		uint16 tili;						/// 体力值
		uint16 charm;						/// 魅力值
		uint8  profession;				/// 职业
		uint32 country;					/// 国家
		uint16 pkmode;					/// pk模式

		uint16 wdTire;					/// 疲劳状态 0为非 1为疲劳
		uint32 platform;					/// 平台属性 
		uint32 platform2;					/// 平台属性2 
		uint32 fivepoint;					/// 五行点数
		uint32 honor;						/// 荣誉值
		uint32 maxhonor;					/// 最大荣誉值
		uint32 money;						/// 金币
		uint32 ticket;					/// 礼券
		uint32 card;						/// 金卡
		uint32 bitmask;					/// 角色掩码
		uint32 weiShouYiZhiExp;			/// 尾兽意志经验
		uint32 weiShouYiZhiNextExp;		/// 尾兽意志下级经验
		uint8 weiShouYiZhiLevel;			/// 尾兽意志等级
		uint32 weiShouYiZhiMakeObj;		/// 尾兽意志属性加成

		uint16 ignorepdef;				/// 忽视物理防御
		uint16 ignoremdef;				/// 忽视魔法防御
		uint16 twodefence;				/// 绝对防御
		uint16 fatalrate;					///	致命一击
		uint16 hitRecovery;				/// 击中回复
		uint16 dpddam;					/// 物理伤害减少
		uint16 dmddam;					///	魔法伤害减少
		uint16 reflectp;					/// 伤害反弹
		uint16 stable;					/// 定身
		uint16 stabledef;					/// 定身抗性
		uint16 slow;						/// 减速
		uint16 slowdef;					/// 减速抗性
		uint16 reel;						/// 眩晕
		uint16 reeldef;					/// 眩晕抗性
		uint16 chaos;						/// 混乱
		uint16 chaosdef;					/// 混乱抗性
		uint16 cold;						/// 冰冻
		uint16 colddef;					/// 冰冻抗性

		uint16 npcdamage;					/// npc额外伤害
		uint32 power;						/// 战斗力
		uint16 str;						/// 力量
		uint16 inte;						/// 智力
		uint16 dex;						/// 敏捷

		uint32 dwBodyID;					///	变身卡id 
		uint32 dwCardLevel;				/// 变身卡等级 

		uint16 maxtili;					/// 最大体力值
		uint16 realdam;					/// 真实伤害
		uint16 realdef;					/// 减免伤害

		uint16 attPetCard;				/// 伙伴变身卡
		uint32 attPetMode;				/// 伙伴模式

		uint8 vip;						/// vip等级
		uint16 tiliTime;					/// 体力购买次数
		uint16 powerRank;					/// 战斗力排名
		uint16 levelRank;					/// 等级排名
		uint16 jingyingCopyTime;			/// 精英次数 
		uint16 leaderCopyTime;			/// 魔王购买次数
		uint16 moneyTime;					/// 聚宝盆次数
		uint16 honorTime;					/// 声望次数
		uint16 starTime;					/// 升星石次数
		uint16 fumoTime;					/// 尘次数
		uint16 heroTime;					/// 英雄石次数
		uint16 goldMineTime;				/// 开矿次数

		uint8 freeReliveTime;				/// 每日免费原地次数
		uint32 vipMoney;					/// 充值元宝数
		uint32 vipGiveMoney;				/// vip值 

		uint8 loginDay;					/// 登陆天数
		uint32 ziluolan;					/// 紫罗兰声望
		uint16 pataCopyTime;				/// 爬塔购买次数
		uint16 gvePataCopyTime;			/// GEV爬塔购买次数

		uint32 starcard;					/// 星卡数量
		uint32 worldPower;				/// 世界战斗力
		uint32 monthLoginBit;				/// 登陆领奖bit
		uint32 openTime;					/// 开服秒数
		uint32 dayVipMoney;				// 每日充值
		uint16 moGuVipTime; //魔古山vip购买次数
	};

	/// 主用户数据
	const uint8 MAIN_USER_DATA_USERCMD_PARA = 1;
	struct stMainUserDataUserCmd : public stDataUserCmd {
		stMainUserDataUserCmd()
		{
			byParam = MAIN_USER_DATA_USERCMD_PARA;
		}
		t_MainUserData data;
	};

	
	//////////////////////////////////////////////////////////////
	/// 地图数据指令定义开始
	//////////////////////////////////////////////////////////////
	const uint8 MAPSCREEN_USERCMD = 5;
	struct stMapScreenUserCmd : public stNullUserCmd{
		stMapScreenUserCmd()
		{
			byCmd = MAPSCREEN_USERCMD;
		}
	};

	struct t_MapBuildingData{
		uint32_t dwMapBuildingTempID;			/**< 建筑物的临时ID */
		uint32_t dwBuildingDataID;				/**< 建筑物的数据ID */
		uint32_t x;
		uint32_t y;
		uint32_t dwContract;					/**< 地契编号 */
		char pstrName[MAX_NAMESIZE];		/**< 建筑物的名称 */
		char pstrUserName[MAX_NAMESIZE];	/**< 建筑物的所有者名称 */
	};
	enum eSkillPosType
	{
		eSkillPosType_none,
		eSkillPosType_change,
		eSkillPosType_profession,
		eSkillPosType_weishou,
	};

	enum ePickType
	{
		ePickType_all = 1,			//全体
		ePickType_user = 2,			//个人
		ePickType_team = 3,			//队伍
		ePickType_union = 4,		//帮派
		ePickType_country = 5,		//国家
		ePickType_camp = 6,			//阵营
		ePickType_limit = 7, //boss掉落一分钟限制，1分钟内不能拾取，1分钟后可以所有玩家拾取

	};

	//////////////////////////////////////////////////////////////
	/// 数据指令定义开始
	//////////////////////////////////////////////////////////////
	/// 定义人物状态
	enum {
		// debuff
		USTATE_DEATH = 0,						//	死亡
		USTATE_POISON = 1,						//	中毒（角色渲染为绿色）
		USTATE_ICE = 2,						//	冰冻（角色渲染为蓝色）
		USTATE_FREEZE = 3,						//	冻结（角色渲染为蓝色，无法移动，无法攻击，无法使用道具）
		USTATE_ROCK = 4,						//	石化（角色渲染为棕黄色，无法移动，无法攻击，无法使用道具）
		USTATE_PALSY = 5,						//	麻痹（角色渲染为淡蓝色，无法攻击）
		USTATE_NO_MOVE = 6,						//  不可移动
		USTATE_STUN = 7,						//  晕迷（无法移动，无法攻击）
		USTATE_SILENT = 8,						//  沉默（不允许施放任何技能，包括普通攻击）
		USTATE_CONFUSION = 9,						//  混乱（所有操作得到的效果无法预知）
		USTATE_CRAZY = 10,						//	疯狂（所有操作无效，胡乱攻击）
		USTATE_BLIND = 11,						//	失明（除ui外全黑）
		USTATE_SLOW = 12,						//  减速
		USTATE_DEF_DOWN = 13,						//  减防
		USTATE_MDEF_DOWN = 14,						//  减魔防
		USTATE_MANA_BURN = 15,						//  法力燃烧
		USTATE_SATI_HALO = 16,						//  献祭光环
		USTATE_ICE_SHIELD = 17,						//  寒冰护体
		USTATE_ICE_BURST = 18,						//  冰爆
		USTATE_SHOCKWAVE_SLOW = 19,						//  冲击波减速
		USTATE_BLOOD_RING = 20,						//  鲜血光环
		USTATE_BLADE_SHAPE = 21,						//  剑刃
		USTATE_DARK_MIND = 22,						//  黑暗意识
		USTATE_SOUL_BURN = 23,						//  灵魂燃烧
		USTATE_TOXIC_SMOG = 24,						//  毒雾
		USTATE_PLAGUE = 25,						//  瘟疫
		USTATE_DRAGON_CURSE = 26,						//  龙之诅咒
		USTATE_CORRIOSION = 27,						//  腐蚀皮肤
		USTATE_HOLLY_SIGN_1 = 28,						//  神圣标记
		USTATE_HOLLY_SIGN_2 = 29,						//  神圣标记2
		USTATE_BROKEN_BLADE = 30,						//  碎刃
		USTATE_BETRAYER_CLOAK = 31,						//  背叛者斗篷
		USTATE_CORRIOSION_SKIN = 32,						//  腐蚀皮肤
		USTATE_SLEEP = 33,						//  睡眠状态

		// buff
		USTATE_HIDE = 80,						//	隐身
		USTATE_HIDE_NOMOVE_STATE = 81,						//  隐身不可移动状态
		USTATE_HIDE_PET = 82,						//  宠物隐身状态
		USTATE_RELIVERING = 83,						//	复活光环
		USTATE_WEISHOU = 84,						//  尾兽化
		USTATE_FURY_MODE = 85,						//  狂怒
		USTATE_KEEP_IMAGE = 86,						//  残影	
		USTATE_LIQUID_MOVE = 87,						//  忽视所有动态阻挡
		USTATE_PDEF_UP = 88,						//  物防提高
		USTATE_MDEF_UP = 89,						//  魔防提高
		USTATE_HP_UP = 90,						//  血量提高
		USTATE_SPEED_UP = 91,						//  速度提高
		USTATE_GOD_MODE = 92,						//  天神下凡
		USTATE_REDUCE_DMG = 93,						//	减免伤害
		USTATE_DODGE_UP = 94,					    //  闪避提升
		USTATE_RENEW_LIFE = 95,					    //  恢复生命
		USTATE_RASH = 96,						//  鲁莽
		USTATE_REDUCE_MDMG = 97,						//  魔法减免
		USTATE_DOUBLE_HORNOR = 98,						//  双倍成就
		USTATE_TRIPLE_HORNOR = 99,						//  三倍成就
		USTATE_FOURTH_HORNOR = 100,						//  四倍成就
		USTATE_FIFTH_HORNOR = 101,						//  五倍成就
		USTATE_MAGIC_SHILED = 102,						//  魔法盾
		USTATE_COURAGE_POWER = 103,						//  勇气之力
		USTATE_DEATH_STORM = 104,						//  死亡风暴
		USTATE_ICE_BARRIER = 105,						//  寒冰屏障
		USTATE_TRUE_GOD_MODE = 106,						//  真天神下凡
		USTATE_DEVIL_FORCE = 107,						//  恶魔之力
		USTATE_TITAN_MARK = 108,						//  泰坦印记
		USTATE_TWENTYFIFTH_HORNOR = 109,						//  二十五倍成就

		// quest
		USTATE_START_QUEST = 150,						//	任务开始
		USTATE_DOING_QUEST = 151,						//	任务进行中
		USTATE_FINISH_QUEST = 152,						//	任务完成

		// state
		USTATE_SITDOWN = 180,						//  坐下	
		USTATE_PRIVATE_STORE = 181,						//  摆摊
		USTATE_RIDE = 182,						//	骑乘

		// other
		USTATE_TEAM_ATTACK_ONE_DIM = 200,						//  组队集中攻击同一角色状态

		USTATE_RED_NAME = 444,						//  红名buff

		MAX_STATE = 256                 /// 最大状态数
	};
	enum enumMapDataType{
		MAPDATATYPE_NPC,
		MAPDATATYPE_USER,
		MAPDATATYPE_BUILDING,
		MAPDATATYPE_ITEM,
		MAPDATATYPE_PET
	};
	struct t_MapObjectData {
		uint32 dwMapObjectTempID;				/**< 物品的临时ID */
		uint32 dwObjectID;                       /**< 物品的数据ID */
		char pstrName[MAX_NAMESIZE];			/**<  物品名 */
		uint32 x;
		uint32 y;
		uint32 wdNumber;                         /**< 数量 */
		uint16  wdLevel;                          /**< 等级 */
		uint8 upgrade;
		uint8 kind;
		uint32 dwOwner;
		uint8 pickRange;					//拾取半径 单位格子
		uint8 pickType;
	};

	//宠物的类型
	enum petType
	{
		PET_TYPE_NOTPET = 0,	//不是宠物，也可以用来表示请求列表
		PET_TYPE_RIDE = 1,	//坐骑
		PET_TYPE_PET = 2,	//宠物
		PET_TYPE_SUMMON = 3,	//召唤兽
		PET_TYPE_TOTEM = 4,	//图腾
		PET_TYPE_GUARDNPC = 5,	//要护卫的npc
		PET_TYPE_SEMI = 6,    //半宠物，跟主人同生同死，AI完全没关系
		PET_TYPE_CARTOON = 7     //卡通宝宝 //TODO NOUSE
	};

	struct t_NpcData {
		uint32 dwMapNpcDataPosition;	/**< npc的数据位置 */
		uint32 dwNpcDataID;			/**< npc的数据ID */
		uint16  movespeed;		/// 移动速度
		uint16 attackspeed;		/// 攻击速度

		uint8 byDirect;				/**< Npc的方向 */
		uint8 level;///等级
		uint8 campID;				//阵营id
		uint32 defType;			//攻击访问类型eDefType
		uint16 forceAvatar;		//强制显示npc
		uint16 pic;
	};
	struct t_MapNpcData :public t_NpcData
	{
		uint8 byState[(MAX_STATE + 7) / 8];			/**< Npc的状态 */
	};
	struct t_MapPetData {
		uint32 tempID;//npc的临时ID

		char name[MAX_NAMESIZE]; //npc的名称
		uint8 masterType;	///主人的类型  0：玩家 1：npc
		uint32 masterID;	///主人的id -1：没有主人
		char masterName[MAX_NAMESIZE];///主任的名字
		uint8 pet_type;//宠物类型
	};

	/// 地图上面删除人物
	const uint8 REMOVEUSER_MAPSCREEN_USERCMD_PARA = 11;
	struct stRemoveUserMapScreenUserCmd : public stMapScreenUserCmd{
		stRemoveUserMapScreenUserCmd()
		{
			byParam = REMOVEUSER_MAPSCREEN_USERCMD_PARA;
		}

		uint32 dwUserTempID;			/**< 用户临时ID */
	};
	/// 定义物品动作类型
	enum{
		OBJECTACTION_DROP,			/// 掉落物品
		OBJECTACTION_UPDATE,		/// 更新物品数据
	};
	/// 在地图上增加物品
	const uint8 ADDMAPOBJECT_MAPSCREEN_USERCMD_PARA = 20;
	struct stAddMapObjectMapScreenUserCmd : public stMapScreenUserCmd{
		stAddMapObjectMapScreenUserCmd()
		{
			byParam = ADDMAPOBJECT_MAPSCREEN_USERCMD_PARA;
			memset(&data,0, sizeof(data));
		}
		uint8	action;
		t_MapObjectData data;
	};

	/// 在地图上删除物品
	const uint8_t REMOVEMAPOBJECT_MAPSCREEN_USERCMD_PARA = 21;
	struct stRemoveMapObjectMapScreenUserCmd : public stMapScreenUserCmd{
		stRemoveMapObjectMapScreenUserCmd()
		{
			byParam = REMOVEMAPOBJECT_MAPSCREEN_USERCMD_PARA;
		}
		uint32 dwMapObjectTempID;	/**< 物品的临时ID */
		uint32 ownerID;			/**< 物品主人 */
	};

	/// 在地图上增加建筑物
	const uint8 ADDMAPBUILDING_MAPSCREEN_USERCMD_PARA = 30;
	struct stAddMapBuildingMapScreenUserCmd : public stMapScreenUserCmd{
		stAddMapBuildingMapScreenUserCmd()
		{
			byParam = ADDMAPBUILDING_MAPSCREEN_USERCMD_PARA;
		}

		t_MapBuildingData data;
	};


	/// 在地图上删除建筑物
	const uint8 REMOVEMAPBUILDING_MAPSCREEN_USERCMD_PARA = 31;
	struct stRemoveMapBuildingMapScreenUserCmd : public stMapScreenUserCmd{
		stRemoveMapBuildingMapScreenUserCmd()
		{
			byParam = REMOVEMAPBUILDING_MAPSCREEN_USERCMD_PARA;
		}

		uint32 dwMapBuildingTempID;	/**< 建筑物的临时ID */
	};
	/// 在地图上删除NPC
	const uint8 REMOVEMAPNPC_MAPSCREEN_USERCMD_PARA = 51;
	struct stRemoveMapNpcMapScreenUserCmd : public stMapScreenUserCmd{
		stRemoveMapNpcMapScreenUserCmd()
		{
			byParam = REMOVEMAPNPC_MAPSCREEN_USERCMD_PARA;
		}
		uint32 dwMapNpcDataPosition;	/**< npc的数据位置 */
	};
	struct MapData_ItemHeader {
		uint16 size;// 数量
		uint8 type;// 类型  enumMapDataType
	};
	/// 用户本身的数据，不包括状态
	struct t_MapUserSculpt{
		uint32 dwHorseID;					/// 马牌编号
		uint32 dwStarSet;					/// 星套级别
		uint32 dwBodyID;					///	变身卡id 
		uint32 dwCardLevel;				/// 变身卡星级 
		uint32 dwRightHandID;				/// 右手物品ID
	};
	struct t_UserData
	{
		uint32 dwUserTempID;					//// 用户临时ID

		char  name[MAX_NAMESIZE + 1];			/// 角色名称
		uint8 chartype;						/// avrtar类型
		uint8  profession;					/// 职业类型
		uint8  sex;							/// 性别
		uint32 goodness;						/// 善恶度

		t_MapUserSculpt sculpt;
		uint32 dwHairRGB;					/// 头发颜色0xffrrggbb
		uint32 dwBodyColorSystem;			/// 衣服系统颜色0xffrrggbb
		uint32 dwBodyColorCustom;			/// 衣服定制颜色0xffrrggbb
		uint32 dwLeftWeaponColor;			/// 左手武器颜色0xffrrggbb
		uint32 dwRightWeaponColor;			/// 右手武器颜色0xffrrggbb
		uint16  attackspeed;					/// 攻击速度
		uint16  movespeed;					/// 移动速度
		uint8 country;						/// 国家
		uint32 dwChangeFaceID;				//易容后的NPC的类型ID
		//uint8_t live_skills[6]; 				//生活技能等级
		uint8 level;							//Level : 1(<10)   11(>=10)
		uint32 exploit;				// 功勋值

		uint32 dwUnionID;			// 帮会ID
		uint32 dwSeptID;				// 家族ID

		char  caption[MAX_NAMESIZE];		// 顶部头衔（某城城主或某国国王）
		uint32 dwTeamState;					// 队伍状态
		uint32 dwArmyState;			//1为队长，2为将军,0为未加入军队

		uint8 TeamNum;					//队伍成员个数 
		uint8 autoIntoTeam;				//是否设置了自动入队
		uint8 campID;					//阵营id
		uint32 hp;					//当前血
		uint32 maxhp;					//血量上限

		uint8 pkMode;					//pk模式
		uint32 teamId;				//组队id

		uint32 cardSystemLevel;		//祭坛等级
		uint8 countryPower[4];			//用户国家权限
		uint32 platformIDLevel;			// 平台等级

	};
	struct t_MapUserData : public t_UserData
	{
		uint8_t state[(MAX_STATE + 7) / 8];	/// 角色状态
	};

	struct t_MapUserDataPos :public t_MapUserData
	{
		uint16_t x;					/**< 坐标*/
		uint16_t y;
		uint8_t byDir;					// 方向（如果为 -1 则不确定方向）
	};
	struct t_MapNpcDataPos :public t_MapNpcData
	{
		uint16_t x;					/**< 坐标*/
		uint16_t y;
		uint8_t byDir;					// 方向（如果为 -1 则不确定方向）
	};
	struct t_MapBuildingDataPos :public t_MapBuildingData
	{
		uint16_t x;					/**< 坐标*/
		uint16_t y;
		uint8_t byDir;					// 方向（如果为 -1 则不确定方向）
	};
	// 地图数据打包发送
	const uint8 MAPDATA_MAPSCREEN_USERCMD_PARA = 55;
	struct stMapDataMapScreenUserCmd : public stMapScreenUserCmd{
		stMapDataMapScreenUserCmd()
		{
			byParam = MAPDATA_MAPSCREEN_USERCMD_PARA;
			mdih.size = 0;
			mdih.type = 0;
		}
		MapData_ItemHeader mdih;
		union
		{
			t_MapUserDataPos mud[0];
			t_MapNpcDataPos mnd[0];
			t_MapBuildingDataPos mbd[0];
			t_MapPetData mpd[0];
		};
		uint16_t getLen()
		{
			if (mdih.type == MAPDATATYPE_USER)
				return sizeof(*this) + mdih.size*sizeof(mud[0]);
			if (mdih.type == MAPDATATYPE_NPC)
				return sizeof(*this) + mdih.size*sizeof(mnd[0]);
			if (mdih.type == MAPDATATYPE_BUILDING)
				return sizeof(*this) + mdih.size*sizeof(mbd[0]);
			if (mdih.type == MAPDATATYPE_PET)
				return sizeof(*this) + mdih.size*sizeof(mpd[0]);
			return 0;
		}
	};

#define SELECTED_HPMP_PROPERTY_USERCMD_PARA 56//通知选择自己的玩家hp和mp的变化
	struct stRTSelectedHpMpPropertyUserCmd : public stMapScreenUserCmd
	{
		stRTSelectedHpMpPropertyUserCmd()
		{
			byParam = SELECTED_HPMP_PROPERTY_USERCMD_PARA;
		}
		uint8 byType;//enumMapDataType
		uint32 dwTempID;//临时编号
		uint64 dwHP;//当前血
		uint64 dwMaxHp;//最大hp
		uint32 dwMP;//当前mp
		uint32 dwMaxMp;//最大mp
	};
	/// 清除地图上物品保护
	const uint8 CLEAR_OBJECTOWNER_MAPSCREEN_USERCMD_PARA = 58;
	struct stClearObjectOwnerMapScreenUserCmd : public stMapScreenUserCmd{
		stClearObjectOwnerMapScreenUserCmd()
		{
			byParam = CLEAR_OBJECTOWNER_MAPSCREEN_USERCMD_PARA;
		}
		uint32 dwMapObjectTempID;	/**< 物品的临时ID */
	};
	/// 批量删除NPC指令
	const uint8 BATCHREMOVENPC_MAPSCREEN_USERCMD_PARA = 61;
	struct stBatchRemoveNpcMapScreenUserCmd : public stMapScreenUserCmd {
		stBatchRemoveNpcMapScreenUserCmd()
		{
			byParam = BATCHREMOVENPC_MAPSCREEN_USERCMD_PARA;
		}
		uint16	num;
		uint32   id[0];
		uint16 getLen() { return sizeof(*this) + num*sizeof(id[0]); }
	};

	/// 批量删除USER指令
	const uint8 BATCHREMOVEUSER_MAPSCREEN_USERCMD_PARA = 62;
	struct stBatchRemoveUserMapScreenUserCmd : public stMapScreenUserCmd {
		stBatchRemoveUserMapScreenUserCmd()
		{
			byParam = BATCHREMOVEUSER_MAPSCREEN_USERCMD_PARA;
		}
		uint16	num;
		uint32   id[0];
		uint16 getLen() { return sizeof(*this) + num*sizeof(id[0]); }
	};

	/// 地图上增加人物和坐标
	const uint8 ADDUSER_AND_POS_MAPSCREEN_STATE_USERCMD_PARA = 70;
	struct stAddUserAndPosMapScreenStateUserCmd : public stMapScreenUserCmd {
		stAddUserAndPosMapScreenStateUserCmd()
		{
			byParam = ADDUSER_AND_POS_MAPSCREEN_STATE_USERCMD_PARA;
			forceMove = 0;
		}
		t_MapUserDataPos data;
		uint8 forceMove;
	};
	/// 在地图上增加NPC和坐标
	const uint8 ADDMAPNPC_AND_POS_MAPSCREEN_STATE_USERCMD_PARA = 71;
	struct stAddMapNpcAndPosMapScreenStateUserCmd : public stMapScreenUserCmd{
		stAddMapNpcAndPosMapScreenStateUserCmd()
		{
			byParam = ADDMAPNPC_AND_POS_MAPSCREEN_STATE_USERCMD_PARA;
			type = 0;
		}
		t_MapNpcDataPos data;
		uint8 type; //0非重生 1重生
	};

	//////////////////////////////////////////////////////////////
	/// 魔法指令定义开始
	//////////////////////////////////////////////////////////////
	const uint8 MAGIC_USERCMD = 18;
	struct stMagicUserCmd : public stNullUserCmd{
		stMagicUserCmd()
		{
			byCmd = MAGIC_USERCMD;
		}
	};
	/// 定义攻击类型
	enum ATTACKTYPE{
		ATTACKTYPE_U2U, /// 用户攻击用户
		ATTACKTYPE_U2N, /// 用户攻击Npc
		ATTACKTYPE_N2U, /// Npc攻击用户
		ATTACKTYPE_U2B, /// 用户攻击建筑
		ATTACKTYPE_U2P, /// 用户攻击点
		ATTACKTYPE_N2N  /// Npc攻击Npc
	};
	/// 攻击指令
	const uint8 MAGIC_USERCMD_PARA = 5;
	struct stAttackMagicUserCmd : public stMagicUserCmd{

		stAttackMagicUserCmd()
		{
			byParam = MAGIC_USERCMD_PARA;
			dwUserTempID = dwDefenceTempID = wdMagicType = xDes = yDes = byDirect = byAttackType = byAction = bySeq = num = 0;
			xPixel = 0;
			yPixel = 0;
		}
		uint16 getLen() { return sizeof(*this) + num*sizeof(dwTempIDList[0]); }

		//BEGIN_MUTABLE_VARS
		uint32 dwUserTempID;          /**< 攻击临时编号 */
		uint32 dwDefenceTempID;       /**< 防御临时编号 */

		uint16 wdMagicType;           /**< 魔法类型 */

		uint16 xDes;                  /**< 目标点 */
		uint16 yDes;                  /**< 目标点 */
		uint32 xPixel;                //像素坐标x
		uint32 yPixel;                //像素坐标y
		//END_MUTABLE_VARS

		uint8 bySeq;                  //攻击序列

		uint8 byDirect;               /**< 攻击方向 */
		uint8 byAttackType;           /**< 攻击类型：ATTACKTYPE_U2U,ATTACKTYPE_U2N,ATTACKTYPE_N2U */
		uint8 byAction;               // 攻击动作  AniTypeEnum ( Ani_Null 为不确定的，由客户端确定 )
		uint8 num;
		uint32 dwTempIDList[0];       // 攻击目标列表
	};
};

#pragma pack()