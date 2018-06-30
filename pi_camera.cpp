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
		m_piCam.get_frame(matGray,matColor)
		m_birdCount.process_thread(matGray,matColor);

	}
	
	
}

int main()
{
    PiCommon picom;
	CBirdCounter* birdCounterPtr = new CBirdCounter();
	PiCam* picam = new PiCam();

	std::cout<< "Init Video" << std::endl;
	
	
	std::vector<std::thread> threads;
	if(picam->init_cam())
	{
		//VideoWriter vWriter;
	
        picom.printStdLog( "Starting all threads" );
		threads.push_back(std::thread(&startFrame_thread, birdCounterPtr,picam));
		threads.push_back(std::thread(&process_thread, birdCounterPtr,picam));
		//threads.push_back(std::thread(&PiCam::getFrame_thread, picam));
		//threads.push_back(std::thread(&CBirdCounter::process_thread, birdCounterPtr,picam));
		//threads.push_back(std::thread(&CBirdCounter::tweet_thread, birdCounterPtr));
		//threads.push_back(std::thread(&CBirdCounter::save_thread, birdCounterPtr));
		
		
		for (auto& thread : threads) {
			thread.join();
		}
		std::cout<< "Thread Initialized" << std::endl;
	}
	
	
	delete picam;
	delete birdCounterPtr;

    return 0;
	
	
}
