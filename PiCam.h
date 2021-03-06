#pragma once
#include "stdio.h"
#include "stdlib.h"
#include "opencv2/opencv.hpp"
#include "PiCommon.h"
#include <thread>
#include <mutex>
#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>


class PiCam
{
public:
    PiCam();
    ~PiCam();

    bool init_cam(int nWidth = 320, int nHeight = 240, int nFPS = 60);
    void runFrame_thread();
	void save_thread();

   

    void get_frame(cv::Mat &matGray);
    void set_record();
    void set_save_directory(std::string file_directory);
    inline int getRealFrameRate(){ return m_nMeasured_fps;}
	void resetFrames();

protected:

    // Image related variables
	cv::VideoCapture m_cap;
	//std::deque<cv::Mat> m_matFrameColor;
	std::deque<cv::Mat> m_matFrameGray;
    std::deque<cv::Mat> m_videoFrames;
    PiCommon m_picom;

    // Init default Size
	int m_nFrameWidth;
	int m_nFrameHeight;
	int m_nFPS;
	bool m_bRecording_flag = false;
	bool m_bEnableRecord = true;
    int m_nMeasured_fps = 0;
    
    std::string m_save_directory;
    std::mutex m_mutex; 


    int m_nSave_duration_minutes = 5;

    void status_check(std::string item, std::string value, bool bstatus);


};
