#pragma once

template<typename T>
class uptr
{
public:
	uptr()
	{
		ptr = NULL;
	}

	uptr(T* p)
	{
		ptr = p;
	}

	uptr(const uptr& p)
	{
		ptr = p.ptr;
		const_cast<uptr&>(p).ptr = NULL;
	}

	uptr& operator=(uptr& p)
	{
		T* swap = p.ptr;
		p.ptr = ptr;
		ptr = swap;
		return *this;
	}

	~uptr()
	{
		destroy();
	}

	operator bool() const
	{
		return (NULL != ptr);
	}

	T* operator->() const
	{
		return ptr;
	}

	T& operator*()  const
	{
		return *ptr;
	}

	T* get()
	{
		return ptr;
	}
private:
	void destroy()
	{
		if (ptr)
		{
			delete ptr;
			release();
		}
	}

	void release()
	{
		ptr = NULL;
	}

	T* ptr;
};

template<class T, class U> inline bool operator==(const uptr<T>& l, const uptr<U>& r)
{
	return (l.get() == r.get());
}

template<class T, class U> inline bool operator!=(const uptr<T>& l, const uptr<U>& r)
{
	return (l.get() != r.get());
}

template<class T, class U> inline bool operator<=(const uptr<T>& l, const uptr<U>& r)
{
	return (l.get() <= r.get());
}

template<class T, class U> inline bool operator<(const uptr<T>& l, const uptr<U>& r)
{
	return (l.get() < r.get());
}

template<class T, class U> inline bool operator>=(const uptr<T>& l, const uptr<U>& r)
{
	return (l.get() >= r.get());
}

template<class T, class U> inline bool operator>(const uptr<T>& l, const uptr<U>& r)
{
	return (l.get() > r.get());
}


