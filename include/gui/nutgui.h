#pragma once

#include <switch.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h> 
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#include "gui/window.h"
#include "nx/buffer.h"

struct RGBA
{
	int R;
	int G;
	int B;
	int A;
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
	}
};

class Body : public Window
{
public:
	Body(Window* p, string id, Rect r) : Window(p, id, r)
	{
	}
};

class NutGui : public Window
{
public:
	NutGui(string Title, string Footer, Theme Theme) : m_focus(0), Window(string("root"), Rect(0, 0, 1280, 720))
	{
		menu = (Menu*)windows().push(sptr<Window>(new Menu(this, string("menu"), Rect(30, 88, 410 - 30, 646 - 88)))).get();
		body = (Body*)windows().push(sptr<Window>(new Body(this, string("body"), Rect(411, 88, 1249 - 411, 646 - 88 )))).get();

		focus() = body;


		romfsInit();
		SDL_Init(SDL_INIT_EVERYTHING);
		SDL_CreateWindowAndRenderer(1280, 720, 0, &_window, &_renderer);
		_surface = SDL_GetWindowSurface(_window);
		SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
		IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG | IMG_INIT_WEBP | IMG_INIT_TIF);
		TTF_Init();

		SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255);

		ttf = Theme.FontPath;
		fntLarge = TTF_OpenFont(Theme.FontPath.c_str(), 25);
		fntMedium = TTF_OpenFont(Theme.FontPath.c_str(), 20);
		bgs = surfInit(Theme.BackgroundPath);
		bgt = texInit(bgs);
		txtcolor = { Theme.TextColor.R, Theme.TextColor.G, Theme.TextColor.B, Theme.TextColor.A };
		selcolor = { Theme.SelectedTextColor.R, Theme.SelectedTextColor.G, Theme.SelectedTextColor.B, Theme.SelectedTextColor.A };
		vol = 64;
		title = Title;
		footer = Footer;
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
	}

	u64 keysDown(u64 keys) override
	{
		if (focus())
		{
			keys = focus()->keysDown(keys);
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

		if (kDown & KEY_A)
		{
			//text1 = "A pressed!";
			renderGraphics();
		}

		return true;
	}

	void render()
	{
		hidScanInput();

		HeldInput = hidKeysHeld(CONTROLLER_P1_AUTO);
		PressedInput = hidKeysDown(CONTROLLER_P1_AUTO);
		ReleasedInput = hidKeysUp(CONTROLLER_P1_AUTO);

		SDL_RenderClear(_renderer);
		drawBack(bgs, bgt);
		drawText(titleX, titleY, txtcolor, title, fntLarge);



		
		drawText(footerX, footerY, txtcolor, footer, fntLarge);
		SDL_RenderPresent(_renderer);
	}

	void renderGraphics()
	{
		SDL_RenderClear(_renderer);
		drawBack(bgs, bgt);
		drawText(titleX, titleY, txtcolor, title, fntLarge);




		drawText(footerX, footerY, txtcolor, footer, fntLarge);
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
	SDL_Color txtcolor;
	SDL_Color selcolor;
	string ttf;
	TTF_Font *fntMedium;
	TTF_Font *fntLarge;

	SDL_Surface *bgs;
	SDL_Texture *bgt;
	string title;
	string footer;
	int vol;
	int titleX = 60;
	int titleY = 30;
	int footerX = titleX;
	int footerY = 672;
	int optX = 55;
	int optY = 115;

	u64 HeldInput = 0;
	u64 PressedInput = 0;
	u64 ReleasedInput = 0;

	// Current frame/iteration, incremented by "flushGraphics" every loop
	int Frame = 0;

	static Theme HorizonLight()
	{
		return{ "romfs:/Graphics/Background.Light.png", "romfs:/Fonts/NintendoStandard.ttf",{ 0, 0, 0, 255 },{ 140, 140, 140, 255 } };
	}

	static Theme HorizonDark()
	{
		return{ "romfs:/Graphics/Background.Dark.png", "romfs:/Fonts/NintendoStandard.ttf",{ 255, 255, 255, 255 },{ 140, 140, 140, 255 } };
	}

	// Theme being used by the current console
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
	Buffer<sptr<Window>> m_windows;
	Window* m_focus;
	Menu* menu;
	Body* body;
};