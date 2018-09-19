#pragma once
#include "nut.h"

class Thread2
{
public:
	Thread2()
	{
		m_shouldRun = true;
		m_theadExited = false;
		memset(&t, 0, sizeof(t));
	}

	~Thread2()
	{
		if (shouldRun())
		{
			m_shouldRun = false;
		}
#ifdef __SWITCH__
		if (!threadFinished())
		{
			threadWaitForExit(&t);
		}
		threadClose(&t);
		print("thread died\n");
#else
		WaitForSingleObject(t, INFINITE);
#endif
	}

	bool start()
	{
#ifdef __SWITCH__
		memset(&t, 0, sizeof(t));
		if (threadCreate(&t, (void(*)(void*))&__threadEntry, this, 0x20000, 45, getNextCpuId()))
		{
			error("Failed to create download thread!\n");
			return;
		}

		if (threadStart(&t))
		{
			error("Failed to start download thread!\n");
			return;
		}
#else
		SECURITY_ATTRIBUTES   threadAttributes;
		ZeroMemory(&threadAttributes, sizeof(threadAttributes));
		t = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&__threadEntry, this, 0, &threadId);
#endif
		return true;
	}

	virtual bool init()
	{
		return true;
	}

	virtual bool step()
	{
		nxSleep(10);
		return true;
	}

	virtual void exit()
	{
	}

	static void WINAPI __threadEntry(Thread2* p)
	{
		if (!p || !p->init())
		{
			if (p)
			{
				p->shouldRun() = false;
			}

			return;
		}

		while (p->shouldRun() && p->step())
		{
		}

		p->exit();
		p->threadFinished() = true;
	}

	bool& shouldRun() { return m_shouldRun;	}
	bool& threadFinished() { return m_theadExited; }

protected:
	bool m_shouldRun;
	bool m_theadExited;
#ifdef __SWITCH__
	Thread t;
#else
	HANDLE t;
	DWORD threadId;
#endif
};