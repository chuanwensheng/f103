#pragma once

//#define PC_DEBUG
#ifdef PC_DEBUG
#include <opencv2/opencv.hpp>
#endif

#include "base_resample.h"
#include "bitmap.h" 

namespace Tools
{
	class CImgResizer
	{
		public:

			//--- resize tool
			//P.s. srcImg & dstImg must be memory allocated before enter to resize function
			static void resize(unsigned char *srcImg, int srcImg_w, int srcImg_h, int srcImg_c, unsigned char* dstImg, int dstImg_w, int dstImg_h);

			//--- crop tool
			static void crop(unsigned char *srcImg, int srcImg_w, int srcImg_h, int srcImg_c, unsigned char* dstImg, int dsImg_x, int dstImg_y, int dstImg_w, int dstImg_h);


			//--- convert tool
			// imgVector convert tool 
#ifdef PC_DEBUG
			static void ConvertImgVecotrToImgMat(vector<base::uint8> imgVector, int img_w, int img_h, int img_c, cv::Mat &imgMat);
#endif
			static void ConvertImgVecotrToImgArray(vector<base::uint8> imgVector, unsigned char *imgArray);

			//imgArray convert tool 
			static void ConvertImgArrayToImgVector(unsigned char *imgArray, int imgArrayLen, vector<base::uint8> &imgVector);
#ifdef PC_DEBUG
			static void ConvertImgArrayToImgMat(unsigned char *imgArray, int img_w, int img_h, int img_c, cv::Mat &imgMat);
#endif
			// ImgMat convert tool
#ifdef PC_DEBUG
			static void ConvertImgMatToImgArray(cv::Mat &imgMat, int dataLen, unsigned char* imgArray);

			static void ConvertImgMatRoImgVector(cv::Mat &imgMat, vector<base::uint8> &imgVector);
#endif
	};
}

