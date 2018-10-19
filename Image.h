#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#ifndef IMGAE_H
#define IMAGE_H

typedef struct
{
	unsigned long bfSize;
	unsigned short bfReserved1;
	unsigned short bfReserved2;
	unsigned long bfOffBits;
} BitmapHeader;

typedef struct
{
	unsigned long biSize;
	long biWidth;
	long biHeight;
	unsigned short biPlanes;
	unsigned short biBitCount;
	unsigned long biCompression;
	unsigned long biSizeImage;
	long biXPelsPerMeter;
	long biYPelsPerMeter;
	unsigned long biClrUsed;
	unsigned long biClrImportant;
} BitmapInfoHeader;

typedef struct
{
	unsigned char rgbBlue; //该颜色的蓝色分量
	unsigned char rgbGreen; //该颜色的绿色分量
	unsigned char rgbRed; //该颜色的红色分量
	unsigned char rgbReserved; //保留值
} RgbQuad;

typedef struct
{
	int width;
	int height;
	int channels;
	unsigned char* imageData;
	BitmapHeader bmpHeader;
	BitmapInfoHeader infoHeader;
} Image;

Image* LoadImage(const char* path)
{
	Image* img = NULL;
	FILE* pFile;
	unsigned short fileType;
	int channels = 1;
	int width = 0;
	int height = 0;
	int step = 0;
	int offset = 0;
	unsigned char pixVal;
	RgbQuad* quad;
	int i, j, k;

	img = (Image*)malloc(sizeof(Image));
	memset(img,0,sizeof(Image));
	pFile = fopen(path, "rb");
	if (!pFile)
	{
		free(img);
		return NULL;
	}

	fread(&fileType, sizeof(unsigned short), 1, pFile);
	if (fileType == 0x4D42)
	{
		fread(&(img->bmpHeader), sizeof(BitmapHeader), 1, pFile);
		fread(&(img->infoHeader), sizeof(BitmapInfoHeader), 1, pFile);
		if (img->infoHeader.biBitCount == 8)
		{
			channels = 1;
			width = img->infoHeader.biWidth;
			height = img->infoHeader.biHeight;
			offset = (channels*width) % 4;
			if (offset != 0)
			{
				offset = 4 - offset;
			}
			img->width = width;
			img->height = height;
			img->channels = 1;
			img->imageData = (unsigned char*)malloc(sizeof(unsigned char)*width*height);
			step = channels * width;
			quad = (RgbQuad*)malloc(sizeof(RgbQuad) * 256);
			fread(quad, sizeof(RgbQuad), 256, pFile);
			free(quad);
			for (i = 0; i < height; i++)
			{
				for (j = 0; j < width; j++)
				{
					fread(&pixVal, sizeof(unsigned char), 1, pFile);
					img->imageData[(height - 1 - i)*step + j] = pixVal;
				}
				if (offset != 0)
				{
					for (j = 0; j < offset; j++)
					{
						fread(&pixVal, sizeof(unsigned char), 1, pFile);
					}
				}
			}
		}
		else if (img->infoHeader.biBitCount == 24)
		{
			channels = 3;
			width = img->infoHeader.biWidth;
			height = img->infoHeader.biHeight;
			img->width = width;
			img->height = height;
			img->channels = 3;
			img->imageData = (unsigned char*)malloc(sizeof(unsigned char)*width * 3 * height);
			step = channels * width;
			offset = (channels*width) % 4;
			if (offset != 0)
			{
				offset = 4 - offset;
			}
			for (i = 0; i < height; i++)
			{
				for (j = 0; j < width; j++)
				{
					for (k = 0; k < 3; k++)
					{
						fread(&pixVal, sizeof(unsigned char), 1, pFile);
						img->imageData[(height - 1 - i)*step + j * 3 + k] = pixVal;
					}
				}
				if (offset != 0)
				{
					for (j = 0; j < offset; j++)
					{
						fread(&pixVal, sizeof(unsigned char), 1, pFile);
					}
				}
			}
		}
	}
	return img;
}

bool SaveImage(const char* path, Image* img)
{
	FILE *pFile;
	unsigned short fileType;
	BitmapHeader bmpHeader;
	BitmapInfoHeader infoHeader;
	int step;
	int offset;
	unsigned char pixVal = '\0';
	int i, j;
	RgbQuad* quad;

	pFile = fopen(path, "wb");
	if (!pFile)
	{
		return false;
	}

	fileType = 0x4D42;
	fwrite(&fileType, sizeof(unsigned short), 1, pFile);

	if (img->channels == 3)//24位，通道，彩图
	{
		step = img->channels*img->width;
		offset = step % 4;
		if (offset != 4)
		{
			step += 4 - offset;
		}

		bmpHeader.bfSize = img->height*step + 54;
		bmpHeader.bfReserved1 = 0;
		bmpHeader.bfReserved2 = 0;
		bmpHeader.bfOffBits = 54;
		fwrite(&bmpHeader, sizeof(BitmapHeader), 1, pFile);

		infoHeader.biSize = 40;
		infoHeader.biWidth = img->width;
		infoHeader.biHeight = img->height;
		infoHeader.biPlanes = 1;
		infoHeader.biBitCount = 24;
		infoHeader.biCompression = 0;
		infoHeader.biSizeImage = img->height*step;
		infoHeader.biXPelsPerMeter = 0;
		infoHeader.biYPelsPerMeter = 0;
		infoHeader.biClrUsed = 0;
		infoHeader.biClrImportant = 0;
		fwrite(&infoHeader, sizeof(BitmapInfoHeader), 1, pFile);

		for (i = img->height - 1; i > -1; i--)
		{
			for (j = 0; j < img->width; j++)
			{
				pixVal = img->imageData[i*img->width * 3 + j * 3];
				fwrite(&pixVal, sizeof(unsigned char), 1, pFile);
				pixVal = img->imageData[i*img->width * 3 + j * 3 + 1];
				fwrite(&pixVal, sizeof(unsigned char), 1, pFile);
				pixVal = img->imageData[i*img->width * 3 + j * 3 + 2];
				fwrite(&pixVal, sizeof(unsigned char), 1, pFile);
			}
			if (offset != 0)
			{
				for (j = 0; j < offset; j++)
				{
					pixVal = 0;
					fwrite(&pixVal, sizeof(unsigned char), 1, pFile);
				}
			}
		}
	}
	else if (img->channels == 1)//8位，单通道，灰度图
	{
		step = img->width;
		offset = step % 4;
		if (offset != 4)
		{
			step += 4 - offset;
		}

		bmpHeader.bfSize = 54 + 256 * 4 + img->width;
		bmpHeader.bfReserved1 = 0;
		bmpHeader.bfReserved2 = 0;
		bmpHeader.bfOffBits = 54 + 256 * 4;
		fwrite(&bmpHeader, sizeof(BitmapHeader), 1, pFile);

		infoHeader.biSize = 40;
		infoHeader.biWidth = img->width;
		infoHeader.biHeight = img->height;
		infoHeader.biPlanes = 1;
		infoHeader.biBitCount = 8;
		infoHeader.biCompression = 0;
		infoHeader.biSizeImage = img->height*step;
		infoHeader.biXPelsPerMeter = 0;
		infoHeader.biYPelsPerMeter = 0;
		infoHeader.biClrUsed = 256;
		infoHeader.biClrImportant = 256;
		fwrite(&infoHeader, sizeof(BitmapInfoHeader), 1, pFile);

		quad = (RgbQuad*)malloc(sizeof(RgbQuad) * 256);
		for (i = 0; i < 256; i++)
		{
			quad[i].rgbBlue = i;
			quad[i].rgbGreen = i;
			quad[i].rgbRed = i;
			quad[i].rgbReserved = 0;
		}
		fwrite(quad, sizeof(RgbQuad), 256, pFile);
		free(quad);

		for (i = img->height - 1; i > -1; i--)
		{
			for (j = 0; j < img->width; j++)
			{
				pixVal = img->imageData[i*img->width + j];
				fwrite(&pixVal, sizeof(unsigned char), 1, pFile);
			}
			if (offset != 0)
			{
				for (j = 0; j < offset; j++)
				{
					pixVal = 0;
					fwrite(&pixVal, sizeof(unsigned char), 1, pFile);
				}
			}
		}
	}
	fclose(pFile);

	return true;
}

void FreeImage(Image* img)
{
	free(img->imageData);
	free(img);
}

#endif

