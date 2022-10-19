#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "Image.hpp"

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

Image::Image(): m_Width(0), m_Height(0), m_Channels(0), m_Baseline(m_Height), m_AdvanceHeight(0), m_Size(0), m_Data(NULL) { };

Image::Image(int w, int h, int channels) : m_Width(w), m_Height(h), m_Channels(channels), m_Baseline(h)
{
	this->m_AdvanceHeight = 0;
	this->m_Size = this->m_Width * this->m_Height * this->m_Channels;
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
		default:
			success = 0;
			break;
	}

	return success;
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
		printf("[Image::colorMask] \e[31m[ERROR] Color mask requires at least 3 channels, but this image has %d channels\e[0m\n", m_Channels);
	else
		for (size_t i = 0; i < m_Size; i += m_Channels)
		{
			m_Data[i] *= r;
			m_Data[i + 1] *= g;
			m_Data[i + 2] *= b;
		}
}

void Image::gradient(const Color& startColor, const Color& stopColor)
{
	PROFILE_SCOPE("Image::gradient");

	uint8_t r, g, b;
	uint8_t* px;
	float n;

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

void Image::rotate(double degrees)
{
	PROFILE_SCOPE("Image::rotate");
	//!Non-orthogonal rotation results in "holes" in image

	if ((degrees < 0.001f && degrees > -0.001f) || degrees > 359.999f) // literally wont move
		return;

	#ifdef DEBUG
		printf("[Image::rotate] degrees = %lf\n", degrees);
	#endif

	if (degrees < -0.001f)
		degrees += 360.0f;

	auto roundoff = [](double value, unsigned char precision) -> double
	{
		double pow_10 = std::pow(10.0f, (double)precision);
		return std::round(value * pow_10) / pow_10;
	};

	uint8_t *dstPx, *srcPx;

	#ifdef DEBUG
		printf("[Image::rotate] Scale up image by 2\n");
	#endif

	Image scaledImage = Image::scaleUp(*this, 2);
	Image rotatedImage(scaledImage.m_Width, scaledImage.m_Height, scaledImage.m_Channels);
	
	double x0 = (double)(scaledImage.m_Width / 2);
	double y0 = (double)(scaledImage.m_Height / 2);

	double rad = degrees * ((std::atan(1) * 4) / 180);
	double s = roundoff(std::sin(rad * 1.0f), 6);
	double c = roundoff(std::cos(rad * 1.0f), 6);

	#ifdef DEBUG
		printf("[Image::rotate] X center = %lf, Y center = %lf, cosine = %lf, sine = %lf\n", x0, y0, c, s);
	#endif
	
	for (int y = 0; y < scaledImage.m_Height; ++y)
	{
		double y1 = (double)y - y0;

		for(int x = 0; x < scaledImage.m_Width; ++x)
		{
				//slices off first row and first column
				double x1 = (double)x - x0;

				double x2 = roundoff(((c*x1) - (s*y1)) + x0, 1);
				double y2 = roundoff(((s*x1) + (c*y1)) + y0, 1);

				if ((int)x2 >= 0 && (int)x2 < rotatedImage.m_Width && (int)y2 >= 0 && (int)y2 < rotatedImage.m_Height)
				{
					dstPx = &rotatedImage.m_Data[((int)x2 + (int)y2 * rotatedImage.m_Width) * rotatedImage.m_Channels];
					srcPx = &scaledImage.m_Data[((int)x + (int)y * scaledImage.m_Width) * scaledImage.m_Channels];

					memcpy(dstPx, srcPx, m_Channels);
				}
		}
	}

	#ifdef DEBUG
		printf("[Image::rotate] Scale down image by 2\n");
	#endif

	*this = Image::scaleDown(rotatedImage, 2);
}

void Image::overlay(const Image &source, int x, int y)
{
	PROFILE_SCOPE("Image::overlay");

	#ifdef DEBUG
		printf("[Image::overlay] x = %d, y = %d\n", x, y);
	#endif

	uint8_t *srcPx;
	uint8_t *dstPx;
	int dy, dx;

	for (int sy = 0; sy < source.m_Height; ++sy)
	{
		dy = sy + y;
		
		if (dy < 0)
			continue;
		else if (dy >= m_Height)
			break;

		for (int sx = 0; sx < source.m_Width; ++sx)
		{
			dx = sx + x;

			if (dx < 0)
				continue;
			else if (dx >= m_Width)
				break;

			srcPx = &source.m_Data[(sx + sy * source.m_Width) * source.m_Channels];
			dstPx = &m_Data[(dx + dy * m_Width) * m_Channels];

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

void Image::overlayText(const Font& font, const std::string& txt, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	PROFILE_SCOPE("Image::overlayText");

	#ifdef DEBUG
		printf("[Image::overlayText] text = %s, x = %d, y = %d, RGBA = {%d, %d, %d, %d}\n", txt.c_str(), x, y, r, g, b, a);
	#endif

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

		#ifdef DEBUG
			printf("[Image::overlayText] overlaying \"%c\"...", txt[i]);
		#endif

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

void Image::overlayText(const Font &font, const std::string& txt, int x, int y, const Color& color = {255, 255, 255, 255})
{
	overlayText(font, txt, x, y, color.r, color.g, color.b, color.a);
}

void Image::rasterizeText(const Font& font, const std::string& txt, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	PROFILE_SCOPE("Image::rasterizeText");

	#ifdef DEBUG
		printf("[Image::rasterizeText] text = %s, RGBA = {%d, %d, %d, %d}\n", txt.c_str(), r, g, b, a);
	#endif

	int len = txt.length();
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
			printf("[Image::rasterizeText] rasterizing \"%c\"...\n", txt[i]);
			printf("[Image::rasterizeText] x = %d, y = %d, width = %d, height = %d, advance = %d\n", c.x, c.y, c.width, c.height, c.advance);
		#endif

		Image character(c.width + (c.advance < c.width ? 0 : (c.advance - c.width)), c.height, 4);
		character.m_Baseline = std::abs(c.y - 1);
		character.m_AdvanceHeight = c.height - character.m_Baseline;

		handleRaster(font, character, c, r, g, b, a);
		
		if (!character.isEmpty())
		{
			if (!this->isEmpty())
				concat(character);
			else
				*this = character;
		}

		free(c.image); //free anyway
	}
}

void Image::rasterizeText(const Font& font, const std::string& txt, const Color& color = {255, 255, 255, 255})
{
	rasterizeText(font, txt, color.r, color.g, color.b, color.a);
}

void Image::rasterizeCharacter(const Font& font, unsigned long charCode, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	PROFILE_SCOPE("Image::rasterizeCharacter");

	#ifdef DEBUG
		printf("[Image::rasterizeCharacter] charCode = %c, RGBA = {%d, %d, %d, %d}\n", (int)charCode, r, g, b, a);
	#endif

	SFT_Char c;

	if (&font.m_SFT == NULL) return;

	if (sft_char(&font.m_SFT, charCode, &c) != 0)
	{
		printf("\e[31m[ERROR] Font is missing character '%c'\e[0m\n", (int)charCode);

		return;
	}

	#ifdef DEBUG
		printf("[Image::rasterizeCharacter] x = %d, y = %d, width = %d, height = %d, advance = %d\n", c.x, c.y, c.width, c.height, c.advance);
	#endif

	Image character(c.width + (c.advance < c.width ? 0 : (c.advance - c.width)), (c.width == 0 && c.advance != 0) ? 1 : c.height, 4);
	character.m_Baseline = std::abs(c.y - 1);
	character.m_AdvanceHeight = c.height - character.m_Baseline;

	if (character.m_AdvanceHeight < 0)
		character.m_AdvanceHeight = 0;

	handleRaster(font, character, c, r, g, b, a);
	
	if (!character.isEmpty())
	{
		if (!this->isEmpty())
			concat(character);
		else
			*this = character;
	}

	free(c.image); //free anyway
}

void Image::rasterizeCharacter(const Font& font, unsigned long charCode, const Color& color)
{
	rasterizeCharacter(font, charCode, color.r, color.g, color.b, color.a);
}

void Image::drawLine(int x0, int y0, int x1, int y1, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	PROFILE_SCOPE("Image::drawLine");

	auto sign = [](int value) -> int
	{
		return value < 0 ? -1 : value > 0 ? 1 : 0;
	};

	int interchange;
	uint8_t *dstPx;
	uint8_t color[4] = {r, g, b, a};

	//Bresenham's Line Algorithm
 
	float y = y0;
	float x = x0;
	float deltaX = std::abs(x1 - x0);
	float deltaY = std::abs(y1 - y0);
	float S1 = sign(x1 - x0);
	float S2 = sign(y1 - y0);

	if (deltaY > deltaX)
	{
		std::swap(deltaX, deltaY);
		interchange = 1;
	}
	else
		interchange = 0;

	float E = 2 * deltaY - deltaX;
	float A = 2 * deltaY;
	float B = 2 * deltaY - 2 * deltaX;

	for (int i = 0; i < deltaX; ++i)
	{
		if (x >= m_Width || y >= m_Height) continue;

		dstPx = &m_Data[((int)x + (int)y * m_Width) * m_Channels];

		for (int chnl = 0; chnl < m_Channels; ++chnl)
			dstPx[chnl] = color[chnl];

		if (E < 0)
		{
			if (interchange == 1) y += S2;
			else x += S1;
			E = E + A;
		}
		else
		{
			y += S2;
			x += S1;
			E = E + B;
		}
	}
}

void Image::drawLine(int x0, int y0, int x1, int y1, const Color& color)
{
	drawLine(x0, y0, x1, y1, color.r, color.g, color.b, color.a);
}

void Image::handleRaster(const Font& font, Image& chr, SFT_Char& c, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	PROFILE_SCOPE("Image::handleRaster");

	int32_t dx, dy;
	uint8_t *dstPx;
	uint8_t srcPx;
	uint8_t color[4] = {r, g, b, a};
	float srcAlpha, dstAlpha, outAlpha;

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
				srcAlpha = (srcPx / 255.f) * (a / 255.f);
				dstAlpha = chr.m_Channels < 4 ? 1 : dstPx[3] / 255.f;

				if (srcAlpha > .99 && dstAlpha > .99)
					memcpy(dstPx, color, chr.m_Channels);
				else
				{
					outAlpha = srcAlpha + dstAlpha * (1 - srcAlpha);

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
	PROFILE_SCOPE("Image::crop");

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

void Image::concat(const Image& image, ImagePosition position, int space)
{
	PROFILE_SCOPE("Image::concat");

	if (image.isEmpty()) return;

	if (this->isEmpty() && !image.isEmpty())
		*this = image;
	else if (!image.isEmpty())
		*this = Image::concat(*this, image, position, space);
};

Image Image::concat(const Image& left, const Image& right, ImagePosition position, int space)
{
	PROFILE_SCOPE("static Image::concat");

	int new_w, new_h, new_adv_h, new_baseline, new_channels;

	if (left.isEmpty()) return right;
	if (right.isEmpty()) return left;

	new_channels = left.m_Channels > right.m_Channels ? left.m_Channels : right.m_Channels;

	//?if possible, add some sort of padding between images

	switch (position)
	{
		case ImagePosition::RIGHT:
		case ImagePosition::LEFT:
		default:
			new_w = left.m_Width + right.m_Width + space;
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
			new_h = left.m_Height + right.m_Height + space;
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
			newImage.overlay(right, left.m_Width + space, newImage.m_Baseline > 0 ? newImage.m_Baseline - right.m_Baseline : 0); 
			break;
		case ImagePosition::LEFT:
			#ifdef DEBUG
				printf("[Image::concat] right side concatenation\n");
			#endif
			newImage.overlay(right, 0, newImage.m_Baseline > 0 ? newImage.m_Baseline - right.m_Baseline : 0);
			#ifdef DEBUG
				printf("[Image::concat] left side concatenation\n");
			#endif
			newImage.overlay(left, right.m_Width + space, newImage.m_Baseline > 0 ? newImage.m_Baseline - right.m_Baseline : 0);
			break;
		case ImagePosition::TOP:
			#ifdef DEBUG
				printf("[Image::concat] right side concatenation\n");
			#endif
			newImage.overlay(right, 0, 0);
			#ifdef DEBUG
				printf("[Image::concat] left side concatenation\n");
			#endif
			newImage.overlay(left, 0, right.m_Height + space);
			break;
		case ImagePosition::BOTTOM:
			#ifdef DEBUG
				printf("[Image::concat] left side concatenation\n");
			#endif
			newImage.overlay(left, 0, 0);
			#ifdef DEBUG
				printf("[Image::concat] right side concatenation\n");
			#endif
			newImage.overlay(right, 0, left.m_Height + space);
			break;
	}

	#ifdef DEBUG
		printf("\n");
	#endif

	return newImage;
};

void Image::scaleUp(int times)
{
	*this = Image::scaleUp(*this, times);
}

Image Image::scaleUp(const Image& source, int times)
{
	PROFILE_SCOPE("Image::scaleUp");

	Image scaledImage(source.m_Width * times, source.m_Height * times, source.m_Channels);
	uint8_t *dstPx, *srcPx;

	for (int y = 0; y < source.m_Height; ++y)
	{
		for (int x = 0; x < source.m_Width; ++x)
		{
			srcPx = &source.m_Data[(x + y * source.m_Width) * source.m_Channels];

			for (int scaledY = 0; scaledY < times; ++scaledY)
				for (int scaledX = 0; scaledX < times; ++scaledX)
				{
					dstPx = &scaledImage.m_Data[((times * x + scaledX) + (times * y + scaledY) * scaledImage.m_Width) * scaledImage.m_Channels];
					memcpy(dstPx, srcPx, source.m_Channels);
				}
		}
	}

	return scaledImage;
}

void Image::scaleDown(int times)
{
	*this = Image::scaleDown(*this, times);
}

Image Image::scaleDown(const Image& source, int times)
{
	PROFILE_SCOPE("Image::scaleDown");

	Image scaledImage(source.m_Width / times, source.m_Height / times, source.m_Channels);
	uint8_t *dstPx, *srcPx;
	int r, g, b, a, denom;
	denom = times*times;

	for (int y = 0; y < scaledImage.m_Height; ++y)
	{
		for (int x = 0; x < scaledImage.m_Width; ++x)
		{
			dstPx = &scaledImage.m_Data[(x + y * scaledImage.m_Width) * scaledImage.m_Channels];

			for (int scaledY = 0; scaledY < times; ++scaledY)
				for (int scaledX = 0; scaledX < times; ++scaledX)
				{
					srcPx = &source.m_Data[((times * x + scaledX) + (times * y + scaledY) * source.m_Width) * source.m_Channels];
					//color is messed up
					//get dominant color and use it

					r += srcPx[0];
					g += srcPx[1];
					b += srcPx[2];
					a += scaledImage.m_Channels > 3 ? srcPx[3] : 0;
				}
			
			r /= denom;
			g /= denom;
			b /= denom;
			a /= denom;

			uint8_t dstColor[4] = {(uint8_t)r, (uint8_t)g, (uint8_t)b, (uint8_t)a};
			memcpy(dstPx, dstColor, source.m_Channels);
			r = g = b = a = 0;
		}
	}

	return scaledImage;
}

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