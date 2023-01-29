#pragma once
#include <stdio.h>
#include <iostream>
#include <cstring>


#define NUM_SAMPLES 5			//number of samples for each pixel  (default: 20)
#define MATCHES_TH 1			//threshold of matching with samples (default: 2)  
//#define DIST_TH 10      		//threshold of distance between sample value and testing value (default: 20)
#define SUBSAMPLE_FACTOR 10		//subsampling Rate or samples update rate (default: 16) Suggest: 16, 5, 1
//#define FG_MATCH_TH 100		//threshold of fg match count

class CVibeBackgroundModelSimple
{
	public:
		//constructor and destructor
		CVibeBackgroundModelSimple();
		~CVibeBackgroundModelSimple();

		bool Initial(unsigned char* pProcessFrame, int iwidth, int iheight);	//init the module
		void UpdateBackground(unsigned char* curFrame);							//do BG update
		void GetFgOnly(unsigned char* curFrame, unsigned char* FgFrame);		//get FG 
		bool IsBackgroundReady();												//check if BG is ready
		void processFirstFrame(unsigned char* frame);							//set the BG / reset the BG 

		//
		unsigned char* m_fgMask;
		int m_iwidth;
		int m_iheight;
		int m_fgMatchingCntTh;

		unsigned char** m_samples;		//BG sample 
		unsigned char* m_fgMatchCnt;
		unsigned char* m_fgMatchCntBinaryMap;

		int ipixelDiffThreshold = 5;
		int m_iframeCnt;

	private:
		unsigned char* m_processFrame;
		int c_xoff[9] = { -1, 0, 1, -1, 1, -1, 0, 1, 0 };  //x neighbour
		int c_yoff[9] = { -1, 0, 1, -1, 1, -1, 0, 1, 0 };  //y neighbour
};

