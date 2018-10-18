#include "BirdCounter.h"

#define PROGRAM_FOLDER "/home/pi/projects/FlightOfSwallows/"

CBirdCounter::CBirdCounter()
{
	//m_pMOG = cv::createBackgroundSubtractorMOG2(50, 25, false);
	m_pMOG = cv::createBackgroundSubtractorKNN(50, 50, false);
	//m_pMOG->SetVarThreshold(12);
	m_ratios.push_back(0);
}


CBirdCounter::~CBirdCounter()
{
}

void CBirdCounter::insertText(cv::Mat & matDisplay, int nFPS, double nMean, double nThreshold)
{
	float fFontSize = 0.4;
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
		int fArea = (b1 & b2).area();

		if (fArea > fMax)
		{
			fMax = fArea;
			nBestIdx = i;
		}
	}

	return nBestIdx;
}

bool CBirdCounter::countBird(cv::Mat matForeBird, cv::Mat matRealSrc, cv::Mat &matDisplay, bool bDisplay)
{
	int nSrcWidth = matRealSrc.cols;
	int nSrcHeight = matRealSrc.rows;
	bool bExistBird = false;

	std::vector<std::vector<cv::Point>> bird_blocks;
	cv::Mat contourFore ;
	matForeBird.copyTo(contourFore);
	cv::findContours(contourFore, bird_blocks, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

	//cv::Mat matDisplay;
	matRealSrc.copyTo(matDisplay);

	// Set bounding block for detected birds
	int nMinBirdSize = 2;
	std::vector<BirdData> bird_candidates;
	//for (int n = 0; n < birdRects.size(); n++)
	for (int n = 0; n < bird_blocks.size(); n++)
	{
		//cv::Rect birdBox = birdRects[n];
		cv::Rect birdBox = boundingBirds(bird_blocks[n]);

		float fNonZeroRatio = cv::countNonZero(matForeBird(birdBox)) / (float)birdBox.area();
		if (fNonZeroRatio < 0.25)
			continue;


		birdBox.x *= 2;
		birdBox.y *= 2;
		birdBox.width *= 2;
		birdBox.height *= 2;
		if (birdBox.width <= nMinBirdSize &&  birdBox.height <= nMinBirdSize)
			continue;

		// Remove abnormal looking shapes
		float fSizeRatio = (birdBox.width > birdBox.height) ? birdBox.width / (float)birdBox.height : birdBox.height / (float)birdBox.width;
		if (fSizeRatio > 10)
			continue;


		if (birdBox.area() > 15000)
			continue;


		bird_candidates.push_back(BirdData(birdBox));
	}

	//Check for registered birds
	std::vector<BirdData> new_birds;
	float fIntersectRatio = 2;
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
				
				if (m_birds[i].predict())
					new_birds.push_back(m_birds[i]);
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

				// Only saves detect birds
				//if (m_birds[i].vec_trail.size() < 30)
				if(m_birds[i].bMatched && m_birds[i].vec_trail.size() < 5)
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
	int nIn_Line = 0.35 * nSrcWidth;
	int nOut_Line = 0.6 * nSrcWidth;
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
	std::cout << " Bird In : " << m_nCount_In << "\t" << " Bird Out : " << m_nCount_Out << std::endl;
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
				//std::string str_speed = std::to_string(fNonZeroRatio) + "%";
				std::string str_speed = std::to_string(new_birds[i].fAverageSpeed) + " km/h";
				
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

void CBirdCounter::process_thread(cv::Mat matFrameGray, cv::Mat matFrameColor)
{
	PiCommon picom;
	double dTime = 0;
	//for (;;)
	//{
		dTime = cv::getTickCount();
		double dForeRatio = 0.0;
		int nFrameWidth = matFrameGray.cols;//m_nFrameWidth;
		int nFrameHeight = matFrameGray.rows;//m_nFrameHeight;

		// Exceptions
		if (matFrameGray.empty()){
			picom.printStdLog( "matFrameGray Empty");	
			return;
		}
			
		cv::Mat matLocalGray;
		cv::Mat matLocalColor;
		cv::Mat matLocalFore;
		matLocalGray = matFrameGray;
		matLocalColor = matFrameColor;
		// Motion detections
		// Reduce size for faster calculation
		cv::Mat smallLocalGray;
		cv::Mat finalGray;
		cv::resize(matLocalGray, smallLocalGray, cv::Size(0, 0), 0.5, 0.5);
		cv::GaussianBlur(smallLocalGray, smallLocalGray, cv::Size(5, 5), 5);

		// Remove Noise Area
		cv::Rect noiseRect = cv::Rect(smallLocalGray.cols - (smallLocalGray.cols * 0.28)
			, 0
			, (smallLocalGray.cols * 0.28)
			, smallLocalGray.rows);
		cv::Rect signalRect = cv::Rect( 0, 0
			, (smallLocalGray.cols * 0.7)
			, smallLocalGray.rows);
		smallLocalGray(noiseRect).setTo(0);

		cv::Mat matEqualized;
		finalGray = cv::Mat::zeros(smallLocalGray.size(), CV_8UC1);
		cv::equalizeHist(smallLocalGray(signalRect), matEqualized);
		matEqualized.copyTo(finalGray(signalRect));
		
		//cv::imshow("finalGray", finalGray);
		
		cv::Rect cntROI(0.4 * finalGray.cols, 0, 0.2 * finalGray.cols, finalGray.rows);

		double dBG_mean = cv::mean(smallLocalGray(signalRect))[0];
		//std::cout << cv::mean(smallLocalGray)[0] << "  " << cv::mean(finalGray)[0] << std::endl;
	


		// Learn bacjground and extract foreground
		if (m_nToggleLearn >= 0 ) {
			m_pMOG->apply(finalGray, matLocalFore,-1);
			m_nToggleLearn = 0;
			//picom.printStdLog( "Learning BG",1);
			//cv::imshow("matLocalFore_ori", matLocalFore);
		}
		m_nToggleLearn++;

		// Set all to zero if average intensity is low
		if (dBG_mean < 50) {
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



		

		cv::Mat matElement = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));
		cv::morphologyEx(matLocalFore, matLocalFore, cv::MORPH_OPEN, matElement, cv::Point(-1, -1), 2);
		cv::morphologyEx(matLocalFore, matLocalFore, cv::MORPH_CLOSE, matElement, cv::Point(-1, -1), 2);

#ifdef PERSONAL_COMPUTER
		if (!matLocalFore.empty())
		{
			cv::imshow("matLocalFore", matLocalFore);
		
		}
#endif

		//picom.printStdLog( "Calculate ratio",1);
		// Calculate ratio
		// Detect sudden change of video quality
		dForeRatio = (double)cv::countNonZero(matLocalFore(cntROI)) / (double)(cntROI.area());
		m_ratios[0] = dForeRatio;

		cv::Mat matDisplayWithBirds;
		// Skip when too large changes occur
		if (dForeRatio > 0.5) {
			m_nSaturationCount++;
			m_nCountContinuosValid = 0;

			// picom.printStdLog( "Large Change");		
			// Clear all bird datas
			m_birds.clear();
		}
		else {
			//picom.printStdLog( "countBird",1);finalGray
			//cv:cvtColor(finalGray, matLocalColor, cv::COLOR_GRAY2BGR);
			//cv::resize(matLocalColor, matLocalColor, cv::Size(), 2, 2);
			bool bExistBird = countBird(matLocalFore, matLocalColor, matDisplayWithBirds,true);
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

		if (m_bOnce)
		{
			init_birdLog();
			picom.printStdLog( "m_bOnce",1);
			m_nCountContinuosValid = 0;
			//m_bTweeterFlag = true;
			m_dTemperature = picom.get_temperature();
			m_piTweet.tweet_bird_thread(matDisplayWithBirds
				, -1
				, m_dTime 
				, m_dTemperature
				, m_nCount_In
				, m_nCount_Out
				, m_ratios
			);
			m_bOnce = false;
			m_fSecCount_tweet = 0;
			m_fSecCount_status = 0;
			//m_fSecCount = 10000;
		}
		
#ifdef PERSONAL_COMPUTER
		if (!matDisplayWithBirds.empty())
		{
			cv::imshow("matDisplayWithBirds",matDisplayWithBirds);
			cv::waitKey(1);
		}
#endif

	

		// Record video if threshold shows continuos 10 frames
		//m_nCountContinuosValid = 1;
		if(m_bDisplay){
			matDisplayWithBirds.copyTo(m_matSaveImage);
		}

		static int nPre_inBird = 0;
		static int nPre_outBird = 0;
		int nMinutesToTweet = 15;
		if (m_nCountContinuosValid >= 1)
		{
			// For Uploading pictures every 1000 birds 
			int nPrintBirdNumFlag = 2500;
		
			// At Least 15 mins inteval
			if(m_fSecCount_tweet > 60 * 15)
			if ((m_nCount_In % nPrintBirdNumFlag == 0 && m_nCount_In != nPre_inBird )
				|| (m_nCount_Out % nPrintBirdNumFlag == 0 && m_nCount_Out != nPre_outBird) ) // Write
			{ 
				picom.printStdLog("Tweets bird count flag reached.");
				prepareSaveImage(matLocalFore, matDisplayWithBirds, m_nFps_real, dForeRatio);
				
				m_dTemperature = picom.get_temperature();
				m_piTweet.tweet_bird_thread(matDisplayWithBirds
					, dForeRatio
					, m_dTime
					, m_dTemperature
					, m_nCount_In
					, m_nCount_Out
					, m_ratios);
				
				m_bMotionDetected = true;
		
				m_fSecCount_tweet = 0;

				nPre_inBird = m_nCount_In;
				nPre_outBird = m_nCount_Out;
			}
			m_nCountContinuosValid = 0;

		}


#ifndef PERSONAL_COMPUTER
		// usleep(100);
#endif

		dTime = cv::getTickCount() - dTime;
		dTime = (dTime / cv::getTickFrequency() * 1000);
		m_dTime = (m_dTime + dTime) / 2;
		m_nFps_real = 1 / (m_dTime / 1000);
		//std::cout << dTime << "\t" << m_dTime << "\n";
		
		// Upload graph

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

		//static bool bone_time = false;
		//if (!bone_time) {
		//	m_piTweet.tweet_graph_thread("2018-10-06");
		//	bone_time = true;
		//}
		//picom.printStdLog( " Status update",1);
		// Status update
		
		
		//picom.printStdLog( " TIME : " + std::to_string(m_fSecCount),1);
		if (m_fSecCount_status > 60 * 5)
		{
			m_fSecCount_status = 0;
			picom.printStdLog( " Start  printStatus_thread",1);
			printStatus_thread();
			picom.printStdLog( " printStatus_thread DONE",1);
		}

	//std::cout << std::endl;

	//}
}

void CBirdCounter::insertBirdLog(BirdData bird_data) {
	BirdCell tempCell;
	tempCell.detect_time = std::chrono::system_clock::now();
	tempCell.nSpeed = (int)bird_data.fAverageSpeed;
	tempCell.nTrailLength = bird_data.vec_trail.size();

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

	// out-data print
	for (int i = 0; i < m_outBird_logs.size(); i++)
	{
		std::time_t tt = std::chrono::system_clock::to_time_t(m_outBird_logs[i].detect_time);

		m_out_file << std::strtok(std::ctime(&tt), "\n") << "\t"
			<< m_outBird_logs[i].nSpeed << "\t"
			<< m_outBird_logs[i].nTrailLength << "\n";
	}

	m_inBird_logs.clear();
	m_outBird_logs.clear();

}

void _printStatus_thread_func( int nTime, int nSaturation, int nOverflow, int nCountIn, int nCountOut)
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
				<< "\n"
				;
		picom.printStdLog( "Sat=" + std::to_string(nSaturation) + "\t"
			+ "Over="  		+ std::to_string( nOverflow) + "\t"
			+ "Time=" 		+ std::to_string( nTime)  +  "\t"
			+ "Bird_in=" 	+ std::to_string( nCountIn)  +  "\t"
			+ "Bird_out="	+ std::to_string( nCountOut)  +  "\t"
			+ temperature_val +  "\t"
			+ core_val +  "\t"
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
		, m_dTime
		, m_nSaturationCount
		, m_nOverflowCount
		, m_nCount_In
		, m_nCount_Out
	);
	t.detach();

}
