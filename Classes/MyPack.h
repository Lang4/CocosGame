/**
 * 我的资源管理
 **/
#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <stdio.h>

/***************************************************文件管理************************************************************/

class MyPack;
class MyFileInfo{ // 文件信息
public:
	int offset; // 纹理编号
	std::string name; // 文件名字
	MyFileInfo()
	{
		memset(this,0,sizeof(*this));
	}
};


class MyFileInfos{
public:
	std::string infosName;
	std::vector<MyFileInfo > infos; // 图片信息集合
};



class MyFileBinary{
public:
	int startPos; // 起始位置
	std::string content; // 内容
	int contentSize;
	MyFileBinary()
	{
		startPos = 0;
		contentSize = 0;
	}
	void load(FILE *hFile)
	{
		fseek(hFile,startPos + 2 * sizeof(int),SEEK_SET);
		if (contentSize && content.empty())
		{
			content.resize(contentSize);
			fread(&content[0],content.size(),1,hFile);
		}
	}
	void unload()
	{
		content.clear();
	}
};

class ICheckSameGroup{
public:
	virtual bool check(const std::string &src, const std::string &dest){
		return src == dest;
	}
};
class IPackGetBack{
public:
	virtual void backGet(int offsetSize, int allSize) = 0;
};
class MyGroup{
public:
	std::string name; // Group 的名字
	std::vector<MyFileInfos> files; // 图片集合
	typedef std::vector<MyFileInfos>::iterator FILES_ITER;
	std::vector<MyFileBinary> binaries; // 二进制数据集合
	std::vector<MyGroup> childs; // 子组
	typedef std::vector<MyGroup>::iterator CHILDS_ITER;
	/**
	 * 增加组
	 * \param grpName 组名字
	 * \return 组指针
	 */
	MyGroup * addGrp(const char *grpName)
	{
		MyGroup * grp = getGrp(grpName,NULL);
		if (grp) return grp;
		MyGroup newGrp;
		newGrp.name = grpName;
		childs.push_back(newGrp);
		return & childs.back();
	}
	/**
	 * 获取组
	 * \param grpName 组名字
	 * \return 组指针
	 */
	MyGroup * getGrp(const char *grpName, ICheckSameGroup *check)
	{
		for (CHILDS_ITER iter = childs.begin(); iter != childs.end(); ++iter)
		{

			if (!check &&iter->name == std::string(grpName)) return &(*iter);
			if (check && check->check(iter->name, grpName)) return &(*iter);
		}
		return NULL;
	}

	MyFileInfos * getFileInfos(int index)
	{
		if (index < files.size())
			return &files[index];
		return NULL;
	}
	MyFileInfos * getFileInfosByName(const char *fileName)
	{
		for (FILES_ITER iter = files.begin(); iter != files.end();++iter)
		{
			if (iter->infosName == fileName)
			{
				return  &(*iter);
			}
		}
		return NULL;
	}
	/**
	 * 获取文件内容
	 */
	MyFileBinary* getFileContent(int index,FILE *hFile)
	{
		MyFileInfos *info = getFileInfos(index);
		if (info && info->infos.size())
		{
			int offset = info->infos[0].offset;
			if (offset < binaries.size())
			{
				MyFileBinary *bin = &binaries[offset];
				if (bin)
				{
					bin->load(hFile);
				}
				return bin;
			}
		}
		return NULL;
	}
	MyFileBinary* getFileContent(const char *fileName,FILE *hFile)
	{
		MyFileInfos *info = getFileInfosByName(fileName);
		if (info && info->infos.size())
		{
			int offset = info->infos[0].offset;
			if (offset < binaries.size())
			{
				MyFileBinary *bin = &binaries[offset];
				if (bin)
				{
					bin->load(hFile);
				}
				return bin;
			}
		}
		return NULL;
	}
	/**
	 * 增加文件
	 */
	void addFile(const char *fileName)
	{
		// 先查看是否已存在信息
		int offset = -1;
		MyFileInfos *fileInfos = NULL;
		for (FILES_ITER iter = files.begin(); iter != files.end();++iter)
		{
			if (iter->infosName == fileName)
			{
				if (iter->infos.size())
					offset = iter->infos[0].offset;
				fileInfos = &(*iter);
			}
		}
		if (-1 == offset || offset >= binaries.size()) // 说明无binary
		{
			// 加入到binaries
			MyFileBinary bin;
			int id = binaries.size();
			binaries.push_back(bin);
			std::string &content = binaries.back().content;
			readFile(fileName,content);
			// 创建File
			if (NULL == fileInfos)
			{
				MyFileInfos infos;
				infos.infosName = fileName;
				MyFileInfo fileInfo;
				fileInfo.offset = id;
				fileInfo.name = fileName;
				infos.infos.push_back(fileInfo);
				files.push_back(infos);
			}
			else // 文件存在里面无内容
			{
				fileInfos->infosName = fileName;
				MyFileInfo fileInfo;
				fileInfo.offset = id;
				fileInfo.name = fileName;
				fileInfos->infos[0] = fileInfo;
			}
		}
		else // 说明有binary 只要更新binary 即可
		{
			MyFileBinary *bin = &binaries[offset];
			if (bin)
			{
				std::string &content = bin->content;
				readFile(fileName,content);
			}
		}
	}
	/**
	 * 从文件读数据
	 */
	void readFile(const char *fileName,std::string &content)
	{
		FILE* hFile = fopen(fileName,"rb");
		if (!hFile) return;
		int file_begin = ftell(hFile); 
		fseek(hFile, 0, SEEK_END); 
		int file_end = ftell(hFile); 
		int count = file_end - file_begin;
		fseek(hFile, 0, SEEK_SET); 
		if (0 == count)
		{
			if (hFile)
			{
				fclose(hFile);
			}
			return;
		} 

		if (count) content.resize(count);
		if(fread(&content[0], count, 1, hFile) < 1) 
		{
		} 
		if (hFile)
		{
			fclose(hFile);
		}
	}
	int getFileCount()
	{
		return files.size();
	}
};


class MyPack{
public:
	bool load(const char *packName, IPackGetBack *iBack = NULL);// 加载
	void unload(); // 卸载
	
	std::vector<MyGroup> groups; // 组
	typedef std::vector<MyGroup>::iterator GROUPS_ITER;
	/**
	 * 增加文件
	 * \param fileName 文件名字
	 * \param grpID 组号
	 */
	void addFile(const char *fileName,int grpID); // 加入文件

	/**
	 * 增加文件
	 * \param fileName 文件名字
	 * \param grpName 组名字
	 * \return 组指针
	 */
	MyGroup * addFile(const char *fileName,const char *grpName);

	/**
	 * 增加组
	 * \param grpName 组名字
	 * \return 组指针
	 **/
	MyGroup * addGrp(const char *grpName);
	
	/**
	 * 获取组
	 * \param 组名字
	 * \return 组指针
	 */
	MyGroup * getGrp(const char *grpName, ICheckSameGroup *check);
	/**
	 * 增加文件
	 * 传入文件的路径
	 * \return 成功 失败
	 */
	bool addFile(const char *path);

	void write(const char *fileName); // 写入到Pack文件
	void read(const char *fileName);
	/**
	 * 获取文件内容
	 * \param grpID 组号
	 * \param offset 偏移
	 * \return 内容
	 */
	MyFileBinary * getFileContent(int grpID,int offset)
	{
		if (grpID>=0 && grpID < groups.size())
		{
			MyGroup * group = &groups[grpID];
			if (group)
			{
				return group->getFileContent(offset,hFile);
			}
		}
		return NULL;
	}
	/**
	 * 获取文件内容
	 * \param grpID 组号
	 * \param fileName 文件名字
	 * \param 获取文件内容
	 */
	MyFileBinary * getFileContent(int grpID,const char *fileName)
	{
		if (grpID>=0 && grpID < groups.size())
		{
			MyGroup * group = &groups[grpID];
			if (group)
			{
				return group->getFileContent(fileName,hFile);
			}
		}
		return NULL;
	}
	/**
	 * 获取组下 文件的的内容
	 */
	MyFileBinary * getFileContent(const char *grpName,const char *fileName)
	{
		MyGroup * group = getGrp(grpName,NULL);
		if (group) return group->getFileContent(fileName,hFile);
		return NULL;
	}

	/**
	 * 获取组下 文件的的内容
	 */
	MyFileBinary * getFileContent(const char *fileName, ICheckSameGroup *check=NULL);
	/**
	 * 获取上一层的库
	 */
	MyGroup * getGrpByPath(const char *fileName, ICheckSameGroup *check = NULL);
	/**
	 * 获取组下编号的文件内容
	 */
	MyFileBinary * getFileContent(MyGroup *group, int index);
	void save(); // 保存
	MyPack()
	{
		hFile = NULL;
	}
	~MyPack()
	{
		unload();
	}
private:
	FILE *hFile; // 文件句柄
	std::string packName;
};

