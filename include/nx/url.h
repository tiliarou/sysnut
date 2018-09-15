#pragma once
#include "nx/string.h"
#include <uriparser/Uri.h>

class Url
{
public:
	Url(const Url& s)
	{
		isValid() = s.isValid();

		scheme() = s.scheme();
		user() = s.user();
		password() = s.password();
		host() = s.host();
		port() = s.port();
		path() = s.path();
		query() = s.query();
		fragment() = s.fragment();
	}

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
		isValid_ = uriParseUriA(&state_, uri.c_str()) == URI_SUCCESS;

		if (isValid_)
		{
			scheme() = fromRange(uriParse_.scheme);

			user() = fromRange(uriParse_.userInfo);

			if (user().length())
			{
				auto bits = user().split(':');
				if (bits->size() > 1)
				{
					user() = decode((*bits)[0]);
					password() = decode((*bits)[1]);
				}
				else
				{
					user() = decode(user());
				}
			}

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
		return str().c_str();
	}

	operator const string&()
	{
		return str();
	}

	string& str()
	{
		uri_ = "";

		if (scheme().length() && host().length())
		{
			uri_ += scheme() + "://";

			if (user().length())
			{
				uri_ += encode(user());

				if (password().length())
				{
					uri_ += ":";
					uri_ += encode(password());
				}

				uri_ += "@";
			}

			uri_ += host();

			if (port() > 0)
			{
				char tmp[16];
				tmp[0] = '\0';

				itoa(port(), tmp, 10);
				uri_ += ":";
				uri_ += tmp;
			}
		}
		uri_ += path();
		return uri_;
	}

	const char* c_str()
	{
		return str().c_str();
	}

	static string encode(const string s)
	{
		string result;
		result.resize(s.size() * 3);
		uriEscapeA(s.c_str(), (char*)result.buffer(), false, false);
		result.resize(strlen(result.c_str()) + 1);
		return result;
	}

	static string decode(const string s)
	{
		string result = s;
		uriUnescapeInPlaceA((char*)result.buffer());
		result.resize(strlen(result.c_str()) + 1);
		return result;
	}

	const bool& isValid() const { return isValid_; }
	bool& isValid() { return isValid_; }

	const string& scheme() const { return m_scheme; }
	const string& user() const { return m_user; }
	const string& password() const { return m_password; }
	const string& host() const { return m_host; }
	const int& port() const { return m_port; }
	const string& path() const { return m_path; }
	const string& query() const { return m_query; }
	const string& fragment() const { return m_fragment; }

	string& scheme() { return m_scheme; }
	string& user() { return m_user; }
	string& password() { return m_password; }
	string& host() { return m_host; }
	int& port() { return m_port; }
	string& path() { return m_path; }
	string& query() { return m_query; }
	string& fragment() { return m_fragment; }

protected:
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