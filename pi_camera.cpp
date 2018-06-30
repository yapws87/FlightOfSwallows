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

	}
	
	
}

int main(int argc, char * argv[])
{
	PiCommon picom;
	if (argc < 3)
	{
		picom.printStdLog("Not enough arguments given. Default value will be use." );
	}
	int nInit_width = atoi(argv[1]);
	int nInit_height = atoi(argv[2]);
	int nInit_fps = atoi(argv[3]);
	
	std::vector<std::thread> threads;
	if(m_piCam.init_cam(nInit_width,nInit_height,nInit_fps))
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
