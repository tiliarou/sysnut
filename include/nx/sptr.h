#pragma once
#include "nx/primitives.h"

#ifndef BLAH
#include <memory>
#define sptr std::shared_ptr
#else
class RefCounter
{
public:
	RefCounter()
	{
		counter = 0;
	}

	RefCounter(const RefCounter& src)
	{
		counter = src.counter;
	}

	long count()
	{
		if (!counter)
		{
			return 0;
		}
		return *counter;
	}

	RefCounter& operator++()
	{
		if (counter)
		{
			*counter++;
		}
		else
		{
			counter = new long(1);
		}
		return *this;
	}

	RefCounter& operator--()
	{
		if (counter)
		{
			if (*counter > 0)
			{
				*counter--;
			}
		}
		else
		{
			counter = new long(1);
		}
		return *this;
	}

	operator bool() const
	{
		return counter && *counter;
	}

	void destroy()
	{
		if (counter)
		{
			delete counter;
			counter = NULL;
		}
	}
private:
	long* counter;
};

template<typename T>
class sptr
{
public:
	sptr()
	{
		ptr = NULL;
	}

	sptr(T* p)
	{
		ptr = p;

		if (ptr)
		{
			acquire();
		}
	}

	sptr(const sptr& p)
	{
		ptr = p.ptr;
		counter = p.counter;
		acquire();
		//const_cast<sptr&>(p).ptr = NULL;
	}

	sptr& operator=(T* p)
	{
		release();
		counter.destroy();
		ptr = p;
		if (ptr)
		{
			acquire();
		}
		return *this;
	}

	/*sptr& operator=(sptr& p)
	{
		T* swap = p.ptr;
		p.ptr = ptr;
		ptr = swap;
		return *this;
	}*/

	~sptr()
	{
		release();
	}

	void acquire()
	{
		++counter;
	}

	void release()
	{
		if (counter)
		{
			--counter;
		}

		if (!counter)
		{
			destroy();
		}
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
			ptr = NULL;
		}

		counter.destroy();
	}

	T* ptr;
	RefCounter counter;
};

#endif