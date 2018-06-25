#pragma once
#include "stdlib.h"
#include "stdio.h"
#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <chrono>
#include <thread>
#include <mutex>
#include "opencv2/opencv.hpp"

#ifndef PERSONAL_COMPUTER
#include "unistd.h"
#endif

class PiCommon
{
protected:
	std::mutex m_mutex;
	double m_dTickCount = 0;

public:
	PiCommon();
	~PiCommon();

	std::string getString_fromCmd(std::string cmd);
		
	// Time related functions
	std::string get_current_date();
	std::string get_current_time();
	std::string get_current_time_and_date();
	std::string get_current_day();
	std::string get_yesterday_date();
	void uniSleep(int nSleepTime_milisec);

	bool isDateDiff();
	bool isHourDiff();

	inline std::string getCurrentDate() { return m_currentDate; }
	inline std::string getYesterdayDate() { return m_previousDate; }
	void printStdLog(std::string data);

	void countFPSStart();
	void countFPSEnd();
	inline int getCountFPS(){ return m_nFps_real; };

protected:
	std::string m_currentDate = "";
	std::string m_previousDate = "";

	int m_nFps_real = 0; // Current FPS
	int m_nFrameAcc = 0; // Collection of frame number
	int m_dTimeAcc = 0; // Collection of time
};

