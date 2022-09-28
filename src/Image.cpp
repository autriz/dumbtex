#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "Image.hpp"

#include <math.h>

Font::Font(const char* fontFile, uint16_t size) 
{
	if(!setFont(fontFile)) {
		printf("\e[31m[ERROR] Failed to load %s\e[0m\n", fontFile);
		return;
	}

	setSize(size);
}

Font::~Font()
{
	sft_freefont(m_SFT.font);
}

bool Font::setFont(const char* fontFile)
{
	if (m_SFT.font != NULL)
	{
		sft_freefont(m_SFT.font);
		m_SFT.font = NULL;
	}

	if((m_SFT.font = sft_loadfile(fontFile)) == NULL) {
		printf("\e[31m[ERROR] TTF font failed\e[0m\n");
		m_FontFile = "";
		return false;
	}

	m_FontFile = fontFile;

	return true;
}

void Font::setSize(uint16_t size) 
{
	m_SFT.xScale = size;
	m_SFT.yScale = size;
}

Image::Image(): m_Width(0), m_Height(0), m_Channels(0), m_Size(0), m_Baseline(m_Height), m_AdvanceHeight(0), m_Data(NULL) { };

Image::Image(int w, int h, int channels) : m_Width(w), m_Height(h), m_Channels(channels), m_Baseline(h)
{
	this->m_Size = this->m_Width * this->m_Height * this->m_Channels;
	this->m_AdvanceHeight = 0;
	this->m_Data = new uint8_t[this->m_Size];

	memset(this->m_Data, 0, this->m_Size);
}

Image::Image(const Image &other) : Image(other.m_Width, other.m_Height, other.m_Channels)
{
	this->m_Baseline = other.m_Baseline;
	this->m_AdvanceHeight = other.m_AdvanceHeight;

	memcpy(m_Data, other.m_Data, m_Size);
}

Image::~Image()
{
	delete[] m_Data;
}

bool Image::write(const char *filename)
{
	return write(filename, getFileType(filename));
}

bool Image::write(const char *filename, ImageType type)
{
	PROFILE_SCOPE("Image::write");

	int success;

	switch (type)
	{
		case PNG:
			success = stbi_write_png(filename, m_Width, m_Height, m_Channels, m_Data, m_Width * m_Channels);
			break;
		case BMP:
			success = stbi_write_bmp(filename, m_Width, m_Height, m_Channels, m_Data);
			break;
		case JPG:
			success = stbi_write_jpg(filename, m_Width, m_Height, m_Channels, m_Data, 100);
			break;
		case TGA:
			success = stbi_write_tga(filename, m_Width, m_Height, m_Channels, m_Data);
			break;
	}

	if (!success)
	{
		// printf("\e[31;1m Failed to write \e[36m%s\e[0m, %d, %d, %d, %zu\n", filename, m_Width, m_Height, m_Channels, m_Size);
		return false;
	}
	
	return true;
}

ImageType Image::getFileType(const char *filename)
{
	const char *ext = strrchr(filename, '.');

	if (ext != nullptr)
	{
		if (strcmp(ext, ".png") == 0)
			return ImageType::PNG;
		else if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0)
			return ImageType::JPG;
		else if (strcmp(ext, ".bmp") == 0)
			return ImageType::BMP;
		else if (strcmp(ext, ".tga") == 0)
			return ImageType::TGA;
		else
			return ImageType::UNKNOWN;
	}

	return ImageType::NO_TYPE;
}

Details Image::getDetails() const
{
	return { m_Width, m_Height, m_Channels, m_Baseline, m_Size };
}

bool Image::isEmpty() const
{
	return m_Size == 0;
};

void Image::colorMask(float r, float g, float b)
{
	if (m_Channels < 3)
		printf("\e[31m[ERROR] Color mask requires at least 3 channels, but this image has %d channels\e[0m\n", m_Channels);
	else
		for (int i = 0; i < m_Size; i += m_Channels)
		{
			m_Data[i] *= r;
			m_Data[i + 1] *= g;
			m_Data[i + 2] *= b;
		}
}

void Image::gradient(const Color& startColor, const Color& stopColor)
{
	uint8_t r, g, b;
	uint8_t* px;
	float n, alpha;

	for (int x = 0; x < m_Width; ++x)
	{
		n = (float)x / (m_Width - 1);
		r = (float)startColor.r * (1.0f - n) + (float)stopColor.r * n;
		g = (float)startColor.g * (1.0f - n) + (float)stopColor.g * n;
		b = (float)startColor.b * (1.0f - n) + (float)stopColor.b * n;

		for (int y = 0; y < m_Height; ++y)
		{
			px = &m_Data[(x + y * m_Width) * m_Channels];

			px[0] = r;
			px[1] = g;
			px[2] = b;
		}
	}
}

void Image::flip(AXIS axis)
{
	PROFILE_SCOPE("Image::flip");

	uint8_t tmp[4];
	uint8_t *px1;
	uint8_t *px2;

	if (axis == AXIS::Y)
		for (int y = 0; y < m_Height; ++y)
			for (int x = 0; x < m_Width / 2; ++x)
			{
				px1 = &m_Data[(x + y * m_Width) * m_Channels];
				px2 = &m_Data[((m_Width - 1 - x) + y * m_Width) * m_Channels];

				memcpy(tmp, px1, m_Channels);
				memcpy(px1, px2, m_Channels);
				memcpy(px2, tmp, m_Channels);
			}
	else
		for (int x = 0; x < m_Width; ++x)
			for (int y = 0; y < m_Height / 2; ++y)
			{
				px1 = &m_Data[(x + y * m_Width) * m_Channels];
				px2 = &m_Data[(x + (m_Height - 1 - y) * m_Width) * m_Channels];

				memcpy(tmp, px1, m_Channels);
				memcpy(px1, px2, m_Channels);
				memcpy(px2, tmp, m_Channels);
			}
}

void Image::bilInterp(float oldRow, float oldCol, int *row, int *col)
{
	// // The Q values represent the 4 pixels nearest the original pixel.
	// // The result values are averages of the Q values.
	// // P is the NEW intensity value that is returned.

	// double Q11, Q12, Q21, Q22, P, result1, result2 = 0.0;
	// int R1, R2, C1, C2 = 0;

	// // Ex:
	// // oldRow,oldCol = 180.6,60.4
	// // R1,C1 = 180,60
	// // R2,C2 = 181,61
	// // We need to cast to int since
	// // these are actually pixel coordinates
	
	// R1 = static_cast<int>(floor(oldRow));
	// R2 = static_cast<int>(ceil(oldRow));
	// C1 = static_cast<int>(floor(oldCol));
	// C2 = static_cast<int>(ceil(oldCol));

	// // If the R,C (row, column) values are outside of the old image after,
	// // applying the rotation equations, then we set them to a default
	// // intensity, otherwise we get the pixel's neighbouring 4 values.
	// // 
	// // Imagine we rotate 45 degrees on a rectangular image
	// // and do the min row,col shifting in the inverse 
	// // mapping function, then we'll have an image where:
	// //		new image width > old image width
	// //		new image height < old image height
	// // So some value in the new image will need to be set to default
	// // after rotation.

	// if (((R1 < 0) || (!(R1 < m_Height))) || 
	// 	((C1 < 0) || (!(C1 < m_Width))))
	// 	Q11 = DEFAULTINTENSITY;
	// else
	// 	Q11 = *(oldArray[R1][C1]);
	
	// if (((R1 < 0) || (!(R1 < m_Height))) ||
	// 	((C2 < 0) || (!(C2 < m_Width))))
	// 	Q12 = DEFAULTINTENSITY;
	// else
	// 	Q12 = *(oldArray[R1][C2]);

	// if (((R2 < 0) || (!(R2 < m_Height))) ||
	// 	((C1 < 0) || (!(C1 < m_Width))))
	// 	Q21 = DEFAULTINTENSITY; 
	// else
	// 	Q21 = *(oldArray[R2][C1]);

	// if (((R2 < 0) || (!(R2 < m_Height))) ||
	// 	((C2 < 0) || (!(C2 < m_Width))))
	// 	Q22 = DEFAULTINTENSITY;
	// else
	// 	Q22 = *(oldArray[R2][C2]);

	// // Now we will do the actual bilinear interpolation part!
	// // We should check for division by 0 too!

	// if ((R2 - R1) == 0)
	// {
	// 	result1 = 0.0;
	// 	result2 = 0.0;
	// }
	// else
	// {	
	// 	// Calculate weightings
	// 	result1 = ((R2 - oldRow)/(R2 - R1))*Q11 
	// 		+ ((oldRow - R1)/(R2 - R1))*Q21;
	// 	result2 = ((R2 - oldRow)/(R2 - R1))*Q12 
	// 		+ ((oldRow - R1)/(R2 - R1))*Q22;
	// }

	// if ((C2 - C1) == 0)
	// {
	// 	P = 0.0;
	// }
	// else
	// {
	// 	P = ((C2 - oldCol)/(C2 - C1))*result1
	// 	+ ((oldCol - C1)/(C2 - C1))*result2;
	// }

	// // Assign the new array at r,c the interpolated intensity!
	// *(rotatedArray[row][col]) = P;
}

void Image::rotate(double degrees)
{
	PROFILE_SCOPE("Image::rotate");
	// sorta works with 90-180-270-360 degrees
	//!Non-orthogonal rotation results in "holes" in image
	// implement image resizing for rotation
	// for example, rotate 4 edge pixels (0,0) (1,0) (0,1) (1,1) to degrees 
	// and get the highest x and y and use them as width and height

	if ((degrees < 0.001f && degrees > -0.001f) || degrees > 359.999f) // literally wont move
		return;

	if (degrees < -0.001f)
		degrees += 360.0f;

	auto roundoff = [](double value, unsigned char precision)->double
	{
		double pow_10 = std::pow(10.0f, (double)precision);
		return std::round(value * pow_10) / pow_10;
	};

	double x0 = (double)(m_Width / 2);
	double y0 = (double)(m_Height / 2);
	double tempX = 0.0f;
	double tempY = 0.0f;
	double rad = degrees * ((std::atan(1) * 4) / 180);
	double s = roundoff(std::sin(rad * 1.0f), 6);
	double c = roundoff(std::cos(rad * 1.0f), 6);

	uint8_t *dstPx, *srcPx;
	Image rotatedImg(m_Width, m_Height, m_Channels);

	printf("%f, %f\n", s, c);	
	for (int y = 0; y < m_Height; ++y)
	{
		double y1 = (double)y - y0;

		for(int x = 0; x < m_Width; ++x)
		{
				//slices off first row and first column
				double x1 = (double)x - x0;

				double x2 = roundoff(((c*x1) - (s*y1)) + x0, 1);
				double y2 = roundoff(((s*x1) + (c*y1)) + y0, 1);

				tempX = x2;
				tempY = y2;

				//use bilinear interpolation to fix missing pixels
				//or nearest neighbor interpolation with rounding towards 0 (flooring)

				if ((int)x2 >= 0 && (int)x2 < rotatedImg.m_Width && (int)y2 >= 0 && (int)y2 < rotatedImg.m_Height)
				{
					dstPx = &rotatedImg.m_Data[((int)x2 + (int)y2 * rotatedImg.m_Width) * m_Channels];
					srcPx = &m_Data[((int)x + (int)y * m_Width) * m_Channels];

					memcpy(dstPx, srcPx, m_Channels);
				}
		}
	}

	*this = rotatedImg;

}

void Image::overlay(const Image &source, int x, int y)
{
	//? I think that algo could be optimized
	//? Instead of iteration through each color, I could just smash row with memcpy()
	PROFILE_SCOPE("Image::overlay");
	#ifdef DEBUG
		printf("[Image::overlay] x: %d, y: %d\n", x, y);
	#endif

	uint8_t *srcPx;
	uint8_t *dstPx;

	for (int sy = 0; sy < source.m_Height; ++sy)
	{
		if (sy + y < 0)
			continue;
		else if (sy + y >= m_Height)
			break;
		for (int sx = 0; sx < source.m_Width; ++sx)
		{
			if (sx + x < 0)
				continue;
			else if (sx + x >= m_Width)
				break;

			srcPx = &source.m_Data[(sx + sy * source.m_Width) * source.m_Channels];
			dstPx = &m_Data[((sx + x) + (sy + y) * m_Width) * m_Channels];

			float srcAlpha = source.m_Channels < 4 ? 1 : srcPx[3] / 255.f;
			float dstAlpha = m_Channels < 4 ? 1 : dstPx[3] / 255.f;

			if (srcAlpha > .99 && dstAlpha > .99)
			{
				if (source.m_Channels >= m_Channels)
					memcpy(dstPx, srcPx, m_Channels);
				else
					// In case our source image is grayscale and the dest one isn't
					memset(dstPx, srcPx[0], m_Channels);
			}
			else
			{
				float outAlpha = srcAlpha + dstAlpha * (1 - srcAlpha);

				if (outAlpha < .01)
					memset(dstPx, 0, m_Channels);
				else
				{
					for (int chnl = 0; chnl < m_Channels; ++chnl)
						dstPx[chnl] = (uint8_t)BYTE_BOUND((srcPx[chnl] / 255.f * srcAlpha + dstPx[chnl] / 255.f * dstAlpha * (1 - srcAlpha)) / outAlpha * 255.f);
					
					if (m_Channels > 3)
						dstPx[3] = (uint8_t)BYTE_BOUND(outAlpha * 255.f);
				}
			}
		}
	}
}

void Image::overlayText(const std::string& txt, const Font& font, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	size_t len = txt.length();
	SFT_Char c;
	int32_t dx, dy;
	uint8_t *dstPx;
	uint8_t srcPx;
	uint8_t color[4] = {r, g, b, a};

	for (size_t i = 0; i < len; ++i)
	{
		if (sft_char(&font.m_SFT, txt[i], &c) != 0)
		{
			printf("\e[31m[ERROR] Font is missing character '%c'\e[0m\n", txt[i]);

			continue;
		}

		for (uint16_t sy = 0;sy < c.height;++sy) 
		{
			dy = sy + y + c.y;

			if(dy < 0) 
				continue;
			else if(dy >= m_Height) 
				break;

			for(uint16_t sx = 0;sx < c.width;++sx) 
			{
				dx = sx + x + c.x;

				if(dx < 0) 
					continue;
				else if(dx >= m_Width) 
					break;

				dstPx = &m_Data[(dx + dy * m_Width) * m_Channels];
				srcPx = c.image[sx + sy * c.width];

				if(srcPx != 0) 
				{
					float srcAlpha = (srcPx / 255.f) * (a / 255.f);
					float dstAlpha = m_Channels < 4 ? 1 : dstPx[3] / 255.f;

					if(srcAlpha > .99 && dstAlpha > .99) 
						memcpy(dstPx, color, m_Channels);
					else 
					{
						float outAlpha = srcAlpha + dstAlpha * (1 - srcAlpha);

						if(outAlpha < .01) 
							memset(dstPx, 0, m_Channels);
						else {
							for(int chnl = 0; chnl < m_Channels; ++chnl) 
								dstPx[chnl] = (uint8_t)BYTE_BOUND((color[chnl]/255.f * srcAlpha + dstPx[chnl]/255.f * dstAlpha * (1 - srcAlpha)) / outAlpha * 255.f);
							
							if(m_Channels > 3) 
								dstPx[3] = (uint8_t)BYTE_BOUND(outAlpha * 255.f);
						}
					}
				}
			}
		}

		x += c.advance;
		free(c.image);
	}
}

void Image::overlayText(const std::string& txt, const Font &font, int x, int y, const Color& color = {255, 255, 255, 255})
{
	overlayText(txt, font, x, y, color.r, color.g, color.b, color.a);
}

void Image::rasterizeText(const std::string& txt, const Font& font, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	PROFILE_SCOPE("Image::rasterizeText");

	size_t len = txt.length();
	SFT_Char c;

	if (&font.m_SFT == NULL) return;

	for (int i = 0; i < len; i++)
	{
		// if (font.m_SFT.font == NULL) throw Latex::ConversionException("", __FILE__, __LINE__);
		if (sft_char(&font.m_SFT, txt[i], &c) != 0)
		{
			printf("\e[31m[ERROR] Font is missing character '%c'\e[0m\n", txt[i]);
			continue;
		};

		#ifdef DEBUG
			printf("[Image::rasterizeText] char: %c, x: %d, y: %d, width: %d, height: %d, advance: %d\n", txt[i], c.x, c.y, c.width, c.height, c.advance);
		#endif

		Image character(c.width + (c.advance < c.width ? 0 : (c.advance - c.width)), c.height, 4);
		character.m_Baseline = std::abs(c.y - 1);
		character.m_AdvanceHeight = c.height - character.m_Baseline;

		handleRaster(font, character, c, r, g, b, a);
		
		if (!character.isEmpty())
			concat(character);

		free(c.image); //free anyway
	}
}

void Image::rasterizeText(const std::string& txt, const Font& font, const Color& color = {255, 255, 255, 255})
{
	rasterizeText(txt, font, color.r, color.g, color.b, color.a);
}

void Image::rasterizeCharacter(const unsigned long charCode, const Font& font, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	PROFILE_SCOPE("Image::rasterizeCharacter");

	SFT_Char c;

	if (&font.m_SFT == NULL) return;

	if (sft_char(&font.m_SFT, charCode, &c) != 0)
	{
		printf("\e[31m[ERROR] Font is missing character '%c'\e[0m\n", charCode);

		return;
	}

	#ifdef DEBUG
		printf("[Image::rasterizeCharacter] char: %c, x: %d, y: %d, width: %d, height: %d, advance: %d\n\n", charCode, c.x, c.y, c.width, c.height, c.advance);
	#endif

	Image character(c.width + (c.advance < c.width ? 0 : (c.advance - c.width)), (c.width == 0 && c.advance != 0) ? 1 : c.height, 4);
	character.m_Baseline = std::abs(c.y - 1);
	character.m_AdvanceHeight = c.height - character.m_Baseline;

	if (character.m_AdvanceHeight < 0)
		character.m_AdvanceHeight = 0;

	handleRaster(font, character, c, r, g, b, a);
	
	if (!character.isEmpty())
		concat(character);

	free(c.image); //free anyway
}

void Image::rasterizeCharacter(const unsigned long charCode, const Font& font, const Color& color = {255, 255, 255, 255})
{
	rasterizeCharacter(charCode, font, color.r, color.g, color.b, color.a);
}

void Image::handleRaster(const Font& font, Image& chr, SFT_Char& c, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	PROFILE_SCOPE("Image::handleRaster");

	int32_t dx, dy;
	uint8_t *dstPx;
	uint8_t srcPx;
	uint8_t color[4] = {r, g, b, a};

	for (int32_t sy = 0; sy < c.height; ++sy)
	{
		dy = sy + chr.m_Height + c.y - (chr.m_Height - std::abs(c.y + 1));
		
		if (dy < 0)
			continue;
		else if (dy >= chr.m_Height)
			break;

		for (int32_t sx = 0; sx < c.width; ++sx)
		{
			dx = sx + c.x;

			if (dx < 0)
				continue;
			else if (dx >= chr.m_Width)
				break;

			dstPx = &(chr.m_Data[(dx + dy * chr.m_Width) * chr.m_Channels]);
			srcPx = c.image[sx + sy * c.width];

			if (srcPx != 0)
			{
				float srcAlpha = (srcPx / 255.f) * (a / 255.f);
				float dstAlpha = chr.m_Channels < 4 ? 1 : dstPx[3] / 255.f;

				if (srcAlpha > .99 && dstAlpha > .99)
					memcpy(dstPx, color, chr.m_Channels);
				else
				{
					float outAlpha = srcAlpha + dstAlpha * (1 - srcAlpha);

					if (outAlpha < .01)
						memset(dstPx, 0, chr.m_Channels);
					else
					{
						for (int chnl = 0; chnl < chr.m_Channels; ++chnl)
							dstPx[chnl] = (uint8_t)BYTE_BOUND((color[chnl] / 255.f * srcAlpha + dstPx[chnl] / 255.f * dstAlpha * (1 - srcAlpha)) / outAlpha * 255.f);
						
						if (chr.m_Channels > 3)
							dstPx[3] = (uint8_t)BYTE_BOUND(outAlpha * 255.f);
					}
				}
			}
		}
	}
}

void Image::crop(uint16_t cx, uint16_t cy, uint16_t cw, uint16_t ch)
{
	m_Size = cw * ch * m_Channels;

	uint8_t *croppedImage = new uint8_t[m_Size];
	memset(croppedImage, 0, m_Size);

	for (uint16_t y = 0; y < ch; ++y)
	{
		if (y + cy >= m_Height)
			break;

		for (uint16_t x = 0; x < cw; ++x)
		{
			if (x + cx >= m_Width)
				break;

			memcpy(&croppedImage[(x + y * cw) * m_Channels], &m_Data[(x + cx + (y + cy) * m_Width) * m_Channels], m_Channels);
		}
	}

	m_Width = cw;
	m_Height = ch;

	delete[] m_Data;
	m_Data = croppedImage;
	croppedImage = nullptr;
}

Image Image::cropCopy(uint16_t cx, uint16_t cy, uint16_t cw, uint16_t ch) 
{
	Image imgCopy(*this);
	imgCopy.crop(cx, cy, cw, ch);

	return imgCopy;
}

void Image::resize(uint16_t nw, uint16_t nh) 
{

};

Image Image::resizeCopy(uint16_t nw, uint16_t nh) 
{
	Image imgCopy(*this);
	imgCopy.resize(nw, nh);

	return imgCopy;
};

void Image::resizeNN(uint16_t nw, uint16_t nh)
{
	uint16_t sx, sy;
	
	m_Size = nw * nh * m_Channels;
	uint8_t *newImage = new uint8_t[m_Size];

	float scaleX = (float)nw / (m_Width);
	float scaleY = (float)nh / (m_Height);

	for (uint16_t y = 0; y < nh; ++y)
	{
		sy = (uint16_t)(y / scaleY);

		for (uint16_t x = 0; x < nw; ++x)
		{
			sx = (uint16_t)(x / scaleX);

			memcpy(&newImage[(x + y * nw) * m_Channels], &m_Data[(sx + sy * m_Width) * m_Channels], m_Channels);
		}
	}

	m_Width = nw;
	m_Height = nh;

	delete[] m_Data;
	m_Data = newImage;
	newImage = nullptr;
};

void Image::concat(const Image& image, ImagePosition position)
{
	PROFILE_SCOPE("Image::concat");

	if (this->isEmpty() && !image.isEmpty())
		*this = image;
	else if (!image.isEmpty())
		*this = Image::concat(*this, image, position);
};

Image Image::concat(const Image& left, const Image& right, ImagePosition position)
{
	PROFILE_SCOPE("static Image::concat");

	int new_w, new_h, new_adv_h, new_baseline, new_channels;

	new_channels = left.m_Channels > right.m_Channels ? left.m_Channels : right.m_Channels;

	//?if possible, add some sort of padding between images

	switch (position)
	{
		case ImagePosition::RIGHT:
		case ImagePosition::LEFT:
		default:
			new_w = left.m_Width + right.m_Width;
			new_h = (left.m_Height - left.m_AdvanceHeight) > (right.m_Height - right.m_AdvanceHeight) ? left.m_Height : right.m_Height;
			new_adv_h = left.m_AdvanceHeight > right.m_AdvanceHeight ? left.m_AdvanceHeight : right.m_AdvanceHeight;
			// new_h += ((left.m_AdvanceHeight < right.m_AdvanceHeight) && (left.isEmpty() == false)) ? right.m_AdvanceHeight - left.m_AdvanceHeight : 0;
			new_baseline = left.m_Baseline > right.m_Baseline ? left.m_Baseline : right.m_Baseline;

			if ((new_h - new_adv_h) < (right.m_Height - right.m_AdvanceHeight)) new_h += (right.m_Height - (new_h - new_adv_h));
			if (new_baseline != new_h - new_adv_h) new_h += new_baseline - (new_h - new_adv_h);
			if (new_baseline != new_h - new_adv_h + 1) new_h += new_baseline - (new_h - new_adv_h + 1);
			break;
		case ImagePosition::TOP:
		case ImagePosition::BOTTOM:
			new_w = left.m_Width > right.m_Width ? left.m_Width : right.m_Width;
			new_h = left.m_Height + right.m_Height;
			new_adv_h = 0;
			new_baseline = new_h - new_adv_h;
			break;
	}

	#ifdef DEBUG
		printf("[Image::concat] left.width = %d, right.width = %d\n", left.m_Width, right.m_Width);
		printf("[Image::concat] left.height = %d, right.height = %d\n", left.m_Height, right.m_Height);
		printf("[Image::concat] left.adv_height = %d, right.adv_height = %d\n", left.m_AdvanceHeight, right.m_AdvanceHeight);
		printf("[Image::concat] left.baseline = %d, right.baseline = %d\n", left.m_Baseline, right.m_Baseline);
		printf("[Image::concat] new_h - new_adv_h = %d\n", (new_h - new_adv_h));
		printf("[Image::concat] right.height - right.advance_height = %d, left.height - left.advance_height = %d\n", (right.m_Height - right.m_AdvanceHeight), (left.m_Height - left.m_AdvanceHeight));
		printf("[Image::concat] new height = %d, new width = %d, new adv height = %d, new baseline = %d\n", new_h, new_w, new_adv_h, new_baseline);		
	#endif

	Image newImage(new_w, new_h, new_channels);
	newImage.m_AdvanceHeight = new_adv_h;
	newImage.m_Baseline = new_baseline;
	
	#ifdef DEBUG
		printf("[Image::concat] newImage height: %d, newImage width: %d, newImage adv height: %d, newImage baseline: %d\n\n", newImage.m_Height, newImage.m_Width, newImage.m_AdvanceHeight, newImage.m_Baseline);
	#endif

	switch(position)
	{
		case ImagePosition::RIGHT:
		default:
			#ifdef DEBUG
				printf("[Image::concat] left side concatenation\n");
			#endif
			newImage.overlay(left, 0, newImage.m_Baseline > 0 ? newImage.m_Baseline - left.m_Baseline : 0); 
			#ifdef DEBUG
				printf("[Image::concat] right side concatenation\n");
			#endif
			newImage.overlay(right, left.m_Width, newImage.m_Baseline > 0 ? newImage.m_Baseline - right.m_Baseline : 0); 
			break;
		case ImagePosition::LEFT:
			#ifdef DEBUG
				printf("[Image::concat] right side concatenation\n");
			#endif
			newImage.overlay(right, 0, newImage.m_Baseline > 0 ? newImage.m_Baseline - right.m_Baseline : 0);
			#ifdef DEBUG
				printf("[Image::concat] left side concatenation\n");
			#endif
			newImage.overlay(left, right.m_Width, newImage.m_Baseline > 0 ? newImage.m_Baseline - right.m_Baseline : 0);
			break;
		case ImagePosition::TOP:
			#ifdef DEBUG
				printf("[Image::concat] right side concatenation\n");
			#endif
			newImage.overlay(right, 0, 0);
			#ifdef DEBUG
				printf("[Image::concat] left side concatenation\n");
			#endif
			newImage.overlay(left, 0, right.m_Height);
			break;
		case ImagePosition::BOTTOM:
			#ifdef DEBUG
				printf("[Image::concat] left side concatenation\n");
			#endif
			newImage.overlay(left, 0, 0);
			#ifdef DEBUG
				printf("[Image::concat] right side concatenation\n");
			#endif
			newImage.overlay(right, 0, left.m_Height);
			break;
	}

	#ifdef DEBUG
		printf("\n");
	#endif

	return newImage;
};

void Image::operator=(const Image& origin) {
	if (origin.isEmpty()) 
		return;

	m_Width = origin.m_Width;
	m_Height = origin.m_Height;
	m_AdvanceHeight = origin.m_AdvanceHeight;
	m_Channels = origin.m_Channels;
	m_Baseline = origin.m_Baseline;
	m_Size = origin.m_Size;

	delete[] m_Data;
	m_Data = new uint8_t[m_Size];

	memcpy(m_Data, origin.m_Data, m_Size);
};