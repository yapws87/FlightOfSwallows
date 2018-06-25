#pragma once
#include "stdio.h"
#include "stdlib.h"
#include "opencv2/opencv.hpp"

class PiOpenCV
{
public:
	PiOpenCV();
	~PiOpenCV();

	cv::Mat pi_DFT(cv::Mat matSrc);
	cv::Mat pi_iDFT(cv::Mat matComplex);

	cv::Mat pi_RemoveHorizontalNoise(cv::Mat matSrc);
	cv::Mat piExample(cv::Mat matSrc);
};

