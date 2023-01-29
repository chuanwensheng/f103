#include "sigmastar_f103_vca.h"

static string getSysTime()
{
    struct timeval tv; 
    struct timezone tz;    
    struct tm *p;    
    gettimeofday(&tv, &tz);
	p = localtime(&tv.tv_sec);

    char time_str[256];
	sprintf(time_str, "%d_%02d_%02d_%02d_%02d_%02d_%03d", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, tv.tv_usec/1000);
	string dateName = string(time_str);
    return dateName;
}

// #define PC_DEBUG
#ifdef PC_DEBUG
#include <opencv2/opencv.hpp>


// InitLogging("", INFO, "./data/log");


//--- functions for show ---
inline void my_printMat_1channel(cv::Mat &img)
{
	printf("=========== src image =============\n");
	for (int i_row = 0; i_row < img.rows; i_row++)
	{
		for (int i_col = 0; i_col < img.cols; i_col++)
		{
			uchar garyPixel = img.at<uchar>(i_row, i_col);

			printf("%4d,", garyPixel);
		}
		printf("\n");
	}
	printf("===================================\n");
	//system("pause");
}

//color table
cv::Scalar color_table[] =
{
	// B, G, R
	cv::Scalar(0, 0, 255),		// 0: Red
	cv::Scalar(0, 255, 0),		// 1: Lime
	cv::Scalar(255, 0, 0),		// 2: Blue
	cv::Scalar(0, 0, 128),		// 3: Maroon
	cv::Scalar(0, 128, 0),		// 4: Green
	cv::Scalar(128, 0, 0),		// 5: Navy
	cv::Scalar(0, 255, 255),	// 6: Yellow
	cv::Scalar(255, 255, 0),	// 7: Aqua
	cv::Scalar(255, 0, 255),	// 8: Fuchsia
	cv::Scalar(0, 128, 128),	// 9: Olive
	cv::Scalar(128, 128, 0),	// 10: Teal
	cv::Scalar(128, 0, 128),	// 11: Purple
	cv::Scalar(0, 128,255)		// 12: Orange
};

//get color by id
cv::Scalar GetColor(int iId)
{
	return color_table[iId % (sizeof(color_table) / sizeof(cv::Scalar))];
}

//draw ROI
void DrawROIs(cv::Mat &SrcMat, CVcaROI &roi, int lineThickness)
{
	//get ROI contour and convert type Point2f to Point
	std::vector<std::vector<cv::Point>> ROIContours;
	std::vector<cv::Point> tmpContour;
	//if (roi.roiEnable) 
	//{
		for (int i_point = 0; i_point < roi.roiPoints.size(); i_point++)
		{
			int p_x = roi.roiPoints[i_point].x * SrcMat.size().width;
			int p_y = roi.roiPoints[i_point].y * SrcMat.size().height;
			tmpContour.push_back(cv::Point(p_x, p_y));
		}
		ROIContours.push_back(tmpContour);
	//}

	//draw the contours
	cv::Scalar color;
	if (roi.isMotionTigger && roi.isHumanTigger) 
	{
		color = GetColor(0);	//motion & human evnet: red
	}
	else if (roi.isMotionTigger)
	{
		color = GetColor(6);	//motion evnet: yellow
	}
	else if (roi.isHumanTigger)
	{
		color = GetColor(12);   //human evnet: orange
	}
	else 
	{
		color = GetColor(4);	//no event: green
	}
	cv::drawContours(SrcMat, ROIContours, 0, color, lineThickness + 1);

	//draw text
	std::string motionEventText = (roi.isMotionTigger)? " M:1" : " M:0";
	std::string humanEventText = (roi.isHumanTigger) ? " H:1" : " H:0";
	std::string text = "ID:" + std::to_string(roi.id) + motionEventText + humanEventText;
	cv::putText(SrcMat, text, ROIContours[0][0], cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 0, 0), 2);
}

//--- for CCL
void ScaleImageShowCCL(int* dataArrayPtr, int iwidth, int iheight, cv::Mat& Src, cv::Size2i scale)
{
	int LargeWidth = iwidth * scale.width;
	int LargeHeight = iheight * scale.height;
	cv::Mat LargeReprojectImg = cv::Mat::zeros(cv::Size2i(LargeWidth, LargeHeight), CV_8UC3);

	//int idx;
	for (int i = 0; i < iheight; i++)
	{
		//idx = i * iwidth;
		for (int j = 0; j < iwidth; j++)
		{
			int idx2 = i * iwidth + j;
			if (dataArrayPtr[idx2] > 0)
			{
				//printf("%d, ", dataArrayPtr[idx2]);
				for (int y = 0; y < scale.height; y++)
				{
					for (int x = 0; x < scale.width; x++)
					{
						cv::Scalar color = GetColor(dataArrayPtr[idx2]);
						LargeReprojectImg.at<cv::Vec3b>(i * scale.width + y, j * scale.height + x)[0] = color[0];
						LargeReprojectImg.at<cv::Vec3b>(i * scale.width + y, j * scale.height + x)[1] = color[1];
						LargeReprojectImg.at<cv::Vec3b>(i * scale.width + y, j * scale.height + x)[2] = color[2];
					}
				}
			}
		}
	}
	Src = LargeReprojectImg;
}
#endif 

#define CAL_TIMECOST
#pragma region [local function]

//local functions
inline static int Max(int Data1, int Data2)
{
	int Result = Data1 - Data2;
	if (Result > 0)
	{
		return Data1;
	}
	else
	{
		return Data2;
	}
}

inline static int Min(int Data1, int Data2)
{
	int Result = Data1 - Data2;
	if (Result > 0)
	{
		return Data2;
	}
	else
	{
		return Data1;
	}
}

inline bool SortBboxByScore(CVcaBBox a, CVcaBBox b)
{
	return (a.score) > (b.score);
}

inline bool SortBboxByArea(CVcaBBox a, CVcaBBox b)
{
	return (a.x2-a.x1)*(a.y2-a.y1) > (b.x2 - b.x1)*(b.y2 - b.y1);
}

inline bool SortObjByUnshow(CVcaObj a, CVcaObj b)
{
	return (a.unshowCounter) < (b.unshowCounter);
}


inline void PrintObjList(std::string titleStr ,std::vector<CVcaObj> &objList)
{
	std::cout << "****** " << titleStr << " ******" << std::endl;
	for (int i=0; i < objList.size(); i++)
	{
		std::cout << "List[" << i << "]\n" << objList[i] << std::endl;
	}
	std::cout << "********************************" << std::endl;
}

#pragma endregion

//Constructor
CVcaAnalyzer::CVcaAnalyzer() 
{
	m_iFrameCnt = 0;
	m_skip = 3;

	//set downsample cell size 
	m_downsampleCellSize.width = 4;
	m_downsampleCellSize.height = 4;

	//human classifier model default path
	m_humanClassifyModelPath = "./models/yft.m";

	//tracking
	m_curMaxTrackerId = 0;
	m_matching_NoShowCntEraseThreshold = 3;
	m_isInitedFlag = false;

	//google::InitGoogleLogging("lds");
    //google::SetLogDestination(google::INFO, "./log_");
}

//Destructor
CVcaAnalyzer::~CVcaAnalyzer() 
{
	for (int i_roi=0; i_roi < ROI_MAX_NUM; i_roi++)
	{
		if (m_vcaSettingOpt.m_RoiList[i_roi].RoiBitmap != NULL) 
		{
			free(m_vcaSettingOpt.m_RoiList[i_roi].RoiBitmap);
			m_vcaSettingOpt.m_RoiList[i_roi].RoiBitmap = NULL;
		}
	}
}

#pragma region [°Ñ¼Æ³]©w]
//scenarioSetting: [true: indoor, false: outdoor] 
void CVcaAnalyzer::Set_Scenario(bool scenarioSetting) 
{
	if (scenarioSetting)
	{
		m_vcaSettingOpt.m_eScenario = EScenario::eInDoor;
	}
	else 
	{
		m_vcaSettingOpt.m_eScenario = EScenario::eOutDoor;
	}
}

//dayNightModeSetting: [true: dayMode, false: nightMode]
void CVcaAnalyzer::Set_DayNightMode(bool dayNightModeSetting) 
{
	if (dayNightModeSetting)
	{
		m_vcaSettingOpt.m_eDayNightMode = EDayNightMode::eDayMode;
	}
	else
	{
		m_vcaSettingOpt.m_eDayNightMode = EDayNightMode::eNightMode;
	}
}

//motionOnOffFlag: [true: on, false: off]
void CVcaAnalyzer::Set_MotionOnOffFlag(bool motionOnOffFlag) 
{
	m_vcaSettingOpt.m_motionOnOffFlag = motionOnOffFlag;
}

//motionSensitiveLevel: [1,2,3,4,5], ¼Æ¦r¶V¤j¶VÆF±Ó 
//motionSensitiveLevel -> m_motionThreshold
//  1 -> 0.3
//  2 -> 0.2
//  3 -> 0.1
//  4 -> 0.05
//  5 -> 0.01
void CVcaAnalyzer::Set_MotionSensitiveLevel(int motionSensitiveLevel) 
{
	if (1 <= motionSensitiveLevel &&  motionSensitiveLevel <= 5)
	{
		m_vcaSettingOpt.m_motionSensitiveLevel = motionSensitiveLevel;
		switch (motionSensitiveLevel)
		{
			case 1:
				m_vcaSettingOpt.m_motionThreshold = 0.3;
				break;
			case 2:
				m_vcaSettingOpt.m_motionThreshold = 0.2;
				break;
			case 3:
				m_vcaSettingOpt.m_motionThreshold = 0.1;
				break;
			case 4:
				m_vcaSettingOpt.m_motionThreshold = 0.05;
				break;
			case 5:
				m_vcaSettingOpt.m_motionThreshold = 0.01;
				break;
		}
	}
	else 
	{
		printf("The motion sensitive level must in range: (1 ~ 5).  Setting value %d is illegal, so it is set to 3.\n", motionSensitiveLevel);
		m_vcaSettingOpt.m_motionSensitiveLevel = 3;
		m_vcaSettingOpt.m_motionThreshold = 0.3;
	}
}

//humanOnOffFlag: [true: on, false: off]
void CVcaAnalyzer::Set_HumanOnOffFlag(bool humanOnOffFlag) 
{
	m_vcaSettingOpt.m_humanOnOffFlag = humanOnOffFlag;
}

//humanSensitiveLevel: [1,2,3,4,5], ¼Æ¦r¶V¤j¶VÆF±Ó 
//humanSensitiveLevel -> m_humanThreshold
//  1 -> 90
//  2 -> 80
//  3 -> 70
//  4 -> 60
//  5 -> 50
void CVcaAnalyzer::Set_HumanSensitiveLevel(int humanSensitiveLevel) 
{
	if (1 <= humanSensitiveLevel && humanSensitiveLevel <= 5)
	{
		m_vcaSettingOpt.m_humanSensitiveLevel = humanSensitiveLevel;
		m_vcaSettingOpt.m_humanThreshold = (float)(10 - humanSensitiveLevel) * 10.0;
	}
	else
	{
		printf("The human sensitive level must in range: (1 ~ 5).  Setting value %d is illegal, so it is set to 3.\n", humanSensitiveLevel);
		m_vcaSettingOpt.m_humanSensitiveLevel = 3;
		m_vcaSettingOpt.m_humanThreshold = 70;
	}
}

//roiID: [1,2], roiPoints: [0.0 ~ 1.0] float
bool CVcaAnalyzer::Set_ROI_create(int roiID, std::vector<std::vector<float>> roiPoints)
{
	bool resFlag = false;
	//check ROI number
	bool checkFlag = true;
	if (m_vcaSettingOpt.m_ROINumber < 2) 
	{
		//check the roi id be 1 or 2
		if (roiID!=1 && roiID!=2)
		{
			std::cout << "Create the ROI failed. The ROI ID must be 1 or 2." << std::endl;
			checkFlag = false;
		}
		else
		{
			//check the roi id is exist
			if(m_vcaSettingOpt.m_RoiList[roiID].roiEnable == true)
			{
				std::cout << "Create the ROI failed. The ROI ID:" << roiID <<  " is already exist." << std::endl;
				checkFlag = false;
			}
			else 
			{
				//check roiPoints is correct
				if (!CheckRoiPointsCorrect(roiPoints))
				{
					checkFlag = false;
				}
			}
		}
	}
	else 
	{
		std::cout << "Create the ROI failed. The number of ROI must be less than 2." << std::endl;
		checkFlag = false;
	}

	if (checkFlag)
	{
		m_vcaSettingOpt.m_RoiList[roiID].roiEnable = true;
		std::vector<CVcaPoint2f> pointList;
		for (int i_point = 0; i_point < roiPoints.size(); i_point++)
		{
			CVcaPoint2f p(roiPoints[i_point][0], roiPoints[i_point][1]);
			pointList.push_back(p);
		}
		m_vcaSettingOpt.m_RoiList[roiID].roiPoints = pointList;
		m_vcaSettingOpt.m_RoiList[roiID].RoiBitmapSize = m_downsampleSize;
		GenRoiBitmap(&m_vcaSettingOpt.m_RoiList[roiID], m_vcaSettingOpt.m_RoiList[roiID].RoiBitmapSize);
		
		m_vcaSettingOpt.m_ROINumber++;
		std::cout << "Create the new ROI, id: " << roiID << std::endl;
		resFlag = true;
	}
	return resFlag;
}

bool CVcaAnalyzer::Set_ROI_remove(int roiID)
{
	printf("--- remove ROI %d --- \n", roiID);
	bool resFlag = false;
	if (m_vcaSettingOpt.m_RoiList[roiID].roiEnable==true)
	{
		m_vcaSettingOpt.m_RoiList[roiID].roiEnable = false;
		std::vector<CVcaPoint2f>().swap(m_vcaSettingOpt.m_RoiList[roiID].roiPoints);

		m_vcaSettingOpt.m_ROINumber--;
		std::cout << "Remove the ROI, id: " << roiID << std::endl;
		resFlag = true;
	}
	else
	{
		std::cout << "The Specified ROI id: " << roiID <<" is NOT exist. Can NOT remove it." << std::endl;
		resFlag = false;
	}
	
	return resFlag;
}

bool CVcaAnalyzer::Set_ROI_modify(int roiID, std::vector<std::vector<float>> roiPoints)
{
	bool resFlag = false;
	//check the roi id is not exist
	bool checkFlag = true;
	if (m_vcaSettingOpt.m_RoiList[roiID].roiEnable==false)
	{
		std::cout << "Modify the ROI failed. The ROI id: " << roiID << " is NOT exist." << std::endl;
		checkFlag = false;
	}
	else
	{
		//check roiPoints is correct
		if (!CheckRoiPointsCorrect(roiPoints))
		{
			checkFlag = false;
		}
	}
		
	//do modify
	if (checkFlag)
	{
		std::vector<CVcaPoint2f> newPointList;
		for (int i_point = 0; i_point < roiPoints.size(); i_point++)
		{
			CVcaPoint2f p(roiPoints[i_point][0], roiPoints[i_point][1]);
			newPointList.push_back(p);
		}
		m_vcaSettingOpt.m_RoiList[roiID].roiPoints = newPointList;
		GenRoiBitmap(&m_vcaSettingOpt.m_RoiList[roiID], m_vcaSettingOpt.m_RoiList[roiID].RoiBitmapSize);

		std::cout << "Modify the the ROI, id: " << roiID << std::endl;
		resFlag = true;
	}

	return resFlag;
}

#pragma endregion

//show settingOpt
void CVcaAnalyzer::Show_SettingOpt()
{
	std::cout << "============================================" << std::endl;
	std::cout << m_vcaSettingOpt << std::endl;
#ifdef PC_DEBUG
	std::cout << ">> m_motionThreshold: " << m_vcaSettingOpt.m_motionThreshold << std::endl;
	std::cout << ">> m_humanThreshold: " << m_vcaSettingOpt.m_humanThreshold << std::endl;
#endif 
	std::cout << "============================================" << std::endl;
#ifdef PC_DEBUG
	//show bitmap
	for (int i_roi=0; i_roi < ROI_MAX_NUM; i_roi++)
	{
		if (m_vcaSettingOpt.m_RoiList[i_roi].roiEnable)
		{
			cv::Mat roiBitmapMat(m_vcaSettingOpt.m_RoiList[i_roi].RoiBitmapSize.height, m_vcaSettingOpt.m_RoiList[i_roi].RoiBitmapSize.width, CV_8UC1, (unsigned char*)m_vcaSettingOpt.m_RoiList[i_roi].RoiBitmap);
			std::string windowsName = "roi bitmap " + std::to_string(i_roi);
		}
	}
#endif 
	  
}

//init
bool CVcaAnalyzer::Init(int imageWidth, int imageHeight, std::string modelPath)
{
	std::cout << " Init the module... " << std::endl;
	std::cout << " version: " << VcaVersion << std::endl;
	 
	//set frame size (input img size) & init m_frameColor, m_frameGray memory
	m_frameSize.width = imageWidth;
	m_frameSize.height = imageHeight;

	//set downsampleSize & init the m_downsampleFrame memory
	m_downsampleSize.width = imageWidth / m_downsampleCellSize.width;
	m_downsampleSize.height = imageHeight / m_downsampleCellSize.height;

	//set default ROI 0 (wholeFrameROI)
	m_vcaSettingOpt.m_RoiList[0].id = 0;
	m_vcaSettingOpt.m_RoiList[0].roiEnable = true;
	m_vcaSettingOpt.m_RoiList[0].roiPoints.push_back(CVcaPoint2f(0.0, 0.0));
	m_vcaSettingOpt.m_RoiList[0].roiPoints.push_back(CVcaPoint2f(1.0, 0.0));
	m_vcaSettingOpt.m_RoiList[0].roiPoints.push_back(CVcaPoint2f(1.0, 1.0));
	m_vcaSettingOpt.m_RoiList[0].roiPoints.push_back(CVcaPoint2f(0.0, 1.0));
	m_vcaSettingOpt.m_RoiList[0].RoiBitmapSize = m_downsampleSize;
	GenRoiBitmap(&m_vcaSettingOpt.m_RoiList[0], m_vcaSettingOpt.m_RoiList[0].RoiBitmapSize);

	//set ROI 1 init
	m_vcaSettingOpt.m_RoiList[1].id = 1;
	m_vcaSettingOpt.m_RoiList[1].roiEnable = false;
	std::vector<CVcaPoint2f>().swap(m_vcaSettingOpt.m_RoiList[1].roiPoints);
	m_vcaSettingOpt.m_RoiList[1].RoiBitmapSize = m_downsampleSize;

	//set ROI 2 init
	m_vcaSettingOpt.m_RoiList[2].id = 2;
	m_vcaSettingOpt.m_RoiList[2].roiEnable = false;
	std::vector<CVcaPoint2f>().swap(m_vcaSettingOpt.m_RoiList[2].roiPoints);
	m_vcaSettingOpt.m_RoiList[2].RoiBitmapSize = m_downsampleSize;

	//init human detector
	m_humanClassifyModelPath = modelPath;
	bool initFlag = m_humanClassifier.Initial(m_humanClassifyModelPath);

	return initFlag;
}

//algo analyze
int CVcaAnalyzer::Analyze(unsigned char *imageData, int imageWidth, int imageHeight, int imageFormat, bool isCamereMovingFlag, CVcaOutput &output)
{
	double t_s, t_e;
	int downsampleWidth = m_downsampleSize.width;
	int downsampleHeight = m_downsampleSize.height;
	int downSize = downsampleWidth * downsampleHeight;
	CleanOutput(output);

	unsigned char* downsampleFrame = (unsigned char*)malloc(downSize * sizeof(unsigned char));
	std::vector<CVcaObj> detectedObjs;
	std::vector<CVcaObj> trackingObjs;

	// camera is moving, then set the m_isInitedFlag as false
	if (isCamereMovingFlag)
	{
		printf(">>  Camera is moving!\n");
		m_isInitedFlag = false;
		m_iFrameCnt = 0;
	}

	// init first frame
	if (m_isInitedFlag == false)
	{
		//图像前处理
		preprocess(imageData, m_frameSize.width, m_frameSize.height, imageFormat, downsampleFrame);
		//处理第1帧数据
		ProcessFirstFrame(downsampleFrame, downsampleWidth, downsampleHeight);
		std::cout << "init first frame finished." << std::endl;

		m_isInitedFlag = true;
	}
	else
	{
		if (m_iFrameCnt % m_skip == 0)
		{
			//图像前处理
			preprocess(imageData, m_frameSize.width, m_frameSize.height, imageFormat, downsampleFrame);

			//对下采样图像做背景更新
			unsigned char* fgMask = (unsigned char*)malloc(downSize * sizeof(unsigned char));
			bool bgReadyFlag = UpdateBackground(downsampleFrame, downsampleWidth, downsampleHeight, fgMask);
			if (!bgReadyFlag)
			{
				m_iFrameCnt++;
				return 0;
			}
			
			//ROI区域motion检测
			RoiMotionCheck(fgMask);

			//筛选闭合区域
			std::cout << "blob module detect." << std::endl;
			std::vector<CVcaBBox> detectBBoxes = blob(fgMask, downsampleWidth, downsampleHeight);
			freeMemory(fgMask);

			//目标分类
			std::cout << "object classify module." << std::endl;
			ObjClassify(imageData, m_frameSize.width, m_frameSize.height, 3, imageFormat, detectBBoxes, detectedObjs);
			std::vector<CVcaBBox>().swap(detectBBoxes);

			//motion目标跟踪
			std::cout << "object tracking module." << std::endl;
			ObjectTracking(detectedObjs, trackingObjs);
			
			//ROI区域人形检测
			std::cout << "ROI human check." << std::endl;
			RoiHumanCheck(detectedObjs);
			std::vector<CVcaObj>().swap(detectedObjs);
		} 
	} 	
	//free memory
	freeMemory(downsampleFrame);
	
	//update the output
	std::cout << "update output." << std::endl;
	UpdateOutput(trackingObjs, output);
	std::vector<CVcaObj>().swap(trackingObjs);
	m_iFrameCnt++;

	return 0;
}

//初始化第1帧
int CVcaAnalyzer::ProcessFirstFrame(unsigned char* imageData, int imageWidth, int imageHeight)
{
	//init the bg module
	bool bgInitFlag = m_Background.Initial(imageData, imageWidth, imageHeight);
	if (!bgInitFlag)
	{
		printf(" Background initial fail!\n");
		return -1;
	}
	m_Background.processFirstFrame(imageData);
	m_Background.m_iframeCnt = 0;

	return 0;
}

//背景更新
bool CVcaAnalyzer::UpdateBackground(unsigned char *imageData, int imageWidth, int imageHeight, unsigned char *fgMask)
{
	//更新背景数据
	m_Background.UpdateBackground(imageData);

	//计算非零像素
	int inonZeroCnt = 0;
	for (int icurPixelIdx = 0; icurPixelIdx < imageWidth * imageHeight; icurPixelIdx++)
	{
		if (m_Background.m_fgMask[icurPixelIdx] == 255)
		{
			inonZeroCnt++;
		}
	}

	//统计大于75%的目标区域，并更新为背景
	float Ratio = (float)inonZeroCnt / (float)(imageWidth * imageHeight);
	if (Ratio > 0.75)
	{
		m_Background.processFirstFrame(imageData);
		m_Background.m_iframeCnt = 0;
	}

    //拷贝目标图像
	memcpy(fgMask, m_Background.m_fgMask, imageWidth * imageHeight * sizeof(unsigned char));
	bool bgReadyFlag = m_Background.IsBackgroundReady();
	
	return bgReadyFlag;
}

//筛选闭合区域
std::vector<CVcaBBox> CVcaAnalyzer::blob(unsigned char *fgMask, int imageWidth, int imageHeight)
{
	int imemlen = imageWidth * imageHeight;
	int* ccl_Label = (int*)malloc(imemlen * sizeof(int));
	memset(ccl_Label, 0, imemlen * sizeof(int));

	//确定闭合区域
	m_ccl.labeling(fgMask, ccl_Label, imageWidth, imageHeight, 1);
	int DetectorNum = m_ccl.GenCCLRegion(m_Detector, m_iCCL_IDlist, MAX_CCL_REGION_NUM, ccl_Label, imageWidth, imageHeight);
	free(ccl_Label);

	//筛选检测框
	std::vector <CVcaBBox> tmp_detectBBoxes;
	for (int i_bbox = 0; i_bbox < DetectorNum; i_bbox++)
	{
		int area = m_Detector[i_bbox].height * m_Detector[i_bbox].width;
		if (area > 20)
		{
			CVcaBBox ccl_bbox;
			ccl_bbox.x1 = float(m_Detector[i_bbox].x) * float(m_frameSize.width / imageWidth);
			ccl_bbox.x2 = float(m_Detector[i_bbox].x + m_Detector[i_bbox].width) * float(m_frameSize.width / imageWidth);
			ccl_bbox.y1 = float(m_Detector[i_bbox].y) * float(m_frameSize.height / imageHeight);
			ccl_bbox.y2 = float(m_Detector[i_bbox].y + m_Detector[i_bbox].height) * float(m_frameSize.height / imageHeight);
			tmp_detectBBoxes.push_back(ccl_bbox);
		}
	}
	std::sort(tmp_detectBBoxes.begin(), tmp_detectBBoxes.end(), SortBboxByArea);

	return tmp_detectBBoxes;
}

//目标分类
int CVcaAnalyzer::ObjClassify(unsigned char* imageData, int imageWidth, int imageHeight, int imageChannel, int imageFormat, std::vector<CVcaBBox>& detectBBoxes, std::vector<CVcaObj>& detectedObjs)
{
	//image clone
	int sz = imageWidth * imageHeight;
	unsigned char* frameColor = (unsigned char*)malloc(sz * 3 * sizeof(unsigned char));
	std::cout << "clone data to new buffer." << std::endl;
	ImageClone(imageData, imageWidth, imageHeight, imageFormat, frameColor);

	//detected bboxes
	std::cout << "detected bboxes size: " << detectBBoxes.size() << std::endl;
	for (int i_bbox = 0; i_bbox < detectBBoxes.size(); i_bbox++)
	{
		if (i_bbox > MAX_HUMAN_DETECT_NUM) 
			break;
		int bbox_x = (int)detectBBoxes[i_bbox].x1;
		int bbox_y = (int)detectBBoxes[i_bbox].y1;
		int bbox_w = (int)(detectBBoxes[i_bbox].x2 - detectBBoxes[i_bbox].x1);
		int bbox_h = (int)(detectBBoxes[i_bbox].y2 - detectBBoxes[i_bbox].y1);

		//图像裁剪
		unsigned char* cropImgData = (unsigned char *)malloc(bbox_w * bbox_h * imageChannel * sizeof(unsigned char));
		Tools::ImageProcess::crop(frameColor, imageWidth, imageHeight, imageChannel, bbox_x, bbox_y, bbox_w, bbox_h, cropImgData);

		//inference
		float nScore = m_humanClassifier.Inference(cropImgData, bbox_w, bbox_h, imageChannel);
		freeMemory(cropImgData);
		detectBBoxes[i_bbox].score = nScore * 100;
		if (detectBBoxes[i_bbox].score >= m_vcaSettingOpt.m_humanThreshold)
		{
			//convert to CVcaObj
			float f_x = detectBBoxes[i_bbox].x1 / m_frameSize.width;
			float f_y = detectBBoxes[i_bbox].y1 / m_frameSize.height;
			float f_w = (detectBBoxes[i_bbox].x2 - detectBBoxes[i_bbox].x1) / m_frameSize.width;
			float f_h = (detectBBoxes[i_bbox].y2 - detectBBoxes[i_bbox].y1) / m_frameSize.height;

			CVcaObj newObj;
			newObj.id = detectedObjs.size() + 1;
			newObj.rect = CVcaRect2f(f_x, f_y, f_w, f_h);
			newObj.prePosition = CVcaPoint2f(f_x + (f_w / 2.0), (f_y + (f_h / 2.0)));
			newObj.curPosition = CVcaPoint2f(f_x + (f_w / 2.0), (f_y + (f_h / 2.0)));
			detectedObjs.push_back(newObj);
		}
	}
	freeMemory(frameColor);

	return 0;
}

//motion目标跟踪
int CVcaAnalyzer::ObjectTracking(std::vector<CVcaObj>& detectedObjs, std::vector<CVcaObj>& trackingObjs)
{
	int curMaxTrackerId = 0;
	if (trackingObjs.size() == 0)
	{
		trackingObjs = detectedObjs;
		if (trackingObjs.size() > 0)
		{
			curMaxTrackerId = trackingObjs[trackingObjs.size() - 1].id;
			m_curMaxTrackerId = curMaxTrackerId;
		}
	}
	else
	{
		DoTracking(detectedObjs, trackingObjs);
	}	

	return 0;
}


//check points of ROI
bool CVcaAnalyzer::CheckRoiPointsCorrect(std::vector<std::vector<float>> &roiPoints)
{
	//check that the number of point must be less then 6 and greater than 3
 	if (roiPoints.size() < 3 || 6 < roiPoints.size())
	{
		std::cout << " The number of ROI points must be  less then 6 and greater than 3." << std::endl;
		return false;
	}

	//check that the point have 2 coordinate value & the value is in the correct range [0.0 ~ 1.0]
	for (int i_point = 0; i_point < roiPoints.size(); i_point++)
	{
		if (roiPoints[i_point].size() != 2) 
		{
			std::cout << " The point must have 2 coordinate value." << std::endl;
			return false;
		}
		else 
		{
			// point's x coordinate
			if (roiPoints[i_point][0] < 0.0 || 1.0 < roiPoints[i_point][0]) 
			{
				std::cout << " The point coordinate value must be in range [0.0 ~ 1.0]." << std::endl;
				return false;
			}
			// point's y coordinate
			if (roiPoints[i_point][1] < 0.0 || 1.0 < roiPoints[i_point][1])
			{
				std::cout << " The point coordinate value must be in range [0.0 ~ 1.0]." << std::endl;
				return false;
			}
		}
	}

	return true;
}

void CVcaAnalyzer::GenRoiBitmap(CVcaROI *pRoi, CVcaSize2i roiBitmapSize) 
{
	int bitmapDataLen = roiBitmapSize.width * roiBitmapSize.height;
	//create / clean the bitmap
	if (pRoi->RoiBitmap != NULL) 
	{
		//clean bitmap
		memset(pRoi->RoiBitmap, 0, bitmapDataLen * sizeof(unsigned char));
	}
	else 
	{
		//create bitmap
		pRoi->RoiBitmap = (unsigned char*)malloc(bitmapDataLen * sizeof(unsigned char));
		memset(pRoi->RoiBitmap, 0, bitmapDataLen * sizeof(unsigned char));
	}

	//convert norn_point to bitmap_point
	std::vector<CVcaPoint2i> bitmapPoint;
	for (int i=0; i < pRoi->roiPoints.size(); i++)
	{
		CVcaPoint2i point;
		point.x = ceil((float)roiBitmapSize.width * pRoi->roiPoints[i].x);
		point.y = ceil((float)roiBitmapSize.height * pRoi->roiPoints[i].y);
		bitmapPoint.push_back(point);
	}

	Tools::CImgProcess::FillConvexPolygon(pRoi->RoiBitmap, roiBitmapSize.width, roiBitmapSize.height, bitmapPoint, bitmapPoint.size(), 255);
	std::vector<CVcaPoint2i>().swap(bitmapPoint);
}

void CVcaAnalyzer::RoiMotionCheck(unsigned char *fgMask)
{
	//check the intersection of ROI & fgMask
	for (int i_roi=0; i_roi < ROI_MAX_NUM; i_roi++)
	{
		int intersectionPixelNum = 0;
		int roiPixelNum = 0;
		CVcaROI *roi = &m_vcaSettingOpt.m_RoiList[i_roi];
		if (roi->roiEnable) 
		{
			for (int y = 0; y < roi->RoiBitmapSize.height; y++)
			{
				for (int x = 0; x < roi->RoiBitmapSize.width; x++)
				{
					int offset = (y * roi->RoiBitmapSize.width) + x;
					if (roi->RoiBitmap[offset] == (unsigned char)255 && fgMask[offset] == (unsigned char)255)
					{
						intersectionPixelNum++;
					}
					if (roi->RoiBitmap[offset] == (unsigned char)255)
					{
						roiPixelNum++;
					}
				}
			}

			//compute ratio of motion
			float ratioOfMotion = (float)intersectionPixelNum / (float)(roiPixelNum + 0.5);
			if (ratioOfMotion >= m_vcaSettingOpt.m_motionThreshold)
			{
				//ROI is motion trigger
				m_vcaSettingOpt.m_RoiList[i_roi].isMotionTigger = true;
			}
			else 
			{
				//ROI is NOT motion trigger
				m_vcaSettingOpt.m_RoiList[i_roi].isMotionTigger = false;
			}
		}
	}
}

void CVcaAnalyzer::RoiHumanCheck(std::vector<CVcaObj> &humanObjList)
{
	for (int i_roi = 0; i_roi < ROI_MAX_NUM; i_roi++)
	{
		if (m_vcaSettingOpt.m_RoiList[i_roi].roiEnable) 
		{
			//convert roi point to fit frame location point
			std::vector<CVcaPoint2i> roiPoints;
			for (int i_p=0; i_p < m_vcaSettingOpt.m_RoiList[i_roi].roiPoints.size(); i_p++)
			{
				int x = m_vcaSettingOpt.m_RoiList[i_roi].roiPoints[i_p].x * (float)m_frameSize.width;
				int y = m_vcaSettingOpt.m_RoiList[i_roi].roiPoints[i_p].y * (float)m_frameSize.height;
				roiPoints.push_back(CVcaPoint2i(x, y));
			}

			//check is the humanbbox center point in the roi
			bool findHumanInRoi = false;
			for (int i_obj = 0; i_obj < humanObjList.size(); i_obj++)
			{
				int centerP_x = (humanObjList[i_obj].rect.x * m_frameSize.width) + ((humanObjList[i_obj].rect.w * (float)m_frameSize.width) / 2.0);
				int centerP_y = (humanObjList[i_obj].rect.y * m_frameSize.height) + ((humanObjList[i_obj].rect.h * (float)m_frameSize.height) / 2.0);
				CVcaPoint2i centerPoint(centerP_x, centerP_y);

				//roiPoints in roi.
				if (Tools::CImgProcess::InConvexPolygon(roiPoints, m_vcaSettingOpt.m_RoiList[i_roi].roiPoints.size(), centerPoint))
				{
					findHumanInRoi = true;
					break;
				}
			}
			std::vector<CVcaPoint2i>().swap(roiPoints);
		
			//update the isHumanTigger flag
			if (findHumanInRoi)
			{
				m_vcaSettingOpt.m_RoiList[i_roi].isHumanTigger = true;
			}
			else 
			{
				m_vcaSettingOpt.m_RoiList[i_roi].isHumanTigger = false;
			}
		}
	}
}

void CVcaAnalyzer::DoTracking(std::vector<CVcaObj> &detectedObjs, std::vector<CVcaObj> &trackingObjs)
{
	//first, each trackingObjs's unshowCounter +1
	for (int i=0; i < trackingObjs.size(); i++)
	{
		trackingObjs[i].unshowCounter++;
	}

	//clean the m_detector2TrackerDist & m_detectorMatchingResult
	for (int i = 0; i < MAX_HUMAN_DETECT_NUM; i++)
	{
		m_detectorMatchingResult[i] = -1;
		for (int j = 0; j < MAX_HUMAN_DETECT_NUM; j++)
		{
			m_detector2TrackerDist[i][j] = 0;
		}
	}

	//calculate distance matrix
	for (int i_det=0; i_det < detectedObjs.size(); i_det++)
	{
		for (int i_track = 0; i_track < trackingObjs.size(); i_track++)
		{
			int dist_x1 = (detectedObjs[i_det].rect.x * 100) - (trackingObjs[i_track].rect.x * 100);
			int dist_y1 = (detectedObjs[i_det].rect.y * 100) - (trackingObjs[i_track].rect.y * 100);
			int dist_x2 = ((detectedObjs[i_det].rect.x * 100) + (detectedObjs[i_det].rect.w * 100)) - ((trackingObjs[i_track].rect.x * 100) + (trackingObjs[i_track].rect.w * 100));
			int dist_y2 = ((detectedObjs[i_det].rect.y * 100) + (detectedObjs[i_det].rect.h*100)) - ((trackingObjs[i_track].rect.y * 100) + (trackingObjs[i_track].rect.h * 100));

			if (dist_x1 < 0) { dist_x1 = -dist_x1; }
			if (dist_y1 < 0) { dist_y1 = -dist_y1; }
			if (dist_x2 < 0) { dist_x2 = -dist_x2; }
			if (dist_y2 < 0) { dist_y2 = -dist_y2; }

			int dist = dist_x1 + dist_y1 + dist_x2 + dist_y2;
			m_detector2TrackerDist[i_det][i_track] = dist;
		}
	}

	//detectedObj & trackingObj mapping
	for (int i_det=0; i_det < detectedObjs.size(); i_det++)
	{
		//find the trackingObj that has the minimum distance between detectedObj
		int minDist = INT_MAX;
		int matchedTrackerIdx = -1;
		for (int i_track = 0; i_track < trackingObjs.size(); i_track++)
		{
			if (minDist > m_detector2TrackerDist[i_det][i_track])
			{
				minDist = m_detector2TrackerDist[i_det][i_track];
				matchedTrackerIdx = i_track;
			}
		}
		//minDist > threshold
		if (matchedTrackerIdx > -1 && minDist < MATCH_DIST_THRESHOLD)
		{
			int minDist2 = INT_MAX;
			int minDistDetectorIdx = -1;
			for (int idet1 = 0; idet1 < detectedObjs.size(); idet1++)
			{
				if (minDist2 > m_detector2TrackerDist[idet1][matchedTrackerIdx])
				{
					minDist2 = m_detector2TrackerDist[idet1][matchedTrackerIdx];
					minDistDetectorIdx = idet1;
				}
			}

			if (minDistDetectorIdx == i_det)
			{
				m_detector2TrackerDist[i_det][matchedTrackerIdx] = 1;

				//update the trackingObj info using detectedObj info
				trackingObjs[matchedTrackerIdx].unshowCounter = 0;
				trackingObjs[matchedTrackerIdx].trackingConter++;
				trackingObjs[matchedTrackerIdx].rect.x = detectedObjs[i_det].rect.x;
				trackingObjs[matchedTrackerIdx].rect.y = detectedObjs[i_det].rect.y;
				trackingObjs[matchedTrackerIdx].rect.w = detectedObjs[i_det].rect.w;
				trackingObjs[matchedTrackerIdx].rect.h = detectedObjs[i_det].rect.h;

				//updata the curPosition, and if moveDist > threshold, then update the prePosition & moveDirection
				trackingObjs[matchedTrackerIdx].curPositionTime = detectedObjs[i_det].curPositionTime;
				trackingObjs[matchedTrackerIdx].curPosition = detectedObjs[i_det].curPosition;
				
				float moveDiff = trackingObjs[matchedTrackerIdx].prePosition.x - detectedObjs[i_det].curPosition.x;
				float moveDist = std::abs(moveDiff);

				//update the moveDirection & moveSpeedLevel
				if (moveDiff > 0.01)
				{
					//right
					trackingObjs[matchedTrackerIdx].moveDirection = 1;
					trackingObjs[matchedTrackerIdx].moveSpeedLevel = moveDist;
				}
				else if (moveDiff < -0.01)
				{
					//left
					trackingObjs[matchedTrackerIdx].moveDirection = -1;
					trackingObjs[matchedTrackerIdx].moveSpeedLevel = moveDist;
				}
				else
				{
					//none
					trackingObjs[matchedTrackerIdx].moveDirection = 0;
					trackingObjs[matchedTrackerIdx].moveSpeedLevel = 0;
				}

				//update the prePosition  
				if(moveDist > 0.01 || moveDist < -0.01)
				{
					trackingObjs[matchedTrackerIdx].prePositionTime = trackingObjs[matchedTrackerIdx].curPositionTime;
					trackingObjs[matchedTrackerIdx].prePosition = trackingObjs[matchedTrackerIdx].curPosition;
				}

				//Mark this detectedObj has been matched
				detectedObjs[i_det].matched = 1;

				//update the m_detectorMatchingResult 
				m_detectorMatchingResult[i_det] = matchedTrackerIdx;
			}
		}
	}

	//sort
	std::sort(trackingObjs.begin(), trackingObjs.end(), SortObjByUnshow);
	//delete the trackingObj which unshowCounter > threshold
	for (int i_track = trackingObjs.size() - 1; i_track >= 0; i_track--)
	{
		if(trackingObjs[i_track].unshowCounter >= m_matching_NoShowCntEraseThreshold)
		{
			trackingObjs.erase(trackingObjs.begin() + i_track);
		}
	}

	//if trackingObjs number < MAX_HUMAN_DETECT_NUM, then add new detectedObj to trackingObjs
	if (trackingObjs.size() < MAX_HUMAN_DETECT_NUM)
	{
		for (int i_det = 0; i_det < detectedObjs.size(); i_det++)
		{
			int iIsfound = 0;
			if (detectedObjs[i_det].matched <= 0)
			{
				CVcaObj newObj;
				newObj.id = m_curMaxTrackerId + 1;
				newObj.rect = CVcaRect2f(detectedObjs[i_det].rect.x, detectedObjs[i_det].rect.y, detectedObjs[i_det].rect.w, detectedObjs[i_det].rect.h);
				newObj.prePositionTime = detectedObjs[i_det].prePositionTime;
				newObj.prePosition = CVcaPoint2f(detectedObjs[i_det].rect.x + (detectedObjs[i_det].rect.w / 2.0), (detectedObjs[i_det].rect.y + (detectedObjs[i_det].rect.h / 2.0)));
				newObj.curPositionTime = detectedObjs[i_det].curPositionTime;
				newObj.curPosition = CVcaPoint2f(detectedObjs[i_det].rect.x + (detectedObjs[i_det].rect.w / 2.0), (detectedObjs[i_det].rect.y + (detectedObjs[i_det].rect.h / 2.0)));
				newObj.trackingConter++;
				trackingObjs.push_back(newObj);
				m_curMaxTrackerId++;

				if (trackingObjs.size() >= MAX_HUMAN_DETECT_NUM)
				{
					return;
				}
			}
		}
	}
}

void CVcaAnalyzer::GetNowTimeStamp(tm &now_time) 
{
	time_t time_seconds = time(0);
#ifdef _WIN32
	localtime_s(&now_time, &time_seconds);
#elif __unix__
	localtime_r(&time_seconds, &now_time);
#endif

}

//--- for output
void CVcaAnalyzer::CleanOutput(CVcaOutput &output)
{
	output.m_isMotionEventFlag = false;
	output.m_motionEventRoiId = -1;
	output.m_isHumanEventFlag = false;
	output.m_humanEventRoiId = -1;
	output.m_isTrackingTargetExist = false;
	output.m_trackingTargetRect = CVcaRect2f(0,0,0,0);
	output.m_trackingTargetDirection = 0;
	output.m_trackingTargetSpeedLevel = 0.0;
}

void CVcaAnalyzer::UpdateOutput(std::vector<CVcaObj>& trackingObjs, CVcaOutput &output)
{
	//clean the output
	CleanOutput(output);
	//update ROI event
	for (int i_roi=0; i_roi < ROI_MAX_NUM; i_roi++)
	{
		if (m_vcaSettingOpt.m_RoiList[i_roi].roiEnable)
		{
			//result of motion in ROI
			if (m_vcaSettingOpt.m_motionOnOffFlag)
			{
				if (m_vcaSettingOpt.m_RoiList[i_roi].isMotionTigger)
				{
					output.m_isMotionEventFlag = true;
					output.m_motionEventRoiId = m_vcaSettingOpt.m_RoiList[i_roi].id;
				}
			}
			//human result in ROI
			if (m_vcaSettingOpt.m_humanOnOffFlag)
			{
				if (m_vcaSettingOpt.m_RoiList[i_roi].isHumanTigger)
				{
					output.m_isHumanEventFlag = true;
					output.m_humanEventRoiId = m_vcaSettingOpt.m_RoiList[i_roi].id;
				}
			}
		}
	}

	//update tracking target (select the biggest tracker as target)
	if (m_vcaSettingOpt.m_motionOnOffFlag || m_vcaSettingOpt.m_humanOnOffFlag)
	{
		//find the biggest tracker
		int biggestTrackerIdx = -1;
		float biggestArea = -1.0;
		for (int i_track = 0; i_track < trackingObjs.size(); i_track++)
		{
			if (trackingObjs[i_track].rect.area() > biggestArea)
			{
				biggestTrackerIdx = i_track;
				biggestArea = trackingObjs[i_track].rect.area();
			}
		}
		
		if (biggestTrackerIdx != -1)
		{
			output.m_isTrackingTargetExist = true;
			output.m_trackingTargetRect = trackingObjs[biggestTrackerIdx].rect;
			output.m_trackingTargetDirection = trackingObjs[biggestTrackerIdx].moveDirection;
			output.m_trackingTargetSpeedLevel = trackingObjs[biggestTrackerIdx].moveSpeedLevel;
		}
	}
}

//image clone
int CVcaAnalyzer::ImageClone(unsigned char* srcData, int imageWidth, int imageHeight, int imageFormat, unsigned char* dstData)
{
	int sz = imageWidth * imageHeight;
	if (imageFormat == 0) //YUV
	{
		memcpy(dstData, srcData, sz * sizeof(unsigned char));
		memcpy(dstData + sz, srcData, sz * sizeof(unsigned char));
		memcpy(dstData + (2 * sz), srcData, sz * sizeof(unsigned char));
	}
	else if(imageFormat == 1) //RGB
	{
		Tools::ImageProcess::ConvertRGB2BGR(srcData, imageWidth, imageHeight, 3, dstData);
	}
	else if (imageFormat == 2) //BGR
	{
		memcpy(dstData, srcData, sz * 3 * sizeof(unsigned char));
	}
	else
	{
		printf(" imageFormat is Not correct!\n");
		return -1;
	}
	return 0;
}

//preprocess
int CVcaAnalyzer::preprocess(unsigned char* srcData, int imageWidth, int imageHeight, int imageFormat, unsigned char* dstData)
{
	int sz = imageWidth * imageHeight;
	unsigned char* frameGray = (unsigned char*)malloc(sz * sizeof(unsigned char));

	//convert format
	switch (imageFormat)
	{
	case 0: //YUV
		Tools::CImgProcess::ConvertYUVtoGray(srcData, imageWidth, imageHeight, frameGray);
		break;
	case 1: case 2: //RGB or BGR
 		Tools::CImgProcess::ConvertBGRtoGray(srcData, imageWidth, imageHeight, frameGray);
		break;
	default:
		printf(" imageFormat is not correct!\n");
		return -1;
	}
	//downsample
	pyr_down(frameGray, imageWidth, imageHeight, dstData);
	freeMemory(frameGray);

	return 0;
}

/*****************************************************
function:pyr_up(unsigned char* srcImg, int width, int height, unsigned char* dstImg)
¤Uªö?¨ç?
input: ?¹³ªº?width¡A?¹³ªº°ªheight¡A­ì?¹³image¡]??®æ¦¡¡^
output: ¤Uªö??¹³dstImg
data:2020/11/4
*******************************************************/
void pyr_up(unsigned char* srcImg, int width, int height, unsigned char* dstImg)
{
	int odd = 4;
	for (int i = 0; i < height; i= i+odd)
	{
		for (int j = 0; j < width; j = j+odd)
		{
			dstImg[i * width + j] = srcImg[i / odd * width / odd + j / odd];
			dstImg[(i + 1) * width + (j + 1)] = 0;
			dstImg[(i + 1) * width + j] = 0;
			dstImg[i * width + (j + 1)] = 0;
		}
	}
}

/*****************************************************
function:pyr_down(unsigned char* srcImg, int width, int height, unsigned char* dstImg)
¤Uªö?¨ç?
input: ?¹³ªº?width¡A?¹³ªº°ªheight¡A­ì?¹³image¡]??®æ¦¡¡^
output: ¤Uªö??¹³dstImg
data:2020/11/4
*******************************************************/
void CVcaAnalyzer::pyr_down(unsigned char* srcImg, int width, int height, unsigned char* dstImg)
{
	int odd = 4;
	int dwidth = width / odd;
	for (int i = 0; i < height; i+=odd)
	{
		int di = i / odd;
		for (int j = 0; j < width; j+=odd)
		{
			int dj = j / odd;
			dstImg[di * dwidth + dj] = srcImg[i * width + j];
		}
	}
}

//free memory
void CVcaAnalyzer::freeMemory(unsigned char* data)
{
	if (data != NULL)
	{
		free(data);
		data = NULL;
	}
}

//保存binary数据
int CVcaAnalyzer::saveBinaryFiles(unsigned char* data, int width, int height, int channel, std::string& outPath)
{
	system(("mkdir " + outPath).c_str());
	std::string savePreStr = outPath + "/" + getSysTime();
	string saveImgFileName = savePreStr + ".raw";
	FILE* fp = fopen(saveImgFileName.c_str(),"wb");
	if (NULL == fp)
	{
		std::cout << "[SAVE_IMG_TAG] fopen file error!" << std::endl;
	}
	else
	{
		fwrite(data, height * width * channel, 1, fp);
		fclose(fp);
	}

	return 0;
}

//保存日志
int CVcaAnalyzer::saveTimeFiles(CVcaOutput& output, int width, int height, bool gMoveFlag, std::string& outPath)
{
	system(("mkdir " + outPath).c_str());
	string savePreStr = outPath + "/" + getSysTime();
	string saveTxtFileName = savePreStr + ".txt";
	FILE* ftxt = fopen(saveTxtFileName.c_str(),"wb");
	if (NULL == ftxt)
	{
		std::cout << "[SAVE_TXT_TAG] fopen file error!" << std::endl;
	}
	else
	{
		fprintf(ftxt,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%.2f,%.2f,%.2f,%.2f,%.2f\n",height, width, (gMoveFlag?1:0)?1:0,
				output.m_isMotionEventFlag?1:0, output.m_motionEventRoiId, output.m_isHumanEventFlag?1:0, output.m_humanEventRoiId,
				output.m_isTrackingTargetExist?1:0, output.m_trackingTargetDirection, output.m_trackingTargetSpeedLevel,
				output.m_trackingTargetRect.x,output.m_trackingTargetRect.y,output.m_trackingTargetRect.w,output.m_trackingTargetRect.h);
		fclose(ftxt);
	}
}