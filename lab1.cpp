#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "Image.h"

Image* CropQuarter(Image* img)
{
	int w = img->width / 2,h = img->height / 2;
	int i,j;
	unsigned char* newData = NULL;
	Image* newImg = (Image*)malloc(sizeof(Image));
	if (img->channels == 3)
	{
		newData = new unsigned char[w*h*3];
		for (i = h - 1; i > -1; i--)
		{
			for (j = 0; j < w; j++)
			{
				newData[i*w*3+j*3] = img->imageData[i*img->width * 3 + j * 3];
				newData[i*w*3+j*3+1] = img->imageData[i*img->width * 3 + j * 3 + 1];
				newData[i*w*3+j*3+2] = img->imageData[i*img->width * 3 + j * 3 + 2];
			}
		}
	}
	else if(img->channels == 1)
	{
		newData = new unsigned char[w*h];
		for (i = h - 1; i > -1; i--)
		{
			for (j = 0; j < w; j++)
			{
				newData[i*w+j] = img->imageData[i*img->width + j];
			}
		}
	}
	newImg->channels = img->channels;
	newImg->imageData=newData;
	newImg->width=w;
	newImg->height=h;
	return newImg;
}

bool SaveRaw(const char* path, Image* img)
{
	FILE *pFile;
	pFile = fopen(path, "wb");
	if (!pFile)
	{
		return false;
	}
	fwrite(img->imageData, sizeof(unsigned char) * img->width * img->height * img->channels, 1, pFile);
	fclose(pFile);
}

void ProcessImage(const char* path,const char* rawPath,const char* quadPath)
{
	Image* img = LoadImage(path);
	Image* quaterImage;
	
	if(img==NULL){
		printf("NULL\n");
		return;
	}

	printf("文件大小:%ld个字节\n",img->bmpHeader.bfSize);
	printf("位图数据起始字节:%ld\n",img->bmpHeader.bfOffBits);
	printf("位图宽度:%d\n",img->width);
	printf("位图高度:%d\n",img->height);
	printf("每个像素的位数:%d\n",img->infoHeader.biBitCount);
	
	SaveRaw(rawPath,img);
	printf("已保存位图数据到文件%s\n",rawPath);
	
	quaterImage = CropQuarter(img);
	SaveImage(quadPath,quaterImage);
	printf("已保存左上角1/4的数据到文件%s\n",quadPath);
	
	FreeImage(quaterImage);
	FreeImage(img);
}

int main()
{
	ProcessImage("lena.bmp","lena.raw","lenas.bmp");
	ProcessImage("lenac.bmp","lenac.raw","lenacs.bmp");
	return 0;
}

