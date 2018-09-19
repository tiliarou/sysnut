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
#include "nx/font.h"

static SDL_Renderer* _renderer = 0;
static SDL_Surface* _surface = 0;
static SDL_Color txtcolor;
static SDL_Color selcolor;

static Fonts* fonts;

struct RGBA
{
	int R;
	int G;
	int B;
	int A;
};

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
		icon() = "";

		buttonTextB() = "Back";
	}

	Window(Window* parent, string id, Rect r) : m_parent(parent)
	{
		this->id() = id;
		this->rect() = r;

		_renderer = 0;
		_surface = 0;
	}

	virtual void draw()
	{
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
		SDL_Surface *surface = TTF_RenderUTF8_Blended(font, text.c_str(), scolor);
		SDL_SetSurfaceAlphaMod(surface, 255);
		SDL_Rect position = { x + rect().x, y + rect().y, surface->w, surface->h };
		SDL_BlitSurface(surface, NULL,_surface, &position);
		SDL_FreeSurface(surface);
	}

	void drawRect(int x, int y, int w, int h, SDL_Color scolor)
	{
		SDL_Rect rect;
		rect.x = x + this->rect().x;
		rect.y = y + this->rect().y;
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

	virtual void invalidate()
	{
		if (parent())
		{
			parent()->invalidate();
		}

		m_redraw = true;
	}

	virtual void onFocus()
	{
		m_isFocused = true;
	}

	virtual void onDefocus()
	{
		m_isFocused = false;
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

	bool& isFocused() { return m_isFocused; }

	string& icon() { return m_icon; }
	string& buttonTextA() { return m_buttonTextA; }
	string& buttonTextB() { return m_buttonTextB; }
	string& buttonTextX() { return m_buttonTextX; }
	string& buttonTextY() { return m_buttonTextY; }

protected:
	string m_icon;

	string m_buttonTextA;
	string m_buttonTextB;
	string m_buttonTextX;
	string m_buttonTextY;
	bool m_isFocused = false;
	bool m_redraw = false;
	Window*& parent() { return m_parent; }
	Rect m_rect;
	string m_id;
	Window* m_parent;
};

class TextBlock
{
public:
	TextBlock()
	{
	}

	TextBlock(SDL_Color color, string text, TTF_Font *font) : color(color), text(text), font(font)
	{
		if (TTF_SizeUTF8(font, text.c_str(), &width, &height))
		{
			width = height = 0;
		}
	}

	int width;
	int height;
	SDL_Color color;
	TTF_Font* font;
	string text;
};

class P
{
public:
	const static u8 ALIGN_LEFT = 1;
	const static u8 ALIGN_RIGHT = 2;
	const static u8 ALIGN_CENTER = 3;

	const static u8 VALIGN_TOP = 4;
	const static u8 VALIGN_BOTTOM = 8;
	const static u8 VALIGN_CENTER = 12;

	P(Window* window, Rect r, u8 align = ALIGN_LEFT | VALIGN_TOP) : m_window(window), m_r(r), m_align(align)
	{
	}

	void drawText(SDL_Color color, string text, TTF_Font *font)
	{
		blocks.push(TextBlock(color, text, font));
	}

	void render()
	{
		int tallest = 0;
		int totalWidth = 0;
		int cursorX = 0;

		int halign = m_align & 3;
		int valign = m_align & 12;


		for (auto& block : blocks)
		{
			totalWidth += block.width;
			if (block.height > tallest)
			{
				tallest = block.height;
			}
		}

		cursorX = 0;

		for (auto& block : blocks)
		{
			int x, y;
			switch (valign)
			{
				case VALIGN_CENTER:
					y = ((m_r.height - block.height) / 2);
					break;
				case VALIGN_BOTTOM:
					y = m_r.height - block.height;
					break;
				case VALIGN_TOP:
				default:
					y = 0;
			}

			switch (halign)
			{
				case ALIGN_CENTER:
					x = ((m_r.width - block.width) / 2) + cursorX;
					break;
				case ALIGN_RIGHT:
					x = m_r.width - totalWidth + cursorX;
					break;
				case ALIGN_LEFT:
				default:
					x = cursorX;
			}

			m_window->drawText(x + m_r.x, y + m_r.y, block.color, block.text, block.font);
			cursorX += block.width;
		}
	}

private:
	Array<TextBlock> blocks;
	Window* m_window;
	Rect m_r;
	u8 m_align;
};