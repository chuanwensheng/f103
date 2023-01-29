#include "tools_imgresizer_nocv.h"

namespace Tools
{

#pragma region [resize function]
	void CImgResizer::resize(unsigned char *srcImg, int srcImg_w, int srcImg_h, int srcImg_c, unsigned char* dstImg, int dstImg_w, int dstImg_h) 
	
	{
		//convert imgArray to imgVector 
		printf("srcImg_w, srcImg_h, srcImg_c %d, %d, %d\n", srcImg_w, srcImg_h, srcImg_c);
		int srcDataLen = srcImg_w * srcImg_h* srcImg_c;
		vector<base::uint8> src_imgVector;
		ConvertImgArrayToImgVector(srcImg, srcDataLen, src_imgVector);

		//do resize
		base::KernelType kernel = (base::KernelType)atoi("1");
		vector<base::uint8> dst_imgVector(dstImg_w * dstImg_h * srcImg_c);
		//printf("[CImgResizer] resize img: (%i x %i) -> (%i x %i) , kernel: %i \n", srcImg_w, srcImg_h, dstImg_w, dstImg_h, kernel);
		if (!base::ResampleImage24(&src_imgVector.at(0), srcImg_w, srcImg_h, &dst_imgVector.at(0), dstImg_w, dstImg_h, kernel))
		{
			printf("[CImgResizer] Error resampling image!\n");
			return;
		}

		//convert imgVector to imgArray
		ConvertImgVecotrToImgArray(dst_imgVector, dstImg);
 	}
#pragma endregion

#pragma region [crop image function] 
	void CImgResizer::crop(unsigned char *srcImg, int srcImg_w, int srcImg_h, int srcImg_c, unsigned char* dstImg, int dstImg_x, int dstImg_y, int dstImg_w, int dstImg_h)
	{
		int dstIdx = 0;

		for (int y = dstImg_y; y < (dstImg_y + dstImg_h); y++)
		{
			for (int x = dstImg_x; x < (dstImg_x + dstImg_w); x++)
			{
				for (int c = 0; c < srcImg_c; c++)
				{
					int srcIdx = y * srcImg_w*srcImg_c + x * srcImg_c + c;
					dstImg[dstIdx] = srcImg[srcIdx];
					dstIdx++;
				}
			}
		}
	}
#pragma endregion


#pragma region [convert tool function]
	//--- convert tool function
	// imgVector convert tool 
#ifdef PC_DEBUG
	void CImgResizer::ConvertImgVecotrToImgMat(vector<base::uint8> imgVector, int img_w, int img_h, int img_c, cv::Mat &imgMat)
	{
		if (img_c == 1)  // 1-channel
		{
			imgMat = cv::Mat(img_h, img_w, CV_8UC1, cv::Scalar(0));
			for (int y = 0; y < imgMat.size().height; y++)
			{
				for (int x = 0; x < imgMat.size().width; x++)
				{
					int offset = (y * img_w) + x;
					imgMat.at<uchar>(y, x) = (unsigned char)imgVector[offset];
				}
			}
		}
		else //3-channel
		{
			imgMat = cv::Mat(img_h, img_w, CV_8UC3, cv::Scalar(0, 0, 0));

			for (int y = 0; y < imgMat.size().height; y++)
			{
				for (int x = 0; x < imgMat.size().width; x++)
				{
					int offset = (y * img_w  * img_c) + (x * img_c);
					for (int c = 0; c < img_c; c++)
					{
						imgMat.at<cv::Vec3b>(y, x)[c] = (unsigned char)imgVector[offset + c];
					}
				}
			}
		}
	}
#endif

	void CImgResizer::ConvertImgVecotrToImgArray(vector<base::uint8> imgVector, unsigned char *imgArray)
	{
		for (int i = 0; i < imgVector.size(); i++)
		{
			imgArray[i] = (unsigned char)imgVector[i];
		}
	}

	//imgArray convert tool 
	void CImgResizer::ConvertImgArrayToImgVector(unsigned char *imgArray, int imgArrayLen, vector<base::uint8> &imgVector)
	{
		for (int i = 0; i < imgArrayLen; i++)
		{
			imgVector.push_back((base::uint8)imgArray[i]);
		}
	}

#ifdef PC_DEBUG
	void CImgResizer::ConvertImgArrayToImgMat(unsigned char *imgArray, int img_w, int img_h, int img_c, cv::Mat &imgMat)
	{
		if (img_c == 1) //1-channel
		{
			imgMat = cv::Mat(img_h, img_w, CV_8UC1, cv::Scalar(0));
			int offset = 0;
			for (int y = 0; y < img_h; y++)
			{
				for (int x = 0; x < img_w; x++)
				{
					offset = (y*img_w) + x;
					imgMat.at<uchar>(y, x) = (unsigned char)imgArray[offset];
				}
			}
		}
		else //3-channel
		{
			imgMat = cv::Mat(img_h, img_w, CV_8UC3, cv::Scalar(0, 0, 0));
			int offset = 0;
			for (int y = 0; y < img_h; y++)
			{
				for (int x = 0; x < img_w; x++)
				{
					for (int c = 0; c < img_c; c++)
					{
						offset = (y*img_w*img_c) + (x*img_c) + c;
						imgMat.at<cv::Vec3b>(y, x)[c] = (unsigned char)imgArray[offset];
					}
				}
			}
		}
	}
#endif

	// ImgMat convert tool
#ifdef PC_DEBUG
	void CImgResizer::ConvertImgMatToImgArray(cv::Mat &imgMat, int dataLen, unsigned char* imgArray)
	{
		memcpy(imgArray, imgMat.data, dataLen * sizeof(unsigned char));
	}


	void CImgResizer::ConvertImgMatRoImgVector(cv::Mat &imgMat, vector<base::uint8> &imgVector)
	{
		int dataLen = imgMat.size().width * imgMat.size().height * imgMat.channels();
		for (int i = 0; i < dataLen; i++)
		{
			imgVector.push_back((base::uint8)imgMat.data[i]);
		}
	}
#endif

#pragma endregion

}