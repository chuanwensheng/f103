#include "ccl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// execute 4-connected component label
// srcIMG: 影像資料流
// pLabel: 每個pixel的CCL
// width: 影像高
// height: 影像寬
// bound: 邊界pixel數 (4ccl最小設1)
int CCCL::labeling(unsigned char *srcIMG, int *pLabel, int width, int height, int bound)
{
	memset(pLabel, 0, width*height * sizeof(int));
	int y = 0, x = 0;
	int ni = 0, nj = 0;
	int a = 0, b = 0;
	int laNo = 1;
	int labAcc = 0;
	int iRow = 0;
	//labeling

	for (y = bound; y<(height - bound); y++)
	{
		iRow = iRow + width;
		for (x = bound; x<(width - bound);x++)
		{
			//initialize
			// idx = y * IMG_WIDTH + x;
			int idx = iRow + x;
			pLabel[idx] = 0;

			if (srcIMG[idx] != 0)
			{
				a = pLabel[idx - width];
				b = pLabel[idx - 1];

				if ((a == 0) && (b == 0))
				{
					pLabel[idx] = laNo;
					laNo++;
					labAcc++;
				}
				else if ((a != 0) && (b == 0))
				{
					pLabel[idx] = a;
				}
				else if ((a == 0) && (b != 0))
				{
					pLabel[idx] = b;
				}
				else
				{
					if (a == b) pLabel[idx] = a;
					if (a != b)
					{
						pLabel[idx] = a;

						//update
						for (ni = y; ni>0; ni--)
						{
							for (nj = x; nj>0; nj--)
							{
								if (pLabel[ni*width + nj] == b) pLabel[ni*width + nj] = a;
							}
						}
						//laNo--;
						labAcc--;
					}
				}
			}
		}
	}

	return 0;
}

_Rect2i FindRect(int id, int *pLabel, int width, int height)
{
	_Rect2i cclRect;
	cclRect.x = width - 1;
	cclRect.y = height - 1;
	cclRect.width = 0;
	cclRect.height = 0;

	int iContainNum = 0;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			if (pLabel[y*width +x] == id)
			{
				iContainNum++;
				if (cclRect.x > x)
				{
					cclRect.x = x;
				}

				if (cclRect.y > y)
				{
					cclRect.y = y;
				}

				if (cclRect.width < x)
				{
					cclRect.width = x;
				}

				if (cclRect.height < y)
				{
					cclRect.height = y;
				}
			}
		}
	}

	cclRect.width = cclRect.width - cclRect.x + 1;
	cclRect.height = cclRect.height - cclRect.y + 1;
	cclRect.iContainNum = iContainNum;

	return cclRect;
}

// get CCL result info
// m_CCL_Region: bbox x, y, w, h
// m_iCCL_IDlist: bbox id (=ccl)
// maxNum: max ccl region number
// lab: 每個pixel的ccl值
// width: 影像寬
// height: 影像高
int CCCL::GenCCLRegion(_Rect2i *m_CCL_Region, int *m_iCCL_IDlist, int maxNum, int *lab, int width, int height)
{	
	memset(m_CCL_Region, 0, maxNum * sizeof(_Rect2i));
	memset(m_iCCL_IDlist, 0, maxNum * sizeof(int));

	int iidArray[MAX_CCL_REGION_NUM];

	for (int i = 0; i < MAX_CCL_REGION_NUM; i++)
	{
		iidArray[i] = 0;
	}

	int iIdNum = 0;
	int ibuflen = width * height;

	for (int id = 1; id < ibuflen; id++)
	{
		if (lab[id] < MAX_CCL_REGION_NUM)
		{
			iidArray[lab[id]]++;
		}		
	}

	for (int i = 1; i < MAX_CCL_REGION_NUM; i++)
	{
		if (iidArray[i] > 0)
		{		
			int icurid = i;
			_Rect2i TargetRect = FindRect(icurid, lab, width, height);
			m_CCL_Region[iIdNum] = TargetRect;
			m_iCCL_IDlist[iIdNum] = icurid;
			iIdNum++;

			if (maxNum <= iIdNum)
			{
				break;
			}
		}
	}

	/*printf("ccl ===========\n");
	for (int icc = 0; icc < iIdNum; icc++)
	{
		printf("%d, %d, %d, %d, %d, %d\n", icc, m_CCL_Region[icc].x, m_CCL_Region[icc].y, m_CCL_Region[icc].width, m_CCL_Region[icc].height, m_CCL_Region[icc].width * m_CCL_Region[icc].height);	
	}
	printf("ccl end===========\n");//*/
	return iIdNum;
}