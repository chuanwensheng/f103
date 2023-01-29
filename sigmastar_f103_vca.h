#pragma once

/*
*  Version: v1.0.5
*  ReleaseData: 2021.06.03
*  Maintainer: Belle
*/

#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cstring>
#include <climits>
#include <unistd.h>
#include <sys/time.h>
#include <sys/timeb.h>

#include "c_vcaobjs.h"
#include "tools_imgprocess_nocv.h"
#include "bg_vibe_simple.h"
#include "ccl.h"
#include "classify_human_lds.h"
#include "lds_image_process.h"

#include "FileHelper.h"
#include "Mutex.h"
#include "Logger.h"
using namespace FrameWork;

// #include <glog/logging.h>

#define VcaVersion "v1.0.5"
#define MAX_HUMAN_DETECT_NUM 10
#define MATCH_DIST_THRESHOLD 50


class CVcaAnalyzer
{
	public:
		//Constructor and Destructor
		CVcaAnalyzer();
		~CVcaAnalyzer();

		//�ѼƳ]�w
		void Set_Scenario(bool scenarioSetting);									//scenarioSetting: [true: indoor, false: outdoor] 
		void Set_DayNightMode(bool dayNightModeSetting);							//dayNightModeSetting: [true: dayMode, false: nightMode]
		void Set_MotionOnOffFlag(bool motionOnOffFlag);							    //motionOnOffFlag: [true: on, false: off]
		void Set_MotionSensitiveLevel(int motionSensitiveLevel);					//motionSensitiveLevel: [1,2,3,4,5], �Ʀr�V�j�V�F�� 
		void Set_HumanOnOffFlag(bool humanOnOffFlag);								//humanOnOffFlag: [true: on, false: off]
		void Set_HumanSensitiveLevel(int humanSensitiveLevel);						//motionSensitiveLevel: [1,2,3,4,5], �Ʀr�V�j�V�F�� 
		bool Set_ROI_create(int roiID, std::vector<std::vector<float>> roiPoints);	//roiPoints: [0.0 ~ 1.0] float
		bool Set_ROI_remove(int roiID);												//roiID: [1,2]
		bool Set_ROI_modify(int roiID, std::vector<std::vector<float>> roiPoints);	//roiID: [1,2], roiPoints: [0.0 ~ 1.0] float
		void Show_SettingOpt();

		//algo init
		bool Init(int imageWidth, int imageHeight, std::string modelPath);
		//algo analyze
		int Analyze(unsigned char *imageData, int imageWidth, int imageHeight, int imageFormat, bool isCamereMovingFlag, CVcaOutput &output);
		

	private:
		CVcaSettingOpt m_vcaSettingOpt;
		
		bool m_isInitedFlag;				// [true: module is inited, false: alg is NOT inited]
		bool m_isRoiStrategyEnable;			// [true: enable, false: unable]
		int m_iFrameCnt;
		int m_skip;

		CVcaSize2i m_frameSize;
		CVcaSize2i m_downsampleSize;
		CVcaSize2i m_downsampleCellSize;

		//BG module
		CVibeBackgroundModelSimple m_Background;

		//CCL module
		CCCL m_ccl;
		int m_iDetector_Num;
		int m_iCCL_IDlist[MAX_CCL_REGION_NUM];
		_Rect2i m_Detector[MAX_CCL_REGION_NUM];

		//classify model path
		std::string m_humanClassifyModelPath;
		CHumanClassificationLDS m_humanClassifier;

		//obj matching 
		int m_matching_NoShowCntEraseThreshold;
		int m_detector2TrackerDist[MAX_HUMAN_DETECT_NUM][MAX_HUMAN_DETECT_NUM];
		int m_detectorMatchingResult[MAX_HUMAN_DETECT_NUM];
		int m_curMaxTrackerId;
	
	private:
		//process first frame
		int ProcessFirstFrame(unsigned char *imageData, int imageWidth, int imageHeight);
		//update background
		bool UpdateBackground(unsigned char *imageData, int imageWidth, int imageHeight, unsigned char *fgMask);
		//blob detection
		std::vector<CVcaBBox> blob(unsigned char *fgMask, int imageWidth, int imageHeight);
		//object classify
		int ObjClassify(unsigned char *frameColor, int imageWidth, int imageHeight, int imageChannel, int imageFormat, std::vector<CVcaBBox>& detectBBoxes, std::vector<CVcaObj>& detectedObjs);
		//object tracking
		int ObjectTracking(std::vector<CVcaObj>& detectedObjs, std::vector<CVcaObj>& trackingObjs);
	
		//check points
		bool CheckRoiPointsCorrect(std::vector<std::vector<float>> &roiPoints);
		//Generate bitmap for ROI
		void GenRoiBitmap(CVcaROI *pRoi, CVcaSize2i roiBitmapSize);
		//check motion in ROI
		void RoiMotionCheck(unsigned char *fgMask);
		// check human in ROI
		void RoiHumanCheck(std::vector<CVcaObj> &huamnObjList);
		//do tracking
		void DoTracking(std::vector<CVcaObj> &detectedObjs, std::vector<CVcaObj> &trackingObjs);
		//get now timestamp
		void GetNowTimeStamp(tm &now_time);

		//clean output
		void CleanOutput(CVcaOutput &output);
		//update output
		void UpdateOutput(std::vector<CVcaObj>& trackingObjs, CVcaOutput& output);
		//image clone
		int ImageClone(unsigned char* srcData, int imageWidth, int imageHeight, int imageFormat, unsigned char* dstData);
		//preprocess
		int preprocess(unsigned char* srcData, int imageWidth, int imageHeight, int imageFormat, unsigned char* dstData);
		
		//pyr_up process
		void pyr_up(unsigned char* srcImg, int width, int height, unsigned char* dstImg);
		//pyrDown process
		void pyr_down(unsigned char* srcImg, int width, int height, unsigned char* dstImg);
		//free memory
		void freeMemory(unsigned char* data);

		//save binary data files
		int saveBinaryFiles(unsigned char* data, int width, int height, int channel, std::string& outPath);
		//save txt files
		int saveTimeFiles(CVcaOutput& output, int width, int height, bool gMoveFlag, std::string& outPath);
};