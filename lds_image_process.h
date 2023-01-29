#pragma once

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <algorithm>
#include "base_resample.h"
#include "c_vcaobjs.h"

using namespace std;


typedef struct  
{  
    unsigned int   bfSize;           /* Size of file */  
    unsigned short bfReserved1;      /* Reserved */  
    unsigned short bfReserved2;      /* ... */  
    unsigned int   bfOffBits;        /* Offset to bitmap data */  
} BITMAPFILEHEADER;

typedef struct
{  
    unsigned int   biSize;           /* Size of info header */  
    int            biWidth;          /* Width of image */  
    int            biHeight;         /* Height of image */  
    unsigned short biPlanes;         /* Number of color planes */  
    unsigned short biBitCount;       /* Number of bits per pixel */  
    unsigned int   biCompression;    /* Type of compression to use */  
    unsigned int   biSizeImage;      /* Size of image data */  
    int            biXPelsPerMeter;  /* X pixels per meter */  
    int            biYPelsPerMeter;  /* Y pixels per meter */  
    unsigned int   biClrUsed;        /* Number of colors used */  
    unsigned int   biClrImportant;   /* Number of important colors */  
} BITMAPINFOHEADER;

typedef struct tagRGBQUAD
{  
    unsigned char rgbBlue; 
    unsigned char rgbGreen;
    unsigned char rgbRed; 
    unsigned char rgbReserved; 
} RGBQUAD;

//保存bmp位图
void saveBmp(const char *filename, unsigned char *rgbbuf, int width, int height);


namespace Tools
{
    class ImageProcess
    {
        public:
            ImageProcess();
            ~ImageProcess();

            //图像分割算法
            int OTSU(unsigned char* data, int size);
            void binarization(unsigned char *data, int size, int t);
            void AdaptiveThreshold(unsigned char *data, int w, int h, int rblk, int cblk);

            //几何变换
            static void ConvertImgVecotrToImgArray(vector<base::uint8> imgVector, unsigned char *imgArray);
            static void ConvertImgArrayToImgVector(unsigned char *imgArray, int imgArrayLen, vector<base::uint8> &imgVector);

            static void resize(unsigned char *srcImg, int srcImg_w, int srcImg_h, int srcImg_c, int dstImg_w, int dstImg_h, unsigned char* dstImg);
            static void crop(unsigned char *srcImg, int srcImg_w, int srcImg_h, int srcImg_c, int dsImg_x, int dstImg_y, int dstImg_w, int dstImg_h, unsigned char* dstImg);

            static void SwapRB(unsigned char *srcImage, int w, int h, unsigned char *dstImage);
            static void ConvertBGRtoGray(unsigned char *image, int w, int h, unsigned char *grayImage);
            static void ConvertYUVtoGray(unsigned char *image, int w, int h, unsigned char *grayImage);

            //check is point inside the poly
			static bool InConvexPolygon(std::vector<CVcaPoint2i> poly, int len, CVcaPoint2i p);
            static void FillConvexPolygon(unsigned char *image, int w, int h, std::vector<CVcaPoint2i> contourPoints, int iPolyNum, int fillValue);

            static void Convert1Channelto3Channel(unsigned char *oneChannelImg, int w, int h, unsigned char *threeChannelImg);
            
            //rgb2bgr
            static void ConvertRGB2BGR(unsigned char* rgbImg, int w, int h, int channels, unsigned char* bgrImg);

            //降采样
            static void pyr_down(unsigned char* imageData, int imageWidth, int imageHeight, int channel, unsigned char* dstData);
            //上采样
            static void pyr_up(unsigned char* srcImg, int width, int height, unsigned char* dstImg);

            //形态学操作
            static unsigned char *dilation(unsigned char *data, int w, int h, int value);
            static unsigned char *erosion(unsigned char *data, int w, int h, int value);
            static unsigned char *opening(unsigned char *data, int w, int h, int value);
            static unsigned char *closing(unsigned char *data, int w, int h, int value);

        private:
            int m_nWidth;
            int m_nHeight;
            int m_nSize;
    };
}