#pragma once

#include <stdint.h>
#include <vector>
#include <algorithm>
#include <cstring>
#include <math.h>

//share param
#define BITMASK_0	0xFF
#define BITMASK_1   0xFF00
#define BITMASK_2   0xFF0000
#define BITMASK_3   0xFF000000
#define BITOFFSET_0 0
#define BITOFFSET_1 8
#define BITOFFSET_2 16
#define BITOFFSET_3 24

//frame difference param
#define DEFAULT_FRAME_DIFF_THR 15
#define FD_HALF_CRLL

//#define PCDEBUG
#ifdef PCDEBUG
#include <opencv2/opencv.hpp>
#endif 

#include "c_vcaobjs.h"


namespace Tools
{
	class CImgProcess
	{
		public:
			//--- img process
			//filter
			static void ImgProc_Median3x3(unsigned char* corrupted, unsigned char* smooth, int w, int h);

			//--- cell process
			//downsample
			static void CellProc_ImgDownSample(unsigned char *mat1, int Frame_W, int Frame_H, unsigned char *cellMap, int CellMap_W, int CellMap_H, int Cell_W, int Cell_H);

			//frame difference
			static void FrameDiff_CellByCell(unsigned char *pre_Frame, unsigned char *cut_Frame, int Frame_W, int Frame_H,
				int iThreshold, int iThresholdNum,
				unsigned char *cellMap, int iStride_CellMap, int iCellMapHeight,
				int iCell_W, int iCell_H
			);

			static bool IsEnoughCellDiff_4Byte(unsigned char *pCurFrame, unsigned char *pPreFrame, int iThreshold, int iThresholdNum, int iStride, int iCell_W, int iCell_H);

			//---
			static void FillConvexPolygon(unsigned char *image, int w, int h, std::vector<CVcaPoint2i> contourPoints, int iPolyNum, int fillValue);

			//---
			static void SwapRB(unsigned char *srcImage, int w, int h, unsigned char *dstImage);

			static void ConvertBGRtoGray(unsigned char *image, int w, int h, unsigned char *grayImage);
			static void ConvertYUVtoGray(unsigned char *image, int w, int h, unsigned char *grayImage);
			static void Convert1Channelto3Channel(unsigned char *oneChannelImg, int w, int h, unsigned char *threeChannelImg);

			//---		
			//cell process
			static int CellProc_CalMedianValue(unsigned char *pCurFrame, int iStride, int iCellWidth, int iCellHeight);

			//---
			//check is point inside the poly
			static bool InConvexPolygon(std::vector<CVcaPoint2i> poly, int len, CVcaPoint2i p);

			// -----------add other process code------------
			static void ConvertBGR2Gray(unsigned char *image, int w, int h, unsigned char *grayImage);
	        static void ConvertYUV2BGR(unsigned char *image, int w, int h, unsigned char *grayImage);
			
	};

}