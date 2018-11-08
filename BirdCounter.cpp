#include "BirdCounter.h"

#define PROGRAM_FOLDER "/home/pi/projects/FlightOfSwallows/"

CBirdCounter::CBirdCounter()
{
	//m_pMOG = cv::createBackgroundSubtractorMOG2(90 * 2 , 35, false);
	m_pMOG = cv::createBackgroundSubtractorKNN(90 * 10, 30, false);
	//m_pMOG->SetVarThreshold(12);
	m_avgIntensity = 0;
}


CBirdCounter::~CBirdCounter()
{
}

void CBirdCounter::start_measure()
{
	m_dTickCount = cv::getTickCount();
}

void CBirdCounter::end_measure(int nInsertFPS)
{
	// -----------------  Calculate FPS
	double dTime;
	if(!nInsertFPS){
		dTime = (cv::getTickCount() - m_dTickCount) / cv::getTickFrequency() ;
		double dFPS;
		dFPS = 1 / dTime;
		m_nFps_real = (m_nFps_real + dFPS) / 2;
		if (m_nFps_real > 1000)
			m_nFps_real = 0;

	}
	else{
		m_nFps_real = nInsertFPS;
	}
	
}

void CBirdCounter::insertText(cv::Mat & matDisplay, int nFPS, double nMean, double nThreshold)
{
	float fFontSize = 0.4f;
	std::string strline;
	if (!matDisplay.empty())
	{
		strline = "FPS : " + std::to_string(nFPS);
		putText(matDisplay, strline, cv::Point(5, 25), cv::FONT_HERSHEY_SIMPLEX, fFontSize, cv::Scalar(0, 100, 255));

		strline = "Count Out : " + std::to_string(m_nCount_Out);
		putText(matDisplay, strline, cv::Point(5, 40), cv::FONT_HERSHEY_SIMPLEX, fFontSize, cv::Scalar(0, 100, 255));

		strline = "Count In : " + std::to_string(m_nCount_In);
		putText(matDisplay, strline, cv::Point(5, 55), cv::FONT_HERSHEY_SIMPLEX, fFontSize, cv::Scalar(0, 100, 255));

		// A bar to makes text more readable
		cv::Mat matBar = cv::Mat::zeros(matDisplay.rows, matDisplay.cols, CV_8UC3);
		cv::rectangle(matBar, cv::Rect(0, matDisplay.rows - 14, matDisplay.cols, 14), cv::Scalar(100, 100, 100), -1);
		cv::subtract(matDisplay, matBar, matDisplay);

#ifndef PERSONAL_COMPUTER
		//Get local time for display
		time_t rawtime;
		struct tm* timeinfo;

		time(&rawtime);
		timeinfo = localtime(&rawtime);
		putText(matDisplay, asctime(timeinfo), cv::Point(60, matDisplay.rows - 5), cv::FONT_HERSHEY_SIMPLEX, fFontSize, cv::Scalar(0, 100, 255));
#endif

	}
	else
	{
		//imshow("matDisplay",frame);
	}
}


void CBirdCounter::prepareSaveImage(const cv::Mat matFg, cv::Mat matDisplay, int nfps, double dThreshold)
{
	if (matDisplay.empty())
		return;
	//cv::Mat matDisplay;
	cv::Mat matResized;
	//m_matFrameColor.copyTo(matDisplay);

	// Display conditions
	insertText(matDisplay, nfps, 0, dThreshold);

	// Insert mini thresholded video
	int nThreshImgSize = 50;
	cv::resize(matFg, matResized, cv::Size(nThreshImgSize, nThreshImgSize), 0, 0, 0);
	cv::cvtColor(matResized, matResized, CV_GRAY2BGR);
	matResized.copyTo(matDisplay(cv::Rect(5, matDisplay.rows - nThreshImgSize - 5, nThreshImgSize, nThreshImgSize)));

	matDisplay.copyTo(m_matSaveImage);

}

void CBirdCounter::resetBirdCount()
{
	m_nCount_In = 0;
	m_nCount_Out = 0;
}


cv::Rect CBirdCounter::boundingBirds(std::vector<cv::Point> birdPts)
{
	cv::Point minPt(INT_MAX, INT_MAX);
	cv::Point maxPt(0, 0);
	for (int n = 0; n < birdPts.size(); n++)
	{
		minPt.x = birdPts[n].x < minPt.x ? birdPts[n].x : minPt.x;
		minPt.y = birdPts[n].y < minPt.y ? birdPts[n].y : minPt.y;

		maxPt.x = birdPts[n].x >= maxPt.x ? birdPts[n].x : maxPt.x;
		maxPt.y = birdPts[n].y >= maxPt.y ? birdPts[n].y : maxPt.y;
	}

	return cv::Rect(minPt, maxPt);

}


cv::Rect CBirdCounter::EnlargeSafeROI(cv::Rect oriRect, float fScale, int nBirdDirection)
{
	cv::Point centerPt = (oriRect.tl() + oriRect.br()) / 2;
	
	int nMinLength = 25;
	cv::Point radiusX(std::max(oriRect.width,nMinLength), 0);
	cv::Point radiusY(0, std::max(oriRect.height, nMinLength));
	radiusX *= fScale * 0.5f;
	radiusY *= fScale * 0.5f * 0.5f;

	cv::Point tl = centerPt + radiusY;
	cv::Point br = centerPt - radiusY;

	if (nBirdDirection == BIRD_FLY_OUT){
		tl += radiusX;
	}
	else if(nBirdDirection == BIRD_FLY_IN){
		br -= radiusX;
	}
	else
	{
		tl += radiusX;
		br -= radiusX;
	}

	return cv::Rect(tl, br);
}

int CBirdCounter::findBestBirdIndex(BirdData bird_src, std::vector<BirdData> bird_candidates, float fIntersectRatio, bool bLimit)
{
	float fMax = 0;
	int nBestIdx = -1;


	for (int i = 0; i < bird_candidates.size(); i++)
	{
		
		if (bLimit && bird_candidates[i].isMatched())
			continue;

		int nLocDiff = bird_candidates[i].location.x - bird_src.location.x;

		// IF direction is not same, skip
		if (bird_src.nBirdDirection == BIRD_FLY_IN && nLocDiff >= 0)
			continue;
		if (bird_src.nBirdDirection == BIRD_FLY_OUT && nLocDiff <= 0)
			continue;
		
		
		cv::Rect b1 = EnlargeSafeROI(bird_src.boundingBox, fIntersectRatio, bird_src.nBirdDirection);
		cv::Rect b2 = EnlargeSafeROI(bird_candidates[i].boundingBox, fIntersectRatio, bird_src.nBirdDirection);
		float fArea = (b1 & b2).area();

		if (fArea > fMax)
		{
			fMax = fArea;
			nBestIdx = i;
		}
	}

	return nBestIdx;
}

bool CBirdCounter::countBird(cv::Mat matForeBird, cv::Mat matRealSrc, cv::Mat &matDisplay,float fScale, bool bDisplay)
{
	int nBoxSizeLimit = 5000;
	float fBoxSizeRatio = 10;
	int nMinBirdSize = 4;
	float fNonZeroRatioThresh = 0.25;
	float fIntersectRatio = 2;
	
	int nSrcWidth = matRealSrc.cols;
	int nSrcHeight = matRealSrc.rows;
	bool bExistBird = false;

	//cv::Mat matForeBig;
	//cv::resize(matForeBird, matForeBig, cv::Size(0, 0), fScale, fScale, cv::INTER_NEAREST);

	std::vector<std::vector<cv::Point>> bird_blocks;
	cv::Mat contourFore ;
	matForeBird.copyTo(contourFore);
	cv::findContours(contourFore, bird_blocks, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	//cv::Mat matDisplay;
	matRealSrc.copyTo(matDisplay);

	// Set bounding block for detected birds
	
	std::vector<BirdData> bird_candidates;

	// Return if too many spurious blobs
	if (bird_blocks.size() > 30)
		return false;

	for (int n = 0; n < bird_blocks.size(); n++)
	{
		//cv::Rect birdBox = birdRects[n];
		//cv::Rect birdBox = boundingBirds(bird_blocks[n]);
		cv::Rect birdBox = cv::boundingRect(bird_blocks[n]);

		float fNonZeroRatio = cv::countNonZero(matForeBird(birdBox)) / (float)birdBox.area();
		if (fNonZeroRatio < fNonZeroRatioThresh)
			continue;
		
		if (birdBox.width <= nMinBirdSize/2 &&  birdBox.height <= nMinBirdSize/2)
			continue;

		// Remove abnormal looking shapes
		float fSizeRatio = (birdBox.width > birdBox.height) ? birdBox.width / (float)birdBox.height : birdBox.height / (float)birdBox.width;
		if (fSizeRatio > fBoxSizeRatio)
			continue;

		if (birdBox.area() > nBoxSizeLimit/2)
			continue;

		//bird_candidates.push_back(BirdData(birdBox));
		if (birdBox.width >= 22/2 || birdBox.height >= 30/2)
		{
			int nMaxBox = birdBox.width > birdBox.height ? birdBox.width : birdBox.height;
			cv::Mat matROI = matForeBird(birdBox);
			cv::Mat matCross = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
			cv::morphologyEx(matROI, matROI, cv::MORPH_OPEN, cv::Mat(), cv::Point(-1,-1), nMaxBox / 10);
			

			std::vector<std::vector<cv::Point>> subBlocks;
			cv::findContours(matROI, subBlocks, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
			for (int i = 0; i < subBlocks.size(); i++) {
				cv::Rect temp = cv::boundingRect(subBlocks[i]);
				temp = temp + birdBox.tl();

				temp.x *= fScale;
				temp.y *= fScale;
				temp.width *= fScale;
				temp.height *= fScale;

				bird_candidates.push_back(BirdData(temp));
			
			}
		}
		else
		{
			birdBox.x *= fScale;
			birdBox.y *= fScale;
			birdBox.width *= fScale;
			birdBox.height *= fScale;
			bird_candidates.push_back(BirdData(birdBox));
		}
	}

	// Return if too many spurious birds
	if (bird_candidates.size() > 10)
		return false;
	//std::cout << "bird_candidates : " << bird_candidates.size() << std::endl;

	//Check for registered birds First
	std::vector<BirdData> new_birds;
	for (int i = 0; i < m_birds.size(); i++)
	{
		// Skip all the unknown birds
		if (m_birds[i].nBirdDirection == BIRD_UNKNOWN)
			continue;

		if (!m_birds[i].isMatched())
		{
			int nidx = findBestBirdIndex(m_birds[i], bird_candidates, fIntersectRatio);

			// If best bird found
			if (nidx >= 0)
			{
				m_birds[i].update(bird_candidates[nidx].boundingBox);
				bird_candidates[nidx].update(cv::Rect());
				// Only saves detect birds
				if (m_birds[i].bMatched)
					new_birds.push_back(m_birds[i]);
			}
			else
			{	
				if (m_birds[i].predict()) {

					// Birds within FOV
					if (m_birds[i].location.x >= 0
						&& m_birds[i].location.x < nSrcWidth) {
					
						new_birds.push_back(m_birds[i]);
					}
				}
			}
		}
	}

	// FInd the remaining unknown birds
	for (int i = 0; i < m_birds.size(); i++)
	{
		if (!m_birds[i].isMatched())
		{
			int nidx = findBestBirdIndex(m_birds[i], bird_candidates, fIntersectRatio, true);

			// If best bird found
			if (nidx >= 0)
			{
				m_birds[i].update(bird_candidates[nidx].boundingBox);
				bird_candidates[nidx].update(cv::Rect());
				new_birds.push_back(m_birds[i]);
			}
		}
	}

	// Check for new birds
	for (int i = 0; i < bird_candidates.size(); i++)
	{
		if (!bird_candidates[i].isMatched())
		{
			BirdData newbird(bird_candidates[i].boundingBox);
			new_birds.push_back(newbird);
		}
	}

	// Count Birds
	int nIn_Line = (int)(0.35 * nSrcWidth);
	int nOut_Line = (int)(0.6 * nSrcWidth);
	int nCnt_line = (nIn_Line + nOut_Line) / 2;

	for(int i = 0;  i < new_birds.size(); i++)
	{
		// Check for uncounted birds
		if (!new_birds[i].isCounted())
		{
			// If birds direction are known
			if (new_birds[i].nBirdDirection != BIRD_UNKNOWN)
			{
				// Count Flying Out
				if (new_birds[i].nBirdDirection == BIRD_FLY_OUT)
				{
					if (new_birds[i].location.x > nOut_Line)
					{
						if (new_birds[i].vec_trail[0].x < nCnt_line)
						{
							new_birds[i].setCounted();
							m_nCount_Out++;
							bExistBird = true;
							insertBirdLog(new_birds[i]);
						}
					}
					
				}
				// Count Flying In
				else if(new_birds[i].nBirdDirection == BIRD_FLY_IN)
				{
					if (new_birds[i].location.x < nIn_Line)
					{
						if (new_birds[i].vec_trail[0].x >= nCnt_line)
						{
							new_birds[i].setCounted();
							m_nCount_In++;
							bExistBird = true;
							insertBirdLog(new_birds[i]);
						}
						
					}
				}

			}
		}
	}
#ifdef PERSONAL_COMPUTER
	//std::cout << " Bird In : " << m_nCount_In << "\t" << " Bird Out : " << m_nCount_Out << std::endl;
#endif
	if (bDisplay)
	{
		// Display new birds
		int nWaitTime = 1;
		cv::Scalar color_in = cv::Scalar(255, 100, 0);
		cv::Scalar color_out = cv::Scalar(0, 0, 255);
		cv::Scalar color_unknown = cv::Scalar(50, 50, 50);
		cv::line(matDisplay, cv::Point(nIn_Line, 0), cv::Point(nIn_Line, nSrcHeight), color_in, 1);
		cv::line(matDisplay, cv::Point(nOut_Line, 0), cv::Point(nOut_Line, nSrcHeight), color_out, 1);
		for (int i = 0; i < new_birds.size(); i++)
		{
			cv::Scalar birdColor;
			if (new_birds[i].nBirdDirection == BIRD_FLY_IN )
			{
				birdColor = color_in;
				nWaitTime = 0;
			}
			else if (new_birds[i].nBirdDirection == BIRD_FLY_OUT )
			{
				birdColor = color_out;
				nWaitTime = 0;
			}
			else {
				birdColor = color_unknown;
				nWaitTime = 1;
			}

			// Draw if detected birds has trail
			if (new_birds[i].vec_trail.size() >= 0)
			{
				cv::rectangle(matDisplay, new_birds[i].boundingBox, birdColor, 1);
				for (int dot = 0; dot < new_birds[i].vec_trail.size(); dot++)
				{
					cv::circle(matDisplay, new_birds[i].vec_trail[dot], 3, birdColor, 1);
					if (dot > 0)
						cv::line(matDisplay, new_birds[i].vec_trail[dot - 1], new_birds[i].vec_trail[dot], birdColor);
				}

				//cv::Rect safeRect = new_birds[i].getSafeRect(matForeBird.cols, matForeBird.rows);
				//float fNonZeroRatio = cv::countNonZero(matForeBird(safeRect)) / (float)new_birds[i].boundingBox.area();
				std::string str_speed = std::to_string(new_birds[i].fAverageSpeed) + "%";
				//std::string str_speed = std::to_string(new_birds[i].boundingBox.width)
					//+ " " + std::to_string(new_birds[i].boundingBox.height)
					////+ " km/h"
					//;
				//std::string str_speed = std::to_string(new_birds[i].fAverageSpeed) + " km/h";
				
				putText(matDisplay, str_speed, new_birds[i].boundingBox.tl() - cv::Point(0,5), cv::FONT_HERSHEY_SIMPLEX, 0.3, birdColor);


			}

		}
	}

	// Clear matches
	for (int i = 0; i < new_birds.size(); i++)
	{
		new_birds[i].bMatched = false;
	}

	m_birds = new_birds;


	return bExistBird;
}

void CBirdCounter::process_thread(cv::Mat matFrame)
{
	PiCommon picom;
	double dTime = 0;
	//for (;;)
	//{
		dTime = (double)cv::getTickCount();
		double dForeRatio = 0.0;
		int nFrameWidth = matFrame.cols;//m_nFrameWidth;
		int nFrameHeight = matFrame.rows;//m_nFrameHeight;

		// Exceptions
		if (matFrame.empty()){
			picom.printStdLog( "matFrameGray Empty");	
			return;
		}
			
		cv::Mat matLocalGray;
		cv::Mat matLocalColor;
		cv::Mat matLocalFore;
		cv::cvtColor(matFrame, matLocalColor, cv::COLOR_GRAY2BGR);
		matLocalGray = matFrame;
		//matLocalColor = matFrameColor;

		// Motion detections
		// Reduce size for faster calculation
		float fScale = 0.5;
		cv::Mat smallLocalGray;
		cv::Mat finalGray;
		cv::resize(matLocalGray, smallLocalGray, cv::Size(0, 0), fScale, fScale,cv::INTER_NEAREST);
		cv::GaussianBlur(smallLocalGray, smallLocalGray, cv::Size(7, 7), 9);

		// Remove Noise Area
		cv::Rect noiseRect = cv::Rect(smallLocalGray.cols - (smallLocalGray.cols * 0.28)
			, 0
			, (smallLocalGray.cols * 0.28)
			, smallLocalGray.rows);
		cv::Rect signalRect = cv::Rect( 0, 0
			, (smallLocalGray.cols * 0.7)
			, smallLocalGray.rows);
		cv::Rect hotRect = cv::Rect(smallLocalGray.cols * 0.3
			, smallLocalGray.rows * 0.1
			, (smallLocalGray.cols * 0.3)
			, smallLocalGray.rows * 0.8);
		

		smallLocalGray(noiseRect).setTo(0);


		cv::Mat matEqualized;
		finalGray = cv::Mat::zeros(smallLocalGray.size(), CV_8UC1);
		cv::equalizeHist(smallLocalGray(signalRect), matEqualized);
		matEqualized.copyTo(finalGray(signalRect));
		
		cv::Rect cntROI(0.4 * finalGray.cols, 0, 0.2 * finalGray.cols, finalGray.rows);

		

		double dBG_mean = cv::mean(smallLocalGray(hotRect))[0];
		static int nAvgIntensity_count = 20;

		
		m_avgIntensity = m_avgIntensity*(0.7) + dBG_mean*(0.3);
		if(nAvgIntensity_count > 0)
		{
			nAvgIntensity_count--;
			return;
		}

#ifndef PERSONAL_COMPUTER
		// control brightness of input
		static int nBrightness_offset = 50;
		int nMaxBrightness = 70;
		int nMinBrightness = 35;
		int nIncrement = 2;
		if (m_avgIntensity > 135 && nBrightness_offset > nMinBrightness) {
			nBrightness_offset = nBrightness_offset - nIncrement;
			nBrightness_offset = nBrightness_offset < nMinBrightness ? nMinBrightness : nBrightness_offset;
			picom.getString_fromCmd("v4l2-ctl -c brightness=" + std::to_string(nBrightness_offset));
			
			picom.printStdLog("[mean] " + std::to_string(m_avgIntensity) + " : Reduce brightness to " + std::to_string(nBrightness_offset));
			m_bResetFrames = true;
			picom.uniSleep(500);
			nAvgIntensity_count = 20;
			
			return;
		}
		else if (m_avgIntensity < 90 && nBrightness_offset < nMaxBrightness) {
			nBrightness_offset = nBrightness_offset + nIncrement;
			nBrightness_offset = nBrightness_offset > nMaxBrightness ? nMaxBrightness : nBrightness_offset;
			picom.getString_fromCmd("v4l2-ctl -c brightness=" + std::to_string(nBrightness_offset));
			picom.printStdLog("[mean] " + std::to_string(m_avgIntensity) + " : Increase brightness to " + std::to_string(nBrightness_offset));
			m_bResetFrames = true;
			picom.uniSleep(500);
			nAvgIntensity_count = 20;
			
			return;
		}
		
#endif


		// Learn bacjground and extract foreground
		if (m_nToggleLearn >= 0 ) {
			m_pMOG->apply(finalGray, matLocalFore,-1);
			m_nToggleLearn = 0;
		}
		m_nToggleLearn++;

		// Set all to zero if average intensity is low
		if (m_avgIntensity < 80) {
			matLocalFore = cv::Mat::zeros(finalGray.size(), CV_8UC1);
		}

		// Get the background image
		cv::Mat matBG;
		m_pMOG->getBackgroundImage(matBG);
		if (matBG.empty())
		{
			picom.printStdLog( "matBG Empty",1);	
			return;
		}

		// Morphology process
		cv::erode(matLocalFore, matLocalFore, cv::Mat());
		//cv::Mat matElement = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));
		//cv::morphologyEx(matLocalFore, matLocalFore, cv::MORPH_OPEN, matElement, cv::Point(-1, -1), 2);
		//cv::morphologyEx(matLocalFore, matLocalFore, cv::MORPH_CLOSE, matElement, cv::Point(-1, -1), 2);

		// Calculate ratio
		// Detect sudden change of video quality
		dForeRatio = (double)cv::countNonZero(matLocalFore(cntROI)) / (double)(cntROI.area());
		

		cv::Mat matDisplayWithBirds;
		// Skip when too large changes occur
		//std::cout << "dForeRatio : " << dForeRatio << std::endl;
		if (dForeRatio > 0.2) {
			m_nSaturationCount++;
			m_nCountContinuosValid = 0;
			m_birds.clear();
		}
		else {

			// Main process
			bool bExistBird = countBird(matLocalFore, matLocalColor, matDisplayWithBirds,1/fScale,true);
			if (bExistBird) {
				m_nCountContinuosValid++;
				 printBirdLog();
				//picom.printStdLog( "BIRD EXIST");
			}
		}

		// If dForeRatio is higher add count continous ratio
		// Only inform when some thing middle sized moved.
		// Avoid room light change

		auto tempTime = std::chrono::system_clock::now();
		
		std::chrono::duration<double> elapsed_seconds = tempTime - m_statusTime;
		m_fSecCount_tweet += elapsed_seconds.count();
		m_fSecCount_status += elapsed_seconds.count();
		m_statusTime = tempTime;


		// -------------------------------- Process for first frame 
		if (m_bOnce)
		{
			init_birdLog();
			picom.printStdLog( "m_bOnce");
			m_nCountContinuosValid = 0;
			//m_bTweeterFlag = true;
			m_dTemperature = picom.get_temperature();

			//prepareSaveImage(matLocalFore, matDisplayWithBirds, m_nFps_real, dForeRatio);
			m_piTweet.tweet_bird_thread(matLocalGray
				, -1
				, m_dTime * 1000
				, m_dTemperature
				, m_nCount_In
				, m_nCount_Out
				, m_avgIntensity
			);
			m_bOnce = false;
			m_fSecCount_tweet = 0;
			m_fSecCount_status = 0;
			//m_fSecCount = 10000;
		}
		

		// -------------------------------- Tweet Image
		static int nPre_inBird = 0;
		static int nPre_outBird = 0;
		static bool bLargeNumber_flag = false;
		int nMinutesToTweet = 15;
		int nPrintBirdNumFlag = 5000; // For Uploading pictures every 5000 birds 
		
		// Flag for detectiong large bird number
		if ((m_nCount_In % nPrintBirdNumFlag == 0 && m_nCount_In != nPre_inBird)
			|| (m_nCount_Out % nPrintBirdNumFlag == 0 && m_nCount_Out != nPre_outBird))	
		{
			bLargeNumber_flag = true;
			picom.printStdLog("Tweets bird count flag reached.");
			nPre_inBird = m_nCount_In;
			nPre_outBird = m_nCount_Out;
			
		}

		if (m_nCountContinuosValid >= 1)
		{		
			// At Least 15 mins inteval
			if(m_fSecCount_tweet > 60 * 15 && bLargeNumber_flag)
			//if (m_fSecCount_tweet > 1)
			{ 
				//std::cout << "m_avgIntensity : " << m_avgIntensity << std::endl;
				//picom.printStdLog("Tweets picture condition matched.");
				prepareSaveImage(matLocalFore, matDisplayWithBirds, m_nFps_real, dForeRatio);
				
				m_dTemperature = picom.get_temperature();
				m_piTweet.tweet_bird_thread(matDisplayWithBirds
					, dForeRatio
					, m_dTime * 1000
					, m_dTemperature
					, m_nCount_In
					, m_nCount_Out
					, m_avgIntensity);
				
				m_bMotionDetected = true;
		
				m_fSecCount_tweet = 0;
				bLargeNumber_flag = false;
				nPre_inBird = m_nCount_In;
				nPre_outBird = m_nCount_Out;
				
			}
			m_nCountContinuosValid = 0;

		}

		
		
		
		//--------------------------- Upload graph
		// Flag to avoid multiple calls
		static bool bActivate = false;
		if (picom.get_current_time() == "00:00:01" && bActivate == false)
		{
			picom.printStdLog("New Day!");
			bActivate = true;

			init_birdLog();
			picom.printStdLog("Tweeting Graph!\n");
			m_piTweet.tweet_graph_thread(picom.get_yesterday_date());
			resetBirdCount();

		}
		if (picom.get_current_time() == "00:00:02")
			bActivate = false;

		
		//-------------------------- Update Pi Status
		if (m_fSecCount_status > 60 * 5)
		{
			m_fSecCount_status = 0;
			picom.printStdLog( " Start  printStatus_thread",1);
			printStatus_thread();
			picom.printStdLog( " printStatus_thread DONE",1);
		}


#ifdef PERSONAL_COMPUTER
		prepareSaveImage(matLocalFore, matDisplayWithBirds, m_nFps_real, dForeRatio);
#endif
		m_matSaveImage = matDisplayWithBirds;

		m_dTime = ((double)cv::getTickCount() - dTime) / cv::getTickFrequency();

}

void CBirdCounter::insertBirdLog(BirdData bird_data) {
	BirdCell tempCell;
	tempCell.detect_time = std::chrono::system_clock::now();
	tempCell.nSpeed = (int)bird_data.fAverageSpeed;
	tempCell.nTrailLength = (int)bird_data.vec_trail.size();

	if (bird_data.nBirdDirection == BIRD_FLY_OUT)
		m_outBird_logs.push_back(tempCell);
	else if (bird_data.nBirdDirection == BIRD_FLY_IN)
		m_inBird_logs.push_back(tempCell);
}


void CBirdCounter::init_birdLog()
{
	PiCommon picom;
	std::string filename = std::string(PROGRAM_FOLDER) + "bird_log/" + picom.get_current_date();

	// Close file if is open
	if (m_in_file.is_open())
		m_in_file.close();
	if (m_out_file.is_open())
		m_out_file.close();
	//if (m_status_file.is_open())
	//	m_status_file.close();


	// Create new files
	m_out_file.open(filename + "_out.txt", std::fstream::out | std::fstream::app);
	m_in_file.open(filename + "_in.txt", std::fstream::out | std::fstream::app);
	//m_status_file.open(filename + "_status.txt", std::fstream::out | std::fstream::app);
}

void CBirdCounter::printBirdLog()
{
	if (!m_in_file.is_open() || !m_out_file.is_open())
		return;
	
	// in-data print
	for (int i = 0; i < m_inBird_logs.size(); i++)
	{		
		std::time_t tt = std::chrono::system_clock::to_time_t(m_inBird_logs[i].detect_time);
		m_in_file << std::strtok(std::ctime(&tt), "\n") << "\t"
			<< m_inBird_logs[i].nSpeed << "\t"
			<< m_inBird_logs[i].nTrailLength << "\n";
	}
	m_in_file.flush();

	// out-data print
	for (int i = 0; i < m_outBird_logs.size(); i++)
	{
		std::time_t tt = std::chrono::system_clock::to_time_t(m_outBird_logs[i].detect_time);
		m_out_file << std::strtok(std::ctime(&tt), "\n") << "\t"
			<< m_outBird_logs[i].nSpeed << "\t"
			<< m_outBird_logs[i].nTrailLength << "\n";
	}
	m_out_file.flush();

	m_inBird_logs.clear();
	m_outBird_logs.clear();

}

void _printStatus_thread_func( int nTime, int nSaturation, int nOverflow, int nCountIn, int nCountOut
	, double dAvgIntensity, int nFPS)
{
	PiCommon picom;
	picom.printStdLog("_printStatus_thread_func start",1);
	

	std::string filename = std::string(PROGRAM_FOLDER) + "bird_log/" + picom.get_current_date();
	std::ofstream status_file;
	status_file.open(filename + "_status.txt", std::fstream::out | std::fstream::app);

	if(status_file.is_open())
	{
	// in-data print
		auto timenow = std::chrono::system_clock::now();
		auto tt = std::chrono::system_clock::to_time_t(timenow);
		auto temperature_val = picom.getString_fromCmd("/opt/vc/bin/vcgencmd measure_temp");
		auto core_val = picom.getString_fromCmd("/opt/vc/bin/vcgencmd measure_clock arm");

		status_file << std::strtok(std::ctime(&tt), "\n") << "\t"
				<< "Sat=" << nSaturation << "\t"
				<< "Over=" << nOverflow << "\t"
				<< "Time=" << std::to_string(nTime) << "\t"
				<< "Bird_in=" << nCountIn << "\t"
				<< "Bird_out=" << nCountOut << "\t"
				<< temperature_val<< "\t"
				<< core_val<< "\t"
				<< "Avg_iten=" << dAvgIntensity << "\t"
				<< "FPS=" << nFPS << "\t"
				<< "\n"
				;
		picom.printStdLog( "Sat=" + std::to_string(nSaturation) + "\t"
			+ "Over="  		+ std::to_string( nOverflow) + "\t"
			+ "Time=" 		+ std::to_string( nTime)  +  "\t"
			+ "Bird_in=" 	+ std::to_string( nCountIn)  +  "\t"
			+ "Bird_out="	+ std::to_string( nCountOut)  +  "\t"
			+ temperature_val +  "\t"
			+ core_val +  "\t"
			+ "Avg_iten=" + std::to_string(dAvgIntensity) + "\t"
			+ "FPS=" + std::to_string(nFPS) + "\t"
		);
		status_file.close();
		
	}
	
} 

void CBirdCounter::printStatus_thread()
{
	PiCommon picom;
	picom.printStdLog("printStatus_func ->>.",1);


	std::thread t(_printStatus_thread_func
		//, m_status_file
		, m_dTime * 1000 // in miliseconds
		, m_nSaturationCount
		, m_nOverflowCount
		, m_nCount_In
		, m_nCount_Out
		, m_avgIntensity
		, m_nFps_real
	);
	t.detach();

}
