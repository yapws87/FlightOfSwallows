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

	void tweet_thread_proc();
	void tweet_graph(std::string strdate);
	void tweet_bird( cv::Mat matBird);
	void tweet_image(std::string msg, cv::Mat matImage);

public:
	PiTwitter();
	~PiTwitter();

	void setPathsFunction(std::string piCmd_tweetPic,std::string piCmd_analyzeHisto){
		m_piCmd_tweetPic = _piCmd;
		m_piCmd_analyzeHisto = _piImg;
	}
	void setPathsData(std::string piImg,std::string piHisto,std::string piBirdLog){
		m_piImg = _piImg;
		m_piHisto = _piHisto;
		m_piBirdLog = _piBirdLog;
	}

	void tweet_image(std::string msg, cv::Mat matImage);

	void tweet_thread();

	void tweet_graph_thread( std::string strdate);
	void tweet_bird_thread( cv::Mat matBird);
}