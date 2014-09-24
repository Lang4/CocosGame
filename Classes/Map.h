#pragma once
#include "cocos2d.h"
using namespace cocos2d;
#include "MapCell.h"
/**
 * local map = Terrain:create()
 * map:setMapName("111") -- 地图名称
 * map:setGridCount(200,200) -- 地图大小
 * map:setGridSize(256,256) --网格大小
 * map:setViewGridCount(10,10) -- 视图网格数量
 * map:addToScene(mapScene)--加入到地图层
 **/
class Terrain:public Node{
public:
	Terrain()
	{}
	~Terrain()
	{}
	CREATE_FUNC(Terrain);
	bool init()
	{
		oldStartGrid.x = -1;
		return true;
	}
	/**
	 * 加入到节点
	 */
	void addToScene(Node* scene)
	{
		scene->addChild(this);
	}
	void setUrl(const char *url)
	{
		this->url = url;
	}
	/**
	 * 设置地图名字
	 */
	void setMapName(const char *name)
	{
		mapName = name;
	}
	/**
	 * 设置地图格子数量
	 */
	void setGridCount(int gridWidthCount,int gridHeightCount)
	{
		mapGridCountSize.width = gridWidthCount;
		mapGridCountSize.height = gridHeightCount;
	}

	/**
	 * 设置视图格子数量
	 */
	void setViewGridCount(int viewGridWidthCount,int viewGridHeightCount)
	{
		viewCountSize.width = viewGridWidthCount;
		viewCountSize.height = viewGridHeightCount;
		nowCells.resize(viewCountSize.width * viewCountSize.height);
	}
	/**
	 * 设置块大小 像素大小
	 */
	void setGridSize(float gridWidth,float gridHeight)
	{
		gridPixelSize.width = gridWidth;
		gridPixelSize.height = gridHeight;
	}

	/**
     * 设置位置 
     * 像素位置
	 */
	void showPixelPosition(float x,float y)
	{
		
	}
	void setPosition(float x,float y)
	{
		Node::setPosition(x,y);
		if (x >= 0) nowStartGrid.x = 0;
		if (y >= 0) nowStartGrid.y = 0;
		if (x < 0) nowStartGrid.x = (int)(-x / gridPixelSize.width);
		if (y < 0)nowStartGrid.y = (int)(-y / gridPixelSize.height);
		updateView();
	}
	
	/**
	 * 检查当前位置 更新地图纹理 
	 */
	void updateView();
	/**
	 * 根据起始位置 和 偏移 获取全局索引
	 */
	Point getGloabIndex(const Point& startGrid,int index)
	{
		int offsetx = index % ((int)viewCountSize.width);
		int offsety = index / viewCountSize.width;
		if (offsety < 0) offsety = 0;
		return Point(startGrid.x + offsetx,startGrid.y + offsety);
	}
	/**
	 * 根据其实位置 和 全局索引 获取局部索引
	 */
	int getLocalIndex(const Point &startGrid,const Point &gloabIndex)
	{
		int offsetx = gloabIndex.x - startGrid.x;
		int offsety = gloabIndex.y - startGrid.y;
		if (offsetx < 0 || offsety < 0) return -1;
		return offsetx + offsety * viewCountSize.width;
	}
public:
	Point nowStartGrid; 			// 新的全局起始格子
	Point oldStartGrid; 			// 老的全局起始格子
	Size viewCountSize; 				    // 视图的大小
	Size gridPixelSize;   				// 网格像素大小
	Size mapGridCountSize; 					// 地图网格数量大小
	std::vector<MapCell*> nowCells; // 当前格子集合
	std::vector<MapCell*> oldCells; // 老的格子集合
	std::string mapName; 			// 地图名字
	std::string url; // 所在的URL 跟目录
};