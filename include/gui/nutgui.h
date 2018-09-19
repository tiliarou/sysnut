#pragma once

#include <switch.h>
#include <sys/stat.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h> 
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#include "nx/sddirectory.h"
#include "nx/install.h"
#include "nx/pfs0.h"
#include "gui/window.h"
#include "nx/buffer.h"
#include "nx/font.h"
#include "rapidjson/document.h"
#include "nx/circularbuffer.h"
#include "nx/lock.h"
#include "nx/thread2.h"

string footer;
class NutGui;

NutGui* nutGui = NULL;

CircularBuffer<string, 0x100>& printLog();

class TitleKeyDumper
{
public:
	TitleKeyDumper()
	{
		pmshellInitialize();
	}

	~TitleKeyDumper()
	{
		pmshellExit();
	}

	bool dumpPersonal()
	{
		FsFileSystem tmpMountedFs;

		if (!fsMount_SystemSaveData(&tmpMountedFs, 0x80000000000000E2)) // personal ticket database
		{
			return false;
		}

		print("Successfuly opened personal ticket database!\n");

		mkdir("personalTickets/", 777);

		if (fsdevMountDevice("espersonalized", tmpMountedFs))
		{
			error("Failed to mount espersonalized\n");
			return false;
		}

		auto dir = Directory::openDir("espersonalized:/");
		if (dir)
		{
			for (auto& d : dir->files())
			{
				string from = string("espersonalized:/") + d->name();
				string to = string("personalTickets/") + d->name();

				print("copying file: %s\n", d->name().c_str());

				File::copy(from, to);
			}
			print("Successly dumped personal!\n");
			fsdevUnmountDevice("espersonalized");
			return true;
		}

		fsdevUnmountDevice("espersonalized");
		return false;
	}

	bool dumpCommon()
	{
		FsFileSystem tmpMountedFs;

		if (!fsMount_SystemSaveData(&tmpMountedFs, 0x80000000000000E1)) // common ticket database
		{
			return false;
		}

		print("Successfuly opened common ticket database!\n");

		mkdir("commonTickets/", 777);

		if (fsdevMountDevice("escommon", tmpMountedFs))
		{
			error("Failed to mount escommon\n");
			return false;
		}

		auto dir = Directory::openDir("escommon:/");
		if (dir)
		{
			for (auto& d : dir->files())
			{
				string from = string("escommon:/") + d->name();
				string to = string("commonTickets/") + d->name();

				print("copying file: %s\n", d->name().c_str());

				File::copy(from, to);
			}
			print("Successly dumped common!\n");
			fsdevUnmountDevice("escommon");
			return true;
		}

		fsdevUnmountDevice("escommon");
		return false;
	}

	bool dump()
	{
		return false;
		print("Begining key dump\n");
		bool dumpedCommon = false, dumpedPersonal = false;

		for (int i = 0; i < 100 && (!dumpedCommon || !dumpedPersonal); i++)
		{
			pmshellTerminateProcessByTitleId(0x0100000000000033);

			if (!dumpedPersonal)
			{
				dumpedPersonal = dumpPersonal();
			}

			if (!dumpedCommon)
			{
				dumpedCommon = dumpCommon();
			}
		}

		error("Failed to dump title keys.\n");
		return false;
	}
private:
};


struct Theme
{
	string BackgroundPath;
	string FontPath;
	RGBA TextColor;
	RGBA SelectedTextColor;
};

class Menu : public Window
{
public:
	Menu(Window* p, string id, Rect r) : Window(p, id, r)
	{
		m_selectedIndex = 0;
		buttonTextB() = "Exit";
		buttonTextA() = "OK";
	}

	u64 keysDown(u64 keys) override
	{
		if (keys & KEY_UP)
		{
			if (m_selectedIndex)
			{
				m_selectedIndex--;
			}
			invalidate();
		}

		if (keys & KEY_DOWN)
		{
			if (m_selectedIndex < m_tabs.size() - 1)
			{
				m_selectedIndex++;
			}
			invalidate();
		}
		return keys;
	}

	sptr<Window>& activePanel()
	{
		return m_panels[m_selectedIndex];
	}

	void draw() override
	{
		int y = Fonts::FONT_SIZE_MEDIUM;
		for (u32 i = 0; i < m_tabs.size(); i++)
		{
			if (i == m_selectedIndex)
			{
				if (isFocused())
				{
					drawRect(0, y-((Fonts::FONT_SIZE_HUGE - Fonts::FONT_SIZE_MEDIUM) / 2), width(), Fonts::FONT_SIZE_HUGE, txtcolor);
				}
				if (m_panels[i])
				{
					drawText(Fonts::FONT_SIZE_MEDIUM, y, selcolor, m_panels[i]->icon(), fonts->medium(Fonts::FONT_EXT));
				}
				drawText(Fonts::FONT_SIZE_MEDIUM * 5 / 2, y, selcolor, m_tabs[i], fonts->medium());
				activePanel()->draw();
			}
			else
			{
				if (m_panels[i])
				{
					drawText(Fonts::FONT_SIZE_MEDIUM, y, txtcolor, m_panels[i]->icon(), fonts->medium(Fonts::FONT_EXT));
				}
				drawText(Fonts::FONT_SIZE_MEDIUM * 5 / 2, y, txtcolor, m_tabs[i], fonts->medium());
			}
			y += Fonts::FONT_SIZE_HUGE;
		}
	}

	void add(string tab, Window* panel)
	{
		m_tabs.push(tab);
		m_panels.push(sptr<Window>(panel));
	}

protected:
	Buffer<string> m_tabs;
	Buffer<sptr<Window>> m_panels;
	u32 m_selectedIndex;
};

class Body : public Window
{
public:
	Body(Window* p, string id, Rect r) : Window(p, id, r)
	{
	}
};

template<class T>
class HListWnd : public Window
{
public:
	HListWnd(Window* p, string id, Rect r) : Window(p, id, r)
	{
		m_selectedIndex = 0;
		m_offset = 0;
	}

	virtual void drawItem(int itemIndex, int displayIndex, T& item)
	{
		int y = Fonts::FONT_SIZE_MEDIUM + (displayIndex * Fonts::FONT_SIZE_HUGE);

		if (itemIndex == (int)m_selectedIndex)
		{
			if (isFocused())
			{
				drawRect(0, y - ((Fonts::FONT_SIZE_HUGE - Fonts::FONT_SIZE_MEDIUM) / 2), width(), Fonts::FONT_SIZE_HUGE, txtcolor);
			}
			drawText(Fonts::FONT_SIZE_MEDIUM, y, selcolor, (string)item, fonts->medium());
		}
		else
		{
			drawText(Fonts::FONT_SIZE_MEDIUM, y, txtcolor, (string)item, fonts->medium());
		}
	}

	void draw() override
	{

		if (m_selectedIndex >= m_offset + maxLines)
		{
			m_offset = m_selectedIndex - maxLines + 1;
		}

		if (m_selectedIndex < m_offset)
		{
			m_offset = m_selectedIndex;
		}

		for (int i = 0; i < maxLines && i + m_offset < items().size(); i++)
		{
			drawItem(i + m_offset, i, items()[i + m_offset]);
		}
	}

	virtual void select(u32 selected)
	{
		(void)selected;
	}

	u64 keysDown(u64 keys) override
	{
		if (keys & KEY_UP)
		{
			if (m_selectedIndex)
			{
				m_selectedIndex--;
			}
			invalidate();
		}

		if (keys & KEY_DOWN)
		{
			if (items().size() && m_selectedIndex < items().size() - 1)
			{
				m_selectedIndex++;
			}
			invalidate();
		}

		if (keys & (KEY_RSTICK_DOWN | KEY_R | KEY_ZR)) // page down
		{
			pageMove(true, keys & KEY_ZR);
		}

		if (keys & (KEY_RSTICK_UP | KEY_L | KEY_ZL)) // page up
		{
			pageMove(false, keys & KEY_ZL);
		}

		if (keys & KEY_A && items().size())
		{
			select(m_selectedIndex);
		}
		return keys;
	}

	void pageMove(bool forward, bool fast)
	{
		long currentPosition = (long)m_selectedIndex;
		long size = (long)items().size();

		if (fast)
		{
			long pageSize = size / 8;

			if (pageSize < maxLines)
			{
				pageSize = maxLines;
			}

			long currentPage = currentPosition / pageSize;
			currentPosition = (currentPage + (forward ? 1 : -1)) * pageSize;
		}
		else
		{
			currentPosition += maxLines * (forward ? 1 : -1);
		}

		if (currentPosition >= size)
		{
			currentPosition = size - 1;
		}

		if (currentPosition < 0) currentPosition = 0;


		long offset = currentPosition - (maxLines / 2);

		if (offset + maxLines >= size)
		{
			offset = size - maxLines;
		}
		if (offset < 0) offset = 0;

		m_selectedIndex = currentPosition;
		m_offset = offset;

		invalidate();
	}

	void onFocus() override
	{
		Window::onFocus();
		refresh();
		invalidate();
	}

	virtual void refresh()
	{
	}

	Array<T>& items() { return m_items; }

	Array<T> m_items;
	u32 m_selectedIndex;
	u32 m_offset;

	static const int maxLines = 560 / Fonts::FONT_SIZE_HUGE;
};

class SdWnd : public HListWnd<string>
{
public:
	SdWnd(Window* p, sptr<Directory> directory, string id, Rect r) : HListWnd(p, id, r), dir(directory)
	{
		buttonTextB() = "Back";
		buttonTextA() = "Install";
		buttonTextX() = "Install All";
		buttonTextY() = "Delete prodinfo";

		if (dir)
		{
			if (dir->dirPath().scheme() == "ftp")
			{
				icon() = FON_PC;
			}
			else if (!dir->dirPath().scheme().length())
			{
				icon() = FON_SD;
			}
			else
			{
				icon() = FON_ERROR;
			}
		}
		else
		{
			icon() = FON_ERROR;
		}

		refresh();
	}

	~SdWnd()
	{
	}

	void select(u32 i) override
	{
		(void)i;

		auto fileName = items()[m_selectedIndex];
		auto nsp = dir->openFile<Pfs0>(fileName);
		if (nsp)
		{
			print(string("Installing ") + fileName);
			nsp->install();
			print(string("Installation complete for ") + fileName);
		}
		else
		{
			error("Could not open %s\n", fileName.c_str());
		}

		/*if (threadCreate(&t, &installThread, NULL, 128 * 1024, 0x3B, -2))
		{
			fatal("Failed to create application thread\n");
			return;
		}

		if (threadStart(&t))
		{
			fatal("Failed to start application thread\n");
			return;
		}*/
	}

	void refresh() override
	{
		string nspExt(".nsp");
		items().resize(0);
		for (auto& f : dir->files())
		{
			if (f->name().endsWith(nspExt))
			{
				items().push(f->name());
			}
		}
		//m_files = dir.files().find(".nsp");
	}

	sptr<Directory> dir;
	//Thread t;
};

class TitleRow
{
public:
	TitleRow()
	{
	}

	TitleRow(RightsId rightsId, string name) : m_rightsId(rightsId), m_name(name)
	{
	}

	operator string()
	{
		return hx(m_rightsId.titleId()) + "    " + name();
	}

	RightsId& rightsId() { return m_rightsId; }
	string& name() { return m_name; }

private:
	RightsId m_rightsId;
	string m_name;
};

class TicketWnd : public HListWnd<TitleRow>
{
public:
	TicketWnd(Window* p, string id, Rect r) : HListWnd(p, id, r)
	{
		icon() = FON_TICKET;
		buttonTextB() = "Back";
		buttonTextX() = "Delete";

		refresh();
	}

	void onFocus()
	{
		Window::onFocus();
		refresh();
		invalidate();
	}

	u64 keysDown(u64 keys) override
	{
		keys = HListWnd<TitleRow>::keysDown(keys);

		if (keys & KEY_X && items().size())
		{
			esDeleteTicket(&items()[m_selectedIndex].rightsId(), sizeof(RightsId));
			refresh();
			invalidate();
		}
		if (keys & KEY_Y)
		{
			TitleKeyDumper td;
			td.dump();
		}
		return keys;
	}

	void refresh() override
	{
		Buffer<RightsId> rightsIds;
		u32 rightsIdCount = 0;
		u32 installedTicketCount = 0;
		u8 titleKey[16];

		items().resize(0);


		if (esCountCommonTicket(&installedTicketCount))
		{
			error("Failed to count common tickets\n");
			return;
		}

		rightsIds.resize(installedTicketCount);
		memset(rightsIds.buffer(), 0, rightsIds.sizeBytes());

		if (esListCommonTicket(&rightsIdCount, rightsIds.buffer(), rightsIds.sizeBytes()))
		{
			error("Failed to list common tickets\n");
			return;
		}

		for (unsigned int i = 0; i < rightsIdCount; i++)
		{
			items().push(TitleRow(rightsIds[i], getBaseTitleName(rightsIds[i].titleId())));
		}

		if (m_selectedIndex >= items().size())
		{
			m_selectedIndex = items().size();

			if (m_selectedIndex)
			{
				m_selectedIndex--;
			}
		}
	}
};

class NutWnd : public Window
{
public:
	NutWnd(Window* p, string id, Rect r) : Window(p, id, r)
	{
	}
};

class ConsoleWnd : public Window
{
public:
	ConsoleWnd(Window* p, string id, Rect r) : Window(p, id, r)
	{
		icon() = FON_CHAT;
	}

	void draw() override
	{
		const int maxLines = 560 / Fonts::FONT_SIZE_HUGE;

		int end = printLog().size();
		int i = end > maxLines ? end - maxLines : 0;

		int y = 20;
		while(i < end)
		{
			drawText(20, y, txtcolor, printLog()[i], fonts->medium());
			y += Fonts::FONT_SIZE_MEDIUM;
			i++;
		}
	}

	void onFocus()
	{
		Window::onFocus();
		invalidate();
	}
};

class GuiThread : public Thread2
{
public:
	GuiThread() : Thread2()
	{
	}

	bool step() override
	{
		if (nutGui)
		{
			//nutGui->loop();
			print("tick\n");
		}
		nxSleep(100);
		return true;
	}
};

class NutGui : public Window
{
public:
	NutGui(string Title, string Footer, Theme Theme) : m_focus(0), Window(string("root"), Rect(0, 0, 1280, 720))
	{
		Rect panelRect(411, 88, 1249 - 411, 646 - 88);
		nutGui = this;
		fonts = new Fonts();

		menu = (Menu*)windows().push(sptr<Window>(new Menu(this, string("menu"), Rect(30, 88, 410 - 30, 646 - 88)))).get();
		body = (Body*)windows().push(sptr<Window>(new Body(this, string("body"), Rect(411, 88, 1249 - 411, 646 - 88 )))).get();

		File f;
		string s = "/switch/deez/locations.conf";
		if (f.open(s))
		{
			Buffer<u8> buffer;
			f.read(buffer);
			buffer.push(0);
			rapidjson::Document locations;
			locations.Parse(buffer.c_str());
			if (locations.IsArray())
			{
				for (auto itr = locations.Begin(); itr != locations.End(); ++itr)
				{
					Url url;
					if (!itr->HasMember("title"))
					{
						continue;
					}

					if (itr->HasMember("url"))
					{
						url.set((*itr)["url"].GetString());
					}

					if (itr->HasMember("user"))
					{
						url.user() = (*itr)["user"].GetString();
					}

					if (itr->HasMember("password"))
					{
						url.password() = (*itr)["password"].GetString();
					}

					if (itr->HasMember("port"))
					{
						url.port() = atoi((*itr)["port"].GetString());
					}

					if (itr->HasMember("scheme"))
					{
						url.scheme() = (*itr)["scheme"].GetString();
					}

					if (itr->HasMember("path"))
					{
						url.path() = (*itr)["path"].GetString();
					}

					menu->add((*itr)["title"].GetString(), new SdWnd(this, Directory::openDir(url), url.str(), panelRect));
				}
			}
			else
			{
				menu->add(string("SD Root"), new SdWnd(this, Directory::openDir("/"), string("SD"), panelRect));
			}
		}

		menu->add(string("Common Tickets"), new TicketWnd(this, string("Tickets"), panelRect));
		menu->add(string("Console"), new ConsoleWnd(this, string("CONSOLE"), panelRect));

		setFocus(menu);


		romfsInit();
		SDL_Init(SDL_INIT_EVERYTHING);
		SDL_CreateWindowAndRenderer(1280, 720, 0, &_window, &_renderer);
		_surface = SDL_GetWindowSurface(_window);
		SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
		IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG | IMG_INIT_WEBP | IMG_INIT_TIF);
		TTF_Init();

		SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255);

		bgs = surfInit(Theme.BackgroundPath);
		bgt = texInit(bgs);
		txtcolor = { Theme.TextColor.R, Theme.TextColor.G, Theme.TextColor.B, Theme.TextColor.A };
		selcolor = { Theme.SelectedTextColor.R, Theme.SelectedTextColor.G, Theme.SelectedTextColor.B, Theme.SelectedTextColor.A };
		title = Title;
		footer = Footer;

		registerPrintHook(&printHook);
		guiThread.start();
	}

	~NutGui()
	{
		TTF_Quit();
		IMG_Quit();
		SDL_DestroyRenderer(_renderer);
		SDL_FreeSurface(_surface);
		SDL_DestroyWindow(_window);
		SDL_Quit();
		romfsExit();
		delete fonts;
	}

	static void printHook()
	{
		if (!nutGui)
		{
			return;
		}

		nutGui->invalidate();
		nutGui->loop();
	}

	void setFocus(Window* w)
	{
		if (focus())
		{
			if (focus() == w)
			{
				return;
			}
			focus()->onDefocus();
		}
		focus() = w;
		focus()->onFocus();
	}

	u64 keysDown(u64 keys) override
	{
		if (focus())
		{
			keys = focus()->keysDown(keys);
		}

		if (keys & KEY_LEFT)
		{
			if (focus() != menu)
			{
				setFocus(menu);
				invalidate();
			}
		}

		if (keys & KEY_RIGHT)
		{
			if (focus() == menu)
			{
				setFocus(menu->activePanel().get());
				invalidate();
			}
		}

		if (keys & KEY_A && focus() == menu)
		{
			setFocus(menu->activePanel().get());
			invalidate();
		}


		return keys;
	}

	u64 keysUp(u64 keys) override
	{
		if (focus())
		{
			keys = focus()->keysUp(keys);
		}

		return keys;
	}

	bool loop()
	{
		flushGraphics();

		u64 kDown = keysDown(PressedInput);

		if (kDown & KEY_PLUS)
		{
			return false;
		}

		if (kDown & KEY_B)
		{
			if (focus() == menu)
			{
				return false;
			}

			setFocus(menu);
			invalidate();
		}

		if (m_redraw)
		{
			renderGraphics();
			m_redraw = false;
		}

		return true;

	}

	/*
	void render()
	{
		hidScanInput();

		HeldInput = hidKeysHeld(CONTROLLER_P1_AUTO);
		PressedInput = hidKeysDown(CONTROLLER_P1_AUTO);
		ReleasedInput = hidKeysUp(CONTROLLER_P1_AUTO);

		keysDown(PressedInput);

		SDL_RenderClear(_renderer);
		drawBack(bgs, bgt);
		drawText(titleX, titleY, txtcolor, title, fonts->large());



		
		drawText(footerX, footerY, txtcolor, footer, fonts->large());
		SDL_RenderPresent(_renderer);
	}*/

	void renderGraphics()
	{
		SDL_RenderClear(_renderer);
		drawBack(bgs, bgt);
		drawText(titleX, titleY, txtcolor, title, fonts->large());

		if (menu)
		{
			menu->draw();
		}

		if (printLog().size())
		{
			drawText(footerX, footerY, txtcolor, printLog().last(), fonts->medium());
		}


		if (focus())
		{
			P p(this, Rect(640, 648, 610, 72), P::ALIGN_RIGHT | P::VALIGN_CENTER);

			if (focus()->buttonTextY().size())
			{
				p.drawText(txtcolor, FON_Y, fonts->medium(Fonts::FONT_EXT));
				p.drawText(txtcolor, string(" ") + focus()->buttonTextY() + "   ", fonts->medium());
			}

			if (focus()->buttonTextX().size())
			{
				p.drawText(txtcolor, FON_X, fonts->medium(Fonts::FONT_EXT));
				p.drawText(txtcolor, string(" ") + focus()->buttonTextX() + "   ", fonts->medium());
			}

			if (focus()->buttonTextB().size())
			{
				p.drawText(txtcolor, FON_B, fonts->medium(Fonts::FONT_EXT));
				p.drawText(txtcolor, string(" ") + focus()->buttonTextB() + "   ", fonts->medium());
			}

			if (focus()->buttonTextA().size())
			{
				p.drawText(txtcolor, FON_A, fonts->medium(Fonts::FONT_EXT));
				p.drawText(txtcolor, string(" ") + focus()->buttonTextA() + "   ", fonts->medium());
			}

			p.render();
		}

		SDL_RenderPresent(_renderer);
	}

	void flushGraphics()
	{
		if (Frame == 0)
		{
			renderGraphics();
		}

		Frame++;
		hidScanInput();
		HeldInput = hidKeysHeld(CONTROLLER_P1_AUTO);
		PressedInput = hidKeysDown(CONTROLLER_P1_AUTO);
		ReleasedInput = hidKeysUp(CONTROLLER_P1_AUTO);
	}

	Window*& focus() { return m_focus; }
	Buffer<sptr<Window>>& windows() { return m_windows; }

	SDL_Window *_window;

	SDL_Surface *bgs;
	SDL_Texture *bgt;
	string title;
	//string footer;

	int titleX = 60;
	int titleY = 30;
	int footerX = titleX;
	int footerY = 672;
	int optX = 55;
	int optY = 115;

	u64 HeldInput = 0;
	u64 PressedInput = 0;
	u64 ReleasedInput = 0;

	int Frame = 0;

	static Theme HorizonLight()
	{
		return{ "romfs:/Graphics/Background.Light.png", "romfs:/Fonts/NintendoStandard.ttf",{ 0, 0, 0, 255 },{ 140, 140, 140, 255 } };
	}

	static Theme HorizonDark()
	{
		return{ "romfs:/Graphics/Background.Dark.png", "romfs:/Fonts/NintendoStandard.ttf",{ 255, 255, 255, 255 },{ 140, 140, 140, 255 } };
	}

	static Theme HorizonCurrent()
	{
		ColorSetId id;
		setsysInitialize();
		setsysGetColorSetId(&id);
		setsysExit();
		if (id == ColorSetId_Dark) return HorizonDark();
		else return HorizonLight();
	}

protected:
	GuiThread guiThread;
	Buffer<sptr<Window>> m_windows;
	Window* m_focus;
	Menu* menu;
	Body* body;
	Lock lock;
};