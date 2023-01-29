#pragma once

class _Size2i
{
public:
	int width = 0;
	int height = 0;
};
class _Size2f
{
public:
	float width = 0;
	float height = 0;
};

class _Rect2i
{
public:
	int x = 0;
	int y = 0;
	int width = 0;
	int height = 0;

	int imatched = 0;
	int itrackingcnt = 0;
	int iunshowcnt = 0;
	int iContainNum;
	int iIsHumanCnt = 0;
	int m_trackerid = 0;	

	int m_LongCnt = 0;
	int m_WideCnt = 0;
};
