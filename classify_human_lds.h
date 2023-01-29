#pragma once
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <cstring>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#ifndef _MSC_VER
#include <unistd.h>
#endif
//MNN core
#include "MNN/Interpreter.hpp"
#include "MNN/MNNDefine.h"
#include "MNN/Tensor.hpp"
#include "MNN/ImageProcess.hpp"
//image process
#include "tools_imgprocess_nocv.h"
#include "c_vcaobjs.h"
#include "lds_image_process.h"
#include "tools_imgresizer_nocv.h"


class CHumanClassificationLDS
{
	public:
		CHumanClassificationLDS();
		~CHumanClassificationLDS();

		//functions
		bool Initial();
		bool Initial(std::string &modelPath);
		float Inference(unsigned char *imageData, int imageWidth, int imageHeight,int imageChannel);

	private:		
		const float m_mean_vals[3] = { 0.f, 0.f, 0.f };
		const float m_norm_vals[3] = { 1 / 128.0f, 1 / 128.0f, 1 / 128.0f };
		const int m_INPUT_SIZE_H = 40;
		const int m_INPUT_SIZE_W = 20;
		const int m_CLASS_NUM = 2;
		
		std::string m_modelFilePath;

		int m_num_thread;
		MNN::ScheduleConfig m_config;
		MNN::BackendConfig m_backendConfig;
		std::string m_scores = "softmax";

		int m_srcImgWidth;
		int m_srcImgHeight;
	
	private:
		bool InitBasis();
		float pred(unsigned char *imageData, int imageWidth, int imageHeight, int imageChannel);
		void freeMemory(unsigned char* data);
};

