#pragma once

#include "nx/file.h"

class PartitionFile : public File
{
public:
	PartitionFile();
	PartitionFile(File* parent, u64 offset, u64 sz);
	virtual ~PartitionFile() override;

	virtual bool init() override;
	virtual bool close() override;

	virtual u64 read(Buffer& buffer, u64 sz = 0) override;
	virtual u64 size() override;
	virtual u64 tell() override;
	virtual bool seek(u64 offset, int whence) override;

	u64& partitionOffset() { return m_partitionOffset; }
	u64& partitionSize() { return m_partitionSize; }

protected:
	u64 m_partitionOffset = 0;
	u64 m_partitionSize = 0;
};
