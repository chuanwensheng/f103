#include "bg_vibe_simple.h"


//Constructor
CVibeBackgroundModelSimple::CVibeBackgroundModelSimple()
{
		m_fgMatchingCntTh = 4;
		ipixelDiffThreshold = 10;

		m_iframeCnt = 0;

		m_fgMask = NULL;
		m_fgMatchCnt = NULL;
}

//Destructor
CVibeBackgroundModelSimple::~CVibeBackgroundModelSimple()
{
	for (int i = 0; i < m_iwidth* m_iheight; i++)
	{
		if (m_samples[i] != NULL)
		{
			free(m_samples[i]);
		}
	}
	free(m_samples);

	if (m_fgMask != NULL)
	{
		free(m_fgMask);
	}

	if (m_fgMatchCnt != NULL)
	{
		free(m_fgMatchCnt);
	}
	
#ifdef PC_DEBUG
	printf("---------------- Background free mem ----------------\n");
#endif
}

//--- public functions ---

//init samples, fgMask, fgMatchCnt and process first frame as samples 
bool CVibeBackgroundModelSimple::Initial(unsigned char* pProcessFrame, int iwidth, int iheight)
{
#ifdef PC_DEBUG
	std::cout << "[CVibeBackgroundModelSimple] init..." << std::endl;
#else
	std::cout << "[CBackgroundModule] init..." << std::endl;
#endif

	m_processFrame = pProcessFrame;

	m_iwidth = iwidth;
	m_iheight = iheight;

	//init items
	m_samples = (unsigned char**)malloc(iheight * iwidth * sizeof(unsigned char*));

	for (int i = 0; i < iheight * iwidth; i++)
	{
		m_samples[i] = (unsigned char*)malloc(NUM_SAMPLES * sizeof(unsigned char));
	}

	//init the m_fgMask
	m_fgMask = (unsigned char*)malloc(iheight * iwidth * sizeof(unsigned char));
	if (m_fgMask == NULL) 
	{ 
#ifdef PC_DEBUG
		std::cout << "[CVibeBackgroundModelSimple] Init failed." << std::endl;
#else
		std::cout << "[CBackgroundModule] Init failed." << std::endl;
#endif
		return false; 
	}
	m_fgMatchCnt = (unsigned char*)malloc(iheight * iwidth * sizeof(unsigned char));
	memset(m_fgMatchCnt, 0, iheight * iwidth * sizeof(unsigned char));
	if (m_fgMatchCnt == NULL) 
	{ 
#ifdef PC_DEBUG
		std::cout << "[CVibeBackgroundModelSimple] Init failed." << std::endl;
#else
		std::cout << "[CBackgroundModule] Init failed." << std::endl;
#endif
		return false; 
	}

	//init the m_fgMatchCntBinaryMap
	m_fgMatchCntBinaryMap = (unsigned char*)malloc(iheight * iwidth * sizeof(unsigned char));
	memset(m_fgMatchCntBinaryMap, 0, iheight * iwidth * sizeof(unsigned char));
	if (m_fgMatchCntBinaryMap == NULL) 
	{ 
#ifdef PC_DEBUG
		std::cout << "[CVibeBackgroundModelSimple] Init failed." << std::endl;
#else
		std::cout << "[CBackgroundModule] Init failed." << std::endl;
#endif
		return false; 
	}
	

#ifdef PC_DEBUG
	std::cout << "[CVibeBackgroundModelSimple] init success." << std::endl;
#else
	std::cout << "[CBackgroundModule] init success." << std::endl;
#endif
	return true;
}

//analyse pixel and update samples
void CVibeBackgroundModelSimple::UpdateBackground(unsigned char* curFrame)
{
	int localDiffThreshold = ipixelDiffThreshold;
	if (curFrame == NULL)
	{
		return;
	}

	m_processFrame = curFrame;

	int iUpdateBySampleNum = m_iframeCnt % NUM_SAMPLES;
	int iUpdateByFactor = m_iframeCnt % SUBSAMPLE_FACTOR;

	if (m_iframeCnt < NUM_SAMPLES)
	{
		if (m_iframeCnt == 0)
		{
			for (int icurPixelIdx = 0; icurPixelIdx < m_iwidth * m_iheight; icurPixelIdx++)
			{
				int matches = 0;
				m_samples[icurPixelIdx][0] = m_processFrame[icurPixelIdx];
			}
		}
		else
		{
			for (int icurPixelIdx = 0; icurPixelIdx < m_iwidth * m_iheight; icurPixelIdx++)
			{
				int matches = 0;
				int isum = 0;
				int isumcnt = 0;

				for (int isample = 0; isample < m_iframeCnt; isample++)
				{
					int iBgValue = m_samples[icurPixelIdx][isample];
					int iabsDis = iBgValue - m_processFrame[icurPixelIdx];

					if (iabsDis < 0)
					{
						iabsDis = -iabsDis;
					}
					if (iabsDis < ipixelDiffThreshold)
					{
						matches++;
					}

					if (isample > 0)
					{
						isum = isum + iBgValue;
						isumcnt++;
					}
				}

				if (matches > 0)
				{
					m_samples[icurPixelIdx][iUpdateBySampleNum] = m_processFrame[icurPixelIdx];
				}

			}
		}
	}
	else
	{
		//update 
		for (int i = 0; i < m_iheight; i++)
		{
			for (int j = 0; j < m_iwidth; j++)
			{
				int matches = 0;
				int icurPixelIdx = i * m_iwidth + j;

				//analyse pixel	
				for (int isample = 0; isample < NUM_SAMPLES; isample++)
				{
					int iabsDis = m_samples[icurPixelIdx][isample] - m_processFrame[icurPixelIdx];

					if (iabsDis < 0)
					{
						iabsDis = -iabsDis;
					}
					if (iabsDis < localDiffThreshold)
					{
						matches++;
					}
				}

				//set fgMask and update samples
				if (matches >= MATCHES_TH)
				{
					// It is a background pixel  				
					m_fgMatchCnt[icurPixelIdx] = 0;
					// Set background pixel to 0 
					m_fgMask[icurPixelIdx] = 0;

					//update own samples	
					for (int isample = 0; isample < NUM_SAMPLES; isample++)
					{
						m_samples[icurPixelIdx][isample] = m_processFrame[icurPixelIdx];
					}
				}
				else
				{
					//--> it is a fg pixel
					m_fgMask[icurPixelIdx] = 255;
					m_fgMatchCnt[icurPixelIdx]++;
					//�s�� m_fgMatchingTh ���Q�����I���h���I��
					if (m_fgMatchCnt[icurPixelIdx] > m_fgMatchingCntTh)
					{
						m_samples[icurPixelIdx][iUpdateBySampleNum] = m_processFrame[icurPixelIdx];
					}
				}
			}
		}
	}

	m_iframeCnt++;
}

//check is bg ready
bool CVibeBackgroundModelSimple::IsBackgroundReady()
{
	if (m_iframeCnt > NUM_SAMPLES)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//--- private function ---
void  CVibeBackgroundModelSimple::processFirstFrame(unsigned char* frame)
{
	int row, col;
	m_iframeCnt = 0;

	for (int i = 0; i < m_iheight * m_iwidth; i++)
	{
		m_fgMask[i] = 0;
		m_fgMatchCnt[i] = 0;
	}
}

void CVibeBackgroundModelSimple::GetFgOnly(unsigned char* curFrame, unsigned char* FgFrame)
{
	if (curFrame == NULL)
	{
		return;
	}
	m_processFrame = curFrame;

	//update 
	for (int i = 0; i < m_iheight; i++)
	{
		for (int j = 0; j < m_iwidth; j++)
		{
			int matches = 0;
			int k = 0;

			int icurPixelIdx = i * m_iwidth + j;

			//analyse pixel
			while (matches < MATCHES_TH && k < NUM_SAMPLES)
			{
				int iabsDis = m_samples[icurPixelIdx][k] - m_processFrame[icurPixelIdx];

				if (iabsDis < 0)
				{
					iabsDis = -iabsDis;
				}
				if (iabsDis < ipixelDiffThreshold)
				{
					matches++;
				}

				k++;
			}

			//set fgMask and update samples
			FgFrame[icurPixelIdx] = 0;
			if (matches < MATCHES_TH)
			{
				FgFrame[icurPixelIdx] = 255;
			}
		}
	}
}
