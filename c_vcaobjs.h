#pragma once
#include <stdio.h>
#include <vector>
#include <time.h>
#include <string>
#include <iostream>

//---
#define ROI_MAX_NUM 3

//--- enum
enum EScenario
{
	eInDoor,
	eOutDoor 
};

enum EDayNightMode
{
	eDayMode,
	eNightMode
};

//--- basic obj 

class CVcaPoint2i
{
	public:
		//Constructor
		CVcaPoint2i()
		{
			x = 0;
			y = 0;
		}
		CVcaPoint2i(float x, float y) : x(x), y(y) {}

		int x;
		int y;

		//cout function
		friend std::ostream& operator << (std::ostream& os, const CVcaPoint2i& point)
		{
			os << "(" << point.x << ", " << point.y << ")";
			return os;
		}
};

class CVcaPoint2f
{
	public:
		//Constructor
		CVcaPoint2f() 
		{
			x = 0.0;
			y = 0.0;
		}
		CVcaPoint2f(float x, float y): x(x), y(y){}

		float x;
		float y;

		//cout function
		friend std::ostream& operator << (std::ostream& os, const CVcaPoint2f& point) 
		{
			os << "(" << point.x << ", " << point.y << ")";
			return os;
		}
};

class CVcaSize2i 
{
	public:
		//Constructor
		CVcaSize2i()
		{
			width = 0;
			height = 0;
		}
		CVcaSize2i(int w, int h) : width(w), height(h) {}

		int width;
		int height;

		//cout function
		friend std::ostream& operator << (std::ostream& os, const CVcaSize2i& size)
		{
			os << "(" << size.width << " x " << size.height << ")";
			return os;
		}
};

class CVcaSize2f
{
	public:
		//Constructor
		CVcaSize2f()
		{
			width = 0.0;
			height = 0.0;
		}
		CVcaSize2f(int w, int h) : width(w), height(h) {}

		float width;
		float height;

		//cout function
		friend std::ostream& operator << (std::ostream& os, const CVcaSize2f& size)
		{
			os << "(" << size.width << " x " << size.height << ")";
			return os;
		}
};

class CVcaRect2i
{
	public:
		//Constructor
		CVcaRect2i()
		{
			x = 0;
			y = 0;
			w = 0;
			h = 0;
		}
		CVcaRect2i(int x, int y, int w, int h) :x(x), y(y), w(w), h(h) {}

		//Attributes
		int x;	
		int y;	
		int w;	
		int h;	

		//functions
		CVcaPoint2i tl() 
		{
			return CVcaPoint2i(x, y);
		}
		CVcaPoint2i br()
		{
			return CVcaPoint2i(x+w, y+h);
		}
		int area() 
		{
			return w * h;
		}

		//cout function
		friend std::ostream& operator << (std::ostream& os, const CVcaRect2i& rect)
		{
			os << "(" << rect.x << ", " << rect.y << ", " << rect.w << ", " << rect.h << ")";
			return os;
		}
};

class CVcaRect2f
{
	public:
		//Constructor
		CVcaRect2f()
		{
			x = 0.0;
			y = 0.0;
			w = 0.0;
			h = 0.0;
		}
		CVcaRect2f(float x, float y, float w, float h):x(x), y(y), w(w), h(h){}

		//Attributes
		float x;		// [0.0 ~ 1.0] float
		float y;		// [0.0 ~ 1.0] float
		float w;		// [0.0 ~ 1.0] float
		float h;		// [0.0 ~ 1.0] float

		//functions
		CVcaPoint2f tl()
		{
			return CVcaPoint2f(x, y);
		}
		CVcaPoint2f br()
		{
			return CVcaPoint2f(x + w, y + h);
		}
		float area()
		{
			return w * h;
		}

		//cout function
		friend std::ostream& operator << (std::ostream& os, const CVcaRect2f& rect)
		{
			os << "(" << rect.x << ", " << rect.y << ", " << rect.w << ", " << rect.h << ")";
			return os;
		}
};

class CVcaROI 
{
	public:
		//Constructor
		CVcaROI() 
		{
			id = -1;
			roiEnable = false;
			RoiBitmap = NULL;
			isMotionTigger = false;
			isHumanTigger = false;
		}
		CVcaROI(int id):id(id)
		{
			roiEnable = false;
			RoiBitmap = NULL;
			isMotionTigger = false;
			isHumanTigger = false;
		}
		/*~CVcaROI() 
		{
			if (RoiBitmap!=NULL) { free(RoiBitmap); }
		}*/

		//=== setting attributes ===
		int id;
		std::vector<CVcaPoint2f> roiPoints;
		bool roiEnable;						//��ROI�O�_�ҥ�
		unsigned char *RoiBitmap;			//ROI��mask
		CVcaSize2i RoiBitmapSize;

		//=== tracker attributes ===
		bool isMotionTigger;				//motion event tigger flag
		bool isHumanTigger;					//human event tigger flag

		//cout function
		friend std::ostream& operator << (std::ostream& os, const CVcaROI& roi)
		{
			os << "         ROIID: " << roi.id << "\n";
			if (roi.roiEnable)
			{
				os << "         ROIEnable: True\n";
			}
			else 
			{
				os << "         ROIEnable: False\n";
			}
			
			os << "         ROIPoints: \n";
			for (int i_point=0; i_point < roi.roiPoints.size(); i_point++)
			{
				os << "            p_" << i_point + 1 << ": " << roi.roiPoints[i_point] << "\n";
			}
			os << "         ROIBitmapSize: " << roi.RoiBitmapSize << std::endl;
			os << "         MotionTiggerFlag: " << roi.isMotionTigger << std::endl;
			os << "         HumanTiggerFlag: " << roi.isHumanTigger << std::endl;
			return os;
		}
};

class CVcaBBox
{
	public:
		//Constructor
		CVcaBBox()
		{
			x1 = 0.0;
			y1 = 0.0;
			x2 = 0.0;
			y2 = 0.0;
			score = 0.0;
			label = -1;
		}

		//
		float x1;
		float y1;
		float x2;
		float y2;
		float score;
		int label;

		//cout function
		friend std::ostream& operator << (std::ostream& os, const CVcaBBox& bbox)
		{
			os << "BBox (" << bbox.x1 << "," << bbox.x2 << ") (" << bbox.x2 << "," << bbox.y2 << ") score: " << bbox.score  << " label: " << bbox .label << std::endl;;
			return os;
		}
};

class CVcaObj
{
	public:
		//Constructor
		CVcaObj()
		{
			id = 0;
		
			moveDirection = 0;
			moveSpeedLevel = 0.0;

			matched = 0;
			unshowCounter = 0;
			trackingConter = 0;
		}
		CVcaObj(int id) : id(id) {}

		//=== tracker attributes ===
		int id;						//obj id
		CVcaRect2f rect;			//obj rect
		tm prePositionTime;			//obj previous time (timestamp)
		CVcaPoint2f prePosition;	//obj previous position (bbox center point)  p.s. update it when movedist > threshold 
		tm curPositionTime;			//obj current time (timestamp)
		CVcaPoint2f curPosition;	//obj current position (bbox center point)

		int moveDirection;			//obj move direction: [-1,0,1] ���: ��/�L/�k
		float moveSpeedLevel;		//obj move speed level: [0.0 ~ 1.0] �V����1.0��ܳt�׶V��

		int matched;				//obj matched flag (for tracking)
		int unshowCounter;			//obj unshow frames counter (for tracking)
		int trackingConter;			//obj tracking counter (for tracking)

		//cout function
		friend std::ostream& operator << (std::ostream& os, const CVcaObj& obj)
		{
			os << " id:" << obj.id << std::endl;
			os << " rect: " << obj.rect << std::endl;
			os << " unshowCounter: " << obj.unshowCounter << std::endl;
			os << " trackingConter: " << obj.trackingConter << std::endl;
			std::string prePositionTimeStr = std::to_string(obj.prePositionTime.tm_year + 1900) + "-" +
				std::to_string(obj.prePositionTime.tm_mon + 1) + "-" +
				std::to_string(obj.prePositionTime.tm_mday) + "-" +
				std::to_string(obj.prePositionTime.tm_hour) + ":" +
				std::to_string(obj.prePositionTime.tm_min) + ":" +
				std::to_string(obj.prePositionTime.tm_sec);
			os << " prePositionTime: " << prePositionTimeStr << " prePosition:" << obj.prePosition << std::endl;
			std::string curPositionTimeStr = std::to_string(obj.curPositionTime.tm_year + 1900) + "-" +
				std::to_string(obj.curPositionTime.tm_mon + 1) + "-" +
				std::to_string(obj.curPositionTime.tm_mday) + "-" +
				std::to_string(obj.curPositionTime.tm_hour) + ":" +
				std::to_string(obj.curPositionTime.tm_min) + ":" +
				std::to_string(obj.curPositionTime.tm_sec);
			os << " curPositionTime: " << curPositionTimeStr << " curPosition:" << obj.curPosition << std::endl;
			os << " moveDirection: " << obj.moveDirection << std::endl;
			os << " moveSpeedLevel: " << obj.moveSpeedLevel << std::endl;

			return os;
		}
};

//--- setting option
class CVcaSettingOpt
{
	public:
		//Constructor
		CVcaSettingOpt() 
		{
			//set default opt
			m_eScenario = eInDoor;
			m_eDayNightMode = eDayMode;

			m_motionOnOffFlag = true;
			m_motionSensitiveLevel = 3;
			m_motionThreshold = 0.1;

			m_humanOnOffFlag = true;
			m_humanSensitiveLevel = 3;
			m_humanThreshold = 0.7;

			m_ROINumber = 0;
		}

		//setting options & tracker
		EScenario m_eScenario;				//�Ǥ��ǥ~����: [eInDoor, eOutDoor], default: eInDoor
		EDayNightMode m_eDayNightMode;		//�դѶ©]�Ҧ�: [eDayMode, eNightMode], 

		bool m_motionOnOffFlag;				//���ʰ����}��: [true/false], default: true
		int m_motionSensitiveLevel;			//���ʰ����F�ӫ�: [1,2,3,4,5] default:3, �Ʀr�V�j�V�F�� 
		float m_motionThreshold;			//���ʰ����֭�: motionSensitiveLevel -> m_motionThreshold
											//			   1 -> 0.3
											//			   2 -> 0.2
											//			   3 -> 0.1
											//			   4 -> 0.05
											//			   5 -> 0.01

		bool m_humanOnOffFlag;				//�H�ΰ����}��: [true/false], default: true
		int m_humanSensitiveLevel;			//�H�ΰ����F�ӫ�: [1,2,3,4,5] default:3, �Ʀr�V�j�V�F�� 
		float m_humanThreshold;				//�H�Τ��ƻ֭�: humanSensitiveLevel -> m_humanThreshold
											//             1 -> 0.9
											//             2 -> 0.8
											//             3 -> 0.7
											//             4 -> 0.6
											//             5 -> 0.5

		int m_ROINumber;					//ROI�ϰ�ƶq: [0,1,2], default: 0(��ӵe�����OROI)
		CVcaROI m_RoiList[ROI_MAX_NUM];		//ROI�ϰ��

		//cout function
		friend std::ostream& operator << (std::ostream& os, const CVcaSettingOpt& settingOpt)
		{
			os << "[CVcaSettingOpt]\n";
			if (settingOpt.m_eScenario == EScenario::eInDoor)
			{
				os << "   Scenario: InDoor\n";
			}
			else 
			{
				os << "   Scenario: OutDoor\n";
			}
			if (settingOpt.m_eDayNightMode == EDayNightMode::eDayMode)
			{
				os << "   DayNightMode: DayMode\n";
			}
			else
			{
				os << "   DayNightMode: NightMode\n";
			}
			os << "   motionOnOffFlag: " << settingOpt.m_motionOnOffFlag << "\n";
			os << "   motionSensitiveLevel: " << settingOpt.m_motionSensitiveLevel << "\n";
			os << "   humanOnOffFlag: " << settingOpt.m_humanOnOffFlag << "\n";
			os << "   humanSensitiveLevel: " << settingOpt.m_humanSensitiveLevel << "\n";
			os << "   ROINumber: " << settingOpt.m_ROINumber << "\n";
			os << "   ROIList:\n";
			
			for (int i_roi=0; i_roi < ROI_MAX_NUM; i_roi++)
			{
				os << "      ROI[" << i_roi << "]:\n";
				os << settingOpt.m_RoiList[i_roi];
			}
			return os;
		}
};

//--- output 
class CVcaOutput
{
	public:
		//Constructor
		CVcaOutput() 
		{
			m_isMotionEventFlag = false;
			m_motionEventRoiId = -1;
			m_isHumanEventFlag = false;
			m_humanEventRoiId = -1;
			m_isTrackingTargetExist = false;
			m_trackingTargetDirection = 0;
			m_trackingTargetSpeedLevel = 0.0;
		}

		//Attributes
		bool m_isMotionEventFlag;			//���ʨƥ�o��: [true/false]
		int m_motionEventRoiId;				//���ʨƥ�o��ROI ID: [0, 1, 2], 0��ܾ�ӵe��

		bool m_isHumanEventFlag;			//�H�Ψƥ�o��: [true/false]
		int m_humanEventRoiId;				//�H�Ψƥ�o��ROI ID: [0, 1, 2], 0��ܾ�ӵe��

		bool m_isTrackingTargetExist;		//�ؼЪ��s�b�ƥ�
		CVcaRect2f m_trackingTargetRect;	//�ؼЪ���e�b�e���W����mrect
		int m_trackingTargetDirection;		//�ؼЪ����ʤ�V: [-1,0,1] ���: ��/�L/�k
		float m_trackingTargetSpeedLevel;	//�ؼЪ����ʳt�׵���: [0.0 ~ 1.0] �V����1.0��ܳt�׶V��

		//cout function
		friend std::ostream& operator << (std::ostream& os, const CVcaOutput& output)
		{
			os << "[CVcaOutput]\n";
			if (output.m_isMotionEventFlag)
			{
				os << "   isMotionEventFlag: true" << "\n";
			}
			else 
			{
				os << "   isMotionEventFlag: false" << "\n";
			}
			os << "   motionEventRoiId: " << output.m_motionEventRoiId << "\n";
			if (output.m_isHumanEventFlag)
			{
				os << "   isHumanEventFlag: true\n";
			}
			else 
			{
				os << "   isHumanEventFlag: false\n";
			}
			os << "   humanEventRoiId: " << output.m_humanEventRoiId << "\n";
			if (output.m_isTrackingTargetExist)
			{
				os << "   isTrackingTargetExist: true\n";
			}else
			{
				os << "   isTrackingTargetExist: false\n";
			}
			os << "   trackingTargetRect: " << output.m_trackingTargetRect << "\n";
			os << "   trackingTargetDirection: " << output.m_trackingTargetDirection << "\n";
			os << "   trackingTargetSpeedLevel: " << output.m_trackingTargetSpeedLevel << "\n";
			return os;
		}
};
