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
		CVcaAnalyzerAPI(); //�Ҳժ�l��
		~CVcaAnalyzerAPI();

		//--- �ѼƳ]�w
		//scenarioSetting: [true: indoor, false: outdoor] 
		void Set_Scenario(bool scenarioSetting);	
		//dayNightModeSetting: [true: dayMode, false: nightMode]
		void Set_DayNightMode(bool dayNightModeSetting);	
		//motionOnOffFlag: [true: on, false: off]
		void Set_MotionOnOffFlag(bool motionOnOffFlag);	
		//motionSensitiveLevel: [1,2,3,4,5], �Ʀr�V�j�V�F�� 
		void Set_MotionSensitiveLevel(int motionSensitiveLevel);	
		//humanOnOffFlag: [true: on, false: off]
		void Set_HumanOnOffFlag(bool humanOnOffFlag);		
		//motionSensitiveLevel: [1,2,3,4,5], �Ʀr�V�j�V�F�� 
		void Set_HumanSensitiveLevel(int humanSensitiveLevel);			
		//roiPoints: [0.0 ~ 1.0] float
		bool Set_ROI_create(int roiID, std::vector<std::vector<float>> roiPoints);
		//roiID: [1,2]
		bool Set_ROI_remove(int roiID);									
		//roiID: [1,2], roiPoints: [0.0 ~ 1.0] float
		bool Set_ROI_modify(int roiID, std::vector<std::vector<float>> roiPoints);

		//---
		//��ܷ�e�]�w��T
		void Show_SettingOpt();

		//---
		//��k��l��
		// imageWidth: �n���R���v���e
		// imageHeight: �n���R���v����
		bool Init(int imageWidth, int imageHeight, std::string modelPath);

		//����v�����R
		// imageData: �v����Ƭy
		// imageWidth: �v���e
		// imageHeight: �v����
		// imageFormat: �v���榡�N�� [YUV: 0, RGB:1, BGR: 2]
		// isCamereMovingFlag: ���Y�O�_���b��V���X��
		int Analyze(unsigned char *imageData, int imageWidth, int imageHeight, int imageFormat, bool isCamereMovingFlag, CVcaOutput &output);
};