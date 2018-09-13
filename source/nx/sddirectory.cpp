#include "nx/sddirectory.h"
#ifdef _MSC_VER
#include "dirent.h"
#else
#include <dirent.h>
#endif

SdDirectory::SdDirectory(string path) : Directory()
{
	dirPath() = path;
}

DirectoryFiles& SdDirectory::files()
{
	dirent *de;

	if (m_files.size())
	{
		return m_files;
	}

	DIR *dr = opendir(dirPath().c_str());

	if (dr == NULL)
	{
		return m_files;
	}

	while ((de = readdir(dr)) != NULL)
	{
		if (*de->d_name == '.')
		{
			continue;
		}
		print("file %s\n", de->d_name);
	}
	closedir(dr);
	return m_files;
}