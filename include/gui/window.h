#pragma once

#include <switch.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h> 
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#include "gui/nutgui.h"
#include "nx/string.h"
#include "nx/primitives.h"

static SDL_Renderer* _renderer = 0;
static SDL_Surface* _surface = 0;

struct Rect
{
	Rect()
	{
	}

	Rect(u32 _x, u32 _y, u32 _width, u32 _height) : x(_x), y(_y), width(_width), height(_height)
	{
	}

	u32 x;
	u32 y;
	u32 width;
	u32 height;
};

class Window
{
public:
	Window(string id, Rect r) : m_parent(0)
	{
		this->id() = id;
		this->rect() = r;
	}

	Window(Window* parent, string id, Rect r) : m_parent(parent)
	{
		this->id() = id;
		this->rect() = r;

		_renderer = 0;
		_surface = 0;
	}

	SDL_Surface *surfInit(string Path)
	{
		SDL_Surface *srf = IMG_Load(Path.c_str());
		if (srf)
		{
			Uint32 colorkey = SDL_MapRGB(srf->format, 0, 0, 0);
			SDL_SetColorKey(srf, SDL_TRUE, colorkey);
		}
		return srf;
	}

	SDL_Texture *texInit(SDL_Surface *surf)
	{
		SDL_Texture *tex = SDL_CreateTextureFromSurface(_renderer, surf);
		return tex;
	}

	void drawText(int x, int y, SDL_Color scolor, string text, TTF_Font *font)
	{
		SDL_Surface *surface = TTF_RenderText_Blended_Wrapped(font, text.c_str(), scolor, 1280);
		SDL_SetSurfaceAlphaMod(surface, 255);
		SDL_Rect position = { x, y, surface->w, surface->h };
		SDL_BlitSurface(surface, NULL,_surface, &position);
		SDL_FreeSurface(surface);
	}

	void drawRect(int x, int y, int w, int h, SDL_Color scolor)
	{
		SDL_Rect rect;
		rect.x = x;
		rect.y = y;
		rect.w = w;
		rect.h = h;
		SDL_SetRenderDrawColor(_renderer, scolor.r, scolor.g, scolor.b, scolor.a);
		SDL_RenderFillRect(_renderer, &rect);
	}

	void drawBackXY(SDL_Surface *surf, SDL_Texture *tex, int x, int y)
	{
		SDL_Rect position;
		position.x = x;
		position.y = y;
		position.w = surf->w;
		position.h = surf->h;
		SDL_RenderCopy(_renderer, tex, NULL, &position);
	}

	void drawBack(SDL_Surface *surf, SDL_Texture *tex)
	{
		drawBackXY(surf, tex, 0, 0);
	}

	virtual u64 keysDown(u64 keys) { return keys; }
	virtual u64 keysUp(u64 keys) { return keys; }

	u32 x() { return rect().x; }
	u32 y() { return rect().y; }

	u32 width() { return rect().width; }
	u32 height() { return rect().height; }

	const string& id() const { return m_id; }
	string& id() { return m_id; }

	Rect& rect() { return m_rect; }
protected:
	Window*& parent() { return m_parent; }
	Rect m_rect;
	string m_id;
	Window* m_parent;
};