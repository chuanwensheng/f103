#include "classify_human_lds.h"

//#define PC_DEBUG
#ifdef PC_DEBUG
#include <opencv2/opencv.hpp>
#endif 

using namespace MNN;


//constructor
CHumanClassificationLDS::CHumanClassificationLDS()
{
	m_num_thread = 1;
	m_srcImgWidth = 0;
	m_srcImgHeight = 0;
}

//destructor
CHumanClassificationLDS::~CHumanClassificationLDS()
{

}

//--- public functions ---
bool CHumanClassificationLDS::Initial()
{
	m_modelFilePath = "./models/yft.m";
	return InitBasis();
}

bool CHumanClassificationLDS::Initial(std::string &modelPath)
{
	m_modelFilePath = modelPath;
	return InitBasis();
}

float CHumanClassificationLDS::Inference(unsigned char *imageData, int imageWidth, int imageHeight, int imageChannel)
{
	return pred(imageData, imageWidth, imageHeight, imageChannel);
}

//--- private function ---
bool CHumanClassificationLDS::InitBasis() 
{
	//set config
	m_config.numThread = m_num_thread;
	m_backendConfig.precision = (MNN::BackendConfig::PrecisionMode) 1;
    m_config.backendConfig = &m_backendConfig;

	std::cout << "[CHumanDetectionModule] init success." << std::endl;
	return true;
}

//prediction
float CHumanClassificationLDS::pred(unsigned char *imageData, int imageWidth, int imageHeight, int imageChannel)
{
	float t_s, t_e;
	int srcSize = m_srcImgWidth * m_srcImgHeight * 3;
	int dstSize = m_INPUT_SIZE_H * m_INPUT_SIZE_W * 3;
	if (m_srcImgWidth != imageWidth || m_srcImgHeight != imageHeight)
	{
		m_srcImgWidth = imageWidth;
		m_srcImgHeight = imageHeight;
	}
	unsigned char* srcImg = (unsigned char*)malloc(srcSize * sizeof(unsigned char));
	unsigned char* resizedImg = (unsigned char*)malloc(dstSize * sizeof(unsigned char));
	memcpy(srcImg, imageData, srcSize * sizeof(unsigned char));

	//resize
	Tools::ImageProcess::resize(srcImg, m_srcImgWidth, m_srcImgHeight, imageChannel, m_INPUT_SIZE_W, m_INPUT_SIZE_H, resizedImg);
	freeMemory(srcImg);

	//load model
	std::shared_ptr<MNN::Interpreter> interpreter(MNN::Interpreter::createFromFile(m_modelFilePath.c_str()));
	
	//create session
	auto session = interpreter->createSession(m_config);
	//set input image
	auto input_tensor = interpreter->getSessionInput(session, nullptr);
	//resize tensor
	interpreter->resizeTensor(input_tensor, {1, 3, m_INPUT_SIZE_H, m_INPUT_SIZE_W}); //NCHW
	//resize session
	interpreter->resizeSession(session);
	//create pretreat
	std::shared_ptr<MNN::CV::ImageProcess> pretreat(MNN::CV::ImageProcess::create(MNN::CV::BGR, MNN::CV::RGB, m_mean_vals, 3, m_norm_vals, 3));
    //convert image array to tensor
	pretreat->convert(resizedImg, m_INPUT_SIZE_H, m_INPUT_SIZE_W, 0, input_tensor);
	freeMemory(resizedImg);
	
	//run session (inference)
	t_s = clock();
	interpreter->runSession(session);
	t_e = clock();
	printf("   >> run inference time cost: %f sec.\n", (t_e - t_s) / CLOCKS_PER_SEC);

	//MNN tensor
	MNN::Tensor* tensor_scores = interpreter->getSessionOutput(session, m_scores.c_str());
	MNN::Tensor* tensor_scores_host = new MNN::Tensor(tensor_scores, MNN::Tensor::TENSORFLOW);

	//get inference output
	t_s = clock();
	tensor_scores = interpreter->getSessionOutput(session, m_scores.c_str());
	tensor_scores->copyToHostTensor(tensor_scores_host);
	t_e = clock();
	printf(" >> output postprocess time cost: %f sec.\n", (t_e - t_s) / CLOCKS_PER_SEC);

	float nVal = tensor_scores_host->host<float>()[1];
	//free memory
	if (tensor_scores_host != NULL)
	{
		delete tensor_scores_host;
		tensor_scores_host = NULL;
	}

	return nVal;
}

//free memory
void CHumanClassificationLDS::freeMemory(unsigned char* data)
{
	if (data != NULL)
	{
		free(data);
		data = NULL;
	}
}
