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
	uint8_t operator[](int index) { return index == 0 ? r : (index == 1 ? g : (index == 2 ? b : (index == 3 ? a : 0))); };
	const uint8_t operator[](int index) const { return index == 0 ? r : (index == 1 ? g : (index == 2 ? b : (index == 3 ? a : 0))); }
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

		FontType m_Type;

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
			@param font Font that will be used
			@param txt Text to be ovelaid
			@param x,y Coordinates from which image will be overlaid
			@param r,g,b,a RGBA parameters (0-255)
		*/
		void overlayText(const Font& font, const std::string& txt, int x, int y, uint8_t r = 255, uint8_t g = 255, uint8_t b = 255, uint8_t a = 255);

		/*
			@brief Overlays text onto image
			@param font Font that will be used
			@param txt Text to be ovelaid
			@param x,y Coordinates from which image will be overlaid
			@param color Color struct with RGBA parameters (0-255)
		*/
		void overlayText(const Font& font, const std::string& txt, int x, int y, const Color& color);

		/*
			@brief Rasterizes text
			@param font Font that will be used
			@param txt Text to be rasterized
			@param r,g,b,a RGBA parameters (0-255)
		*/
		void rasterizeText(const Font& font, const std::string& txt, uint8_t r = 255, uint8_t g = 255, uint8_t b = 255, uint8_t a = 255);

		/*
			@brief Rasterizes text
			@param font Font that will be used
			@param txt Text to be rasterized
			@param color Color struct with RGBA parameters (0-255)
		*/
		void rasterizeText(const Font& font, const std::string& txt, const Color& color);

		/*
			@brief Rasterizes character
			@param font Font that will be used
			@param charCode Unicode character code
			@param r,g,b,a RGBA parameters (0-255)
		*/
		void rasterizeCharacter(const Font& font, unsigned long charCode, uint8_t r = 255, uint8_t g = 255, uint8_t b = 255, uint8_t a = 255);

		/*
			@brief Rasterizes character
			@param font Font that will be used
			@param charCode Unicode character code
			@param color Color struct with RGBA parameters (0-255)
		*/
		void rasterizeCharacter(const Font& font, unsigned long charCode, const Color& color = {255, 255, 255, 255});

		/*
			@brief Returns image of a rasterized char with requested height
			@param font Font to be used for getting character
			@param charCode Unicode code of an character
			@param height Requested height of a character
		*/
		Image requestChar(const Font& font, unsigned long charCode, int height);

		/*
			@brief Draws line on an image
			@param x0,y0 First position
			@param x1,y1 Second position
			@param r,g,b,a RGBA parameters (0-255)
		*/
		void drawLine(int x0, int y0, int x1, int y1, uint8_t r = 255, uint8_t g = 255, uint8_t b = 255, uint8_t a = 255);

		/*
			@brief Draws line on an image
			@param x0,y0 First position
			@param x1,y1 Second position
			@param color Color struct with RGBA parameters (0-255)
		*/
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
		void concat(const Image& source, ImagePosition position = ImagePosition::RIGHT, int space = 0);

		/*
			@brief Scales up image
			@param times Value by which image is scaled up by
		*/
		void scaleUp(int times);

		/*
			@brief Scales down image
			@param times Value by which image is scaled down by
		*/
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
		static Image concat(const Image& destination, const Image& source, ImagePosition position = ImagePosition::RIGHT, int space = 0);

		/*
			@brief Scales up image
			@param source Image to be scaled up
			@param times Value by which image is scaled up by
		*/
		static Image scaleUp(const Image& source, int times);

		/*
			@brief Scales down image
			@param source Image to be scaled down
			@param times Value by which image is scaled down by
		*/
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