
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

/**< DBF�ļ�ͷ�ṹ */
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

/// ��¼�� ���ļ��е�λ�õ�ƫ����
const long OFFSET_OF_RECCOUNT = (long)(&((DBFHEAD *)NULL)->RecCount);

/**< DBF�ļ�ÿ�ֶνṹ */
typedef struct tagDBFField
{
	char			Name[11];
	char			Type;
	/************************************************************************/
	// 20120903 zhangep 
	// ��dbf�ļ��У�ͷ�ṹ�м�¼��ĸ�ʽ������32�ֽ�����ʾ�����е�12-15�ֽڵ��÷���ͬ
	// ��Щdbf�ļ��е�12-15�ֽڱ�ʾһ����¼�������һ����¼��ʼ��ƫ��������Щ��12-13�ֽڱ�ʾһ����¼�ʼ��ƫ��������14-15�ֽ���������
	// Ϊ�˼���������dbf�ļ��Ķ�ȡ���������´�����һ��long�͵�offset�ֳ�����unsigned short���������е�һ��unsigned short Offset��ԭ�ȵ�long offset�ĺ���һ��
	// �ڶ���unsigned short Other������������
	// ��������ֻҪһ����¼�ĳ�����65536���ֽ��ڣ���������������
	/************************************************************************/
	//	long			Offset;
	unsigned short Offset;
	unsigned short Other;
	unsigned char	Width;
	unsigned char	Scale;
	char			Reserved[14];
} DBFFIELD, *LPDBFFIELD;

#pragma pack()

/** DBF�ļ����ֶ������ֶκŵ�ӳ��ṹ */
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
	* ���캯��
	*/
	CDBFHelper();

	/**
	* ��������
	*/
	~CDBFHelper();

	/**
	* ��ʼ����DBF�ļ�
	* @param szFileName  �ļ���
	* @return 0��ʾ�ɹ�������ʧ��
	*/
	int BeginCreate(const char* szFileName);

	/**
	* ��DBF�ļ�����һ���ֶ�
	* @param szName  �ֶ�����
	* @param cType   �ֶ�����
	* @param nWidth  �ֶο��
	* @param nScale  �ֶξ���
	* @return 0��ʾ�ɹ�������ʧ��
	*/
	int AddField(const char* szName, char cType, unsigned char nWidth, unsigned char nScale = 0);

	/**
	* ��������DBF�ļ���AddField�������ڴ˺����вű�����д�뵽DBF�ļ���
	* @return 0��ʾ�ɹ�������ʧ��
	*/
	int EndCreate();

	/**
	* ��̬��������DBF�ļ���¼��գ�ZAP����
	* ͬʱ�����ļ�ͷ�еĸ������ڼ���¼������0��
	* @param szFileName  �ļ���
	* @return 0��ʾ�ɹ�������ʧ��(���磺�ļ������������)
	*/
	static int Zap(const char* szFileName);

private:
	/**
	* ˽�о�̬���������ڻ�ȡ��ǰ����
	* date[0]Ϊ�꣨��2λ��00 ~ 99��
	* date[1]Ϊ�£�01 ~ 12��
	* date[2]Ϊ�գ�01 ~ 31��
	* @param date���ڴ�����ڵĵ�ַ
	*/
	static void SetDate(unsigned char* date);

	/**
	* ˽�о�̬���������ڸ���DBF�ļ�ͷ�ĸ�������
	* @param lpFile �ļ�ָ�루�����߱�֤�ļ�ָ����Ч������������ʱ���ر��ļ���
	* @return true��ʾ�ɹ���false��ʾʧ��
	*/
	static bool UpdateDBFDate(FILE* lpFile);

	/// �ļ�ָ��
	FILE* m_file;

	/// �ֶ�ƫ�����������������ֶκ�m_offset��ʾһ����¼�ĳ��ȣ�
	long m_offset;

	/// ����DBF�ļ��ֶε�����
	DBFFIELD m_lpFields[DBF_MAX_FIELDS];

	/// ��¼��ǰDBF�ļ����ֶ���
	unsigned m_fieldCnt;
};

class CDBFField
{
	friend class CHSDBF;
public:
	/**
	* ��ȡ����
	* @return const char *		����
	*/
	inline const char *GetName() const
	{
		return m_FieldInfo.Name;
	}

	/**
	* ��ȡ�к�
	* @return int				�к�
	*/
	inline int GetNo() const
	{
		return m_nNo;
	}

	/**
	* ��ȡ������
	* @return char				������
	*/
	inline char GetType() const
	{
		return m_FieldInfo.Type;
	}

	/**
	* ��ȡ����󳤶�
	* @return int				����󳤶�
	*/
	inline int GetWidth() const
	{
		return m_FieldInfo.Width;
	}

	/**
	* ��ȡ��С����λ��
	* @return int				��С����λ��
	*/
	inline unsigned char GetScale() const
	{
		return m_FieldInfo.Scale;
	}

	/**
	* ��ȡƫ����
	* @return int				�ڼ�¼�е�ƫ����
	*/
	inline int GetOffset() const
	{
		return m_FieldInfo.Offset;
	}

	/**
	* ��ȡ������ֵ
	* @return ������ֵ
	*/
	inline int AsInt() const
	{
		return atol(m_szDataBuf);
	}

	/**
	* ��ȡ�޷���������ֵ
	* @return �޷���������ֵ
	*/
	inline unsigned int AsUInt() const
	{
		return (unsigned int)AsInt();
	}

	/**
	* ��ȡ��������ֵ
	* @return ��������ֵ
	*/
	inline double AsFloat() const
	{
		return atof(m_szDataBuf);
	}

	/**
	* ��ȡ�ַ�����ֵ
	* @return �ַ�����ֵ
	*/
	inline char AsChar() const
	{
		return *m_szDataBuf;
	}

	/**
	* ��ȡ�ַ�������ֵ
	* @return �ַ�������ֵ
	*/
	inline const char *AsString() const
	{
		return m_szDataBuf;
	}

	/**
	* ����������ֵ
	* @param nValue��Ҫ���õ�ֵ
	* @param 0��ʾ�ɹ���������ʾʧ��
	*/
	int SetValue(int nValue);

	/**
	* ����˫��������ֵ
	* @param nValue	Ҫ���õ�ֵ
	* @param true �ɹ���false ʧ��
	*/
	bool SetValue(double lfValue);

	/**
	* ������ֵ
	* @param nValue	Ҫ���õ�ֵ
	* @param 0��ʾ�ɹ���������ʾʧ��
	*/
	int SetValue(const char *szValue);

private:
	int m_nNo;					/**< �к� */
	DBFFIELD m_FieldInfo;		/**< ����Ϣ */
	char	m_szDataBuf[DBF_FIELD_LEN];	/**< �л��� */
	char	*m_lpBuf;			/**< ��¼����ָ�� */
	//��������

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
	DBFHEAD		m_head;			/**< �ļ�ͷ��Ϣ */
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
	//����ö���ࣺ�ɶ��ȵ�
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
	* ��nOpenFlags��ʽ��һ��DBF�ļ�������������С
	* @param szFileName�ļ�����nOpenFlags�ļ��򿪷�ʽ
	* @param iBatchSize����С
	* @return  0��ʾ�ɹ���������ʾʧ��
	*/
	int OpenDbf(const char *szFileName, unsigned int nOpenFlags, int iBatchSize);

	/**
	* �ر�һ��DBF�ļ���û���ͷ��ļ����󻺴棬���ڲ�������ļ�ʱ���á�
	*/
	void CloseDbf();

	/** ˢ���ڴ�
	* @return ��
	*/
	void FreshDbf();

	/** ����ڵ�ǰ��¼���Ƶ�ĳ����¼
	* @param nRecCount Ҫ�����ļ�¼��
	* @return ��
	*/
	void Move(int nRecCount);

	/** ������ļ����ݿ�ʼ���Ƶ�ĳ����¼
	* @param nRecordNo ��¼��
	* @return ��
	*/
	void Go(int nRecordNo);

	/** �ƶ����ļ���¼��ʼ
	* @return ��
	*/
	void MoveFirst();

	/** �ƶ����ļ���¼ĩβ
	* @return ��
	*/
	void MoveLast();

	/** ����ڵ�ǰ��¼����ǰ�ƶ�һ����¼
	* @return ��
	*/
	void MovePrev();

	/** ����ڵ�ǰ��¼������ƶ�һ����¼
	* @return ��
	*/
	void MoveNext();

	/** ���һ���������¼
	* @param nAppendRecCount Ҫ��ӵļ�¼��
	* @return true �ɹ�
	*/
	bool Append(int nAppendRecCount = 1);

	/** �����һ����¼
	* @return true �ɹ���false ʧ��
	*/
	bool AppendNext();

	/**
	* �༭��¼
	* @param nEditRecCount	Ҫ�༭�ļ�¼����
	* @return ture��ʾ���Խ��б༭�ˣ�false��ʾ���ܱ༭
	*/
	bool Edit(int nEditRecCount = 1);

	/** �༭��һ����¼��Ӧ�ó���Ӧ�ø��ݷ���ֵȷ���Ƿ�����༭���ݣ�����Ḳ�����һ������
	* @return 0��ʾ�ɹ��� ����ʧ��,
	*/
	int EditNext();

	/** �ύ�޸Ļ���ӵļ�¼
	* @return 0��ʾ�ɹ�������ʧ�ܣ�д�ļ���
	*/
	int Post();

	/**
	* ��ȡ�ֶ�ֵ����,�����GetField(int nIndex)������
	* @param szFieldName	�ֶ���
	* @param �ֶ�����Ӧ���ֶ�ֵ����
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
	* ��ȡ�ֶ�ֵ����
	* @param nIndex	�ֶ��±�
	* @param �ֶ�����Ӧ���ֶ�ֵ���ã����ʧ�ܻ᷵��һ����Ч��DBFField������
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
	* ��ȡ��Ч��DBFField��ַ�������ж�GetField()�Ƿ�ɹ�
	* @return ��Ч��DBFField��ַ
	*/
	CDBFField* GetNullDFAddr()
	{
		return &m_DFNull;
	}

	/**
	* �Ƿ����ļ�ĩβ
	* @return true��ʾ�ļ�ĩβ����֮false
	*/
	inline bool IsEOF()
	{
		return m_bEOF;
	}

	/**
	* �Ƿ����ļ���ʼ
	* @return true��ʾ�ļ���ʼ����֮false
	*/
	inline bool IsBOF()
	{
		return m_bBOF;
	}

	/**
	* ��ȡ��ǰ��¼��
	* @return ��ǰ��¼��
	*/
	int GetRecordNo()
	{
		return m_nRecordNo;
	}

	/**
	* ��ȡ��ǰ���ļ���¼����
	* @return ��ǰ���ļ���¼����
	*/
	int GetRecordCount()
	{
		return m_head.RecCount;
	}

	/**
	* ��ȡ��ǰ���ļ��ֶ���
	* @return ��ǰ���ļ��ֶ���
	*/
	inline int GetFieldCount()
	{
		return m_nFields;
	}

	/**
	* ��ȡÿ�δ���ļ�¼����
	* @return  ����С
	*/
	inline int GetBatchSize()
	{
		return m_nBatchSize;
	}

	/**
	* ��ȡÿ����¼�ĳ���
	* @return  ÿ����¼�ĳ���
	*/
	inline int GetRecSize() const
	{
		return m_head.RecSize;
	}

	/**
	* ��ȡ�ļ����ݵ�ƫ����
	* @return  �ļ����ݵ�ƫ����
	*/
	inline int GetDataOffset() const
	{
		return m_head.DataOffset;
	}

	BOOL Lock(int iRecordNo);
	void Unlock(int iRecordNo);

public:
	/**
	* �����ݴ��ļ��ж���
	* @param nRecordNo��ʼ��¼�ţ�bForward = true��ǰ��������ݣ���֮�Ӻ���ǰ������
	* @return  0��ʾ�ɹ�������ʧ��
	*/
	int ReadRecord(int nRecordNo, bool bForward = true);
	/**
	* ��������ŵ�map��
	*/
	void ReadFields();
	/**
	* ����¼���ݴ�ŵ�����map��
	*/
	void init();
private:
	/**
	* ���ֲ���
	* @param a      stFieldName2Index���������ַ
	* @param length �ֶζ������鳤��
	* @param key    �����ҵ��ֶ���
	* @param index  �μ�return������ΪNULL
	* @param iType	�Ƿ����ִ�Сд��0�������֣�1������
	* @return true��ʾ�ҵ�����ʱ*indexΪ�±꣬false��ʾδ�ҵ�
	*/
	bool BinarySearch(lpstFieldName2Index a, size_t length, const char* key, size_t *index, int iType = 0);

	/**
	* ��ָ��ƫ�����Ļ�������ֶ�
	* @param iOffset ƫ����
	* @return ��
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
		//�������Լ��ӵ�
		//д��map
		mapData[k++] = map;
		
	}

	/**
	* ����ÿ�δ���ļ�¼����
	* @param nBatchSize ����С
	*/
	inline void SetBatchSize(int nBatchSize)
	{
		if (m_lpFile && nBatchSize > 0)
			m_nBatchSize = nBatchSize;
	}
	FILE*		m_lpFile;		/**< �ļ���� */

	int			m_nFields;		/**< �ļ��ֶθ��� */
	CDBFField	m_lpFields[DBF_MAX_FIELDS];		/**< �ֶ���Ϣ */
	char		m_szFileName[MAX_PATH];			/**< �ļ��� */
	int			m_nBatchSize;					/**< ����С */
	char		*m_lpRecordBuf;					/**< ��¼���� */
	int			m_nBufSize;						/**< �����С */
	int			m_nBufRecNo0; /**< ����ļ�¼ͷ[m_nBufRecNo0, m_nBufRecNo1) */
	int			m_nBufRecNo1; /**< ����ļ�¼β */

	enum AccessFlags
	{
		modeBrowse = 0x0000,
		modeAppend = 0x0001,
		modeEdit = 0x0002
	};
	unsigned int	m_nAccessFlags;

	bool		m_bEOF;		/**< �ļ�ͷ */
	bool		m_bBOF;		/**< �ļ�β */
	int			m_nRecordNo;/**< ��ǰ��¼�� */
	int			m_NoInBatch;/**< ��ǰ������ļ�¼�� */
	int			m_nEditRecNo;/**< ��ʼEdit�ļ�¼�� */
	int			m_nEditRecCount;/**< ��Edit�ļ�¼�� */
	int			m_nAppendRecNo;/**< ��ʼAppend�ļ�¼�� */
	int			m_nAppendRecCount;/**< ��Append�ļ�¼�� */
	lpstFieldName2Index m_lpstFieldName2Index;/**< �ֶ��������õ�����ָ�� */
	CDBFField   m_DFNull;/**< ��Ч���ֶΣ�GetField()������ */

	ENLockMode m_lmLockMode;	///������ʽ
	DWORD m_dwLockTime;
	HANDLE m_hFile;
//���������ֶ��ò���
public:
	map<int, string> mapFiled;
	map<int,map<int, string>> mapData;

};
#endif /// !defined(_HSDBF_H_)
