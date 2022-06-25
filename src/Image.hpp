#pragma once

#include "schrift.h"

#include <stdint.h>
#include <cstdio>
#include <complex>
#include <vector>
#include <cmath>
#include <optional>
#include <functional>

enum ImageType
{
	PNG, JPG, BMP, TGA
};

enum ImagePosition
{
	LEFT, RIGHT, TOP, BOTTOM, LEFT_TOP, RIGHT_TOP, LEFT_BOTTOM, RIGHT_BOTTOM, CENTER
};

struct Cyrillic;

struct Greek;

struct Color;

struct Details;

class Font {

	public:

		Font() = default;

		Font(const char* fontfile, uint16_t size) {
			if((sft.font = sft_loadfile(fontfile)) == NULL) {
				printf("\e[31m[ERROR] Failed to load %s\e[0m\n", fontfile);
				return;
			};
			setSize(size);
		};

		~Font() {
			sft_freefont(sft.font);
		};

		void setSize(uint16_t size) {
			sft.xScale = size;
			sft.yScale = size;
		};

	public:

		SFT sft = {NULL, 12, 12, 0, 0, SFT_DOWNWARD_Y|SFT_RENDER_IMAGE};

};

class Image {
	
	public:

		/* Blank constructor */
		Image();
		/* Gets image from file */
		Image(const char* filename, int channel_force = 0);
		/* Creates image */
		Image(int w, int h, int channels = 3);
		/* Copy constructor */
		Image(const Image& other);
		/* Deconstructor */
		~Image();

		/* 
			@brief Read image from file 
			@param filename path to file
			@return true if file read successfully, false if not
		*/
		bool read(const char* filename, int channel_force = 0);
		
		/* 
			@brief Write image to file 
			@param filename path to file
			@return true if image written successfully, false if not
		*/
		bool write(const char* filename);

		/* 
			@brief Get file type 
			@param filename path to file
			@return file type
		*/
		ImageType getFileType(const char* filename);

		/*
			@brief Get image details
			@return image details (width, height, channels, size, etc.)
		*/
		Details getDetails() const;
	
		/**/
		void diffmap(Image& img);
		void diffmapScale(Image& img, uint8_t scl = 0);

		/**/
		void grayscaleAvg();
		void grayscaleLum();

		/**/
		void colorMask(float r, float g, float b);

		/*
			@brief Flip image by X axis
		*/
		void flipX();
		/*
			@brief Flip image by Y axis
		*/
		void flipY();

		/*
			@brief Overlays image onto image
			@param source Image to be overlaid
			@param x,y Coordinates from which image will be overlaid (top left corner)
		*/
		void overlay(const Image& source, int x, int y);

		/*
			@brief Overlays text onto image
			@param txt Text to be ovelaid
			@param font Font that will be used
			@param x,y Coordinates from which image will be overlaid
			@param r,g,b,a RGBA parameters (0-255)
		*/
		void overlayText(const std::string& txt, const Font& font, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

		void overlayText(const std::string& txt, const Font& font, int x, int y, const Color& color);

		void rasterizeText(const std::string& txt, const Font& font, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

		void rasterizeText(const std::string& txt, const Font& font, const Color& color);

		/*
			@brief Crops image
			@param cx,cy Beginning of cropped image (in pixels)
			@param cw,ch Width and height of cropped image
		*/
		void crop(uint16_t cx, uint16_t cy, uint16_t cw, uint16_t ch);

		/*
			@brief Creates a copy of an image and crops it, leaving origin untouched
			@param cx,cy Beginning of cropped image (in pixels)
			@param cw,ch Width and height of cropped image
			@returns Cropped copy of an image
		*/
		Image cropCopy(uint16_t cx, uint16_t cy, uint16_t cw, uint16_t ch);

		/*
			@brief Resizes existing image
			@param nw, nh New resolution
		*/
		void resize(uint16_t nw, uint16_t nh);

		/*
			@brief Copies existing and resizes copied image, leaving origin untouched
			@param nw, nh New resolution
			@returns Resized copy of an image
		*/
		Image resizeCopy(uint16_t nw, uint16_t nh);

		/*
			@brief Creates blank image (nw*nh) that swaps with current image
			@param nw,nh New resoulution
		*/
		void resizeNN(uint16_t nw, uint16_t nh);

		/*
			@brief Concatenates source image to the 
				   right of the destination image
		*/
		void concat(const Image& source);

		void test();

		/*
			@brief Concatenates source image to the
				   right of the destination image
			@returns Image of two concatenated images
		*/
		static Image concat(const Image& destination, const Image& source);

		void operator=(const Image& origin);

	private:

		/*
			@brief Array of pixels, i.e. [r,g,b,r,g,b,...] or [r,g,b,a,r,g,b,a,...]
		*/
		uint8_t* data = NULL;
		/*
			@brief Size of an image
		*/
		size_t size = 0;
		/*
			@brief Image width
		*/
		int width;
		/*
			@brief Image height
		*/
		int height;
		/*
			@brief Image channels, i.e. channels = 4 represents each pixel as RGBA, 3 as RGB
		*/
		int channels;

		/*
			@brief Line on Y axis, representing line on what height letters should be drawn
		*/
		int baseline;

		/*
			@brief Advance height under baseline
		*/
		int advance_height;

	public:

		/*
			WIP. May be deprecated and removed.
			Optional function handler for math operators ("\sum", "\prod", etc.) and decorative functions("\color")
		*/
		std::optional<std::function<Image(const char*)>> handler;

};

struct Cyrillic {
	enum Capital {
		A = 1040,
		B,
		V,
		G,
		D,
		IE,
		YO = 1025,
		ZHE = 1046,
		Z,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		R,
		S,
		T,
		U,
		F,
		KH,
		CE,
		CHE,
		SH,
		SHCH,
		HARD_SIGN,
		YETA,
		SOFT_SIGN,
		E,
		YU,
		YA
	};
	enum Normal {
		a = 1072,
		b,
		v,
		g,
		d,
		ie,
		yo = 1105,
		zhe = 1078,
		z,
		i,
		j,
		k,
		l,
		m,
		n,
		o,
		p,
		r,
		s,
		t,
		u,
		f,
		kh,
		ce,
		che,
		sh,
		shch,
		hard_sign,
		yeta,
		soft_sign,
		e,
		yu,
		ya
	};
};

struct Greek {
	enum Capital {
		ALPHA = 913,
		BETA,
		GAMMA,
		DELTA,
		EPSILON,
		ZETA,
		ETA,
		THETA,
		IOTA,
		KAPPA,
		LAMBDA,
		MU,
		PI = 928,
		RHO,
		SIGMA,
		TAU,
		PHI = 934,
		PSI = 936,
		OMEGA
	};
	enum Normal {
		/*  */
		alpha = 945,
		beta,
		gamma,
		delta,
		epsilon,
		zeta,
		eta,
		theta,
		iota,
		kappa,
		lambda,
		mu,
		pi = 960,
		rho,
		tau,
		phi = 966,
		psi = 968,
		omega
	};
};

struct Math {
	//	
};

struct Color {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a; //-1 if none
};

struct Details {
	int width;
	int height;
	int channels;
	size_t size;
	int baseline;
};