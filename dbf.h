
#if !defined(_HSDBF_H_)
#define _HSDBF_H_
#include <map> 
#include <string>
#include "StdAfx.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <windows.h>
using namespace std;


#define DBF_MAX_FIELDS  2046
#define  DBF_FIELD_LEN  256
#define  MAX_PATH  260// as windows.h

#pragma pack(1) 

typedef enum tagENLockMode
{
	lmFoxFile,
	lmFoxRecord,
	lmRawFile,
	lmNoLock
}ENLockMode;

/**< DBF文件头结构 */
typedef struct tagDBFHead
{
	char			Mark;
	unsigned char	Year;
	unsigned char	Month;
	unsigned char	Day;
	long			RecCount;
	unsigned short	DataOffset;
	unsigned short	RecSize;
	char 			Reserved[20];
} DBFHEAD, *LPDBFHEAD;

/// 记录数 在文件中的位置的偏移量
const long OFFSET_OF_RECCOUNT = (long)(&((DBFHEAD *)NULL)->RecCount);

/**< DBF文件每字段结构 */
typedef struct tagDBFField
{
	char			Name[11];
	char			Type;
	/************************************************************************/
	// 20120903 zhangep 
	// 在dbf文件中，头结构中记录项的格式都是由32字节来表示，其中第12-15字节的用法不同
	// 有些dbf文件中第12-15字节表示一个记录项相对于一条记录开始的偏移量，有些第12-13字节表示一个记录项开始的偏移量，第14-15字节另有他用
	// 为了兼容这两种dbf文件的读取，故做如下处理，将一个long型的offset分成两个unsigned short变量，其中第一个unsigned short Offset与原先的long offset的含义一样
	// 第二个unsigned short Other变量另有他用
	// 这样处理只要一条记录的长度在65536个字节内，程序都能正常工作
	/************************************************************************/
	//	long			Offset;
	unsigned short Offset;
	unsigned short Other;
	unsigned char	Width;
	unsigned char	Scale;
	char			Reserved[14];
} DBFFIELD, *LPDBFFIELD;

#pragma pack()

/** DBF文件中字段名和字段号的映射结构 */
typedef struct _FieldName2Index
{
	_FieldName2Index()
	{
		lpName = NULL;
		iIndex = -1;
	}
	const char *lpName;
	int iIndex;
}stFieldName2Index, *lpstFieldName2Index;


class CDBFHelper
{
	friend class CHSDBF;

public:
	/**
	* 构造函数
	*/
	CDBFHelper();

	/**
	* 析构函数
	*/
	~CDBFHelper();

	/**
	* 开始创建DBF文件
	* @param szFileName  文件名
	* @return 0表示成功，否则失败
	*/
	int BeginCreate(const char* szFileName);

	/**
	* 给DBF文件增加一个字段
	* @param szName  字段名称
	* @param cType   字段类型
	* @param nWidth  字段宽度
	* @param nScale  字段精度
	* @return 0表示成功，否则失败
	*/
	int AddField(const char* szName, char cType, unsigned char nWidth, unsigned char nScale = 0);

	/**
	* 结束创建DBF文件，AddField的内容在此函数中才被真正写入到DBF文件中
	* @return 0表示成功，否则失败
	*/
	int EndCreate();

	/**
	* 静态方法，把DBF文件记录清空（ZAP），
	* 同时更新文件头中的更新日期及记录数（置0）
	* @param szFileName  文件名
	* @return 0表示成功，否则失败(例如：文件被其他程序打开)
	*/
	static int Zap(const char* szFileName);

private:
	/**
	* 私有静态方法，用于获取当前日期
	* date[0]为年（后2位，00 ~ 99）
	* date[1]为月（01 ~ 12）
	* date[2]为日（01 ~ 31）
	* @param date用于存放日期的地址
	*/
	static void SetDate(unsigned char* date);

	/**
	* 私有静态方法，用于更新DBF文件头的更新日期
	* @param lpFile 文件指针（调用者保证文件指针有效，本方法返回时不关闭文件）
	* @return true表示成功，false表示失败
	*/
	static bool UpdateDBFDate(FILE* lpFile);

	/// 文件指针
	FILE* m_file;

	/// 字段偏移量（当插入所有字段后，m_offset表示一条记录的长度）
	long m_offset;

	/// 保存DBF文件字段的数组
	DBFFIELD m_lpFields[DBF_MAX_FIELDS];

	/// 记录当前DBF文件的字段数
	unsigned m_fieldCnt;
};

class CDBFField
{
	friend class CHSDBF;
public:
	/**
	* 获取列名
	* @return const char *		列名
	*/
	inline const char *GetName() const
	{
		return m_FieldInfo.Name;
	}

	/**
	* 获取列号
	* @return int				列号
	*/
	inline int GetNo() const
	{
		return m_nNo;
	}

	/**
	* 获取列类型
	* @return char				列类型
	*/
	inline char GetType() const
	{
		return m_FieldInfo.Type;
	}

	/**
	* 获取列最大长度
	* @return int				列最大长度
	*/
	inline int GetWidth() const
	{
		return m_FieldInfo.Width;
	}

	/**
	* 获取列小数点位数
	* @return int				列小数点位数
	*/
	inline unsigned char GetScale() const
	{
		return m_FieldInfo.Scale;
	}

	/**
	* 获取偏移量
	* @return int				在记录中的偏移量
	*/
	inline int GetOffset() const
	{
		return m_FieldInfo.Offset;
	}

	/**
	* 获取整型列值
	* @return 整型列值
	*/
	inline int AsInt() const
	{
		return atol(m_szDataBuf);
	}

	/**
	* 获取无符号整型列值
	* @return 无符号整型列值
	*/
	inline unsigned int AsUInt() const
	{
		return (unsigned int)AsInt();
	}

	/**
	* 获取浮点型列值
	* @return 浮点型列值
	*/
	inline double AsFloat() const
	{
		return atof(m_szDataBuf);
	}

	/**
	* 获取字符型列值
	* @return 字符型列值
	*/
	inline char AsChar() const
	{
		return *m_szDataBuf;
	}

	/**
	* 获取字符串型列值
	* @return 字符串型列值
	*/
	inline const char *AsString() const
	{
		return m_szDataBuf;
	}

	/**
	* 设置整型列值
	* @param nValue：要设置的值
	* @param 0表示成功，其他表示失败
	*/
	int SetValue(int nValue);

	/**
	* 设置双精度型列值
	* @param nValue	要设置的值
	* @param true 成功，false 失败
	*/
	bool SetValue(double lfValue);

	/**
	* 设置列值
	* @param nValue	要设置的值
	* @param 0表示成功，其他表示失败
	*/
	int SetValue(const char *szValue);

private:
	int m_nNo;					/**< 列号 */
	DBFFIELD m_FieldInfo;		/**< 列信息 */
	char	m_szDataBuf[DBF_FIELD_LEN];	/**< 列缓存 */
	char	*m_lpBuf;			/**< 记录缓存指针 */
	//内联函数

	inline void InternalSetValueForGet(const char *szValue)
	{
		const char *lpStart = szValue + m_FieldInfo.Offset;
		
		//const char *lpEnd = lpStart + m_FieldInfo.Width - 1;
		const char *lpEnd = lpStart + m_FieldInfo.Width;

		while ((lpEnd >= lpStart) && (*lpEnd == ' '))
		{
			lpEnd--;
		}

		int nLen = int(lpEnd - lpStart + 1);
		memcpy(m_szDataBuf, lpStart, nLen);
		m_szDataBuf[nLen] = '\0';
		m_lpBuf = const_cast<char *>(szValue);
	}
};

class CHSDBF
{
public:
	DBFHEAD		m_head;			/**< 文件头信息 */
	char *LTrim(char *source)
	{
		char *p;

		if (*source == '\0')
			return source;
		p = source;

		while (*p == ' ')
			p++;
		if (*p == '\0')
		{
			*source = '\0';
			return p;
		}
		return strcpy(source, p);
	}

	char *RTrim(char *source)
	{
		char *p;
		if (*source == '\0')
			return source;
		p = source + strlen(source);
		p--;
		while (*p == ' ')p--;
		p++;
		*p = '\0';
		return source;
	}

	char *AllTrim(char *source)
	{
		return LTrim(RTrim(source));
	}
public:
	CHSDBF();
	~CHSDBF();
	//定义枚举类：可读等等
	enum OpenFlags {
		modeRead = (int)0x00000,
		modeWrite = (int)0x00001,
		modeReadWrite = (int)0x00002,
		shareCompat = (int)0x00000,
		shareExclusive = (int)0x00010,
		shareDenyWrite = (int)0x00020,
		shareDenyRead = (int)0x00030,
		shareDenyNone = (int)0x00040,
		modeNoInherit = (int)0x00080
	};
	/**
	* 以nOpenFlags方式打开一个DBF文件，并设置批大小
	* @param szFileName文件名，nOpenFlags文件打开方式
	* @param iBatchSize批大小
	* @return  0表示成功，其他表示失败
	*/
	int OpenDbf(const char *szFileName, unsigned int nOpenFlags, int iBatchSize);

	/**
	* 关闭一个DBF文件，没有释放文件对象缓存，用于操作多个文件时复用。
	*/
	void CloseDbf();

	/** 刷新内存
	* @return 无
	*/
	void FreshDbf();

	/** 相对于当前记录，移到某条记录
	* @param nRecCount 要跳过的记录数
	* @return 无
	*/
	void Move(int nRecCount);

	/** 相对于文件数据开始，移到某条记录
	* @param nRecordNo 记录号
	* @return 无
	*/
	void Go(int nRecordNo);

	/** 移动到文件记录开始
	* @return 无
	*/
	void MoveFirst();

	/** 移动到文件记录末尾
	* @return 无
	*/
	void MoveLast();

	/** 相对于当前记录，向前移动一条记录
	* @return 无
	*/
	void MovePrev();

	/** 相对于当前记录，想后移动一条记录
	* @return 无
	*/
	void MoveNext();

	/** 添加一条或多条记录
	* @param nAppendRecCount 要添加的记录数
	* @return true 成功
	*/
	bool Append(int nAppendRecCount = 1);

	/** 添加下一条记录
	* @return true 成功，false 失败
	*/
	bool AppendNext();

	/**
	* 编辑记录
	* @param nEditRecCount	要编辑的记录数量
	* @return ture表示可以进行编辑了，false表示不能编辑
	*/
	bool Edit(int nEditRecCount = 1);

	/** 编辑下一条记录，应用程序应该根据返回值确定是否继续编辑数据，否则会覆盖最后一条数据
	* @return 0表示成功， 其他失败,
	*/
	int EditNext();

	/** 提交修改或添加的记录
	* @return 0表示成功，其他失败（写文件）
	*/
	int Post();

	/**
	* 获取字段值引用,会调用GetField(int nIndex)函数。
	* @param szFieldName	字段名
	* @param 字段名对应的字段值引用
	*/
	inline CDBFField &GetField(const char *szFieldName)
	{
		if (!szFieldName || !m_lpFile)
		{
			return m_DFNull;
		}

		size_t szIndex = 0;
		if (BinarySearch(m_lpstFieldName2Index, m_nFields, szFieldName, &szIndex))
		{
			return GetField(m_lpstFieldName2Index[szIndex].iIndex);
		}

		return m_DFNull;
	}

	/**
	* 获取字段值引用
	* @param nIndex	字段下标
	* @param 字段名对应的字段值引用，如果失败会返回一个无效的DBFField的引用
	*/
	inline CDBFField &GetField(int nIndex)
	{
		if (nIndex < 0 || nIndex >= m_nFields)
		{
			return m_DFNull;
		}

		return m_lpFields[nIndex];
	}

	/**
	* 获取无效的DBFField地址，用于判断GetField()是否成功
	* @return 无效的DBFField地址
	*/
	CDBFField* GetNullDFAddr()
	{
		return &m_DFNull;
	}

	/**
	* 是否是文件末尾
	* @return true表示文件末尾，反之false
	*/
	inline bool IsEOF()
	{
		return m_bEOF;
	}

	/**
	* 是否是文件开始
	* @return true表示文件开始，反之false
	*/
	inline bool IsBOF()
	{
		return m_bBOF;
	}

	/**
	* 获取当前记录号
	* @return 当前记录号
	*/
	int GetRecordNo()
	{
		return m_nRecordNo;
	}

	/**
	* 获取当前打开文件记录条数
	* @return 当前打开文件记录条数
	*/
	int GetRecordCount()
	{
		return m_head.RecCount;
	}

	/**
	* 获取当前打开文件字段数
	* @return 当前打开文件字段数
	*/
	inline int GetFieldCount()
	{
		return m_nFields;
	}

	/**
	* 获取每次处理的记录条数
	* @return  批大小
	*/
	inline int GetBatchSize()
	{
		return m_nBatchSize;
	}

	/**
	* 获取每条记录的长度
	* @return  每条记录的长度
	*/
	inline int GetRecSize() const
	{
		return m_head.RecSize;
	}

	/**
	* 获取文件数据的偏移量
	* @return  文件数据的偏移量
	*/
	inline int GetDataOffset() const
	{
		return m_head.DataOffset;
	}

	BOOL Lock(int iRecordNo);
	void Unlock(int iRecordNo);

public:
	/**
	* 将数据从文件中读出
	* @param nRecordNo起始记录号，bForward = true从前往后读数据，反之从后往前读数据
	* @return  0表示成功，否则失败
	*/
	int ReadRecord(int nRecordNo, bool bForward = true);
	/**
	* 将列名存放到map中
	*/
	void ReadFields();
	/**
	* 将记录数据存放到两层map中
	*/
	void init();
private:
	/**
	* 二分查找
	* @param a      stFieldName2Index对象数组地址
	* @param length 字段对象数组长度
	* @param key    欲查找的字段名
	* @param index  参见return，可以为NULL
	* @param iType	是否区分大小写，0：不区分，1：区分
	* @return true表示找到，此时*index为下标，false表示未找到
	*/
	bool BinarySearch(lpstFieldName2Index a, size_t length, const char* key, size_t *index, int iType = 0);

	/**
	* 将指定偏移量的缓存读进字段
	* @param iOffset 偏移量
	* @return 无
	*/
	int k = 0;
	inline void Rec2Buf(int iOffset)
	{

		map<int, string> map;
		for (int i = 0; i < m_nFields; i++)
		{
			m_lpFields[i].InternalSetValueForGet(m_lpRecordBuf + iOffset);
			map[i] = m_lpFields[i].m_szDataBuf;
		}
		//下面是自己加的
		//写入map
		mapData[k++] = map;
		
	}

	/**
	* 设置每次处理的记录条数
	* @param nBatchSize 批大小
	*/
	inline void SetBatchSize(int nBatchSize)
	{
		if (m_lpFile && nBatchSize > 0)
			m_nBatchSize = nBatchSize;
	}
	FILE*		m_lpFile;		/**< 文件句柄 */

	int			m_nFields;		/**< 文件字段个数 */
	CDBFField	m_lpFields[DBF_MAX_FIELDS];		/**< 字段信息 */
	char		m_szFileName[MAX_PATH];			/**< 文件名 */
	int			m_nBatchSize;					/**< 批大小 */
	char		*m_lpRecordBuf;					/**< 记录缓存 */
	int			m_nBufSize;						/**< 缓存大小 */
	int			m_nBufRecNo0; /**< 缓存的记录头[m_nBufRecNo0, m_nBufRecNo1) */
	int			m_nBufRecNo1; /**< 缓存的记录尾 */

	enum AccessFlags
	{
		modeBrowse = 0x0000,
		modeAppend = 0x0001,
		modeEdit = 0x0002
	};
	unsigned int	m_nAccessFlags;

	bool		m_bEOF;		/**< 文件头 */
	bool		m_bBOF;		/**< 文件尾 */
	int			m_nRecordNo;/**< 当前记录号 */
	int			m_NoInBatch;/**< 当前批处理的记录号 */
	int			m_nEditRecNo;/**< 开始Edit的记录号 */
	int			m_nEditRecCount;/**< 欲Edit的记录数 */
	int			m_nAppendRecNo;/**< 开始Append的记录号 */
	int			m_nAppendRecCount;/**< 欲Append的记录数 */
	lpstFieldName2Index m_lpstFieldName2Index;/**< 字段名排序用的数组指针 */
	CDBFField   m_DFNull;/**< 无效的字段，GetField()返回用 */

	ENLockMode m_lmLockMode;	///加锁方式
	DWORD m_dwLockTime;
	HANDLE m_hFile;
//下面这俩字段用不到
public:
	map<int, string> mapFiled;
	map<int,map<int, string>> mapData;

};
#endif /// !defined(_HSDBF_H_)
