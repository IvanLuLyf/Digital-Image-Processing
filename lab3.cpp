#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

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
	Image* img;
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

void MakeColor(unsigned char *R,unsigned char *G,unsigned char *B,unsigned char V)
{
	unsigned char Max,Min,Diff;
	float H,S;

	Max = (*R)>(*G)?(*R):(*G);
	Max = Max>(*B)?Max:(*B);

	Min = (*R)<(*G)?(*R):(*G);
	Min = Min<(*B)?Min:(*B);

	Diff = Max - Min;

	if(Max==0)
	{
		S=0;
	}
	else
	{
		S=(float)Diff/(float)Max;
	}
	if((*R)==Max)
	{
		H = (float)((*G)-(*B))/(float)Diff;
	}
	else if ((*G) == Max)
	{
		H = 2 + (float)((*B) - (*R)) / (float)Diff;
	}
	else
	{
		H = 4 + (float)((*R) - (*G)) / (float)Diff;
	}
	H = H * 60;
	if (H < 0)
	{
		H = H + 360;
	}

	if (S == 0)
	{
		*R = *G = *B = V;
		return;
	}
	H /= 60;

	int n = (int)H;

	float f = H - n;

	float a = V * (1 - S);
	float b = V * (1 - S * f);
	float c = V * (1 - S * (1 - f));

	switch (n)
	{
		case 0:
			*R = V;
			*G = (unsigned char)c;
			*B = (unsigned char)a;
			break;
		case 1:
			*R = (unsigned char)b;
			*G = V;
			*B = (unsigned char)a;
			break;
		case 2:
			*R = (unsigned char)a;
			*G = V;
			*B = (unsigned char)c;
			break;
		case 3:
			*R = (unsigned char)a;
			*G = (unsigned char)b;
			*B = V;
			break;
		case 4:
			*R = (unsigned char)c;
			*G = (unsigned char)a;
			*B = V;
			break;
		case 5:
			*R = V;
			*G = (unsigned char)a;
			*B = (unsigned char)b;
			break;
	}
}

Image* HistogramEqualization(Image *img)
{
	Image* newImg = (Image*)malloc(sizeof(Image));
	unsigned char* newData = NULL;
	if (img->channels == 3)//24位，通道，彩图
	{
		unsigned char pixValV,pixValR,pixValG,pixValB;
		newData = new unsigned char[img->height*img->width*3];
		unsigned char * newV = new unsigned char[img->height*img->width];
		int num[256]= {0};
		int minValue=255;
		int maxValue=0;
		for(int i=0; i<img->height; i++)
		{
			for (int j = 0; j < img->width; j++)
			{
				pixValB = img->imageData[i*img->width * 3 + j * 3];
				pixValG = img->imageData[i*img->width * 3 + j * 3 + 1];
				pixValR = img->imageData[i*img->width * 3 + j * 3 + 2];

				pixValV = pixValB>pixValG?pixValB:pixValG;
				pixValV = pixValV>pixValR?pixValV:pixValR;

				newV[i*img->width + j] = pixValV;

				num[pixValV]++;

				if(pixValV>maxValue)
				{
					maxValue=pixValV;
				}
				else if(pixValV<minValue)
				{
					minValue=pixValV;
				}
			}
		}
		printf("%d ,%d\n",minValue,maxValue);//输出最大与最小灰度值

		float *p=new float[256];//p中存放的是灰度级为i的像素在整幅图像中出现的概率
		float *c=new float[256];//定义c，用来存放累积的归一化直方图
		for(int i=0; i<256; i++)
		{
			c[i]=0;
			p[i]=(float)num[i]/(float)((img->width)*(img->height));
		}
		for(int i=0; i<256; i++)
		{
			for(int j=0; j<=i; j++)
			{
				c[i]+=p[j];
			}
		}
		for(int i=0; i<img->height; i++)
		{
			for(int j=0; j<img->width; j++)
			{
				newV[i*img->width + j]=*(c+newV[i*img->width + j])*(maxValue-minValue)+minValue;
				pixValB = img->imageData[i*img->width * 3 + j * 3];
				pixValG = img->imageData[i*img->width * 3 + j * 3 + 1];
				pixValR = img->imageData[i*img->width * 3 + j * 3 + 2];
				
				MakeColor(&pixValR,&pixValG,&pixValB,newV[i*img->width + j]);
				
				newData[i*img->width * 3 + j * 3]=pixValB;
				newData[i*img->width * 3 + j * 3 + 1] = pixValG;
				newData[i*img->width * 3 + j * 3 + 2] = pixValR;
			}
		}
		
		free(newV);
		free(p);
		free(c);
	}
	else if (img->channels == 1)//8位，单通道，灰度图
	{
		unsigned char pixVal;
		newData = new unsigned char[img->height*img->width];
		int num[256]= {0};
		int minGrayValue=255;
		int maxGrayValue=0;
		for(int i=0; i<img->height; i++)
		{
			for (int j = 0; j < img->width; j++)
			{
				pixVal = img->imageData[i*img->width + j];
				newData[i*img->width + j] = pixVal;
				num[pixVal]++;
				if(pixVal>maxGrayValue)
				{
					maxGrayValue=pixVal;
				}
				else if(pixVal<minGrayValue)
				{

					minGrayValue=pixVal;
				}
			}
		}
		printf("%d ,%d\n",minGrayValue,maxGrayValue);//输出最大与最小灰度值
		float *p=new float[256];//p中存放的是灰度级为i的像素在整幅图像中出现的概率
		float *c=new float[256];//定义c，用来存放累积的归一化直方图
		for(int i=0; i<256; i++)
		{
			c[i]=0;
			p[i]=(float)num[i]/(float)((img->width)*(img->height));
		}
		for(int i=0; i<256; i++)
		{
			for(int j=0; j<=i; j++)
			{
				c[i]+=p[j];
			}
		}
		for(int i=0; i<img->height; i++)
		{

			for(int j=0; j<img->width; j++)
			{
				newData[i*img->width + j]=*(c+newData[i*img->width + j])*(maxGrayValue-minGrayValue)+minGrayValue;
			}
		}
		
		free(p);
		free(c);
	}
	newImg->channels = img->channels;
	newImg->imageData=newData;
	newImg->width=img->width;
	newImg->height=img->height;
	return newImg;}

void ProcessImage(const char* path,const char* hePath)
{
	Image* img = LoadImage(path);
	Image* heImage;

	printf("文件大小:%d个字节\n",img->bmpHeader.bfSize);
	printf("位图数据起始字节:%d\n",img->bmpHeader.bfOffBits);
	printf("位图宽度:%d\n",img->width);
	printf("位图高度:%d\n",img->height);
	printf("每个像素的位数:%d\n",img->infoHeader.biBitCount);

	heImage = HistogramEqualization(img);
	SaveImage(hePath,heImage);
	printf("已保存直方图均衡处理的数据到文件%s\n",hePath);

	FreeImage(heImage);
	FreeImage(img);
}

int main()
{
	ProcessImage("lena.bmp","lenahe.bmp");
	ProcessImage("lenac.bmp","lenache.bmp");
	return 0;
}

