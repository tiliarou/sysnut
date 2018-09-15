#include "nx/sddirectory.h"
#ifdef _MSC_VER
#include "win_dirent.h"
#else
#include <dirent.h>
#endif

SdDirectory::SdDirectory(Url path) : Directory()
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
		sptr<FileEntry> fe(new FileEntry(this, de->d_name, 0));
		m_files.push(fe);
	}
	closedir(dr);
	return m_files;
}