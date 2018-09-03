#include "nx/fs.hpp"

#include <cstring>
#include "nx/ipc/tin_ipc.h"
#include "error.hpp"

namespace nx::fs
{
    // IFile
	IFile::IFile()
	{
	}

    IFile::IFile(FsFile& file)
    {
        m_file = file;
    }

    IFile::~IFile()
    {
        fsFileClose(&m_file);
    }

    bool IFile::Read(u64 offset, void* buf, size_t size)
    {
        u64 sizeRead;
        ASSERT_OK(fsFileRead(&m_file, offset, buf, size, &sizeRead), "Failed to read file");
        
        if (sizeRead != size)
        {
            std::string msg = "Size read " + std::string("" + sizeRead) + " doesn't match expected size " + std::string("" + size);
			print("%s\n", msg.c_str());
			return false;
        }

		return true;
    }

    u64 IFile::GetSize()
    {
        u64 sizeOut;

		if (R_FAILED(fsFileGetSize(&m_file, &sizeOut)))
		{
			print("Failed to get file size\n");
			return 0;
		}

        return sizeOut;
    }

    // End IFile

    // IDirectory
	IDirectory::IDirectory()
	{
	}

    IDirectory::IDirectory(FsDir& dir) 
    {
        m_dir = dir;
    }

    IDirectory::~IDirectory()
    {
        fsDirClose(&m_dir);
    }

    bool IDirectory::Read(u64 inval, FsDirectoryEntry* buf, size_t numEntries)
    {
        size_t entriesRead;

		if (R_FAILED(fsDirRead(&m_dir, inval, &entriesRead, numEntries, buf)))
		{
			print("Failed to read directory\n");
			return false;
		}

        if (entriesRead != numEntries)
        {
            std::string msg = "Entries read " + std::string("" + entriesRead) + " doesn't match expected number " + std::string("" + numEntries);
            print(msg.c_str());
			return false;
        }
		return true;
    }

    u64 IDirectory::GetEntryCount()
    {
        u64 entryCount = 0;

		if (R_FAILED(fsDirGetEntryCount(&m_dir, &entryCount)))
		{
			print("Failed to get entry count\n");
			return 0;
		}

        return entryCount;
    }

    // End IDirectory

    IFileSystem::IFileSystem() {}

    IFileSystem::~IFileSystem()
    {
        this->CloseFileSystem();
    }

    Result IFileSystem::OpenSdFileSystem()
    {
		if (R_FAILED(fsMountSdcard(&m_fileSystem)))
		{
			print("Failed to mount sd card\n");
			return -1;
		}
        return 0;
    }

    Result IFileSystem::OpenFileSystemWithId(std::string path, FsFileSystemType fileSystemType, u64 titleId)
    {
        Result rc = 0;

        if (path.length() >= FS_MAX_PATH)
		{
			print("Directory path is too long!\n");
            return -1;
		}

        // libnx expects a FS_MAX_PATH-sized buffer
        path.reserve(FS_MAX_PATH);

        std::string errorMsg = "Failed to open file system with id";
        rc = fsOpenFileSystemWithId(&m_fileSystem, titleId, fileSystemType, path.c_str());

        if (rc == 0x236e02)
            errorMsg = "File " + path + " is corrupt! You may have a bad dump, or fs_mitm may need to be removed.";

		if (R_FAILED(rc))
		{
			print(errorMsg.c_str());
			return -1;
		}
        return 0;  
    }

    void IFileSystem::CloseFileSystem()
    {
        fsFsClose(&m_fileSystem);
    }

    IFile IFileSystem::OpenFile(std::string path)
    {
        if (path.length() >= FS_MAX_PATH)
		{
            print("Directory path is too long!\n");
            //return;
		}

        // libnx expects a FS_MAX_PATH-sized buffer
        path.reserve(FS_MAX_PATH);

        FsFile file;

		if (R_FAILED(fsFsOpenFile(&m_fileSystem, path.c_str(), FS_OPEN_READ, &file)))
		{
			print(("Failed to open file " + path).c_str());
			return IFile();
		}

        return IFile(file);
    }

    IDirectory IFileSystem::OpenDirectory(std::string path, int flags)
    {
        // Account for null at the end of c strings
        if (path.length() >= FS_MAX_PATH)
		{
            print("Directory path is too long!\n");
            //return IDirectory();
		}

        // libnx expects a FS_MAX_PATH-sized buffer
        path.reserve(FS_MAX_PATH);

        FsDir dir;
		if (R_FAILED(fsFsOpenDirectory(&m_fileSystem, path.c_str(), flags, &dir)))
		{
			print(("Failed to open directory " + path).c_str());
			return IDirectory();
		}
        return IDirectory(dir);
    }
}        