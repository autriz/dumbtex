#pragma once

#define BYTE_BOUND(value) value < 0 ? 0 : (value > 255 ? 255 : value)

# define M_PI 3.14159265358979323846 

#include "schrift.h"
#include "Profiler.hpp"

#include <functional>
#include <vector>
#include <string>
#include <cmath>

enum ImageType { UNKNOWN = -2, NO_TYPE = -1, PNG, JPG, BMP, TGA };

enum ImagePosition { LEFT, RIGHT, TOP, BOTTOM };

enum FontType { Normal, Italic, Bold, BoldItalic };

enum AXIS { X, Y };

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

class Font {

	public:

		/*
			@brief Constructs Font instance
		*/
		Font() = default;

		/*
			@brief Constructs Font instance
			@param fontFile Path to font
			@param size Font size
		*/
		Font(const char* fontFile, uint16_t size);

		~Font();

		/*
			@brief Sets font file
			@param fontFile Font file
		*/
		bool setFont(const char* fontFile);

		/*
			@brief Sets font size
			@param size Font size
		*/
		void setSize(uint16_t size);

	public:

		const char* m_FontFile;

		FontType type;

		SFT m_SFT = {NULL, 12, 12, 0, 0, SFT_DOWNWARD_Y};

};

class Image {
	
	public:

		/* Blank constructor */
		Image();

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

		/*
			@brief Check if image instance is blank
		*/
		bool isEmpty() const;

		/**/
		void colorMask(float r, float g, float b);

		/* Overlay linear gradient onto image */
		void gradient(const Color& startColor, const Color& stopColor);

		/*
			@brief Flip image by selected axis
			@param axis Axis to flip by, X or Y
		*/
		void flip(AXIS axis);

		/*
			@brief Rotate image by arbitrary degree. For now works only 90-180-270-360 degrees.
		*/
		void rotate(double degrees);

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
		void rasterizeCharacter(const unsigned long charCode, const Font& font, const Color& color = {255, 255, 255, 255});

		void drawLine(int x0, int y0, int x1, int y1, uint8_t r = 255, uint8_t g = 255, uint8_t b = 255, uint8_t a = 255);

		void drawLine(int x0, int y0, int x1, int y1, const Color& color = {255, 255, 255, 255});

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

		void scaleUp(int times);

		void scaleDown(int times);

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

		static Image scaleUp(const Image& source, int times);

		static Image scaleDown(const Image& source, int times);

		void operator=(const Image& origin);

		/*Friend declarations*/

		friend struct Handlers;

	private:

		/*
			@brief Handle rasterization of a character. Freeing of an image is on a user
			@param font Font to use for rasterization
			@param chr Image pointer
			@param c SFT_Char struct containing character data
			@param r,g,b,a RGBA params (0-255)
		*/
		void handleRaster(const Font& font, Image& chr, SFT_Char& c, uint8_t r = 255, uint8_t g = 255, uint8_t b = 255, uint8_t a = 255);

	private:

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

		/*
			@brief Size of an image
		*/
		size_t m_Size = 0;

		/*
			@brief Array of pixels, i.e. [r,g,b,r,g,b,...] or [r,g,b,a,r,g,b,a,...]
		*/
		uint8_t* m_Data = NULL;

};