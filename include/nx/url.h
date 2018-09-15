#pragma once
#include "nx/string.h"
#include <uriparser/Uri.h>

class Url
{
public:
	Url(const Url&) = delete;

	Url() 
	{
		isValid_ = false;
	}

	Url(string uri)	: uri_(uri)
	{
		set(uri);
	}

	Url(const char* s) : uri_(s)
	{
		set(uri_);
	}

	bool set(string uri)
	{
		UriUriA uriParse_;
		UriParserStateA state_;
		state_.uri = &uriParse_;
		isValid_ = uriParseUriA(&state_, uri_.c_str()) == URI_SUCCESS;

		if (isValid_)
		{
			scheme() = fromRange(uriParse_.scheme);
			user() = fromRange(uriParse_.userInfo);
			host() = fromRange(uriParse_.hostText);
			port() = atoi(fromRange(uriParse_.portText).c_str());
			path() = fromList(uriParse_.pathHead, "/");
			query() = fromRange(uriParse_.query);
			fragment() = fromRange(uriParse_.fragment);
		}
		uriFreeUriMembersA(&uriParse_);
		return isValid_;
	}

	~Url()
	{
	}

	operator const char*()
	{
		return uri_.c_str();
	}

	operator string&()
	{
		return uri_;
	}

	const char* c_str()
	{
		return uri_.c_str();
	}

	bool isValid() const { return isValid_; }

	string& scheme() { return m_scheme; }
	string& user() { return m_user; }
	string& password() { return m_password; }
	string& host() { return m_host; }
	int& port() { return m_port; }
	string& path() { return m_path; }
	string& query() { return m_query; }
	string& fragment() { return m_fragment; }

private:
	string uri_;

	bool        isValid_;

	string m_scheme;
	string m_user;
	string m_password;
	string m_host;
	int m_port = -1;
	string m_path;
	string m_query;
	string m_fragment;

	string fromRange(const UriTextRangeA & rng) const
	{
		if (!rng.first || !rng.afterLast)
		{
			return string();
		}
		string s(rng.first);
		s.resize(rng.afterLast - rng.first);
		s.push('\0');
		return s;
	}

	string fromList(UriPathSegmentA * xs, const string & delim) const
	{
		UriPathSegmentStructA * head(xs);
		string accum;

		while (head)
		{
			accum += delim;
			accum += fromRange(head->text);
			head = head->next;
		}

		return accum;
	}
};