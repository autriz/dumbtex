#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define BYTE_BOUND(value) value < 0 ? 0 : (value > 255 ? 255 : value)

#include "stb_image.h"
#include "stb_image_write.h"

#include "Image.hpp"

Image::Image(): width(0), height(0), channels(0), size(0), baseline(height), advance_height(0), data(NULL) {};

Image::Image(const char *filename, int channel_force)
{
	if (read(filename, channel_force))
	{
		printf("Read %s\n", filename);
		size = width * height * channels;
	}
	else
		printf("Failed to read %s\n", filename);
}

Image::Image(int w, int h, int channels) : width(w), height(h), channels(channels), baseline(h)
{
	this->size = this->width * this->height * this->channels;
	this->advance_height = 0;
	this->data = new uint8_t[this->size];

	memset(this->data, 0, this->size);
}

Image::Image(const Image &other) : Image(other.width, other.height, other.channels)
{
	this->advance_height = other.advance_height;
	memcpy(data, other.data, size);
}

Image::~Image()
{
	stbi_image_free(data);
}

bool Image::read(const char *filename, int channel_force)
{
	data = stbi_load(filename, &width, &height, &channels, channel_force);
	channels = channel_force == 0 ? channels : channel_force;

	return data != NULL;
}

bool Image::write(const char *filename)
{
	int success;

	ImageType type = getFileType(filename);

	switch (type)
	{
		case PNG:
			success = stbi_write_png(filename, width, height, channels, data, width * channels);
			break;
		case BMP:
			success = stbi_write_bmp(filename, width, height, channels, data);
			break;
		case JPG:
			success = stbi_write_jpg(filename, width, height, channels, data, 100);
			break;
		case TGA:
			success = stbi_write_tga(filename, width, height, channels, data);
			break;
	}

	if (success != 0)
	{
		printf("\e[32mWrote \e[36m%s\e[0m, %d, %d, %d, %zu\n", filename, width, height, channels, size);
		return true;
	}
	else
	{
		printf("\e[31;1m Failed to write \e[36m%s\e[0m, %d, %d, %d, %zu\n", filename, width, height, channels, size);
		return false;
	}
}

ImageType Image::getFileType(const char *filename)
{
	const char *ext = strrchr(filename, '.');
	if (ext != nullptr)
	{
		if (strcmp(ext, ".png") == 0)
			return PNG;
		else if (strcmp(ext, ".jpg") == 0)
			return JPG;
		else if (strcmp(ext, ".jpeg") == 0)
			return JPG;
		else if (strcmp(ext, ".bmp") == 0)
			return BMP;
		else if (strcmp(ext, ".tga") == 0)
			return TGA;
	}
	return PNG;
}

Details Image::getDetails() const
{
	return { width, height, channels, size, baseline };
}

void Image::diffmap(Image &img)
{
	int compare_width = fmin(width, img.width);
	int compare_height = fmin(height, img.height);
	int compare_channels = fmin(channels, img.channels);
	for (uint32_t i = 0; i < compare_height; ++i)
		for (uint32_t j = 0; j < compare_width; ++j)
			for (uint8_t k = 0; k < compare_channels; ++k)
			{
				data[(i * width + j) * channels + k] = BYTE_BOUND(abs(data[(i * width + j) * channels + k] - img.data[(i * img.width + j) * img.channels + k]));
			}
}

void Image::diffmapScale(Image &img, uint8_t scl)
{
	int compare_width = fmin(width, img.width);
	int compare_height = fmin(height, img.height);
	int compare_channels = fmin(channels, img.channels);
	uint8_t largest = 0;
	for (uint32_t i = 0; i < compare_height; ++i)
		for (uint32_t j = 0; j < compare_width; ++j)
			for (uint8_t k = 0; k < compare_channels; ++k)
			{
				data[(i * width + j) * channels + k] = BYTE_BOUND(abs(data[(i * width + j) * channels + k] - img.data[(i * img.width + j) * img.channels + k]));
				largest = fmax(largest, data[(i * width + j) * channels + k]);
			}
	scl = 255 / fmax(1, fmax(scl, largest));
	for (int i = 0; i < size; ++i)
		data[i] *= scl;
}

void Image::grayscaleAvg()
{
	if (channels < 3)
		printf("Image %p has less than 3 channels, it is assumed to already be grayscale.", this);
	else
		for (int i = 0; i < size; i += channels)
		{
			//(r+g+b)/3
			int gray = (data[i] + data[i + 1] + data[i + 2]) / 3;
			memset(data + i, gray, 3);
		}
}

void Image::grayscaleLum()
{
	if (channels < 3)
		printf("Image %p has less than 3 channels, it is assumed to already be grayscale.", this);
	else
		for (int i = 0; i < size; i += channels)
		{
			int gray = 0.2126 * data[i] + 0.7152 * data[i + 1] + 0.0722 * data[i + 2];
			memset(data + i, gray, 3);
		}
}

void Image::colorMask(float r, float g, float b)
{
	if (channels < 3)
		printf("\e[31m[ERROR] Color mask requires at least 3 channels, but this image has %d channels\e[0m\n", channels);
	else
		for (int i = 0; i < size; i += channels)
		{
			data[i] *= r;
			data[i + 1] *= g;
			data[i + 2] *= b;
		}
}

void Image::flipX()
{
	uint8_t tmp[4];
	uint8_t *px1;
	uint8_t *px2;
	for (int y = 0; y < height; ++y)
		for (int x = 0; x < width / 2; ++x)
		{
			px1 = &data[(x + y * width) * channels];
			px2 = &data[((width - 1 - x) + y * width) * channels];

			memcpy(tmp, px1, channels);
			memcpy(px1, px2, channels);
			memcpy(px2, tmp, channels);
		}
}

void Image::flipY()
{
	uint8_t tmp[4];
	uint8_t *px1;
	uint8_t *px2;
	for (int x = 0; x < width; ++x)
		for (int y = 0; y < height / 2; ++y)
		{
			px1 = &data[(x + y * width) * channels];
			px2 = &data[(x + (height - 1 - y) * width) * channels];

			memcpy(tmp, px1, channels);
			memcpy(px1, px2, channels);
			memcpy(px2, tmp, channels);
		}
}

void Image::overlay(const Image &source, int x, int y)
{

	uint8_t *srcPx;
	uint8_t *dstPx;

	for (int sy = 0; sy < source.height; ++sy)
	{
		if (sy + y < 0)
			continue;
		else if (sy + y >= height)
			break;
		for (int sx = 0; sx < source.width; ++sx)
		{
			if (sx + x < 0)
				continue;
			else if (sx + x >= width)
				break;
			srcPx = &source.data[(sx + sy * source.width) * source.channels];
			dstPx = &data[(sx + x + (sy + y) * width) * channels];

			float srcAlpha = source.channels < 4 ? 1 : srcPx[3] / 255.f;
			float dstAlpha = channels < 4 ? 1 : dstPx[3] / 255.f;

			if (srcAlpha > .99 && dstAlpha > .99)
			{
				if (source.channels >= channels)
					memcpy(dstPx, srcPx, channels);
				else
					// In case our source image is grayscale and the dest one isnt
					memset(dstPx, srcPx[0], channels);
			}
			else
			{
				float outAlpha = srcAlpha + dstAlpha * (1 - srcAlpha);
				if (outAlpha < .01)
					memset(dstPx, 0, channels);
				else
				{
					for (int chnl = 0; chnl < channels; ++chnl)
						dstPx[chnl] = (uint8_t)BYTE_BOUND((srcPx[chnl] / 255.f * srcAlpha + dstPx[chnl] / 255.f * dstAlpha * (1 - srcAlpha)) / outAlpha * 255.f);
					if (channels > 3)
						dstPx[3] = (uint8_t)BYTE_BOUND(outAlpha * 255.f);
				}
			}
		}
	}
}

void Image::overlayText(const std::string& txt, const Font& font, int x, int y, uint8_t r = 255, uint8_t g = 255, uint8_t b = 255, uint8_t a = 255)
{
	size_t len = txt.length();
	SFT_Char c;
	int32_t dx, dy;
	uint8_t *dstPx;
	uint8_t srcPx;
	uint8_t color[4] = {r, g, b, a};

	for (size_t i = 0; i < len; ++i)
	{

		if (sft_char(&font.sft, txt.c_str()[i], &c) != 0) //txt.c_str()[i]
		{
			printf("\e[31m[ERROR] Font is missing character '%c'\e[0m\n", txt[i]);
			continue;
		}

		for (uint16_t sy = 0;sy < c.height;++sy) 
		{
			dy = sy + y + c.y;
			if(dy < 0) 
				continue;
			else if(dy >= height) 
				break;
			for(uint16_t sx = 0;sx < c.width;++sx) 
			{
				dx = sx + x + c.x;
				if(dx < 0) 
					continue;
				else if(dx >= width) 
					break;
				dstPx = &data[(dx + dy * width) * channels];
				srcPx = c.image[sx + sy * c.width];

				if(srcPx != 0) 
				{
					float srcAlpha = (srcPx / 255.f) * (a / 255.f);
					float dstAlpha = channels < 4 ? 1 : dstPx[3] / 255.f;
					if(srcAlpha > .99 && dstAlpha > .99) 
						memcpy(dstPx, color, channels);
					else 
					{
						float outAlpha = srcAlpha + dstAlpha * (1 - srcAlpha);
						if(outAlpha < .01) 
							memset(dstPx, 0, channels);
						else {
							for(int chnl = 0;chnl < channels;++chnl) 
								dstPx[chnl] = (uint8_t)BYTE_BOUND((color[chnl]/255.f * srcAlpha + dstPx[chnl]/255.f * dstAlpha * (1 - srcAlpha)) / outAlpha * 255.f);
							if(channels > 3) 
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

void Image::rasterizeText(const std::string& txt, const Font& font, uint8_t r = 255, uint8_t g = 255, uint8_t b = 255, uint8_t a = 255)
{
	size_t len = txt.length();
	SFT_Char c;
	int32_t dx, dy;
	uint8_t *dstPx;
	uint8_t srcPx;
	uint8_t color[4] = {r, g, b, a};

	for (int i = 0; i < len; i++)
	{

		if (sft_char(&font.sft, i == 0 ? 8721 : txt.c_str()[i], &c) != 0) //txt.c_str()[i]
		{
			printf("\e[31m[ERROR] Font is missing character '%c'\e[0m\n", txt[i]);
			continue;
		};

		printf("x: %d, y: %d, width: %d, height: %d\n", c.x, c.y, c.width, c.height);

		Image character(c.width + (c.advance - c.width), c.height, 4);
		character.baseline = std::abs(c.y - 1);
		character.advance_height = c.height - character.baseline;

		for (uint16_t sy = 0; sy < c.height; ++sy)
		{
			dy = sy + character.height + c.y - (character.height - std::abs(c.y + 1)); // dy = sy + c.y || dy = sy + character.height + c.y
			if (dy < 0)
				continue;
			else if (dy >= character.height)
				break;
			for (uint16_t sx = 0; sx < c.width; ++sx)
			{
				dx = sx + c.x; // dx = sx + character.width + c.x
				if (dx < 0)
					continue;
				else if (dx >= character.width)
					break;
				dstPx = &(character.data[(dx + dy * character.width) * character.channels]);
				srcPx = c.image[sx + sy * c.width];

				if (srcPx != 0)
				{
					float srcAlpha = (srcPx / 255.f) * (a / 255.f);
					float dstAlpha = character.channels < 4 ? 1 : dstPx[3] / 255.f;
					if (srcAlpha > .99 && dstAlpha > .99)
						memcpy(dstPx, color, character.channels);
					else
					{
						float outAlpha = srcAlpha + dstAlpha * (1 - srcAlpha);
						if (outAlpha < .01)
							memset(dstPx, 0, character.channels);
						else
						{
							for (int chnl = 0; chnl < character.channels; ++chnl)
								dstPx[chnl] = (uint8_t)BYTE_BOUND((color[chnl] / 255.f * srcAlpha + dstPx[chnl] / 255.f * dstAlpha * (1 - srcAlpha)) / outAlpha * 255.f);
							if (character.channels > 3)
								dstPx[3] = (uint8_t)BYTE_BOUND(outAlpha * 255.f);
						}
					}
				}
			}
		}
		concat(character);
		free(c.image);
	}
}

void Image::rasterizeText(const std::string& txt, const Font& font, const Color& color = {255, 255, 255, 255})
{
	rasterizeText(txt, font, color.r, color.g, color.b, color.a);
}

void Image::crop(uint16_t cx, uint16_t cy, uint16_t cw, uint16_t ch)
{
	size = cw * ch * channels;
	uint8_t *croppedImage = new uint8_t[size];
	memset(croppedImage, 0, size);

	for (uint16_t y = 0; y < ch; ++y)
	{
		if (y + cy >= height)
			break;
		for (uint16_t x = 0; x < cw; ++x)
		{
			if (x + cx >= width)
				break;
			memcpy(&croppedImage[(x + y * cw) * channels], &data[(x + cx + (y + cy) * width) * channels], channels);
		}
	}

	width = cw;
	height = ch;

	delete[] data;
	data = croppedImage;
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
	
	size = nw * nh * channels;
	uint8_t *newImage = new uint8_t[size];

	float scaleX = (float)nw / (width);
	float scaleY = (float)nh / (height);

	for (uint16_t y = 0; y < nh; ++y)
	{
		sy = (uint16_t)(y / scaleY);

		for (uint16_t x = 0; x < nw; ++x)
		{
			sx = (uint16_t)(x / scaleX);
			memcpy(&newImage[(x + y * nw) * channels], &data[(sx + sy * width) * channels], channels);
		}
	}

	width = nw;
	height = nh;

	delete[] data;
	data = newImage;
	newImage = nullptr;
};

void Image::concat(const Image& image)
{
	*this = Image::concat(*this, image);
};

Image Image::concat(const Image& left, const Image& right)
{
	int new_w = left.width + right.width;
	int new_h = left.height > right.height ? left.height : right.height;
	int new_channels = left.channels > right.channels ? left.channels : right.channels;
	int new_adv_h = left.advance_height > right.advance_height ? left.advance_height : right.advance_height;

	if ((new_h - new_adv_h) < (right.height - right.advance_height)) new_h += (right.height - (new_h - new_adv_h));

	// printf("left baseline: %d, right baseline: %d\n", left.baseline, right.baseline);
	// printf("left height: %d, right height: %d, left adv height: %d, right adv height: %d\n", left.height, right.height, left.advance_height, right.advance_height);
	// printf("new_h: %d, new_w: %d\n", new_h, new_w);

	Image newImage(new_w, new_h, new_channels);
	newImage.advance_height = new_adv_h;
	newImage.baseline = left.baseline > right.baseline ? left.baseline : right.baseline;

	// printf("newImage height: %d, newImage width: %d, newImage adv height: %d\n\n", newImage.height, newImage.width, newImage.advance_height);

	newImage.overlay(left, 0, newImage.baseline > 0 ? newImage.baseline - left.baseline : 0); 
	newImage.overlay(right, left.width, newImage.baseline > 0 ? newImage.baseline - right.baseline : 0); 
	//same as above

	return newImage;
};

void Image::test() 
{
	Font font("./fonts/OpenSans-Regular.ttf", 50);
	SFT_Char c;

	if (sft_char(&font.sft, 8721, &c) != 0)
	{
		printf("\e[31m[ERROR] Font is missing character '%c'\e[0m\n", 'g');
	}
	else
	{
		Image image(c.width, c.height, 4);
		delete[] image.data;
		image.data = new uint8_t[image.size];
		memcpy(image.data, c.image, image.size);

		image.write("result.png");	
		free(c.image);
	};
}

void Image::operator=(const Image& origin) {
	width = origin.width;
	height = origin.height;
	channels = origin.channels;
	baseline = origin.baseline;
	advance_height = origin.advance_height;
	size = origin.size;
	delete[] data;
	data = new uint8_t[size];
	memcpy(data, origin.data, size);
};