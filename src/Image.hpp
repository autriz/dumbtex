#pragma once

#define BYTE_BOUND(value) value < 0 ? 0 : (value > 255 ? 255 : value)

#include "schrift.h"

#include "Profiler.hpp"

#include <vector>
#include <string>

#include <functional>

enum ImageType { UNKNOWN = -2, NO_TYPE = -1, PNG, JPG, BMP, TGA };

enum ImagePosition { LEFT, RIGHT, TOP, BOTTOM };

enum AXIS { X, Y };

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
		
		/* 
			@brief Gets image from file 
			@param filename File path
			@param channel_force Forced number of color channels (default - 0, that is image's color channels will be applied)
		*/
		Image(const char* filename, int channel_force = 0);

		/* 
			@brief Image creation constructor
			@param w Image width
			@param h Image height
			@param channels Color channels (default - 3, that is RGB) 
		*/
		Image(int w, int h, int channels = 3);

		/* Copy constructor */
		Image(const Image& other);

		/* Deconstructor */
		~Image();

		/* 
			@brief Read image from file 
			@param filename path to file
			@param channel_force forced number of channels (default - 0, that is it won't be forced)
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
			@brief Write image to file. Use only if image type already has been defined
			@param filename path to file
			@return true if image written successfully, false if not
		*/
		bool write(const char* filename, ImageType type);

		/*
			@brief Get image details
			@return image details (width, height, channels, size, etc.)
		*/
		Details getDetails() const;

		/**/
		void colorMask(float r, float g, float b);

		/*
			@brief Flip image by selected axis
			@param axis Axis to flip by, X or Y
		*/
		void flip(AXIS axis);

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
		void overlayText(const std::string& txt, const Font& font, int x, int y, uint8_t r = 255, uint8_t g = 255, uint8_t b = 255, uint8_t a = 255);

		/*
			@brief Overlays text onto image
			@param txt Text to be ovelaid
			@param font Font that will be used
			@param x,y Coordinates from which image will be overlaid
			@param color Color struct with RGBA parameters (0-255)
		*/
		void overlayText(const std::string& txt, const Font& font, int x, int y, const Color& color);

		/*
			@brief Rasterizes text
			@param txt Text to be rasterized
			@param font Font that will be used
			@param r,g,b,a RGBA parameters (0-255)
		*/
		void rasterizeText(const std::string& txt, const Font& font, uint8_t r = 255, uint8_t g = 255, uint8_t b = 255, uint8_t a = 255);

		/*
			@brief Rasterizes text
			@param txt Text to be rasterized
			@param font Font that will be used
			@param color Color struct with RGBA parameters (0-255)
		*/
		void rasterizeText(const std::string& txt, const Font& font, const Color& color);

		/*
			@brief Rasterizes character
			@param charCode Unicode character code
			@param font Font that will be used
			@param r,g,b,a RGBA parameters (0-255)
		*/
		void rasterizeCharacter(const unsigned long charCode, const Font& font, uint8_t r = 255, uint8_t g = 255, uint8_t b = 255, uint8_t a = 255);

		/*
			@brief Rasterizes character
			@param charCode Unicode character code
			@param font Font that will be used
			@param color Color struct with RGBA parameters (0-255)
		*/
		void rasterizeCharacter(const unsigned long charCode, const Font& font, const Color& color);

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
		void concat(const Image& source, ImagePosition position = ImagePosition::RIGHT);

		/* 
			@brief Get file type 
			@param filename path to file
			@return file type
		*/
		static ImageType getFileType(const char* filename);

		/*
			@brief Concatenates source image to the right of 
				   the destination image.

			@returns Image of two concatenated images
		*/
		static Image concat(const Image& destination, const Image& source, ImagePosition position = ImagePosition::RIGHT);

		void operator=(const Image& origin);

	private:

		/*
			@brief Handle rasterization of a character. Freeing of an image is on a user
			@param font Font to use for rasterization
			@param x,y Position on image
			@param chr Image pointer
			@param c SFT_Char struct containing character data
			@param r,g,b,a RGBA params (0-255)
		*/
		void handleRaster(const Font& font, int x, int y,  SFT_Char& c, uint8_t r = 255, uint8_t g = 255, uint8_t b = 255, uint8_t a = 255);

		/*
			@brief Handle rasterization of a character. Freeing of an image is on a user
			@param font Font to use for rasterization
			@param chr Image pointer
			@param c SFT_Char struct containing character data
			@param r,g,b,a RGBA params (0-255)
		*/
		void handleRaster(const Font& font, Image& chr, SFT_Char& c, uint8_t r = 255, uint8_t g = 255, uint8_t b = 255, uint8_t a = 255);

	public:

		/*
			WIP. May be deprecated and removed.
			Optional function handler for math operators ("\sum", "\prod", etc.) and decorative functions("\color")
		*/
		std::function<Image(const char*)> handler;

	private:

		/*
			@brief Array of pixels, i.e. [r,g,b,r,g,b,...] or [r,g,b,a,r,g,b,a,...]
		*/
		uint8_t* m_Data = NULL;
		/*
			@brief Size of an image
		*/
		size_t m_Size = 0;
		/*
			@brief Image width
		*/
		int m_Width;
		/*
			@brief Image height
		*/
		int m_Height;
		/*
			@brief Image channels, i.e. channels = 4 represents each pixel as RGBA, 3 as RGB
		*/
		int m_Channels;

		/*
			@brief Line on Y axis, representing line on what height letters should be drawn
		*/
		int m_Baseline;

		/*
			@brief Advance height under baseline
		*/
		int m_AdvanceHeight;

};

//Maybe switch to static struct arrays

struct Letter
{
	const char* character;
	uint16_t charCode;
};

static Letter cyr_table[] = 
{
	{"A", 1040}, {"B", 1041}, {"V", 1042}, {"D", 1043},
	{"IE", 1044}, {"YO", 1025}, {"ZH", 1046}, {"Z", 1047},
	{"I", 1048}, {"J", 1049}, {"K", 1050}, {"L", 1051},
	{"M", 1052}, {"N", 1053}, {"O", 1054}, {"P", 1055},
	{"R", 1056}, {"S", 1057}, {"T", 1058}, {"U", 1059},
	{"F", 1060}, {"KH", 1061}, {"TS", 1062}, {"CH", 1063},
	{"SH", 1064}, {"SHCH", 1065}, {"\\Cdprime", 1066}, {"\\Yeta", 1067},
	{"\\Cprime", 1068}, {"E", 1069}, {"YU", 1070}, {"YA", 1071},
	{"a", 1072}, {"b", 1073}, {"v", 1074}, {"g", 1075},
	{"d", 1076}, {"ie", 1077}, {"yo", 1105}, {"zh", 1078},
	{"z", 1079}, {"i", 1080}, {"j", 1081}, {"k", 1082},
	{"l", 1083}, {"m", 1084}, {"n", 1085}, {"o", 1086},
	{"p", 1087}, {"r", 1088}, {"s", 1089}, {"t", 1090},
	{"u", 1091}, {"f", 1092}, {"kh", 1093}, {"ts", 1094},
	{"ch", 1095}, {"sh", 1096}, {"shch", 1097}, {"\\cdprime", 1098},
	{"\\yeta", 1099}, {"\\cprime", 1100}, {"e", 1101}, {"yu", 1102},
	{"ya", 1103}
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
	int baseline;
	size_t size;
};