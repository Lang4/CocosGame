#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <stdio.h>
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
	void move(int pos)
	{
		fseek(hFile,pos,SEEK_CUR);
	}
	FilePack(FILE *hFile):hFile(hFile)
	{
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
			fwrite(&a[0],size,1,hFile);
		}	
	}
	template<typename type,int size>
	void read(type (& a)[size])
	{
		if (size)
		{
			fread(&a[0],size,1,hFile);
		}
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


