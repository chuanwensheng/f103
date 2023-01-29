#pragma once

/*
*  Version: v1.0.5
*  ReleaseData: 2021.06.03
*  Maintainer: Belle
*/

#include <stdio.h>
#include <iostream>
#include <vector>

#include "c_vcaobjs.h"
#include "sigmastar_f103_vca.h"

class CVcaAnalyzerAPI
{
	public:
		//Constructor and Destructor
		CVcaAnalyzerAPI(); //模組初始化
		~CVcaAnalyzerAPI();

		//--- 參數設定
		//scenarioSetting: [true: indoor, false: outdoor] 
		void Set_Scenario(bool scenarioSetting);	
		//dayNightModeSetting: [true: dayMode, false: nightMode]
		void Set_DayNightMode(bool dayNightModeSetting);	
		//motionOnOffFlag: [true: on, false: off]
		void Set_MotionOnOffFlag(bool motionOnOffFlag);	
		//motionSensitiveLevel: [1,2,3,4,5], 數字越大越靈敏 
		void Set_MotionSensitiveLevel(int motionSensitiveLevel);	
		//humanOnOffFlag: [true: on, false: off]
		void Set_HumanOnOffFlag(bool humanOnOffFlag);		
		//motionSensitiveLevel: [1,2,3,4,5], 數字越大越靈敏 
		void Set_HumanSensitiveLevel(int humanSensitiveLevel);			
		//roiPoints: [0.0 ~ 1.0] float
		bool Set_ROI_create(int roiID, std::vector<std::vector<float>> roiPoints);
		//roiID: [1,2]
		bool Set_ROI_remove(int roiID);									
		//roiID: [1,2], roiPoints: [0.0 ~ 1.0] float
		bool Set_ROI_modify(int roiID, std::vector<std::vector<float>> roiPoints);

		//---
		//顯示當前設定資訊
		void Show_SettingOpt();

		//---
		//算法初始化
		// imageWidth: 要分析的影像寬
		// imageHeight: 要分析的影像長
		bool Init(int imageWidth, int imageHeight, std::string modelPath);

		//執行影像分析
		// imageData: 影像資料流
		// imageWidth: 影像寬
		// imageHeight: 影像高
		// imageFormat: 影像格式代號 [YUV: 0, RGB:1, BGR: 2]
		// isCamereMovingFlag: 鏡頭是否正在轉向的旗標
		int Analyze(unsigned char *imageData, int imageWidth, int imageHeight, int imageFormat, bool isCamereMovingFlag, CVcaOutput &output);
};