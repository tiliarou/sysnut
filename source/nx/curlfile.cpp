#include "nut.h"
#include "log.h"
#include "nx/curlfile.h"
#include <curl/curl.h>

CurlFile::CurlFile()
{
}

CurlFile::~CurlFile()
{
	close();
}

bool CurlFile::open(string& path, const char* mode)
{
	if (isOpen())
	{
		warning("opening file with closing handle first %s\n", path);
		close();
	}

	this->path() = path;

	size(); // just cache the file size

	return true;
}

bool CurlFile::init()
{
	return true;
}

bool CurlFile::close()
{
	return true;
}

bool CurlFile::seek(u64 offset, int whence)
{
	if (!isOpen())
	{
		error("tried to seek on closed file\n");
		return false;
	}

	switch (whence)
	{
	case SEEK_SET:
		currentPosition() = offset;
		break;
	case SEEK_CUR:
		currentPosition() += offset;
		break;
	case SEEK_END:
		currentPosition() = size() - offset;
		break;
	default:
		return false;
	}
	return false;
}

bool CurlFile::rewind()
{
	return seek(0);
}

u64 CurlFile::tell()
{
	if (!isOpen())
	{
		error("tried to tell on closed file\n");
		return false;
	}

	return currentPosition();
}

size_t curl_size_callback(void *ptr, size_t size, size_t nmemb, void *data)
{
	(void)ptr;
	(void)data;

	return size * nmemb;
}

u64 CurlFile::size()
{
	if (m_size)
	{
		return m_size;
	}

	CURL *curl = curl_easy_init();
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, path().c_str());
		curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, curl_size_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_size_callback);
		curl_easy_setopt(curl, CURLOPT_HEADER, 0L);

		CURLcode res = curl_easy_perform(curl);

		if (res == CURLE_OK)
		{
			double fileSize = 0.0;
			res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &fileSize);
			m_size = (u64)fileSize;
		}

		curl_easy_cleanup(curl);
	}

	return m_size;
}

size_t curlReadCallback(void *ptr, size_t size, size_t nmemb, Buffer<u8>* buffer)
{
	u64 currentSize = buffer->size();
	size *= nmemb;

	buffer->resize(currentSize + size);
	memcpy(buffer->buffer() + currentSize, ptr, size);
	return size;
}

u64 CurlFile::read(Buffer<u8>& buffer, u64 sz)
{
	if (!isOpen())
	{
		error("tried to read from closed file\n");
		return 0;
	}

	if (!sz)
	{
		sz = size();
	}

	buffer.resize(sz);
	buffer.resize(0);

	CURL *curl = curl_easy_init();
	if (curl)
	{
		char tmp[128];
		sprintf(tmp, "%d-%d", (size_t)currentPosition(), (size_t)(currentPosition() + sz - 1));

		curl_easy_setopt(curl, CURLOPT_URL, path().c_str());
		curl_easy_setopt(curl, CURLOPT_RANGE, tmp);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlReadCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

		curl_easy_perform(curl);
	}

	return 0;
}

bool CurlFile::isOpen()
{
	return path().isValid();
}

void CurlFile::onChildless()
{
	close();
}

