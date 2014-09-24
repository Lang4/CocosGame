#include "MyPack.h"
#include "io.h"
#define DEC_FILE_BASE_TYPE(type)\
void write(type &a)\
{\
	write_base(a);\
}\
void read(type &a)\
{\
	read_base(a);\
}

#define IMP_FILE_OBJECT(__class__)\
void operator & (FilePack &ss,__class__ &o)\

#define FILE_BIND(field)\
	if (ss.isTag(FilePack::OUTTAG))\
	{\
		ss.write(o.field);\
	}\
	else ss.read(o.field);

class FilePack{
public:	
	enum INOUT
	{
		INTAG = 2,
		OUTTAG = 3,
	};
	IPackGetBack *iBack;
	INOUT tag;
	bool isTag(INOUT src) {return tag == src;}
	void setTag(INOUT src){tag = src;}
	FILE *hFile;
	unsigned int getNowPos()
	{
		fpos_t filepos;
		fgetpos(hFile, &filepos); 
		return filepos;
	}
	int allSize;
	void move(int pos)
	{
		fseek(hFile,pos,SEEK_CUR);
		if (iBack)
			iBack->backGet(pos, allSize);
	}
	FilePack(FILE *hFile):hFile(hFile)
	{
		iBack = NULL;
		allSize = filelength(fileno(hFile));
	}
	~FilePack()
	{
	}
	template<typename type>
	void write_base(type a)
	{
		fwrite(&a,sizeof(type),1,hFile);
	}
	template<typename type>
	void read_base(type& a)
	{
		fread(&a,sizeof(type),1,hFile);	
		if (iBack)
			iBack->backGet(sizeof(type), allSize);
	}
	DEC_FILE_BASE_TYPE(int);
	DEC_FILE_BASE_TYPE(char);
	DEC_FILE_BASE_TYPE(short);
	DEC_FILE_BASE_TYPE(unsigned int);
	DEC_FILE_BASE_TYPE(unsigned char);
	DEC_FILE_BASE_TYPE(unsigned short);
	DEC_FILE_BASE_TYPE(unsigned long);
	DEC_FILE_BASE_TYPE(float);
	
	template<typename type,int size>
	void write(type (& a)[size])
	{
		if (size)
		{
			fwrite(&a[0], size * sizeof(type), 1, hFile);
		}	
	}
	template<typename type,int size>
	void read(type (& a)[size])
	{
		if (size)
		{
			fread(&a[0], size * sizeof(type), 1, hFile);
		}
		if (iBack)
			iBack->backGet(size * sizeof(type), allSize);
	}

	template<typename type>
	void write(type& a)
	{
		FilePack ss(hFile);
		ss.setTag(OUTTAG);
		ss & a;
	}
	template<typename type>
	void read(type& a)
	{
		FilePack ss(hFile);
		ss.iBack = iBack;
		ss.setTag(INTAG);
		ss & a;
	}
	void read(std::string &a)
	{
		unsigned int len = 0;
		read(len);
		if (len)
		{
			a.resize(len);
			fread(&a[0],len,1,hFile);
			if (iBack)
				iBack->backGet(len, allSize);
		}
	}
	void write(std::string &a)
	{
		unsigned int len = a.size();
		write(len);
		if (len)
		{
			fwrite(&a[0],len,1,hFile);
		}	
	}
};
template<typename __type__>
void operator & (FilePack &ss,std::vector<__type__>& o)
{
	if (ss.isTag(FilePack::OUTTAG))
	{
		int size = o.size();
		ss.write(size);
		for (typename std::vector<__type__>::iterator iter = o.begin(); iter != o.end();++iter)
		{
			__type__ v = *iter;
			ss.write(v);
		}
	}
	else
	{
		int size = 0;
		ss.read(size);
		while (size-- >0)
		{
			__type__ object;
			ss.read(object);
			o.push_back(object);
		}
	}
}

IMP_FILE_OBJECT(MyFileInfo)
{
	FILE_BIND(offset);
	FILE_BIND(name);
}

IMP_FILE_OBJECT(MyFileInfos)
{
	FILE_BIND(infosName);
	FILE_BIND(infos);
}

IMP_FILE_OBJECT(MyFileBinary)
{
	if (ss.isTag(FilePack::OUTTAG))
		o.startPos = ss.getNowPos();
	FILE_BIND(startPos);
	if (ss.isTag(FilePack::INTAG)) // 读取时特殊处理
	{
		unsigned int len = 0;
		ss.read(len);
		if (len)
		{
			o.contentSize = len;
		}
		ss.move(o.contentSize);
	}
	else
		FILE_BIND(content);
}

IMP_FILE_OBJECT(MyGroup)
{
	FILE_BIND(name);
	FILE_BIND(files);
	FILE_BIND(binaries);
	FILE_BIND(childs);
}

IMP_FILE_OBJECT(MyPack)
{
	FILE_BIND(groups);
}

bool MyPack::load(const char *packName,IPackGetBack *iBack)// 加载
{
	hFile = fopen(packName,"rb");
	this->packName = packName;
	if (!hFile) return false;
	FilePack pack(hFile);
	pack.iBack = iBack;
	pack.read(*this);
	return true;
} 

void MyPack::unload()
{
	if (hFile)
		fclose(hFile);
	hFile = NULL;
}
void MyPack::save()
{
	write(packName.c_str());
}

void MyPack::write(const char *fileName)
{
	FILE *hFile = fopen(fileName,"wb");
	if (hFile)
	{
		fseek(hFile,0,SEEK_SET);
		FilePack pack(hFile);
		pack.write(*this);
		fclose(hFile);
	}
}
void MyPack::read(const char *fileName)
{
	FILE *hFile = fopen(fileName,"rb");
	if (!hFile) return;
	FilePack pack(hFile);
	pack.read(*this);
	fclose(hFile);
	return;
}

void MyPack::addFile(const char *fileName,int grpID)
{
	if (grpID >= groups.size()) groups.resize(grpID + 1);

	MyGroup * group =& groups[grpID];
	if (group)
	{
		group->addFile(fileName);
	}
}

MyGroup * MyPack::addFile(const char *fileName,const char *grpName)
{
	MyGroup * grp = addGrp(grpName);
	if (grp)
	{
		grp->addFile(fileName);
	}
	return grp;
}

MyGroup * MyPack::addGrp(const char *grpName)
{
	MyGroup *grp = getGrp(grpName,NULL);
	if (grp) return grp;
	MyGroup group;
	group.name = grpName;
	groups.push_back(group);
	return & groups.back();
}
MyGroup * MyPack::getGrp(const char *grpName, ICheckSameGroup *check)
{
	for (GROUPS_ITER iter = groups.begin(); iter != groups.end();++iter)
	{
		if (!check &&iter->name == std::string(grpName)) return &(*iter);
		if (check && check->check(iter->name, grpName)) return &(*iter);
	}
	return NULL;
}
/**
 * 增加文件
 * 传入文件的路径
 * \return 成功 失败
 */
bool MyPack::addFile(const char *path)
{
	const char *temp = path;
	std::string winPath;
	std::string grpName;
	std::vector<std::string> paths;
	while (*temp != '\0')
	{
		if (*temp == '/')
		{
			paths.push_back(grpName);
			grpName.clear();
			winPath.push_back('\\');
		}
		else
		{
			winPath.push_back(*temp);
			grpName.push_back(*temp);
		}
		temp++;
	}
	if (paths.empty())
	{
		addFile(path,0);
	}
	else 
	{
		MyGroup * group = addGrp(paths[0].c_str());
		for (unsigned int index = 1; index < paths.size();++index)
		{
			group = group->addGrp(paths[index].c_str());
		}
		if (group)
		{
			group->addFile(winPath.c_str());
		}
	}
	return true;
}

/**
 * 获取组下 文件的的内容
 */
MyFileBinary * MyPack::getFileContent(const char *fileName, ICheckSameGroup *check)
{
	const char *temp = fileName;
	std::string winPath;
	std::string grpName;
	std::vector<std::string> paths;
	while (*temp != '\0')
	{
		if (*temp == '/')
		{
			paths.push_back(grpName);
			grpName.clear();
			winPath.push_back('\\');
		}
		else
		{
			winPath.push_back(*temp);
			grpName.push_back(*temp);
		}
		temp++;
	}
	if (paths.empty())
	{
		return getFileContent(0,fileName);
	}
	else 
	{
		MyGroup * group = getGrp(paths[0].c_str(), check);
		std::string path;
		path.append(group->name);
		path.push_back('\\');
		for (unsigned int index = 1; index < paths.size();++index)
		{
			group = group->getGrp(paths[index].c_str(), check);
			
			path.append(group->name);
			path.push_back('\\');
		}
		path.append(grpName);
		if (group)
		{
			return group->getFileContent(path.c_str(),hFile);
		}
	}
	return NULL;
}

/**
* 获取上一层的库
*/
MyGroup * MyPack::getGrpByPath(const char *fileName, ICheckSameGroup *check)
{
	const char *temp = fileName;
	std::string winPath;
	std::string grpName;
	std::vector<std::string> paths;
	while (*temp != '\0')
	{
		if (*temp == '/')
		{
			paths.push_back(grpName);
			grpName.clear();
			winPath.push_back('\\');
		}
		else
		{
			winPath.push_back(*temp);
			grpName.push_back(*temp);
		}
		temp++;
	}
	if (paths.empty())
	{
		if (groups.empty()) return NULL;
		return &groups[0];
	}
	else
	{
		MyGroup * group = getGrp(paths[0].c_str(),check);
		if (!group) return NULL;
		for (unsigned int index = 1; index < paths.size(); ++index)
		{
			group = group->getGrp(paths[index].c_str(),check);
		}
		if (group)
		{
			return group;
		}
	}
	return NULL;
}

/**
* 获取组下编号的文件内容
*/
MyFileBinary * MyPack::getFileContent(MyGroup *group, int index)
{
	MyFileBinary * bin = group->getFileContent(index, hFile);
	return bin;
}