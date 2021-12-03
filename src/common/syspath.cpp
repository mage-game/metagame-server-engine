
#include "platform/file64.h"
#include "syspath.h"

#if defined(__unix)
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

#elif defined(_WIN32)
#include <Windows.h>
#include <direct.h>
#endif


static const char SP = '/';
static const char SSP[] = "/";

void SysPath::Down(estring* path, const char* part)
{
	path->append("/");
	path->append(part);
}

inline bool IsPathSpliter(char c)
{
	return c == '/' || c == '\\';
}

bool SysPath::Up(estring* path)
{
	for(int i = (int)path->length()-1;i>=0;--i)
	{
		if (IsPathSpliter( (*path)[i]))
		{
			path->resize(i);
			return true;
		}
	}
	return false;
}

bool SysPath::SplitFileName(const estring &file, estring *name, estring *ext)
{
	size_t pos = file.find_last_of('.');
	if (pos == file.npos)
	{
		return false;
	}

	*name = file.substr(0, pos);
	*ext = file.substr(pos + 1);
	return true;
}


#if defined(__unix)
int SysPath::GetFilesByDir(const char *foldname, estring *flist, int len)
{
	DIR *dp;
	struct dirent *dirp;
	int count=0;

	if((dp = opendir(foldname)) == NULL)
	{
		return 0;
	}

	while ((dirp = readdir(dp)) != NULL)
	{
		char currfile[1024] = {0};
		sprintf(currfile, "%s/%s", foldname, dirp->d_name);
		struct stat file_stat;
		stat(currfile, &file_stat);
		if (S_ISDIR(file_stat.st_mode))
		{
			continue;
		}
		if (count < len)
		{
			flist[count] = dirp->d_name;
		}
		++count;
	}
	closedir(dp);
	return count;
}

#elif defined(_WIN32)

int SysPath::GetFilesByDir(const char *foldname, estring *flist, int len)
{
	int count = 0;
	WIN32_FIND_DATA FindFileData;
	char DirSpec[MAX_PATH + 8] = {0};
	strncpy (DirSpec, foldname, strlen(foldname) + 1);
	strncat (DirSpec, "\\*", 3);

	HANDLE hFind = FindFirstFile(DirSpec, &FindFileData);

	if (hFind == INVALID_HANDLE_VALUE) 
	{
		return -1;
	}

	do 
	{
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			continue;
		}
		if (count < len)
		{
			flist[count] = FindFileData.cFileName;
		}
		++count;
	}while (FindNextFile(hFind, &FindFileData) != 0);

	DWORD dwError = GetLastError();
	FindClose(hFind);
	if (dwError != ERROR_NO_MORE_FILES) 
	{
		return -1;
	}

	return count;
}

#endif


bool SysPath::CreateDir(const char *dir)
{
	char now_path[1024] = {0};
	char path_copy[1024] = {0};
	strcpy(path_copy, dir);

	#if defined(__unix)
	if (path_copy[0] == '/')
	{
		strcat(now_path, "/");
	}
	#endif

	char *this_level = strtok(path_copy, "\\/");
	while (this_level != 0)
	{
		strcat(now_path, this_level);
		
		#if defined(__unix)

		if (mkdir(now_path, S_IRWXU | S_IRWXG | S_IRWXO) != 0)
		{
			struct stat file_stat;
			if ((stat(now_path, &file_stat) == 0) &&!S_ISDIR(file_stat.st_mode))
			{
				return false;
			}
		}

		#elif defined(_WIN32)

		if (mkdir(now_path) != 0)
		{
			DWORD attributes = GetFileAttributes(now_path);
			if ((attributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
			{
				return false;
			}
		}
		#endif
		
		strcat(now_path, "/");
		this_level = strtok(NULL, "\\/");
	}

	return true;
}


bool SysPath::CreateFileByPath(const char *path)
{
	estring path_str = path;
	if (SysPath::Up(&path_str))
	{
		if (!SysPath::CreateDir(path_str.c_str()))
		{
			return false;
		}
	}

	#ifdef __unix__
	int flag = DEFFILEMODE;	// 3个组可读可写，不可执行
	#endif

	#ifdef _WIN32
	int flag = S_IWRITE | S_IREAD;
	#endif

	int file = -1;
	file = open(path, O_RDWR | O_CREAT | O_TRUNC, flag);
	if (file != -1)
	{
		close(file);
	}

	return (file != -1);
}

