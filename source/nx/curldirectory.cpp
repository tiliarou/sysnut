#include "nx/curldirectory.h"
#include <curl/curl.h>

CurlDirectory::CurlDirectory(string url) : Directory()
{
	dirPath() = url;
}

size_t write_callback(char *ptr, size_t size, size_t nmemb, DirectoryFiles* files)
{
	if (files)
	{
		auto lst = string(ptr).split('\n');
		for (auto& f : *lst)
		{
			files->push(sptr<FileEntry>(new FileEntry(f.trim(), 0)));
		}
	}
	return size;
}

DirectoryFiles& CurlDirectory::files()
{
	if (m_files.size())
	{
		return m_files;
	}

	CURL *curl = curl_easy_init();
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, dirPath().c_str());
		curl_easy_setopt(curl, CURLOPT_DIRLISTONLY, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &m_files);

		CURLcode ret = curl_easy_perform(curl);

		curl_easy_cleanup(curl);
	}

	return m_files;
}