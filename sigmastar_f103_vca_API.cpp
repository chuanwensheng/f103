#include "sigmastar_f103_vca_API.h"

#define CAL_TIMECOST
#ifdef CAL_TIMECOST
	double t_s, t_e;
#endif

static CVcaAnalyzer *g_pVcaAnalyzer = NULL;

//Constructor 
CVcaAnalyzerAPI::CVcaAnalyzerAPI() 
{
	if (g_pVcaAnalyzer == NULL)
	{
		std::cout << "[CVcaAnalyzerAPI] init CVcaAnalyzer" << std::endl;
		g_pVcaAnalyzer = new CVcaAnalyzer;
	}
}

//Destructor
CVcaAnalyzerAPI::~CVcaAnalyzerAPI() 
{
	if (g_pVcaAnalyzer != NULL)
	{
		std::cout << "[CVcaAnalyzerAPI] release CVcaAnalyzer" << std::endl;
		delete g_pVcaAnalyzer;
		g_pVcaAnalyzer = NULL;
	}
}

//锟窖计砞锟絯
#pragma region[Setting functions]
void CVcaAnalyzerAPI::Set_Scenario(bool scenarioSetting) 
{
	g_pVcaAnalyzer->Set_Scenario(scenarioSetting);
};

void CVcaAnalyzerAPI::Set_DayNightMode(bool dayNightModeSetting) 
{
	g_pVcaAnalyzer->Set_DayNightMode(dayNightModeSetting);
};

void CVcaAnalyzerAPI::Set_MotionOnOffFlag(bool motionOnOffFlag) 
{
	g_pVcaAnalyzer->Set_MotionOnOffFlag(motionOnOffFlag);
};

void CVcaAnalyzerAPI::Set_MotionSensitiveLevel(int motionSensitiveLevel) 
{
	g_pVcaAnalyzer->Set_MotionSensitiveLevel(motionSensitiveLevel);
};

void CVcaAnalyzerAPI::Set_HumanOnOffFlag(bool humanOnOffFlag) 
{
	g_pVcaAnalyzer->Set_HumanOnOffFlag(humanOnOffFlag);
};

void CVcaAnalyzerAPI::Set_HumanSensitiveLevel(int humanSensitiveLevel) 
{
	g_pVcaAnalyzer->Set_HumanSensitiveLevel(humanSensitiveLevel);
};

bool CVcaAnalyzerAPI::Set_ROI_create(int roiID, std::vector<std::vector<float>>  roiPoints)
{
	bool nFlag = g_pVcaAnalyzer->Set_ROI_create(roiID, roiPoints);
	return nFlag;
}

bool CVcaAnalyzerAPI::Set_ROI_remove(int roiID) 
{
	bool nFlag = g_pVcaAnalyzer->Set_ROI_remove(roiID);
	return nFlag;
}

bool CVcaAnalyzerAPI::Set_ROI_modify(int roiID, std::vector<std::vector<float>>  roiPoints)
{
	bool nFlag = g_pVcaAnalyzer->Set_ROI_modify(roiID, roiPoints);
	return nFlag;
}

#pragma endregion


//show settingOPt
void CVcaAnalyzerAPI::Show_SettingOpt() 
{	
	g_pVcaAnalyzer->Show_SettingOpt();
}

//init
bool CVcaAnalyzerAPI::Init(int imageWidth, int imageHeight, std::string modelPath)
{
#ifdef CAL_TIMECOST
	t_s = clock();
#endif
	bool flag = g_pVcaAnalyzer->Init(imageWidth, imageHeight, modelPath);

#ifdef CAL_TIMECOST
	t_e = clock();
#endif
	printf("   >> Init time cost: %f sec.\n", (t_e - t_s) / CLOCKS_PER_SEC);
	return flag;
}

//analyze
int CVcaAnalyzerAPI::Analyze(unsigned char *imageData, int imageWidth, int imageHeight, int imageFormat, bool isCamereMovingFlag, CVcaOutput &output)
{
#ifdef CAL_TIMECOST
	t_s = clock();
#endif
	g_pVcaAnalyzer->Analyze(imageData, imageWidth, imageHeight, imageFormat, isCamereMovingFlag, output);

#ifdef CAL_TIMECOST
	t_e = clock();
#endif
	printf("   >> Analyze time cost: %f sec.\n", (t_e - t_s) / CLOCKS_PER_SEC);

	return 0;
}