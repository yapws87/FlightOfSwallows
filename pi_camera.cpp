// PiCam.cpp : Defines the entry point for the console application.
#include "stdio.h"
#include "stdlib.h"
#include "opencv2/opencv.hpp"

#include <thread>
#include "PiCam.h"
#include "BirdCounter.h"
#include "PiCommon.h"


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
	PiCommon picom;
	cv::Mat matGray;
	cv::Mat matDisp;
	m_birdCount.setDisplay(m_nDisplay_flag);
	m_birdCount.start_measure();
	for(;;)
	{
		m_piCam.get_frame(matGray);
		if(!matGray.empty()){
			m_birdCount.process_thread(matGray);
			
			int nRealFPS = m_piCam.getRealFrameRate();
			m_birdCount.end_measure(nRealFPS);
			m_birdCount.start_measure();

			if(m_birdCount.checkReset())
			{
				picom.printStdLog("Reset Frames");
				m_piCam.resetFrames();
			}
		}
		
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

	m_piCam.set_save_directory( "/home/pi/projects/FlightOfSwallows/CCTV" );
	picom.printStdLog("Current Temp : " + std::to_string(picom.get_temperature()));
	if (argc < 4)
	{
		picom.printStdLog("Not enough arguments given. Default value will be use." );
		m_piCam.init_cam();
	}
	else{
		nInit_width = atoi(argv[1]);
		nInit_height = atoi(argv[2]);
		nInit_fps = atoi(argv[3]);
		m_nDisplay_flag = atoi(argv[4]) > 0 ? true : false;
		bCamInitFlag = m_piCam.init_cam(nInit_width,nInit_height,nInit_fps);
		picom.uniSleep(1000);
	}
	
	std::vector<std::thread> threads;
	if(bCamInitFlag)
	{
		//VideoWriter vWriter;
        picom.printStdLog( "Starting Stream thread" );
		threads.push_back(std::thread(streamFrame_thread));
		
		picom.printStdLog( "Starting Process thread" );
		threads.push_back(std::thread(process_thread));

		picom.printStdLog( "Starting Save thread" );
		threads.push_back(std::thread(saveFrame_thread));
		//threads.push_back(std::thread(&CBirdCounter::tweet_thread, birdCounterPtr));

		for (auto& thread : threads) {
			thread.join();
		}
		std::cout<< "Thread Initialized" << std::endl;
	}
	
    return 0;
}
