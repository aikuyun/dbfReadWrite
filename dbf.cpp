/// 20180201, dj, �����Ƿ����ִ�Сд���ҡ�
#include "StdAfx.h"
#include "DBF.h"
#include <io.h>
#include <time.h> //ʹ�õ�ǰʱ��������  by tsl
using namespace std;
BOOL LockEx(HANDLE hFile, int lmLockMode, int iRecordNo, DBFHEAD *pHead, DWORD dwLockTime)
{
	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;
	DWORD dwCount, dwCount2;
	BOOLEAN iResult = FALSE;
	dwCount = GetTickCount();
	dwCount2 = 0;
	do
	{
		if (lmLockMode == lmFoxFile || (lmLockMode == lmFoxRecord && iRecordNo < 1))
		{
			iResult = LockFile(hFile, 0x40000000, 0, 0xC0000000, 0);
		}
		else if (lmLockMode == lmFoxRecord)
		{
			iResult = LockFile(hFile, \
				0x40000000 + pHead->DataOffset + (iRecordNo - 1) * pHead->RecSize, \
				0, pHead->RecSize, 0);
		}
		else if (lmLockMode == lmRawFile)
		{
			iResult = LockFile(hFile, 0x00000000, 0, 0xFFFFFFFF, 0);
		}
		else		///�������ɲ�����
			iResult = TRUE;

		if (iResult == TRUE)
			break;

		dwCount2 = GetTickCount();
	} while (!(((dwCount2 >= dwCount) && (dwCount2 - dwCount >= dwLockTime)) || \
		((dwCount2 < dwCount) && (MAXDWORD - dwCount + dwCount2 >= dwLockTime))));
	if (iResult == FALSE)
		return FALSE;
	return TRUE;
}

void UnlockEx(HANDLE hFile, int lmLockMode, int iRecordNo, DBFHEAD *pHead)
{
	if (hFile == INVALID_HANDLE_VALUE)
		return;
	if ((lmLockMode == lmFoxFile) || ((lmLockMode == lmFoxRecord) && (iRecordNo < 1)))
		UnlockFile(hFile, 0x40000000, 0, 0xC0000000, 0);
	else if (lmLockMode == lmFoxRecord)
		UnlockFile(hFile, \
		0x40000000 + pHead->DataOffset + (iRecordNo - 1) * pHead->RecSize, \
		0, pHead->RecSize, 0);
	else if (lmLockMode == lmRawFile)
		UnlockFile(hFile, 0x00000000, 0, 0xFFFFFFFF, 0);
}

CDBFHelper::CDBFHelper() : m_file(NULL), m_offset(1), m_fieldCnt(0)
{

}

CDBFHelper::~CDBFHelper()
{
	if (m_file)
	{
		fclose(m_file);
	}

}

int CDBFHelper::BeginCreate(const char* szFileName)
{
	if (!szFileName)
	{
		return 1;
	}

	if (m_file)
	{
		fclose(m_file);
	}

	m_offset = 1;
	m_fieldCnt = 0;

	if (NULL == (m_file = fopen(szFileName, "wb")))
	{
		return 2;
	}

	return 0;
}

int CDBFHelper::AddField(const char* szName, char cType, unsigned char nWidth, unsigned char nScale)
{
	if (!m_file)
	{
		return 1;
	}

	if (!szName)
	{
		return 2;
	}

	size_t nameLen = strlen(szName);

	if ((nameLen < 1) || (nameLen > 10))
	{
		return 3;
	}

	switch (cType)
	{
	case 'C':
		if ((nWidth < 1) || (nWidth > 254) || nScale)
		{
			return 4;
		}

		break;
	case 'D':
		if ((nWidth != 8) || nScale)
		{
			return 5;
		}

		break;
	case 'L':
		if ((nWidth != 1) || nScale)
		{
			return 6;
		}

		break;
	case 'N':
		if ((nWidth < 1) || (nWidth > 20) ||
			(nScale < 0) || (nScale > nWidth - 1))
		{
			return 7;
		}

		break;
	default:
		return 8;

		break;
	}

	memset(m_lpFields + m_fieldCnt, 0, sizeof(DBFFIELD));

	char* p = (char*)(m_lpFields + m_fieldCnt);

	for (size_t i = 0; i < nameLen; ++i)
	{
		if ((szName[i] >= 'a') && (szName[i] <= 'z'))
		{
			*p++ = szName[i] - 'a' + 'A';
		}
		else
		{
			*p++ = szName[i];
		}
	}

	m_lpFields[m_fieldCnt].Type = cType;
	m_lpFields[m_fieldCnt].Offset = m_offset;
	m_lpFields[m_fieldCnt].Width = nWidth;
	m_lpFields[m_fieldCnt].Scale = nScale;

	m_offset += nWidth;
	++m_fieldCnt;

	return 0;
}

int CDBFHelper::EndCreate()
{
	if (!m_file)
	{
		return 1;
	}

	if (0 == m_fieldCnt)
	{
		fclose(m_file);
		m_file = NULL;

		return 2;
	}

	DBFHEAD dbfHead;

	dbfHead.Mark = 0x03;

	SetDate(&dbfHead.Year);

	dbfHead.RecCount = 0;
	dbfHead.DataOffset = (unsigned short)(sizeof(DBFHEAD)+(m_fieldCnt * sizeof(DBFFIELD)) + 1);
	dbfHead.RecSize = (unsigned short)m_offset;

	memset(dbfHead.Reserved, 0, sizeof(dbfHead.Reserved));

	if (fwrite(&dbfHead, sizeof(dbfHead), 1, m_file) < 1)
	{
		fclose(m_file);
		m_file = NULL;
		return -2;
	}

	for (unsigned i = 0; i < m_fieldCnt; ++i)
	{
		if (fwrite(m_lpFields + i, sizeof(DBFFIELD), 1, m_file) < 1)
		{
			fclose(m_file);
			m_file = NULL;
			return -2;
		}
	}

	char flag = 0x0d;

	if (fwrite(&flag, 1, 1, m_file) < 1)
	{
		fclose(m_file);
		m_file = NULL;
		return -2;
	}

	flag = 0x1A;

	if (fwrite(&flag, 1, 1, m_file) < 1)
	{
		fclose(m_file);
		m_file = NULL;
		return -2;
	}

	fclose(m_file);
	m_file = NULL;

	return 0;
}

int CDBFHelper::Zap(const char* szFileName)
{
	if (!szFileName)
	{
		return -1;
	}

	FILE* file;

	if (NULL == (file = fopen(szFileName, "rb")))
	{
		return -2;
	}

	HANDLE hFile = (HANDLE)_get_osfhandle(_fileno(file));

	if (!LockEx(hFile, lmFoxFile, 0, NULL, 4000))
		return -10;
	fseek(file, 0, SEEK_END);

	long fileSize = ftell(file);

	// У��DBF�ļ����ٳ���
	if (fileSize < 65)
	{
		fclose(file);
		UnlockEx(hFile, lmFoxFile, 0, NULL);
		return -3;
	}

	fseek(file, 0, SEEK_SET);

	char mark;
	if (fread(&mark, 1, 1, file) < 1)
	{
		fclose(file);
		UnlockEx(hFile, lmFoxFile, 0, NULL);
		return -4;
	}

	// У��DBF�ļ��汾
	if ((mark != 0x03) && (mark != 0x30))
	{
		fclose(file);
		UnlockEx(hFile, lmFoxFile, 0, NULL);
		return -4;
	}

	fseek(file, 8, SEEK_SET);

	unsigned short headSize;

	if (fread(&headSize, 2, 1, file) < 1)
	{
		fclose(file);
		UnlockEx(hFile, lmFoxFile, 0, NULL);
		return 1;
	}

	// У��DBF�ļ�ͷ���ȺϷ�
	if ((headSize < 65) || (headSize > fileSize))
	{
		fclose(file);
		UnlockEx(hFile, lmFoxFile, 0, NULL);
		return -5;
	}

	char* buffer = new char[headSize + 1];

	if (!buffer)
	{
		fclose(file);
		UnlockEx(hFile, lmFoxFile, 0, NULL);
		return -6;
	}

	fseek(file, 0, SEEK_SET);

	if (fread(buffer, headSize, 1, file) < 1)
	{
		fclose(file);
		delete[] buffer;
		UnlockEx(hFile, lmFoxFile, 0, NULL);
		return -7;
	}

	//modified by dj start 20090122
	fseek(file, -1, SEEK_END);
	char flag;
	int iLen = 0;
	flag = fgetc(file);	//ȡ���һ��������
	if (flag == 0x1a)
		iLen = 1;
	//modified by dj end;

	fclose(file);

	// У��DBF�ļ��Ƿ�Ϸ����ļ�ͷ����+��¼����*��¼��С+iLen�Ƿ�����ļ�����
	///��û�н�������ʱ��iLen = 0,�е�ʱ��Ϊ1
	DBFHEAD* lpHead = (DBFHEAD*)buffer;

	if (lpHead->DataOffset + lpHead->RecSize * lpHead->RecCount + iLen != fileSize)
	{
		delete[] buffer;
		UnlockEx(hFile, lmFoxFile, 0, NULL);
		return 8;
	}

	if (NULL == (file = fopen(szFileName, "wb")))
	{
		delete[] buffer;
		UnlockEx(hFile, lmFoxFile, 0, NULL);
		return -8;
	}

	SetDate((unsigned char*)(buffer + 1));

	// ��¼������0
	((DBFHEAD*)buffer)->RecCount = 0;
	buffer[headSize] = 0x1A;

	if (fwrite(buffer, headSize + 1, 1, file) < 1)
	{
		fclose(file);

		delete[] buffer;
		UnlockEx(hFile, lmFoxFile, 0, NULL);
		return -2;
	}

	delete[] buffer;

	fclose(file);
	UnlockEx(hFile, lmFoxFile, 0, NULL);

	return 0;
}

void CDBFHelper::SetDate(unsigned char* date)
{
	time_t now;
	time(&now);
	tm* tp = localtime(&now);

	date[0] = tp->tm_year % 100;
	date[1] = tp->tm_mon + 1;
	date[2] = tp->tm_mday;
}

bool CDBFHelper::UpdateDBFDate(FILE* lpFile)
{
	unsigned char date[3];

	HANDLE hFile = (HANDLE)_get_osfhandle(_fileno(lpFile));
	if (!LockEx(hFile, lmFoxFile, 0, NULL, 4000))
		return false;
	SetDate(date);

	fseek(lpFile, 1, SEEK_SET);

	if (fwrite(date, 3, 1, lpFile) < 1)
	{
		UnlockEx(hFile, lmFoxFile, 0, NULL);
		return false;
	}
	UnlockEx(hFile, lmFoxFile, 0, NULL);
	fflush(lpFile);
	return true;
}

int CDBFField::SetValue(int nValue)
{
	if ('C' == m_FieldInfo.Type)
	{
		*(m_lpBuf + m_FieldInfo.Offset) = nValue & 0xFF;
		m_szDataBuf[0] = nValue & 0xFF;
		m_szDataBuf[1] = '\0';

		return 0;
	}
	else if ('L' == m_FieldInfo.Type)
	{
		char t = 'F';

		if (nValue)
		{
			t = 'T';
		}

		*(m_lpBuf + m_FieldInfo.Offset) = t;
		m_szDataBuf[0] = t;
		m_szDataBuf[1] = '\0';

		return 0;
	}

	if (m_FieldInfo.Scale)
	{
		return SetValue((double)nValue);
	}

	// ��ʱ����
	char szVal[sizeof m_szDataBuf];

	int nLength;

	if (-1 == (nLength = sprintf(szVal, "%d", nValue)))
	{
		return -1;
	}

	if (nLength > m_FieldInfo.Width)
	{
		return -2;
	}


	memcpy(m_lpBuf + m_FieldInfo.Offset + m_FieldInfo.Width - nLength, szVal, nLength);
	memset(m_lpBuf + m_FieldInfo.Offset, ' ', m_FieldInfo.Width - nLength);
	memcpy(m_szDataBuf, szVal, nLength + 1);

	return 0;
}

bool CDBFField::SetValue(double lfValue)
{
	if ('L' == m_FieldInfo.Type)
	{
		char t = 'T';

		if ((lfValue < 1E-22) && (lfValue > -1E-22))
		{
			t = 'F';
		}

		*(m_lpBuf + m_FieldInfo.Offset) = t;
		m_szDataBuf[0] = t;
		m_szDataBuf[1] = '\0';

		return true;
	}

	// ��ʱ����
	char szVal[sizeof m_szDataBuf];

	int nLength;

	// ����ֵ̫С������0������
	if ((lfValue < 1E-22) && (lfValue > -1E-22))
	{
		szVal[0] = '0';

		if (m_FieldInfo.Scale)
		{
			szVal[1] = '.';

			char *p = szVal + 2;

			for (unsigned char nScale = m_FieldInfo.Scale; nScale--;)
			{
				*p++ = '0';
			}

			*p = '\0';

			nLength = 2 + m_FieldInfo.Scale;
		}
		else
		{
			szVal[1] = '\0';
			nLength = 1;
		}
	}
	else
	{
		nLength = sprintf(szVal, "%.*f", m_FieldInfo.Scale, lfValue);

		if (nLength > m_FieldInfo.Width)
		{
			return false;
		}
	}

	memcpy(m_lpBuf + m_FieldInfo.Offset + m_FieldInfo.Width - nLength, szVal, nLength);
	memset(m_lpBuf + m_FieldInfo.Offset, ' ', m_FieldInfo.Width - nLength);
	memcpy(m_szDataBuf, szVal, nLength + 1);

	return true;
}

int CDBFField::SetValue(const char* szValue)
{
	if (!szValue)
	{
		return -1;
	}

	if ('L' == m_FieldInfo.Type &&
		'T' != *szValue && 'F' != *szValue)
	{
		return -2;
	}

	if ('N' == m_FieldInfo.Type)
	{
		if (m_FieldInfo.Scale)
		{
			// ���Ϊ����ƿ����
			return SetValue(atof(szValue));
		}
		else
		{
			return SetValue(atoi(szValue));
		}
	}

	int nLength = (int)strlen(szValue);

	if (nLength > m_FieldInfo.Width)
	{
		return -3;
	}

	memcpy(m_lpBuf + m_FieldInfo.Offset, szValue, nLength);
	memset(m_lpBuf + m_FieldInfo.Offset + nLength, ' ', m_FieldInfo.Width - nLength);
	memcpy(m_szDataBuf, szValue, nLength + 1);

	return 0;
}

CHSDBF::CHSDBF() :m_lpFile(NULL), m_nFields(0), m_nBatchSize(5), m_lpRecordBuf(NULL)
, m_nBufSize(0), m_nRecordNo(0), m_NoInBatch(0), m_nBufRecNo0(0)
, m_nBufRecNo1(0), m_lpstFieldName2Index(0), m_bEOF(false), m_bBOF(false)
, m_nEditRecCount(0), m_nAppendRecCount(0)
{
	memset(&m_head, 0, sizeof(m_head));
	m_szFileName[0] = '\0';
	m_nAccessFlags = modeBrowse;
	m_DFNull.m_FieldInfo.Width = 0;/// ������Ч�ֶ���Ϣ��������ֵ��ʱ�򲻻����
	m_DFNull.m_lpBuf = m_DFNull.m_szDataBuf;
	m_dwLockTime = 4000;	///�a�ϵȴ���ʱ�䡣
	m_hFile = INVALID_HANDLE_VALUE;
	m_lmLockMode = lmFoxFile;
}

CHSDBF::~CHSDBF()
{
	CloseDbf();
	free(m_lpRecordBuf);
	m_lpRecordBuf = NULL;
	delete[] m_lpstFieldName2Index;
	m_lpstFieldName2Index = NULL;
}

/**
* �ֶ�������Ƚ�����
*/
bool NameCmp(stFieldName2Index& elem1, stFieldName2Index& elem2)
{
	return strcmp(elem1.lpName, elem2.lpName) < 0;
}

int CHSDBF::OpenDbf(const char *szFileName, unsigned int nOpenFlags, int iBatchSize)
{
	CloseDbf();

	char szAccess[4] = { 'r', 'b', '\0', '\0' };

	if (nOpenFlags)
	{
		szAccess[2] = '+';
	}

	if (NULL == (m_lpFile = fopen(szFileName, const_cast<const char *>(szAccess))))
	{
		printf("open file: %s, access mode: %s, fail, error code: %d, %s\n", \
			szFileName, szAccess, errno, strerror(errno));
		return -1;
	}

	m_hFile = (HANDLE)_get_osfhandle(_fileno(m_lpFile));

	if (sizeof(m_head) != fread(&m_head, 1, sizeof(m_head), m_lpFile))
	{
		return -2;
	}

	if (m_head.Mark != 0x03 && m_head.Mark != 0x30)///�ж��Ƿ�Ϸ���DBFͷ
	{
		return -3;
	}

	//modified by dj start 20090529
	//modified by dj start 20090122
	fseek(m_lpFile, -1, SEEK_END);

	char flag;
	flag = fgetc(m_lpFile);

	int iFileSize = ftell(m_lpFile);

	if (flag != 0x1a)//�н�����
		iFileSize++;

	//modified by dj end;
	// deleted by hezhen 20121130 20121127028 �����еĳ���дdbf�ⲻ����ͷ�е����������ͷ��������ʵ����������
	//     if (long(m_head.DataOffset + m_head.RecCount * m_head.RecSize + 1) != iFileSize)
	//     {
	//         return -4;
	//     }
	//modified by dj end;

	// modified by hezhen 20121130 20121127028 ͨ������ʵ�������ֽ��Ƿ���ȱʧ�ж�dbf�Ƿ�����
	if ((iFileSize - 1 - m_head.DataOffset) % m_head.RecSize != 0)
	{
		return -4;
	}
	////////////////////////////////////////////////////////////////////////

	unsigned short i = 0;
	fseek(m_lpFile, sizeof(m_head), SEEK_SET);
	for (i = 0; i < (m_head.DataOffset - sizeof(DBFHEAD)) / sizeof(DBFFIELD); i++)
	{
		if (sizeof(DBFFIELD) != fread(&m_lpFields[i].m_FieldInfo, 1, sizeof(DBFFIELD), m_lpFile))
		{
			return -5;
		}

		if (m_lpFields[i].m_FieldInfo.Name[0] == 0x0D) // FoxPro�հ�,����
		{
			break;
		}

		m_lpFields[i].m_nNo = i; // �����к�
		m_lpFields[i].m_lpBuf = NULL;
		m_lpFields[i].m_szDataBuf[0] = '\0';
		//������  �޸�ƫ���������⣺2018-02-01
		m_lpFields[0].m_FieldInfo.Offset = 1;
		///add by dj start 20090529
		///���¼��������
		if (i != 0)
		{
			m_lpFields[i].m_FieldInfo.Offset = m_lpFields[i - 1].m_FieldInfo.Offset + m_lpFields[i - 1].m_FieldInfo.Width;
		}
		///add by dj end;
	}

	m_nFields = i;  // �����ֶ���

	if (NULL == (m_lpstFieldName2Index = new stFieldName2Index[m_nFields]))
	{
		return -6;
	}

	for (i = 0; i < m_nFields; ++i)
	{
		m_lpstFieldName2Index[i].lpName = m_lpFields[i].GetName();
		m_lpstFieldName2Index[i].iIndex = m_lpFields[i].GetNo();
	}

	std::sort(m_lpstFieldName2Index, m_lpstFieldName2Index + m_nFields, NameCmp);
	
	//����ÿ�δ���ļ�¼����
	SetBatchSize(iBatchSize);
	//RecSize:һ����¼���ĳ���
	int nSize = m_head.RecSize * m_nBatchSize;
	//m_nBufSize:�����С 
	if (m_nBufSize < nSize)
	{
		//ָ����=����������*��realloc��Ҫ�ı��ڴ��С��ָ�������µĴ�С����
		void *ptr = realloc(m_lpRecordBuf, nSize);
		if (NULL != ptr)
		{
			m_nBufSize = nSize;
			m_lpRecordBuf = (char *)ptr;
		}
		else
		{
			return -7;
		}
	}
	//
	ReadRecord(1);

	MoveFirst();

	return 0;
}

void CHSDBF::CloseDbf()
{
	if (m_lpFile)
	{
		fclose(m_lpFile);
		m_lpFile = NULL;
	}
	m_hFile = INVALID_HANDLE_VALUE;
}

void CHSDBF::FreshDbf()
{
	DBFHEAD head;

	fseek(m_lpFile, 0, SEEK_SET);

	size_t nBytesRead = fread(&head, 1, sizeof(head), m_lpFile);

	if (nBytesRead != sizeof(head) || head.RecCount == m_head.RecCount)
	{
		return;
	}

	long nOldRecCount = m_head.RecCount;
	m_head.RecCount = head.RecCount;

	if (head.RecCount > nOldRecCount)
	{
		if (nOldRecCount == 0)/// ������¼�¼
		{
			MoveFirst();
		}
		else
		{
			m_bEOF = false;	///����¼���ӵ�ʱ�򣬽�����Ϊfalse	dj����	20090116
		}
	}
	else if (head.RecCount < nOldRecCount)
	{
		if (m_nRecordNo + 1 > head.RecCount) /// ��ǰ�ļ�¼�Ѿ�ɾ��
		{
			MoveLast();
		}
	}
}

void CHSDBF::Move(int nRecCount)
{
	if (nRecCount == 0)
	{
		ReadRecord(m_nRecordNo);
	}
	else if (nRecCount > 0)
	{
		if (m_nRecordNo + nRecCount < m_head.RecCount - 1)
		{
			m_nRecordNo += nRecCount;

			if (m_nRecordNo < m_nBufRecNo0 || m_nRecordNo >= m_nBufRecNo1)
			{
				ReadRecord(m_nRecordNo);
			}
			else
			{
				m_NoInBatch += nRecCount;
				Rec2Buf(m_head.RecSize * m_NoInBatch);
			}
			m_bBOF = false;
			m_bEOF = false;
		}
		else
		{
			MoveLast();
		}
	}
	else if (nRecCount < 0)
	{
		if (m_nRecordNo + nRecCount > 0)
		{
			m_nRecordNo += nRecCount;

			if (m_nRecordNo < m_nBufRecNo0 || m_nRecordNo >= m_nBufRecNo1)
			{
				ReadRecord(m_nRecordNo);
			}
			else
			{
				m_NoInBatch += nRecCount;
				Rec2Buf(m_head.RecSize * m_NoInBatch);
			}
			m_bBOF = false;
			m_bEOF = false;
		}
		else
		{
			MoveFirst();
		}
	}
}

void CHSDBF::Go(int nRecordNo)
{
	Move(nRecordNo - m_nRecordNo);
}

void CHSDBF::MoveFirst()
{
	if (m_head.RecCount > 0)
	{
		if (m_nBufRecNo1 < 1 || m_nBufRecNo0 > 0)
		{
			ReadRecord(0);
		}
		else
		{
			m_NoInBatch = 0;
			Rec2Buf(m_head.RecSize * m_NoInBatch);
		}

		m_nRecordNo = 0;
		m_bEOF = false;
		m_bBOF = false;
	}
	else
	{
		m_bBOF = true;
		m_bEOF = true;
	}
}

//modified by dj start 20090204
void CHSDBF::MoveLast()
{
	if (m_head.RecCount > 0)
	{
		if (m_nBufRecNo1 < m_head.RecCount)
		{
			//ReadRecord(m_head.RecCount - 1 - m_nBatchSize); // !m_nBatchSize̫��?
			ReadRecord(m_head.RecCount - m_nBatchSize);	///m_nBatchSize = 1, m_head.RecCount = 2,���Ϊ0�ǲ��Եġ�
		}
		else
		{
			m_NoInBatch = m_nBatchSize - 1;
			Rec2Buf(m_head.RecSize * m_NoInBatch);
		}

		m_nRecordNo = m_head.RecCount - 1;
		m_bBOF = false;
		m_bEOF = false;
	}
	else
	{
		m_bEOF = true;
		m_bBOF = true;
	}
}
//modified by dj end;

void CHSDBF::MovePrev()
{
	if (m_nRecordNo > 0)
	{
		if (m_nBufRecNo1 < m_nRecordNo || m_nBufRecNo0 >= m_nRecordNo) // ���ڻ�����
		{
			int nRecordNo = m_nRecordNo - m_nBatchSize;
			if (nRecordNo < 0)
			{
				nRecordNo = 0;
			}

			ReadRecord(nRecordNo, false);
		}
		else
		{
			m_NoInBatch--;
			Rec2Buf(m_head.RecSize * m_NoInBatch);
		}
		m_nRecordNo--;
		m_bBOF = false;
		m_bEOF = false;
	}
	else
	{
		m_bBOF = true;
	}
}

void CHSDBF::MoveNext()
{
	if (m_nRecordNo + 1 < m_head.RecCount)
	{
		if (m_nBufRecNo1 <= m_nRecordNo + 1)
		{
			ReadRecord(m_nRecordNo + 1);
		}
		else
		{
			m_NoInBatch++;
			Rec2Buf(m_head.RecSize * m_NoInBatch);
		}
		m_nRecordNo++;
		m_bBOF = false;
		m_bEOF = false;
	}
	else
	{
		m_bEOF = true;
	}
}

bool CHSDBF::Append(int nAppendRecCount)
{
	if (NULL == m_lpFile || nAppendRecCount > m_nBatchSize || nAppendRecCount < 1)
	{
		return false;
	}

	m_nBufRecNo0 = m_head.RecCount;
	m_nBufRecNo1 = m_nBufRecNo0 + nAppendRecCount;
	m_NoInBatch = 0;
	m_nAccessFlags = modeAppend;

	memset(m_lpRecordBuf, ' ', m_nBufSize);

	for (int nIndex = 0; nIndex < m_nFields; nIndex++)
	{
		m_lpFields[nIndex].m_lpBuf = m_lpRecordBuf;
	}

	return true;
}

bool CHSDBF::AppendNext()
{
	if (m_nAccessFlags != modeAppend || m_NoInBatch + 1 >= m_nBatchSize)
	{
		return false;
	}

	m_NoInBatch++;

	for (int nIndex = 0; nIndex < m_nFields; nIndex++)
	{
		m_lpFields[nIndex].m_lpBuf = m_lpRecordBuf + m_head.RecSize * m_NoInBatch;
	}

	return true;
}

bool CHSDBF::Edit(int nEditRecCount)
{
	if (NULL == m_lpFile || nEditRecCount > m_nBatchSize || nEditRecCount < 1)
	{
		return false;
	}

	m_nEditRecNo = m_nRecordNo;
	m_nEditRecCount = nEditRecCount;

	if (m_nEditRecNo + nEditRecCount > m_head.RecCount)// ���nEditRecCount̫���޸�Ҫ�༭������
	{
		m_nEditRecCount = m_head.RecCount - m_nEditRecNo;
	}

	//�����;read�����ܻᶪʧ�༭�����Ա༭ǰ��ȫ��������
	if (m_nEditRecNo < m_nBufRecNo0 || m_nEditRecNo + m_nEditRecCount > m_nBufRecNo1)
	{
		ReadRecord(m_nEditRecNo);
	}
	else
	{
		Rec2Buf(m_head.RecSize * m_NoInBatch);
	}

	m_nAccessFlags = modeEdit;

	return true;
}

int CHSDBF::EditNext()
{
	if (m_nAccessFlags != modeEdit || m_nRecordNo >= m_nEditRecNo + m_nEditRecCount - 1)
	{
		return -1;
	}

	MoveNext();

	if (m_bEOF)
	{
		return -2;
	}

	return 0;
}
int CHSDBF::Post()
{
	if (m_nAccessFlags == modeAppend)
	{
		if (Lock(0))
		{
			///���ļ�ͷ
			fseek(m_lpFile, 0, SEEK_SET);
			fread(&m_head, 1, sizeof(m_head), m_lpFile);

			fseek(m_lpFile, m_head.DataOffset + m_head.RecCount * m_head.RecSize, SEEK_SET);
			if (size_t(m_NoInBatch + 1) > fwrite(m_lpRecordBuf, m_head.RecSize, m_NoInBatch + 1, m_lpFile))
			{
				Unlock(0);
				return -1;
			}

			unsigned char flag = 0x1A;
			if (1 > fwrite(&flag, 1, 1, m_lpFile))
			{
				Unlock(0);
				return -2;
			}

			m_head.RecCount += m_NoInBatch + 1;
			fseek(m_lpFile, OFFSET_OF_RECCOUNT, SEEK_SET);
			if (1 > fwrite(&(m_head.RecCount), sizeof(m_head.RecCount), 1, m_lpFile))///�޸��ļ�ͷ�еļ�¼��
			{
				Unlock(0);
				return -3;
			}

			m_nAppendRecCount = 0;
			fflush(m_lpFile);
			Unlock(0);
		}
		else
		{
			return -6;	///����ʧ��
		}
	}
	else if (m_nAccessFlags == modeEdit)
	{
		if (Lock(m_nEditRecNo + 1))
		{

			fseek(m_lpFile, m_head.DataOffset + m_nEditRecNo * m_head.RecSize, SEEK_SET);
			if (size_t(m_nRecordNo - m_nEditRecNo + 1) > fwrite(m_lpRecordBuf + m_head.RecSize * (m_nEditRecNo - m_nBufRecNo0)
				, m_head.RecSize, (m_nRecordNo - m_nEditRecNo + 1), m_lpFile))
			{
				Unlock(m_nEditRecNo + 1);
				return -4;
			}
			m_nEditRecCount = 0;
			fflush(m_lpFile);
			Unlock(m_nEditRecNo + 1);
		}
		else
		{
			return -7;	///����ʧ�ܡ�
		}
	}

	if (!CDBFHelper::UpdateDBFDate(m_lpFile))
	{
		return -5;
	}



	m_nAccessFlags = modeBrowse;

	return 0;
}

int CHSDBF::ReadRecord(int nRecordNo, bool bForward)
{
	if (nRecordNo < 0 || nRecordNo >= m_head.RecCount)
	{
		return -1;
	}

	if (Lock(nRecordNo + 1))
	{
		fseek(m_lpFile, m_head.DataOffset + nRecordNo * m_head.RecSize, SEEK_SET);

		size_t nBytesRead = fread(m_lpRecordBuf, 1, m_head.RecSize * m_nBatchSize, m_lpFile);

		if (m_head.RecSize * m_nBatchSize != nBytesRead && !feof(m_lpFile))
		{
			Unlock(nRecordNo + 1);
			return -2;
		}

		m_nBufRecNo0 = nRecordNo;
		m_nBufRecNo1 = m_nBufRecNo0 + (int)nBytesRead / m_head.RecSize;
		Unlock(nRecordNo + 1);
	}

	if (bForward)
	{
		m_NoInBatch = 0;
		Rec2Buf(0);
	}
	else
	{
		m_NoInBatch = m_nBufRecNo1 - m_nBufRecNo0 - 1;
		Rec2Buf(m_head.RecSize * m_NoInBatch);
	}

	return 0;
}

//modified by dj start 20090220
/// �����Ƿ����ִ�Сд���ҡ�
bool CHSDBF::BinarySearch(lpstFieldName2Index a, size_t length, const char* key, size_t *index, int iType)
{
	if (0 == length)
	{
		return false;
	}

	//modified by dj start 20090321
	/*
	size_t low = 0;
	size_t high = length - 1;
	size_t mid = 0;
	*/
	int low = 0;
	int high = length - 1;
	int mid = 0;
	//modified by dj end;
	int iRet = 0;

	while (low <= high)
	{
		mid = low + (high - low) / 2;

		if (iType == 0)
			iRet = _stricmp(a[mid].lpName, key);
		else
			iRet = strcmp(a[mid].lpName, key);

		if (iRet < 0)
		{
			low = mid + 1;
		}
		else if (iRet > 0)
		{
			high = mid - 1;
		}
		else
		{
			if (index)
			{
				*index = mid;
			}

			return true;
		}
	}

	return false;
}
//modified by dj end;


BOOL CHSDBF::Lock(int iRecordNo)
{
	
	return LockEx(m_hFile, m_lmLockMode, iRecordNo, &m_head, m_dwLockTime);
}

void CHSDBF::Unlock(int iRecordNo)
{
	
	UnlockEx(m_hFile, m_lmLockMode, iRecordNo, &m_head);
}


//��ȡ��������
void CHSDBF::ReadFields(){

	for (int i = 0; i < m_nFields; i++){
		mapFiled[i] = m_lpFields[i].m_FieldInfo.Name;
	}
}

void CHSDBF::init(){
	//��ʼ����
	int i = 1;
	int count = GetRecordCount();
	while (i <= count){
		//�༭һ��
		Edit(1);
		//��ȡ
		CDBFField cdbfiled;
		cdbfiled = GetField("HQZJCJ");
		double price  = cdbfiled.AsFloat();
		//�����������Ϊʱ��
		srand((unsigned)time(NULL));//��ʼ�������
		//���ģ����
		int num = rand() % 2;
		//����
		if (num == 1)
		{
			price += 0.01 * (rand() % 100);
		}
		//����
		else if (num == 2)
		{
			price -= 0.01 * (rand() % 100);
		}
		//������д��
		cdbfiled.SetValue(price);
		//�ύ����price
		Post();
		i++;
		//��ʽ���
		std::printf("�Ѿ������˵�%d����,�޸�Ϊ%f\n", i - 1, price);
		//��һ��
		MoveNext();
	}
		
	
}


