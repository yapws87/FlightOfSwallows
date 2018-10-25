#pragma once
#include "stdio.h"
#include "stdlib.h"
#include "DPLabel.h"
#include "PiOpenCV.h"
#include "opencv2/opencv.hpp"
#include <opencv2/video/background_segm.hpp>
#include <thread>
#include <mutex>
#include <fstream>
#include <iomanip>
#include <sstream>
#include "PiCommon.h"
#include "PiTwitter.h"

#ifndef PERSONAL_COMPUTER
#include "unistd.h"
#else
#include <chrono>
#include <ctime>
#endif



enum {
	BIRD_UNKNOWN = 0,
	BIRD_FLY_OUT,
	BIRD_FLY_IN

};
struct BirdCell
{
	std::chrono::system_clock::time_point detect_time;
	int nSpeed;
	int nTrailLength;

	BirdCell() {
		nSpeed = 0;
		nTrailLength = 0;
	};
	
};


struct BirdData
{
	cv::Point location;
	cv::Rect boundingBox;
	bool bMatched;
	int nBirdDirection;
	bool bCounted;
	std::vector<cv::Point> vec_trail;
	float fAverageSpeed;
	int nPredictLimit = 5;

	BirdData()
	{
		location = cv::Point(0, 0);
		bMatched = false;
		nBirdDirection = BIRD_UNKNOWN;
		bCounted = false;
		fAverageSpeed = -1;
	}

	BirdData(cv::Rect birdBox)
	{
		*this = BirdData();
		location = (birdBox.tl() + birdBox.br()) / 2;
		boundingBox = birdBox;
		vec_trail.push_back(location);
	}

	inline bool isCounted()
	{
		return bCounted;
	}

	inline void setCounted()
	{
		bCounted = true;
	}

	inline bool isClassified()
	{
		return nBirdDirection != BIRD_UNKNOWN;
	}

	void update(cv::Rect birdBox, bool nPredict = false)
	{
		// If empty use own value
		if (birdBox.area() == 0)
			birdBox = boundingBox;

		location = (birdBox.tl() + birdBox.br()) / 2;
		boundingBox = birdBox;
		bMatched = true;
		vec_trail.push_back(location);
		//if(!nPredict)
		//	nPredictLimit++;

		// If direction is unknown
		if (nBirdDirection == BIRD_UNKNOWN)
		{
			int nTotalTrail = (int)vec_trail.size();
			
			int nDirection = BIRD_UNKNOWN;
			
			// Start measure when the trail is more than 3
			if (nTotalTrail >= 3)
			{
				// Accumulate direction
				for (int i = 1; i < nTotalTrail; i++)
				{
					nDirection += (vec_trail[i].x - vec_trail[i - 1].x);
				}

				// Determine direction 
				// only Motions more than 10 pixel
				if(abs(nDirection) - 10 > 0)
					nBirdDirection = nDirection > 0 ? BIRD_FLY_OUT : BIRD_FLY_IN;
				
			}
		}

		// Calculate Average Speed
		float fAccSpeed = 0;
		float fFrameTime = 0.01086956f; // 1 frame seconds
		// (Length of board in m) x (compensate distance from board in z direction) / (length of board in pixel)
		float fPix2Meter = (1.2f) * 1.1f / (240);

		// Count max of previous 5 frames
		int nValidCount = 0;
		for (int i = (int)vec_trail.size() - 2; i >= (int)vec_trail.size() - 5 && i > 0; i--)
		{
			cv::Point diff = vec_trail[i] - vec_trail[i + 1];
			float fMag = std::sqrt(diff.x*diff.x + diff.y*diff.y);
			fAccSpeed += fMag * fPix2Meter / fFrameTime;
			nValidCount++;
		}
		if (nValidCount > 0)
			fAverageSpeed = fAccSpeed / (float)nValidCount * 3.6; // change m/s to km/h by multiplying 3.6

	}

	bool predict()
	{
		// Not predicting unknown birds
		if (nPredictLimit < 0)
			return false;

		if (nBirdDirection == BIRD_UNKNOWN)
			return false;

		// Calculate the next step by averaging past steps
		int nValidCount = 0;
		cv::Point ptStep(0, 0);
		int nHistoryCount = 3; // Takes the information of previous 3 steps
		for (int i = (int)vec_trail.size() - 2; i >= (int)vec_trail.size() - nHistoryCount && i > 0; i--)
		{
			ptStep += vec_trail[i + 1] - vec_trail[i];
			nValidCount++;
		}
		ptStep /= nValidCount;

		double dMag = std::sqrt(ptStep.x*ptStep.x + ptStep.y*ptStep.y);
		cv::Rect updatedBox = boundingBox + ptStep;


		// Changes too small skip
		if (dMag < 5)
			return false;

		// Opposite direction
		if (nBirdDirection == BIRD_FLY_OUT && ptStep.x < 0)
			return false;

		// Opposite direction
		if (nBirdDirection == BIRD_FLY_IN && ptStep.x > 0)
			return false;

		

		// Update the steps
		update(updatedBox, true);
		nPredictLimit--;

		return true;
	}

	inline bool isMatched() { return bMatched; }

	cv::Rect getSafeRect(int nWidth, int nHeight)
	{
		cv::Rect safeBox = boundingBox;

		if (safeBox.x <= 0 || safeBox.y <= 0)
			return cv::Rect();

		if (safeBox.br().x >= nWidth || safeBox.br().y >= nHeight)
			return cv::Rect();

		return safeBox;
	}

};

class CBirdCounter
{
public:
	CBirdCounter();
	~CBirdCounter();

	void process_thread(cv::Mat matFrameGray);
	
	inline cv::Mat getDispMat(){
			return m_matSaveImage;
	};

	void setDisplay(bool bDisplay)
	{
		m_bDisplay = bDisplay;
	}

	void start_measure();
	void end_measure(int nInsertFPS = 0);
	inline bool checkReset() { 
		if (m_bResetFrames) {
			m_bResetFrames = false;
			return true;
		}
		else
			return false; }

	inline int getBirdInCount() { return m_nCount_In; };
	inline int getBirdOutCount() { return m_nCount_Out; };

protected:

	// Functions
	void insertText(cv::Mat & matDisplay, int nFPS, double nMean, double nThreshold);
	void prepareSaveImage(const cv::Mat matFg, cv::Mat matDisplay, int nfps, double dThreshold);
	
	bool countBird(cv::Mat matForeBird, cv::Mat matRealSrc, cv::Mat &matDisplay,bool bDisplay = false);
	cv::Rect boundingBirds(std::vector<cv::Point> birdPts);
	cv::Rect EnlargeSafeROI(cv::Rect oriRect, float fScale, int nBirdDirection);
	int findBestBirdIndex(BirdData bird_src, std::vector<BirdData> bird_candidates, float fIntersectRatio, bool bLimit = false);

	void insertBirdLog(BirdData bird_data);
	void printBirdLog();
	void printStatus_thread();
	//void printStatus_thread_func();
	void resetBirdCount();
	void init_birdLog();

	// Variables
	std::ofstream m_in_file;
	std::ofstream m_out_file;
	std::ofstream m_status_file;
	std::vector<BirdCell> m_inBird_logs;
	std::vector<BirdCell> m_outBird_logs;
	std::chrono::time_point<std::chrono::system_clock> m_statusTime;

	int m_nCount_In = 0;
	int m_nCount_Out = 0;
	double m_dTemperature = 0;


	// Temp problems
	int m_nFrameWidth = 0;
	int m_nFrameHeight = 0;
	bool m_bDisplay = false;
	bool m_bResetFrames = false;
	//int m_nFps = 0;


	bool m_bOnce = true;
	std::string m_videoFilename;
	
	double m_dthresh = 0;
	std::mutex m_mutex;
	cv::Ptr<cv::BackgroundSubtractor> m_pMOG; //MOG Background subtractor
	cv::Mat m_matForeground;
	cv::Mat m_matSaveImage;
	
	double m_dTime = 0;

	std::deque<cv::Mat> m_videoFrames;


	int m_nOverflowCount = 0;
	int m_nSaturationCount = 0;
	float m_fMinThresh = 0.01f;

	int m_nFps_real = 0;
	int m_nFrameAcc = 0;

	// Tweeter
	std::deque<double> m_preRatio;

	//Time and frame counts
	double m_dTimeAcc = 0;
	double m_dTickCount = 0;
	float m_fSecCount_tweet = 0;
	float m_fSecCount_status = 0;
	int m_nCountContinuosValid = 0;
	bool m_bMotionDetected = false;

	// Previous 10 frames
	double m_avgIntensity;

	std::vector<BirdData> m_birds;
	DPLabel dpLabel;
	int m_nToggleLearn = 0;

	PiOpenCV m_cv;
	PiTwitter m_piTweet;

};

