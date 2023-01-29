#include "tools_imgprocess_nocv.h"

//#define PC_DEBUG
#ifdef PC_DEBUG
#include <opencv2/opencv.hpp>
#endif 


namespace Tools
{
#pragma region [local functions]

	//--- local functions ---
	inline int _ABS(int a)
	{
		if (a < 0)
		{
			a = -a;
		}
		return a;
	}

	inline void _showList(std::vector<int> list)
	{
		for (int i = 0; i < list.size(); i++)
		{
			std::cout << list[i] << ", ";
		}
		std::cout << std::endl;
	}

	inline double _fabs(double dValue)
	{
		if (dValue < 0.0)
		{
			dValue = -dValue;
		}

		return dValue;
	}

	inline double _ang(double x1, double y1, double x2, double y2)
	{
		double eps = 0.001;
		double ans = x1 * x2 + y1 * y2;
		double base = sqrt(x1 * x1 + y1 * y1) * sqrt(x2  *x2 + y2 * y2);
		if (base <= eps) //防止越界
			ans = 0.0;
		ans /= base;
		return acos(ans);
	}

#pragma endregion

	//--- public functions ---

	//--- img process
	//filter
	void CImgProcess::ImgProc_Median3x3(unsigned char* corrupted, unsigned char* smooth, int w, int h)
	{
		for (int j = 1; j < h - 1; j++)
		{
			for (int i = 1; i < w - 1; i++)
			{
				int k = 0;
				unsigned char window[9];

				for (int jj = j - 1; jj < j + 2; ++jj)
				{
					for (int ii = i - 1; ii < i + 2; ++ii)
					{
						window[k++] = corrupted[jj * w + ii];
					}
				}

				for (int m = 0; m < 5; ++m)
				{
					int min = m;
					for (int n = m + 1; n < 9; ++n)
					{
						if (window[n] < window[min])
						{
							min = n;
						}
					}

					unsigned char temp = window[m];
					window[m] = window[min];
					window[min] = temp;
				}
				smooth[j * w + i] = window[4];
			}
		}
	}

	//--- cell process
	//downsample
	void CImgProcess::CellProc_ImgDownSample(unsigned char *mat1, int Frame_W, int Frame_H, unsigned char *pucCellMap, int CellMap_W, int CellMap_H, int Cell_W, int Cell_H)
	{
		//printf("(%d, %d) => (%d, %d)\n", Frame_W, Frame_H, CellMap_W, CellMap_H);
		// CellMap_H = Frame_H / 4
		// CellMap_W = Frame_W / 4
		// pucCellMap: 分块后的数组
		for (int y = 0; y < CellMap_H; y++, pucCellMap += CellMap_W)
		{
			int iFrame_Y_shift = y * Cell_H * Frame_W;
			for (int x = 0; x < CellMap_W; x++)
			{
				int iFrame_offset = iFrame_Y_shift + x * Cell_W;
				int ipixelvalue = CellProc_CalMedianValue(mat1 + iFrame_offset, Frame_W, Cell_W, Cell_H);
				pucCellMap[x] = (unsigned char)ipixelvalue;
			}
		}
	}

	//frame difference
	void CImgProcess::FrameDiff_CellByCell(unsigned char *pre_Frame, unsigned char *cut_Frame, int Frame_W,	int Frame_H,
		int iThreshold, int iThresholdNum, 	
		unsigned char *cellMap, int iStride_CellMap, int iCellMapHeight,
		int iCell_W,int iCell_H	
	)
	{
		if (iThreshold <= 0)
		{
			iThreshold = DEFAULT_FRAME_DIFF_THR;
		}

		if (iThresholdNum <= 0)
		{
			iThresholdNum = iCell_W* iCell_H / 4;
		}

#ifdef FD_HALF_CRLL
		iThresholdNum = iThresholdNum / 4;
#endif

		unsigned char *pucCellMap = cellMap;
		int iStride = Frame_W;
		//int iStride_CellMap = Frame_W/iCell_W;
		//int iCellMapHeight = Frame_H / iCell_H;

		for (int y = 0; y < iCellMapHeight; y++, pucCellMap += iStride_CellMap)
		{
			int iFrameYShift = y * iCell_H* iStride;
			for (int x = 0; x < iStride_CellMap; x++)
			{
				int ioffset = iFrameYShift + x * iCell_W;
				if (IsEnoughCellDiff_4Byte(pre_Frame + ioffset, cut_Frame + ioffset, iThreshold, iThresholdNum, iStride, iCell_W, iCell_H))
				{
					pucCellMap[x] |= 255;
				}
				else
				{
					pucCellMap[x] = 0;
				}
			}
		}
	}

	//frame difference
	bool CImgProcess::IsEnoughCellDiff_4Byte(unsigned char *pCurFrame, unsigned char *pPreFrame, int iThreshold, int iThresholdNum, int iStride, int iCell_W, int iCell_H)
	{
		int iSum = 0;
		int iCnt = 0;
#ifdef FD_HALF_CRLL
		iStride /= 2;
#else
		iStride /= 4;
#endif

		int iProcessWidth = iCell_W / 4;
		//printf("iProcessWidth = %d, iCell_W = %d\n", iProcessWidth, iCell_W);
		unsigned int *puiCur = (unsigned int*)pCurFrame;
		unsigned int *puiPre = (unsigned int*)pPreFrame;

#ifdef FD_HALF_CRLL
		for (int y = 0; y < iCell_H; y += 4)
#else
		for (int y = 0; y < iCell_H; y++)
#endif
		{
			int ilocalXNum = 0;
			for (int x = 0; x < iProcessWidth; x++)
			{
				int iDiff = 0;
				unsigned int uiCur = puiCur[x];
				unsigned int uiPre = puiPre[x];

				iDiff = _ABS((int)((uiCur & BITMASK_0) >> BITOFFSET_0) - (int)((uiPre & BITMASK_0) >> BITOFFSET_0));
				iSum += (static_cast<int16_t>(iThreshold - iDiff) >> 15) & 0x01;

#ifndef FD_HALF_CRLL
				iDiff = std::abs((int)((uiCur & BITMASK_1) >> BITOFFSET_1) - (int)((uiPre & BITMASK_1) >> BITOFFSET_1));
				iSum += (static_cast<int16_t>(iThreshold - iDiff) >> 15) & 0x01;
#endif

				iDiff = _ABS((int)((uiCur & BITMASK_2) >> BITOFFSET_2) - (int)((uiPre & BITMASK_2) >> BITOFFSET_2));
				iSum += (static_cast<int16_t>(iThreshold - iDiff) >> 15) & 0x01;
#ifndef FD_HALF_CRLL
				iDiff = std::abs((int)((uiCur & BITMASK_3) >> BITOFFSET_3) - (int)((uiPre & BITMASK_3) >> BITOFFSET_3));
				iSum += (static_cast<int16_t>(iThreshold - iDiff) >> 15) & 0x01;
#endif

				iCnt += 2;
		

				ilocalXNum += 2;
				//printf("%d, ilocalXNum = %d\n",x, ilocalXNum);

			}

			//printf("ilocalXNum = %d\n", ilocalXNum);
			//system("pause");
			puiCur += iStride;
			puiPre += iStride;
		}

		//printf("%d/%d %d\n", iSum, iCnt, iThresholdNum); system("pause");

		return (iSum > iThresholdNum);
	}
	
	//---
	void CImgProcess::FillConvexPolygon(unsigned char *image, int w, int h, std::vector<CVcaPoint2i> contourPoints, int iPolyNum, int fillValue)
	{
		for (int i = 0; i < iPolyNum; i++)
		{
			int x = contourPoints[i].x;
			int y = contourPoints[i].y;

			if (x < w && y < h)
			{
				image[y * w + x] = (unsigned char)fillValue;
			}
		}

		for (int i = 0; i < h; i++)
		{
			for (int j = 0; j < w; j++)
			{
				CVcaPoint2i point;
				point.x = j;
				point.y = i;

				if (point.x < w && point.y < h)
				{
					if (InConvexPolygon(contourPoints, iPolyNum, point))
					{
						image[i * w + j] = (unsigned char)fillValue;
					}
				}
			}
		}
	}

	//---
	void CImgProcess::SwapRB(unsigned char *srcImage, int w, int h, unsigned char *dstImage)
	{
		int offset = 0;
		for (int y=0; y < h; y++) 
		{
			for (int x=0; x < w; x++) 
			{
				offset = (y * w * 3) + (x * 3);
				dstImage[offset] = srcImage[offset + 2];
				dstImage[offset + 1] = srcImage[offset + 1];
				dstImage[offset + 2] = srcImage[offset];
			}
		}
	}

	void CImgProcess::ConvertBGRtoGray(unsigned char *image, int w, int h, unsigned char *grayImage) 
	{
		int channel = 3;
		int img_offset = 0;
		int gray_offset = 0;

		for (int y=0; y < h; y++) 
		{
			for (int x=0; x < w; x++) 
			{
				img_offset = (y * w + x ) * channel;
				gray_offset = y * w + x;

				grayImage[gray_offset] = image[img_offset];
			}
		}
	}

	void CImgProcess::ConvertYUVtoGray(unsigned char *image, int w, int h, unsigned char *grayImage)
	{
		memcpy(grayImage, image, w * h * sizeof(unsigned char));
		/*int offset = 0;
		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{
				offset = (y * w) + x;
				grayImage[offset] = image[offset];
			}
		}*/
	}

	void CImgProcess::Convert1Channelto3Channel(unsigned char *oneChannelImg, int w, int h, unsigned char *threeChannelImg) 
	{
		int dataLen = w * h;
		int offset_1chImg = 0;
		int offset_3chImg = 0;
		for (int idx=0; idx < dataLen; idx++)
		{
			offset_1chImg = idx;
			offset_3chImg = 3 * idx;

			threeChannelImg[offset_3chImg] = oneChannelImg[offset_1chImg];
			threeChannelImg[offset_3chImg+1] = oneChannelImg[offset_1chImg];
			threeChannelImg[offset_3chImg + 2] = oneChannelImg[offset_1chImg];
		}
	}

	//--- private functions ---
	//cell process
	int  CImgProcess::CellProc_CalMedianValue(unsigned char *pCurFrame, int iStride, int iCellWidth=4, int iCellHeight=4)
	{
#ifdef PC_DEBUG
		printf("---------- CalMedianValue start ----------\n");
		printf("iCellWidth: %d, iCellHeight: %d\n", iCellWidth, iCellHeight);
#endif
        //iStride:偏移 
		//get value of cells 
		int iMedian = 0;
		std::vector<int> pixelValueList;
#ifdef PC_DEBUG
		int count = 0;
#endif
		for (int y = 0; y < iCellHeight; y++)
		{
			int y_shift = y * iStride;
			for (int x = 0; x < iCellWidth; x++)
			{
				//printf("  y: %d\n", y);
				//printf("  x: %d\n", x);

				int offset = y_shift + x;
				unsigned char pixelValue = (unsigned char)(*(pCurFrame+ offset));
				int pixelValue_int = (int)(*(pCurFrame + offset));

#ifdef PC_DEBUG
				//std::cout <<  "cell[" << count <<  "]: " << pixelValue << std::endl;
				printf("cell[%d]: %d\n", count, pixelValue_int);
#endif
				pixelValueList.push_back(pixelValue);
#ifdef PC_DEBUG
				count++;
#endif
			}
		}
		
		// sort
#ifdef PC_DEBUG
		std::cout << "--- before sort ---" << std::endl;
		_showList(pixelValueList);
#endif
		std::sort(pixelValueList.begin(), pixelValueList.end());
#ifdef PC_DEBUG
		std::cout << "--- after sort ---" << std::endl;
		_showList(pixelValueList);
#endif
		
		// get the median value
		int median_idx = pixelValueList.size() / 2;
		iMedian = pixelValueList[median_idx];
#ifdef PC_DEBUG
		printf("iMedian: %d\n", iMedian);
		printf("---------- CalMedianValue end ----------\n");
		system("pause");
#endif
		
		return iMedian;
	}

	//
	bool CImgProcess::InConvexPolygon(std::vector<CVcaPoint2i> poly, int len, CVcaPoint2i p)
	{
		double angle = 0;
		for (int i = 0; i < len; i++)
		{
			double x1 = poly[i].x - p.x;
			double y1 = poly[i].y - p.y;
			double x2 = poly[(i + 1) % len].x - p.x;
			double y2 = poly[(i + 1) % len].y - p.y;
			angle += _ang(x1, y1, x2, y2);
		}
		if (_fabs(angle - 2 * 3.1415926) < 0.000001)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

}