/* This file is part of libschrift.
 *
 * © 2019, 2020 Thomas Oltmann
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. */

#ifndef SCHRIFT_H
#define SCHRIFT_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_MSC_VER)
# define restrict __restrict
#endif

#if defined(_WIN32)
# define WIN32_LEAN_AND_MEAN 1
# include <windows.h>
#else
# include <fcntl.h>
# include <sys/mman.h>
# include <sys/stat.h>
# include <unistd.h>
#endif

#define SFT_DOWNWARD_Y    0x01
#define SFT_CATCH_MISSING 0x04

typedef struct SFT 		    SFT;
typedef struct SFT_Font     SFT_Font;
typedef uint32_t 		    SFT_Glyph;
typedef struct SFT_LMetrics SFT_LMetrics;
typedef struct SFT_GMetrics SFT_GMetrics;
typedef struct SFT_Kerning  SFT_Kerning;
typedef struct SFT_Char     SFT_Char;

struct SFT
{
	SFT_Font *font;
	double xScale;
	double yScale;
	double xOffset;
	double yOffset;
	unsigned int flags;
};

struct SFT_Font {
	const uint8_t *memory;
	unsigned long size;
	#if defined(_WIN32)
		HANDLE mapping;
	#endif
	int source;
	
	uint_least16_t unitsPerEm;
	int_least16_t locaFormat;
	uint_least16_t numLongHmtx;
};

struct SFT_LMetrics
{
	/* The distance from the baseline to the visual top of the text */
	double ascender;
	/* The distance from the baseline to the visual bottom of the text */
	double descender;
	/* The default amount of horizontal padding between consecutive lines */
	double lineGap;
};

struct SFT_GMetrics
{
	/* How much the pen position should advance to the right after rendering the glyph */
	double advanceWidth;
	/* The offset that should be applied along the X axis from the pen position to the edge of the glyph image */
	double leftSideBearing;
	/* An offset along the Y axis relative to the pen position that should be applied when displaying the glyph */
	int    yOffset;
	/* The minimum width that an image needs such that the glyph can be properly rendered into it */
	int    minWidth;
	/* The minimum height that an image needs such that the glyph can be properly rendered into it */
	int    minHeight;
};

struct SFT_Char
{
	uint8_t* image;
	int advance;
	int x;
	int y;
	int width;
	int height;
};

struct SFT_Kerning
{
	/* An amount that should be added to the pen's X position in-between the two glyphs */
	double xShift;
	/* An amount that should be added to the pen's Y position in-between the two glyphs */
	double yShift;
};

/* libschrift uses semantic versioning. */
const char *sft_version(void);

SFT_Font *sft_loadmem(const void *mem, unsigned long size);
SFT_Font *sft_loadfile(const char *filename);
void sft_freefont(SFT_Font *font);

/*
	@brief Calculates the typographic metrics neccessary for laying out multiple lines of text
*/
int sft_lmetrics(const SFT *sft, SFT_LMetrics *lmetrics);
/* 
	@brief Tells where to draw the next glyph with respect to the pen position, and how to update it after rendering the glyph 
*/
int sft_gmetrics(const SFT *sft, SFT_Glyph glyph, SFT_GMetrics *gmetrics);
/* 
	@brief Used to retrieve kerning information for a given pair of glyph ids
*/
int sft_kerning(const SFT *sft, SFT_Glyph leftGlyph, SFT_Glyph rightGlyph, SFT_Kerning* kerning);
/*
	@brief Render a glyph
*/
int sft_char(const SFT *sft, unsigned long charCode, SFT_Char *chr);

#ifdef __cplusplus
}
#endif

#endif
