#pragma once

#include "nx/file.h"

class CurlFile : public File
{
public:
	CurlFile();
	virtual ~CurlFile();
	virtual bool open(string& path, const char* mode = "rb") override;
	virtual bool init();
	virtual bool close();
	bool seek(u64 offset, int whence = 0);
	bool rewind();
	u64 tell();
	u64 size();
	virtual u64 read(Buffer<u8>& buffer, u64 sz = 0) override;

	virtual void onChildless() override;

	bool isOpen();
	//string& path() { return m_path; }

protected:
	u64& currentPosition() { return m_position; }

	u64 m_size = 0;
	u64 m_position = 0;
};