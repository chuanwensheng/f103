#include "lds_image_process.h"

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


//保存位图图像
void saveBmp(const char *filename, unsigned char *rgbbuf, int width, int height)  
{  
	BITMAPFILEHEADER bfh;  
	BITMAPINFOHEADER bih;  

	/* Magic number for file. It does not fit in the header structure due to alignment requirements, so put it outside */  
	unsigned short bfType = 0x4d42;             
	bfh.bfReserved1 = 0;  
	bfh.bfReserved2 = 0;  
	bfh.bfSize = 2+sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + width * height * 3;
	bfh.bfOffBits = 0x36;  

	bih.biSize = sizeof(BITMAPINFOHEADER);  
	bih.biWidth = width;  
	bih.biHeight = height;  
	bih.biPlanes = 1;  
	bih.biBitCount = 24;  
	bih.biCompression = 0;  
	bih.biSizeImage = 0;  
	bih.biXPelsPerMeter = 0x0ec4/*5000*/;  
	bih.biYPelsPerMeter = 0x0ec4/*5000*/;  
	bih.biClrUsed = 0;  
	bih.biClrImportant = 0;  

	FILE *file = fopen(filename, "wb");  
	if (!file)  
	{  
		printf("Could not write file\n");  
		return;  
	}  

	/*Write headers*/  
	fwrite(&bfType,sizeof(bfType),1,file);  
	fwrite(&bfh,sizeof(bfh),1, file);  
	fwrite(&bih,sizeof(bih),1, file);  

	fwrite(rgbbuf, width * height * 3, 1, file);  
	fclose(file);  
}


namespace Tools
{
#pragma region [resize function]
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
		double ans = x1 * x2 + y1 * y2;
		double base = sqrt(x1*x1 + y1 * y1)*sqrt(x2*x2 + y2 * y2);
		ans /= base;
		return acos(ans);
	}
#pragma endregion

	bool ImageProcess::InConvexPolygon(std::vector<CVcaPoint2i> poly, int len, CVcaPoint2i p)
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

	void ImageProcess::FillConvexPolygon(unsigned char *image, int w, int h, std::vector<CVcaPoint2i> contourPoints, int iPolyNum, int fillValue)
	{
		for (int i = 0; i < iPolyNum; i++)
		{
			int x = contourPoints[i].x;
			int y = contourPoints[i].y;

			if (x < w && y < h)
			{
				image[y*w + x] = (unsigned char) fillValue;
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
						image[i*w + j] = (unsigned char)fillValue;
					}
				}
			}
		}
	}

	void ImageProcess::ConvertImgVecotrToImgArray(vector<base::uint8> imgVector, unsigned char *imgArray)
	{
		for (int i = 0; i < imgVector.size(); i++)
		{
			imgArray[i] = (unsigned char)imgVector[i];
		}
	}

	void ImageProcess::ConvertImgArrayToImgVector(unsigned char *imgArray, int imgArrayLen, vector<base::uint8> &imgVector)
	{
		for (int i = 0; i < imgArrayLen; i++)
		{
			imgVector.push_back((base::uint8)imgArray[i]);
		}
	}

	//image resize(最近邻插值法)
	void ImageProcess::resize(unsigned char *srcImg, int srcImg_w, int srcImg_h, int srcImg_c, int dstImg_w, int dstImg_h, unsigned char* dstImg)
	{
		float eps = 0.001;
		int biBitCount = 24;

		//图像每一行字节数必须是4的整数倍
		int widthByte = (srcImg_w * biBitCount / 8 + 3) / 4 * 4;
		int lineByte = (dstImg_w * biBitCount / 8 + 3) / 4 * 4;
		//缩放因子
		float scaleW = dstImg_w / (srcImg_w + eps);
		float scaleH = dstImg_h / (srcImg_h + eps);
		int x1,y1,x2,y2;
		for (int i = 0; i < dstImg_h; i++)
		{
			for (int j = 0; j < dstImg_w; j++)
			{
				for (int k = 0; k < srcImg_c; k++)
				{
					x2 = i;
					y2 = j;
					x1 = (int)(x2 / (scaleH + eps));
					y1 = (int)(y2 / (scaleW + eps));
					//选取范围
					if (x1 >= 0 && x1 <= srcImg_h & y1 >= 0 && y1 <= srcImg_w)
					{
						*(dstImg + i * lineByte + j * 3 + k) = *(srcImg + x1 * widthByte + y1 * 3 + k);
					}
				}
			}
		}
	}

	//image crop
	void ImageProcess::crop(unsigned char *srcImg, int srcImg_w, int srcImg_h, int srcImg_c, int dstImg_x, int dstImg_y, int dstImg_w, int dstImg_h, unsigned char* dstImg)
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

					//printf("(x, y, c, index): (%d, %d, %d, %d)", x, y, c, srcIdx);
				}
				//printf("\n");
			}
		}
	}

	void ImageProcess::SwapRB(unsigned char *srcImage, int w, int h, unsigned char *dstImage)
	{
		int offset = 0;
		for (int y=0; y < h; y++) 
		{
			for (int x=0; x < w; x++) 
			{
				offset = (y * w * 3) + (x*3);
				dstImage[offset] = srcImage[offset+2];
				dstImage[offset+1] = srcImage[offset+1];
				dstImage[offset+2] = srcImage[offset];
			}
		}
	}

	//rgb2bgr
	void ImageProcess::ConvertRGB2BGR(unsigned char* rgbImg, int w, int h, int channels, unsigned char* bgrImg)
	{
		int biBitCount = 24;
		int lineByte = (w * biBitCount / 8 + 3) / 4 * 4;
		for (int i = 0; i < h; i++)
		{
			for (int j = 0; j < w; j++)
			{
				*(bgrImg + i * lineByte + j * 3)     = *(rgbImg + i * lineByte + j * 3 + 2);
				*(bgrImg + i * lineByte + j * 3 + 1) = *(rgbImg + i * lineByte + j * 3 + 1);
				*(bgrImg + i * lineByte + j * 3 + 2) = *(rgbImg + i * lineByte + j * 3);
			}
		}
	}

	//YUV to Gray
	void ImageProcess::ConvertYUVtoGray(unsigned char *image, int w, int h, unsigned char *grayImage)
	{
		memcpy(grayImage, image, w*h * sizeof(unsigned char));
	}

	//BGR to Gray
	void ImageProcess::ConvertBGRtoGray(unsigned char *image, int w, int h, unsigned char *grayImage)
	{
		int channel = 3;
		int img_offset = 0;
		int gray_offset = 0;

		for (int y=0; y < h; y++) 
		{
			for (int x=0; x < w; x++) 
			{
				img_offset = (y * w * channel) + (x*channel);
				gray_offset = (y*w) + x;
				grayImage[gray_offset] = image[img_offset];
			}
		}
	}

	//1-channel to 3-channel
	void ImageProcess::Convert1Channelto3Channel(unsigned char *oneChannelImg, int w, int h, unsigned char *threeChannelImg) 
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

	//图像降采样
	void ImageProcess::pyr_down(unsigned char* imageData, int imageWidth, int imageHeight, int channel, unsigned char* dstData)
	{
		int odd = 4;
		int dwidth = imageWidth / odd;
		for (int i = 0; i < imageHeight; i+=odd)
		{
			int di = i / odd;
			for (int j = 0; j < imageWidth * channel; j+=odd)
			{
				int dj = j / odd;
				dstData[di * dwidth * channel + dj] = imageData[i * imageWidth * channel + j];
			}
		}
	}

	/*大津算法实现函数*/
	/*参数：接受指向包含图像灰度信息的空间的unsigned char型指针以及空间大小*/
	/*返回值：使得类间方差最大的阈值*/
	int ImageProcess::OTSU(unsigned char* data, int size)
	{
		int mings = 255;
		int maxgs = 0;
		for (int i = 0; i <= size - 1; i++) 
		{
			if (data[i] > maxgs) maxgs = data[i];
			if (data[i] < mings) mings = data[i];
		}

		double avg_f, avg_b, portion_f, portion_b;
		int num_f = 0, num_b = 0, total_f = 0, total_b = 0;
		int treshold = mings;
		double var, tempvar;
		for (int gs = mings; gs <= maxgs - 1; gs++) 
		{
			for (int i = 0; i <= size - 1; i++) 
			{
				if (data[i] <= gs) 
				{
					num_b++;
					total_b += data[i];
				}
				else 
				{
					num_f++;
					total_f += data[i];
				}
			}
			portion_f = num_f / size;
			portion_b = num_b / size;
			avg_f = total_f / num_f;
			avg_b = total_b / num_b;
			tempvar = portion_f * portion_b*pow(avg_f - avg_b, 2);
			if (gs == mings) 
			{
				treshold = mings;
				var = tempvar;
			}
			else 
			{
				if (tempvar > var) 
				{
					treshold = gs;
					var = tempvar;
				}
			}
		}
		return treshold;
	}
	/*二值化实现函数*/
	/*参数：接受指向包含图像灰度信息的空间的unsigned char型指针，空间大小以及阈值*/
	/*返回值：无返回值*/
	void ImageProcess::binarization(unsigned char *data, int size, int t)
	{
		for (int i = 0; i <= size - 1; i++) 
		{
			if (data[i] <= t) data[i] = 0;
			else data[i] = 255;
		}
	}

	/*自适应二值化函数*/
	/*参数：接受指向包含图像灰度信息的空间的unsigned char型指针、图像宽度、图像高度、横向块数以及纵向块数*/
	/*返回值：无返回值*/
	/*注意：用户必须保证图像的行数能够整除纵向块数，列数能够整除横向块数*/
	void ImageProcess::AdaptiveThreshold(unsigned char *data, int w, int h, int rblk, int cblk)
	{
		unsigned char *block = new unsigned char[w * h / (rblk * cblk)];
		int block_it, data_it; //data_it是作用于data数据区的迭代器，用于指示data中当前数据块有多少行已经写入block.
		int startpos;          //指示当前data中数据块的起始位置（以下标形式指示位置）。
		int treshold;
		for (int i = 0; i <= rblk - 1; i++) {
			for (int j = 0; j <= cblk - 1; j++) {
				startpos = i * h / rblk * w + j * w / cblk;
				block_it = 0;
				//将data数据区中的块赋给block.
				for (data_it = 0; data_it <= h / rblk - 1; data_it++) {
					for (int k = 0; k <= w / cblk - 1; k++) {
						block[block_it++] = data[startpos + w * data_it + k];
					}
				}
				treshold = OTSU(block, h*w / (rblk*cblk));
				binarization(block, h*w / (rblk*cblk), treshold);
				//将block数据块写回data.
				block_it = 0;
				for (data_it = 0; data_it <= h / rblk - 1; data_it++) {
					for (int k = 0; k <= w / cblk - 1; k++) {
						data[startpos + w * data_it + k] = block[block_it++];
					}
				}
			}
		}
		delete[] block;
	}

	//形态学操作
	/*膨胀*/
	/*参数：接受指向图像数据区的指针、图像宽度、图像高度以及目标（前景）的像素值*/
	/*返回值：返回指向经膨胀操作的数据区的指针*/
	unsigned char* ImageProcess::dilation(unsigned char* data, int w, int h, int value)
	{
		unsigned char *DiData = new unsigned char[w*h];
		int pos; //指示结构元中心位置
		for (int i = 0; i <= w * h; i++) 
		{
			DiData[i] = data[i];
		}
		for (int i = 1; i <= h - 2; i++) 
		{
			for (int j = 1; j <= w - 2; j++) 
			{
				pos = i * w + j;
				if (data[pos] == value) 
				{
					continue;
				}
				else if (data[pos - 1] == value || data[pos + 1] == value || data[pos - w] == value || data[pos + w] == value) 
				{
					DiData[pos] = value;
				}
			}
		}
		for (int i = 1; i <= h - 2; i++) 
		{
			pos = i * w;
			if (data[pos] == value);
			else if (data[pos - w] == value || data[pos + 1] == value || data[pos + w] == value) 
			{
				DiData[pos] = value;
			}
			pos += w - 1;
			if (data[pos] == value);
			else if (data[pos - w] == value || data[pos - 1] == value || data[pos + w] == value) 
			{
				DiData[pos] = value;
			}
		}
		for (int i = 1; i <= w - 2; i++) 
		{
			pos = i;
			if (data[pos] == value);
			else if (data[pos - 1] == value || data[pos + 1] == value || data[pos + w] == value) 
			{
				DiData[pos] = value;
			}
			pos += (h - 1)*w;
			if (data[pos] == value);
			else if (data[pos - 1] == value || data[pos + 1] == value || data[pos - w] == value) 
			{
				DiData[pos] = value;
			}
		}
		if (data[0] == value);
		else if (data[w] == value || data[1] == value) DiData[0] = value;
		if (data[w - 1] == value);
		else if (data[w - 2] == value || data[2 * w - 1] == value) DiData[w - 1] = value;
		if (data[(h - 1)*w] == value);
		else if (data[(h - 1)*w + 1] == value || data[(h - 2)*w] == value) DiData[(h - 1)*w] = value;
		if (data[h*w - 1] == value);
		else if (data[h*w - 2] == value || data[(h - 1)*w - 1] == value) DiData[h*w - 1] = value;
		return DiData;
	}

	/*腐蚀操作*/
	/*参数：接受指向图像数据区的指针、图像宽度、图像高度以及目标（前景）的像素值*/
	/*返回值：返回指向经腐蚀操作的数据区的指针*/
	unsigned char* ImageProcess::erosion(unsigned char* data, int w, int h, int value)
	{
		unsigned char *EroData = new unsigned char[w*h];
		int pos; //指示结构元中心位置
		if (value) value = 0;
		else value = 255;
		for (int i = 0; i <= w * h; i++) 
		{
			EroData[i] = data[i];
		}
		for (int i = 1; i <= h - 2; i++) 
		{
			for (int j = 1; j <= w - 2; j++) 
			{
				pos = i * w + j;
				if (data[pos] == value) 
				{
					continue;
				}
				else if (data[pos - w - 1] == value || data[pos - w] == value || data[pos - w + 1] == value || data[pos - 1] == value || data[pos + 1] == value || data[pos + w - 1] == value || data[pos + w] == value || data[pos + w + 1] == value) 
				{
					EroData[pos] = value;
				}
			}
		}
		for (int i = 1; i <= h - 2; i++) 
		{
			pos = i * w;
			if (data[pos] == value);
			else if (data[pos - w] == value || data[pos - w + 1] == value || data[pos + 1] == value || data[pos + w] == value || data[pos + w + 1] == value) 
			{
				EroData[pos] = value;
			}
			pos += w - 1;
			if (data[pos] == value);
			else if (data[pos - w] == value || data[pos - w - 1] == value || data[pos - 1] == value || data[pos + w - 1] == value || data[pos + w] == value) 
			{
				EroData[pos] = value;
			}
		}
		for (int i = 1; i <= w - 2; i++) 
		{
			pos = i;
			if (data[pos] == value);
			else if (data[pos - 1] == value || data[pos + w - 1] == value || data[pos + w] == value || data[pos + w + 1] == value || data[pos + 1] == value) 
			{
				EroData[pos] = value;
			}
			pos += (h - 1)*w;
			if (data[pos] == value);
			else if (data[pos - 1] == value || data[pos - w - 1] == value || data[pos - w] == value || data[pos - w + 1] == value || data[pos + 1] == value) 
			{
				EroData[pos] = value;
			}
		}
		if (data[0] == value);
		else if (data[1] == value || data[w + 1] == value || data[w] == value) EroData[0] = value;
		if (data[w - 1] == value);
		else if (data[w - 2] == value || data[2 * w - 2] == value || data[2 * w - 1] == value) EroData[w - 1] = value;
		if (data[(h - 1)*w] == value);
		else if (data[(h - 2)*w] == value || data[(h - 2)*w + 1] == value || data[(h - 1)*w + 1] == value) EroData[(h - 1)*w] = value;
		if (data[h*w - 1] == value);
		else if (data[h*w - 2] == value || data[(h - 1)*w - 2] == value || data[(h - 1)*w - 1] == value) EroData[h*w - 1] = value;
		return EroData;
	}

	/*开操作*/
	/*参数：接受指向图像数据区的指针、图像宽度、图像高度以及目标（前景）的像素值*/
	/*返回值：返回指向经开操作的数据区的指针*/
	unsigned char* ImageProcess::opening(unsigned char* data, int w, int h, int value)
	{
		unsigned char *temp;
		unsigned char *OpData;
		temp = erosion(data, w, h, value);
		OpData = dilation(temp, w, h, value);
		delete[] temp;
		return OpData;
	}
	/*闭操作*/
	/*参数：接受指向图像数据区的指针、图像宽度、图像高度以及目标（前景）的像素值*/
	/*返回值：返回指向经闭操作的数据区的指针*/
	unsigned char* ImageProcess::closing(unsigned char* data, int w, int h, int value)
	{
		unsigned char *temp;
		unsigned char *CloData;
		temp = dilation(data, w, h, value);
		CloData = erosion(temp, w, h, value);
		delete[] temp;
		return CloData;
	}

#pragma endregion
}