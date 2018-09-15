#pragma once

#include "nx/primitives.h"
#include "log.h"

#ifdef __SWITCH__
extern "C" {
	#include <switch/services/pl.h>
}
#include <SDL2/SDL_ttf.h>
#endif

#define FON_A "\uE0E0"
#define FON_B "\uE0E1"
#define FON_X "\uE0E2"
#define FON_Y "\uE0E3"
#define FON_L "\uE0E4"
#define FON_R "\uE0E5"
#define FON_ZL "\uE0E6"
#define FON_ZR "\uE0E7"
#define FON_SL "\uE0E8"
#define FON_SR "\uE0E9"
#define FON_UP "\uE0EB"
#define FON_DN "\uE0EC"
#define FON_LT "\uE0ED"
#define FON_RT "\uE0EE"
#define FON_PL "\uE0EF"
#define FON_MI "\uE0F0"
#define FON_HM "\uE0F4"
#define FON_SS "\uE0F5"
#define FON_LS "\uE101"
#define FON_RS "\uE102"
#define FON_L3 "\uE104"
#define FON_R3 "\uE105"

#define FONT_BOOK 4
#define FONT_EXT 0


class Fonts
{
public:
	Fonts() 
	{
		languageCode = 0;
		totalFontFaces = 0;

		memset(&m_fonts, 0, sizeof(m_fonts));

		setGetSystemLanguage(&languageCode);

		if (plGetSharedFont(languageCode, m_fontData, PlSharedFontType_Total, &totalFontFaces))
		{
			error("Could not call plGetSharedFont\n");
			return;
		}
	}

	~Fonts()
	{
	}

	TTF_Font* large(u32 i = FONT_BOOK)
	{
		if (i >= totalFontFaces)
		{
			return NULL;
		}

		if (!m_fonts[i][2])
		{
			m_fonts[i][2] = TTF_OpenFontRW(SDL_RWFromMem((void*)m_fontData[i].address, m_fontData[i].size), 1, 25);
		}
		return m_fonts[i][2];
	}

	TTF_Font* medium(u32 i = FONT_BOOK)
	{
		if (i >= totalFontFaces)
		{
			return NULL;
		}

		if (!m_fonts[i][1])
		{
			m_fonts[i][1] = TTF_OpenFontRW(SDL_RWFromMem((void*)m_fontData[i].address, m_fontData[i].size), 1, 20);
		}
		return m_fonts[i][1];
	}
private:
	TTF_Font* m_fonts[PlSharedFontType_Total][3];
	PlFontData m_fontData[PlSharedFontType_Total];
	size_t totalFontFaces;
	u64 languageCode;
};