#include "stdafx.h"
#include "FileEx.h"

#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <dirent.h> 
#include <unistd.h>
#include <ftw.h>
#include <sys/vfs.h>
#else
#include <shlobj.h>
#endif

/*******************************************************************************
* �������ƣ�	
* ����������	��ȡ�ļ��ָ�����
* ���������	
* ���������	
* �� �� ֵ��	����windows����'\\'��linux����'/'��
* ����˵����	
* �޸�����		�޸���	      �޸�����
* ------------------------------------------------------------------------------
* 2008-03-01	�ܷ�	      ����
*******************************************************************************/
char CFileEx::Separator()
{
#ifdef _WINDOWS_
	return '\\';
#else
	return '/';
#endif
}


/*******************************************************************************
* �������ƣ�	
* ����������	��ȡ��ǰĿ¼��
* ���������	
* ���������	
* �� �� ֵ��	��ǰĿ¼ȫ·����������ĩβ�ġ�\\����/����
* ����˵����	
* �޸�����		�޸���	      �޸�����
* ------------------------------------------------------------------------------
* 2008-01-05	�ܷ�	      ����
*******************************************************************************/
std::string CFileEx::GetCurDirectory()
{
	std::string strRet;
	char buff[256] = {0};
#ifdef _WINDOWS_
	GetCurrentDirectory(256, buff);
	strRet = buff;
#else
	if (getcwd(buff, 256))
	{
		strRet = buff;
	}
#endif
	return strRet;
}

/*******************************************************************************
* �������ƣ�	
* ����������	��ȡ��ִ�г�������Ŀ¼��
* ���������	
* ���������	
* �� �� ֵ��	���ؿ�ִ�г�������Ŀ¼������ֵ������ĩβ�ġ�\\����/����
* ����˵����	
* �޸�����		�޸���	      �޸�����
* ------------------------------------------------------------------------------
* 2008-01-05	�ܷ�	      ����
*******************************************************************************/
std::string CFileEx::GetExeDirectory()
{
	std::string strRet;
	char buff[256] = {0};
#ifdef _WINDOWS_
	GetModuleFileName(NULL, buff, 256);
#else
	readlink("/proc/self/exe", buff, 256); 
#endif
	strRet = buff;
	strRet = strRet.substr(0, strRet.rfind(Separator()));
	return strRet;
}

/*******************************************************************************
* �������ƣ�	
* ����������	���õ�ǰ����Ŀ¼��
* ���������	lpszFolder	-- �����õĹ���Ŀ¼��
* ���������	
* �� �� ֵ��	ִ�гɹ�����TRUE��ִ��ʧ�ܷ���FALSE��
* ����˵����	
* �޸�����		�޸���	      �޸�����
* ------------------------------------------------------------------------------
* 2008-02-26	�ܷ�	      ����
*******************************************************************************/
bool CFileEx::SetCurDirectory(const char * lpszFolder)
{
#ifdef _WIN32
	return !!SetCurrentDirectory(lpszFolder);
#else
	return (chdir(lpszFolder) == 0);
#endif
}

/*******************************************************************************
* �������ƣ�	
* ����������	����ָ���Ķ༶�ļ�Ŀ¼��
* ���������	
* ���������	
* �� �� ֵ��	��windows�����У����Ŀ¼�����ɹ���Ŀ¼�Ѵ��ڷ���true�����򷵻�false��
*				linux���������Ƿ���true��
* ����˵����	
* �޸�����		�޸���	      �޸�����
* ------------------------------------------------------------------------------
* 2008-01-05	�ܷ�	      ����
*******************************************************************************/
bool CFileEx::CreateFolder(const char * lpszFolder)
{
	if (0 == lpszFolder)
	{
		return false;
	}
	std::string strFolder = lpszFolder;
	if (strFolder.empty())
	{
		return false;
	}
	if (Separator() != strFolder[0] && std::string::npos == strFolder.find(':'))
	{
		std::string strCurDir = GetCurDirectory();
		strCurDir += Separator();
		strFolder.insert(strFolder.begin(), strCurDir.begin(), strCurDir.end());
	}
#ifdef _WINDOWS_
	int nRet = SHCreateDirectoryEx(
		NULL, 
		strFolder.c_str(),
		NULL
		);
	return ERROR_SUCCESS == nRet || ERROR_ALREADY_EXISTS == nRet;
#else
	std::string strCmd = "mkdir -p \"" + strFolder + std::string("\"");
	system(strCmd.c_str());
	return true;
#endif
}

/*******************************************************************************
* �������ƣ�	
* ����������	Ϊ����ָ�����ļ�������Ҫ���ļ�Ŀ¼��
* ���������	
* ���������	
* �� �� ֵ��	��windows�����У����Ŀ¼�����ɹ���Ŀ¼�Ѵ��ڷ���true�����򷵻�false��
*				linux���������Ƿ���true��
* ����˵����	
* �޸�����		�޸���	      �޸�����
* ------------------------------------------------------------------------------
* 2008-01-05	�ܷ�	      ����
*******************************************************************************/
bool CFileEx::CreateFolderForFile(const char * lpszFile)
{
	std::string strFolder = lpszFile;
	strFolder = strFolder.substr(0, strFolder.rfind(Separator()));
	return CreateFolder(strFolder.c_str());
}

/*******************************************************************************
* �������ƣ�	
* ����������	��ȡָ��Ŀ¼�µ������ļ���������Ŀ¼����
* ���������	
* ���������	
* �� �� ֵ��	
* ����˵����	
* �޸�����		�޸���	      �޸�����
* ------------------------------------------------------------------------------
* 2008-01-05	�ܷ�	      ����
*******************************************************************************/
void CFileEx::GetSubFiles(const char * lpszFolder, std::vector<std::string> &vecFile)
{
	std::string strFolder = lpszFolder;
	if (strFolder.empty())
	{
		return;
	}
	if (*strFolder.rbegin() == Separator())
	{
		strFolder.erase(strFolder.length(), 1);
	}
	vecFile.clear();
#ifdef _WINDOWS_
	strFolder += "\\*.*";
	CFileFind finder;
	BOOL bWorking = finder.FindFile(strFolder.c_str());
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		if (finder.IsDirectory() || finder.IsDots())
		{
			continue;
		}
		vecFile.push_back(std::string((const char *)finder.GetFilePath()));
	}
#else
	DIR *pDir = NULL;
	struct dirent *pDirent = NULL;
	char arFile[PATH_MAX] = {0};

	if(NULL == (pDir = opendir(strFolder.c_str())))
	{
		return;
	}

	while((pDirent = readdir(pDir)) != NULL)
	{
		struct stat mstat;
		memset(&mstat, 0, sizeof(mstat));
		memset(arFile, 0, sizeof(arFile));
		sprintf(arFile, "%s%c%s", strFolder.c_str(), Separator(), (char *)pDirent->d_name);

		// ��ȡ�ļ����Խṹ ���˴���Ҫʹ��lstat����ʹ��stat�������ж������ļ�����Ϣ
		if (-1 == lstat(arFile, &mstat))
		{
			perror("opendir��lstat");
			continue;
		}

		if (!S_ISDIR(mstat.st_mode))
		{
			vecFile.push_back(std::string(arFile));
		}
	}

	closedir(pDir);
#endif
}

/*******************************************************************************
* �������ƣ�	
* ����������	��ȡָ��Ŀ¼�µ�����Ŀ¼���������ļ�����
* ���������	
* ���������	
* �� �� ֵ��	
* ����˵����	
* �޸�����		�޸���	      �޸�����
* ------------------------------------------------------------------------------
* 2008-01-05	�ܷ�	      ����
*******************************************************************************/
void CFileEx::GetSubFolders(const char * lpszFolder, std::vector<std::string> &vecFolder)
{
	std::string strFolder = lpszFolder;
	if (strFolder.empty())
	{
		return;
	}
	if (*strFolder.rbegin() == Separator())
	{
		strFolder.erase(strFolder.length(), 1);
	}
	vecFolder.clear();
#ifdef _WINDOWS_
	strFolder += "\\*.*";
	CFileFind finder;
	BOOL bWorking = finder.FindFile(strFolder.c_str());
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		if (!finder.IsDirectory() || finder.IsDots())
		{
			continue;
		}
		vecFolder.push_back(std::string((const char *)finder.GetFilePath()));
	}
#else
	DIR *pDir = NULL;
	struct dirent *pDirent = NULL;
	char arFolder[PATH_MAX] = {0};

	if(NULL == (pDir = opendir(strFolder.c_str())))
	{
		return;
	}

	while((pDirent = readdir(pDir)) != NULL)
	{
		struct stat mstat;
		memset(&mstat, 0, sizeof(mstat));
		memset(arFolder, 0, sizeof(arFolder));
		sprintf(arFolder, "%s%c%s", strFolder.c_str(), Separator(), (char *)pDirent->d_name);

		// ��ȡ�ļ����Խṹ ���˴���Ҫʹ��lstat����ʹ��stat�������ж������ļ�����Ϣ
		if (-1 == lstat(arFolder, &mstat))
		{
			perror("opendir��lstat");
			continue;
		}

		if (S_ISDIR(mstat.st_mode))
		{
			if (0 == strcmp(pDirent->d_name, ".") || 0 == strcmp(pDirent->d_name, ".."))
			{
				continue;
			}
			vecFolder.push_back(std::string(arFolder));
		}
	}

	closedir(pDir);
#endif
}

/*******************************************************************************
* �������ƣ�	
* ����������	����ȫ·����ȡ�ļ�����
* ���������	
* ���������	
* �� �� ֵ��	���ػ�ȡ���ļ�����
* ����˵����	
* �޸�����		�޸���	      �޸�����
* ------------------------------------------------------------------------------
* 2008-01-05	�ܷ�	      ����
*******************************************************************************/
std::string CFileEx::Path2FileName(const char *lpszPath)
{
	std::string strRet = lpszPath;
	if (strRet.empty())
	{
		return strRet;
	}
	if (*strRet.rbegin() == Separator())
	{
		strRet.erase(strRet.length() - 1);
	}
	std::string::size_type pos = strRet.rfind(Separator());
	if (std::string::npos == pos)
	{
		return strRet;
	}
	return strRet.substr(pos + 1);
}

/*******************************************************************************
* �������ƣ�	
* ����������	��ȡ����ʣ��ռ䡣
* ���������	lpszPath -- ����Ŀ¼��
* ���������	
* �� �� ֵ��	���ش���ʣ��ռ�Ĵ�С����ȡʧ�ܷ���0����λMB��
* ����˵����	
* �޸�����		�޸���	      �޸�����
* ------------------------------------------------------------------------------
* 2007-11-17	�ܷ�	      ����
*******************************************************************************/
unsigned int CFileEx::GetFreeDiskSpace(const char * lpszPath)
{
#ifdef _WINDOWS_
	//���Ӳ�̿ռ�
	ULARGE_INTEGER ulUserFree;
	ULARGE_INTEGER ulTotal;
	if(!GetDiskFreeSpaceEx(lpszPath, &ulUserFree, &ulTotal, NULL))
	{
		//���Ӳ��ʧ�ܣ�һ������Ϊ����·��������
		return 0;
	}
	else
	{
		unsigned int nMb = 1024 * 1024;
		unsigned int nUserFree = ulUserFree.HighPart * 4096 + ulUserFree.LowPart / nMb;
		unsigned int nTotal = ulTotal.HighPart * 4096 + ulTotal.LowPart / nMb;
		return nUserFree;
	}
	return 0;
#else
	struct statfs stVfs;
	memset( &stVfs, 0, sizeof( stVfs ) );
	if( -1 == statfs(lpszPath, &stVfs ) )
	{
		return 0;
	}
	return (unsigned int)(stVfs.f_bfree / 1024 / 1024 * stVfs.f_bsize);
#endif
}

/*******************************************************************************
* �������ƣ�	
* ����������	ɾ��ָ�����ļ��С�
* ���������	
* ���������	
* �� �� ֵ��	ִ�гɹ�����TRUE��ִ��ʧ�ܷ���FALSE��
* ����˵����	ѭ��ɾ���������е��ļ����ݡ�
* �޸�����		�޸���	      �޸�����
* ------------------------------------------------------------------------------
* 2008-01-05	�ܷ�	      ����
*******************************************************************************/
bool CFileEx::DelFolder(const char * lpszFolder)
{
#ifdef _WINDOWS_
	CString strPath = CString(lpszFolder) + '\0';
	SHFILEOPSTRUCT fs;
	fs.hwnd = NULL;
	fs.wFunc = FO_DELETE;
	fs.pFrom = strPath;
	fs.pTo = NULL;
	fs.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
	fs.fAnyOperationsAborted = TRUE;
	return (0 == SHFileOperation(&fs)); 
#else
	std::string strCmd = std::string("rm -rf \"") + lpszFolder + std::string("\"");
	system(strCmd.c_str());
	return true;
#endif
}

/*******************************************************************************
* �������ƣ�	
* ����������	ɾ��ָ�����ļ���
* ���������	
* ���������	
* �� �� ֵ��	ִ�гɹ�����true��ִ��ʧ�ܷ���false��
* ����˵����	
* �޸�����		�޸���	      �޸�����
* ------------------------------------------------------------------------------
* 2008-01-07	�ܷ�	      ����
*******************************************************************************/
bool CFileEx::DelFile(const char * lpszPath)
{
#ifdef _WINDOWS_
	CFileStatus fs;
	if (!CFile::GetStatus(lpszPath, fs))
	{
		return false;
	}
	fs.m_attribute = 0;
	CFile::SetStatus(lpszPath, fs);
	return !!DeleteFile(lpszPath);
#else
	return 0 == remove(lpszPath);
#endif
}

