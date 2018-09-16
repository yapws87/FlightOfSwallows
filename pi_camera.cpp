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

void startFrame_thread()
{
	m_piCam.runFrame_thread();
}

void process_thread()
{
	cv::Mat matGray, matColor;
	for(;;)
	{
		m_piCam.get_frame(matGray,matColor);
		m_birdCount.process_thread(matGray,matColor);
		cv::imshow("Display",m_matSaveImage);

	}
	
	
}

int main(int argc, char * argv[])
{
	PiCommon picom;
	bool bCamInitFlag = false;

	int nInit_width;
	int nInit_height;
	int nInit_fps;

	if (argc < 3)
	{
		picom.printStdLog("Not enough arguments given. Default value will be use." );
		m_piCam.init_cam();
	}
	else{
		nInit_width = atoi(argv[1]);
		nInit_height = atoi(argv[2]);
		nInit_fps = atoi(argv[3]);
		bCamInitFlag = m_piCam.init_cam(nInit_width,nInit_height,nInit_fps);
	}
	
	
	std::vector<std::thread> threads;
	if(bCamInitFlag)
	{
		//VideoWriter vWriter;
	
        picom.printStdLog( "Starting all threads" );
		threads.push_back(std::thread(startFrame_thread));
		threads.push_back(std::thread(process_thread));
		//threads.push_back(std::thread(&PiCam::getFrame_thread, picam));
		//threads.push_back(std::thread(&CBirdCounter::process_thread, birdCounterPtr,picam));
		//threads.push_back(std::thread(&CBirdCounter::tweet_thread, birdCounterPtr));
		//threads.push_back(std::thread(&CBirdCounter::save_thread, birdCounterPtr));
		
		
		for (auto& thread : threads) {
			thread.join();
		}
		std::cout<< "Thread Initialized" << std::endl;
	}
	
	
	//delete picam;
	//delete birdCounterPtr;

    return 0;
	
	
}
