// PiCam.cpp : Defines the entry point for the console application.
#include "stdio.h"
#include "stdlib.h"
#include "opencv2/opencv.hpp"

#include <thread>
#include "BirdCounter.h"
#include "PiCommon.h"
#include "PiCam.h"

CBirdCounter m_birdCount;
PiCam m_piCam;

int m_nDisplay_flag = 0;

void streamFrame_thread()
{
	m_piCam.runFrame_thread();
}

void saveFrame_thread()
{
	m_piCam.save_thread();
}

void process_thread()
{
	cv::Mat matGray, matColor;
	cv::Mat matDisp;
	for(;;)
	{
		m_piCam.get_frame(matGray,matColor);
		m_birdCount.process_thread(matGray,matColor);
		
		if(m_nDisplay_flag)
		{
			static int disp_count = 0;
			disp_count++;

			if(disp_count > 15)
			{
				matDisp = m_birdCount.getDispMat();
				if(!matDisp.empty())
				{
					cv::imshow("Display",matDisp);
					cv::waitKey(1);
				}
				disp_count = 0;
			}
		}
	}
}

int main(int argc, char * argv[])
{
	PiCommon picom;
	bool bCamInitFlag = false;

	static int nInit_width;
	static int nInit_height;
	static int nInit_fps;

	if (argc < 4)
	{
		picom.printStdLog("Not enough arguments given. Default value will be use." );
		m_piCam.init_cam();
	}
	else{
		nInit_width = atoi(argv[1]);
		nInit_height = atoi(argv[2]);
		nInit_fps = atoi(argv[3]);
		m_nDisplay_flag = atoi(argv[4]);
		bCamInitFlag = m_piCam.init_cam(nInit_width,nInit_height,nInit_fps);
	}
	
	std::vector<std::thread> threads;
	if(bCamInitFlag)
	{
		//VideoWriter vWriter;

        picom.printStdLog( "Starting all threads" );
		threads.push_back(std::thread(streamFrame_thread));
		threads.push_back(std::thread(process_thread));
		threads.push_back(std::thread(saveFrame_thread));
		//threads.push_back(std::thread(&CBirdCounter::tweet_thread, birdCounterPtr));

		for (auto& thread : threads) {
			thread.join();
		}
		std::cout<< "Thread Initialized" << std::endl;
	}
	
    return 0;
}
