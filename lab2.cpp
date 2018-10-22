#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "Image.h"

Image* ShrinkImage(Image* img,int scale,bool bi = false)
{
	int w = img->width / scale,h = img->height / scale;
	int i,j,k,l;
	unsigned char* newData = NULL;
	float val;
	float r,g,b;
	Image* newImg = (Image*)malloc(sizeof(Image));
	if (img->channels == 3)
	{
		newData = new unsigned char[w*h*3];
		for (i = 0; i < h; i++)
		{
			for (j = 0; j < w; j++)
			{
				if(bi)
				{
					r = g = b = 0.0;
					for(k=0; k<scale; k++)
					{
						for(l=0; l<scale; l++)
						{
							b += img->imageData[i*scale*img->width*3 + j*scale*3 + 3*k + 3*img->width*l];
							g += img->imageData[i*scale*img->width*3 + j*scale*3 + 3*k + 3*img->width*l + 1];
							r += img->imageData[i*scale*img->width*3 + j*scale*3 + 3*k + 3*img->width*l + 2];
						}
					}
					r = r / (float)(scale*scale);
					g = g / (float)(scale*scale);
					b = b / (float)(scale*scale);
					newData[i*w*3+j*3] = (int)b;
					newData[i*w*3+j*3+1] = (int)g;
					newData[i*w*3+j*3+2] = (int)r;
				}
				else
				{
					newData[i*w*3+j*3] = img->imageData[i*scale*img->width * 3 + j*scale * 3];
					newData[i*w*3+j*3+1] = img->imageData[i*scale*img->width * 3 + j*scale * 3 + 1];
					newData[i*w*3+j*3+2] = img->imageData[i*scale*img->width * 3 + j*scale * 3 + 2];
				}
			}
		}
	}
	else if(img->channels == 1)
	{
		newData = new unsigned char[w*h];
		for (i = 0; i < h; i++)
		{
			for (j = 0; j < w; j++)
			{
				if(bi)
				{
					val = 0.0;
					for(k=0; k<scale; k++)
					{
						for(l=0; l<scale; l++)
						{
							val += img->imageData[i*scale*img->width + j*scale + k + l *img->width];
						}
					}
					val = val / (float)(scale*scale);
					newData[i*w+j] = (unsigned char)val;
				}
				else
				{
					newData[i*w+j] = img->imageData[i*scale*img->width + j*scale];
				}
			}
		}
	}
	newImg->channels = img->channels;
	newImg->imageData=newData;
	newImg->width=w;
	newImg->height=h;
	return newImg;
}

Image* EnlargeImage(Image* img,int scale)
{
	int i,j,k,l;
	unsigned char* newData = NULL;
	Image* newImg = (Image*)malloc(sizeof(Image));
	if (img->channels == 3)
	{
		newData = new unsigned char[img->width * scale * img->height * scale *3];
		for (i = 0; i < img->height; i++)
		{
			for (j = 0; j < img->width; j++)
			{
				for(k=0; k<scale; k++)
				{
					for(l=0; l<scale; l++)
					{
						newData[i*scale*scale*img->width*3 + j*scale*3 + 3*k + 3*img->width*scale*l] = img->imageData[i*img->width*3 + j*3];
						newData[i*scale*scale*img->width*3 + j*scale*3 + 3*k + 3*img->width*scale*l +1] = img->imageData[i*img->width*3 + j*3+1];
						newData[i*scale*scale*img->width*3 + j*scale*3 + 3*k + 3*img->width*scale*l +2] = img->imageData[i*img->width*3 + j*3+2];
					}
				}
			}
		}
	}
	else if(img->channels == 1)
	{
		newData = new unsigned char[img->width * scale* img->height * scale];
		for (i = 0; i < img->height; i++)
		{
			for (j = 0; j < img->width; j++)
			{
				for(k=0; k<scale; k++)
				{
					for(l=0; l<scale; l++)
					{
						newData[i*scale*scale*img->width + j*scale + k + l *img->width*scale] = img->imageData[i*img->width + j];
					}
				}
			}
		}
	}
	newImg->channels = img->channels;
	newImg->imageData=newData;
	newImg->width = img->width * scale;
	newImg->height = img->height * scale;
	return newImg;
}

void ProcessImage(int scale,const char* path,const char* shinkPath,const char* shinkPath2,const char* enlargePath,const char* enlargePath2)
{
	Image* img = LoadImage(path);
	Image* shirkImage,* enlargeImage;
	
	printf("文件大小:%ld个字节\n",img->bmpHeader.bfSize);
	printf("位图数据起始字节:%ld\n",img->bmpHeader.bfOffBits);
	printf("位图宽度:%d\n",img->width);
	printf("位图高度:%d\n",img->height);
	printf("每个像素的位数:%d\n",img->infoHeader.biBitCount);

	shirkImage = ShrinkImage(img,scale);
	SaveImage(shinkPath,shirkImage);
	printf("已保存缩小%d倍图像的数据到文件%s\n",scale,shinkPath);

	enlargeImage = EnlargeImage(shirkImage,scale);
	SaveImage(enlargePath,enlargeImage);
	printf("已保存放大%d倍图像的数据到文件%s\n",scale,enlargePath);

	FreeImage(shirkImage);
	FreeImage(enlargeImage);

	shirkImage = ShrinkImage(img,scale,true);
	SaveImage(shinkPath2,shirkImage);
	printf("已保存双线性插值缩小%d倍图像的数据到文件%s\n",scale,shinkPath2);

	enlargeImage = EnlargeImage(shirkImage,scale);
	SaveImage(enlargePath2,enlargeImage);
	printf("已保存放大%d倍图像的数据到文件%s\n",scale,enlargePath);

	FreeImage(shirkImage);
	FreeImage(enlargeImage);
	FreeImage(img);
}

int main()
{
	ProcessImage(2,"lena.bmp","lenas.bmp","lenas1.bmp","lenasz.bmp","lenasz1.bmp");
	ProcessImage(4,"lena.bmp","lenas_4.bmp","lenas1_4.bmp","lenasz_4.bmp","lenasz1_4.bmp");
	ProcessImage(2,"lenac.bmp","lenacs.bmp","lenacs1.bmp","lenacsz.bmp","lenacsz1.bmp");
	return 0;
}

