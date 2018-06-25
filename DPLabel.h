#pragma once
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/opencv_modules.hpp>


struct LabelProp {

	int nClass = 0;
	int nNextLabelIdx = -1;
	cv::Point minPt = cv::Point(INT_MAX, INT_MAX);
	cv::Point maxPt = cv::Point(0,0);

	LabelProp()
	{}

	LabelProp(int _nClass, cv::Point curPt)
	{
		nClass = _nClass;
		update(curPt);
	}

	inline void setNext(int nextLabel) {
		nNextLabelIdx = nextLabel;
	}

	inline bool isRoot()
	{
		return nNextLabelIdx < 0 ? true : false;
	}

	void update(cv::Point curPt){
	
		minPt.x = curPt.x < minPt.x ? curPt.x : minPt.x;
		minPt.y = curPt.y < minPt.y ? curPt.y : minPt.y;
		maxPt.x = curPt.x > maxPt.x ? curPt.x : maxPt.x;
		maxPt.y = curPt.y > maxPt.y ? curPt.y : maxPt.y;

	}

	void combine(LabelProp lprop)
	{
		update(lprop.minPt);
		update(lprop.maxPt);
	}

};


class DPLabel
{
public:
	DPLabel();
	~DPLabel();

	
	int run(cv::Mat matBinary);
	cv::Mat getLabelImage();
	std::vector<cv::Rect> getBoundingBoxes();
protected:
	std::vector<int> m_labelTable;
	std::vector<LabelProp> m_labelProps;
	cv::Mat matLabel_first;
	cv::Mat matLabel_second;

	
	void simplifyLabels(std::vector<LabelProp> labelProps
		, std::vector<int> &newLabelTable
		, std::vector < LabelProp > &newLabelProps);
	

};

