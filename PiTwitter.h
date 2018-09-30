#pragma once
#include "stdio.h"
#include "stdlib.h"
#include "PiCommon.h"
#include "opencv2/opencv.hpp"

class PiTwitter
{
protected:
	bool m_bTweeterFlag = false;
	bool m_bTweeterGraph = false;
	bool m_bSaveStatus = false;

	std::string m_piCmd_tweetPic = "sudo python /home/pi/Desktop/Twitter/./twitter_pic.py ";
	std::string m_piCmd_analyzeHisto = "python3 /home/pi/Desktop/Twitter/analyze_birdlog.py ";

	std::string m_piImg = " '/home/pi/openCV_test/image.jpg'";
	std::string m_piHisto = " /home/pi/openCV_test/histogram.jpg ";
	std::string m_piBirdLog = " /home/pi/openCV_test/bird_log/";

	double m_dthresh = 0;

	bool m_bOnce = false;
	double m_dTime = 0;


	void tweet_graph(std::string strdate);
	void tweet_bird( cv::Mat matBird);
	void tweet_image(std::string msg, cv::Mat matImage);

public:
	PiTwitter();
	~PiTwitter();

	void setPathsFunction(std::string piCmd_tweetPic,std::string piCmd_analyzeHisto){
		m_piCmd_tweetPic = piCmd_tweetPic;
		m_piCmd_analyzeHisto = piCmd_analyzeHisto;
	}
	void setPathsData(std::string piImg,std::string piHisto,std::string piBirdLog){
		m_piImg = piImg;
		m_piHisto = piHisto;
		m_piBirdLog = piBirdLog;
	}

	void tweet_graph_thread( std::string strdate);
	void tweet_bird_thread( cv::Mat matBird
	  	, double dThresh
    	, double dTime
    	, int nCountIn
   		 , int nCountOut
    	, std::deque<double> preRatio);
};