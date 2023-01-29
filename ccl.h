#pragma once

#include "common_use_geometry_struct.h"
#define MAX_CCL_REGION_NUM 100
class CCCL
{
public:
	int labeling(unsigned char *srcIMG, int *lab, int IMG_WIDTH, int IMG_HEIGHT, int IMG_BOUND);
	int GenCCLRegion(_Rect2i *m_CCL_Region, int *m_iCCL_IDlist , int maxNum, int *lab, int width, int height);
};

void Insert2(_Rect2i TargetRect, int m_queueNumber, _Rect2i *m_CCL_Region);