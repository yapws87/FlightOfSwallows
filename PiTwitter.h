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

	std::string m_proj_folder = "/home/pi/projects/FlightOfSwallows/";
	std::string m_piCmd_tweetPic = "sudo python " + m_proj_folder + "twitter/./twitter_pic.py ";
	std::string m_piCmd_analyzeHisto = "python3 " + m_proj_folder + "python/analyze_birdlog.py ";

	//std::string m_proj_folder = "C:\\Users\\yapws87\\Desktop\\FlightOfSwallows\\";
	//std::string m_piCmd_tweetPic = "python " + m_proj_folder + "twitter\\.\\twitter_pic.py ";
	//std::string m_piCmd_analyzeHisto = "python " +  m_proj_folder + "python\\analyze_birdlog.py ";
	//std::string m_piBirdLog = m_proj_folder;

	std::string m_piImg =  m_proj_folder + "image.jpg";
	std::string m_piHisto =  m_proj_folder + "histogram.jpg";
	std::string m_piLineGraph =  m_proj_folder + "daily_graph.jpg";
	std::string m_piBirdLog = m_proj_folder + "bird_log/"; 

	double m_dthresh = 0;
	double m_dTime = 0;

	//void tweet_graph(std::string strdate);
	//void tweet_bird( cv::Mat matBird);
	//void tweet_image(std::string msg, cv::Mat matImage);

public:
	PiTwitter();
	~PiTwitter();

	void setPathsFunction(std::string piCmd_tweetPic,std::string piCmd_analyzeHisto){
		m_piCmd_tweetPic = piCmd_tweetPic;
		m_piCmd_analyzeHisto = piCmd_analyzeHisto;
	}
	void setPathsData(std::string piImg,std::string piHisto,std::string piLineGraph,std::string piBirdLog){
		m_piImg = piImg;
		m_piHisto = piHisto;
		m_piLineGraph = piLineGraph;
		m_piBirdLog = piBirdLog;
	}

	void tweet_graph_thread( std::string strdate);
	void tweet_bird_thread( cv::Mat matBird
	  	, double dThresh
    	, double dTime
		, double dTemperature
    	, int nCountIn
   		, int nCountOut
    	, double dAvgIntensity);
};