#include "BirdCounter.h"



CBirdCounter::CBirdCounter()
{
	m_pMOG = cv::createBackgroundSubtractorMOG2(100,2 * 2, false);
	m_ratios.push_back(0);
}


CBirdCounter::~CBirdCounter()
{
}

bool CBirdCounter::initPiCamera(cv::VideoCapture &cap)
{
	PiCommon picom;
	int nFrameWidth = m_nFrameWidth;
	int nFrameHeight = m_nFrameHeight;
	int nFps = m_nFps;


	if (!cap.isOpened())  // check if we succeeded
	{
		picom.printStdLog("Camera Init Fail");
		return false;
	}
	else {
		picom.printStdLog("Camera Init Successful\n");
		picom.printStdLog("Setting parameters..");
	}

	picom.uniSleep(1000);
	// Set parameters for width
	
	cap.set(CV_CAP_PROP_FRAME_WIDTH, nFrameWidth);
	picom.uniSleep(1000);
	if (cap.get(CV_CAP_PROP_FRAME_WIDTH) == nFrameWidth) {
		picom.printStdLog("[PARAM_FRAME_WIDTH] " + std::to_string(nFrameWidth) + " SUCCESS");
	}
	else {
		picom.printStdLog("[PARAM_FRAME_WIDTH] " + std::to_string(nFrameWidth) + " FAIL");
	}

	picom.printStdLog( "Verified Width : " + std::to_string(cap.get(CV_CAP_PROP_FRAME_WIDTH)));
	
	// Set parameters for height
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, nFrameHeight);
	picom.uniSleep(1000);
	if (cap.get(CV_CAP_PROP_FRAME_HEIGHT) == nFrameHeight)
	{
		picom.printStdLog("[PARAM_FRAME_HEIGHT] " + std::to_string(nFrameHeight) + " SUCCESS");
	}
	else {
		picom.printStdLog("[PARAM_FRAME_HEIGHT] " + std::to_string(nFrameHeight) + " FAIL");
	}
	picom.printStdLog("Verified Height : " + std::to_string(cap.get(CV_CAP_PROP_FRAME_HEIGHT)));

	// Set parameters for fps
	cap.set(CV_CAP_PROP_FPS, nFps);
	picom.uniSleep(1000);
	if (cap.get(CV_CAP_PROP_FPS) == nFps) {
		picom.printStdLog("[PARAM_FPS] " + std::to_string(nFps) + " SUCCESS");
	}
	else {
		picom.printStdLog("[PARAM_FPS] " + std::to_string(nFps) + " FAIL");
	}

	picom.printStdLog("Loaded FPS : " + std::to_string(cap.get(CV_CAP_PROP_FPS)));

	picom.printStdLog("");
	
	return true;
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

void CBirdCounter::tweet_bird()
{
	PiCommon picom;

	std::string piMsg;
	std::string piCmd = "sudo python /home/pi/Desktop/Twitter/./twitter_pic.py ";
	std::string piImg = " '/home/pi/openCV_test/image.jpg'";
	std::string piParallel = " &";

	float fMax = 0.1;
	float fMin = m_fMinThresh;
	piMsg = " '";
	if (m_bOnce) {
		piMsg += "[pi] TEST IMAGE... printing printing";
		m_bOnce = false;
	}
	else if (m_dthresh <= fMin + fMax * 0.1)
		piMsg += "[pi] What was that?";
	else if (m_dthresh < fMin + fMax * 0.2)
		piMsg += "[pi] Something is there..";
	else if (m_dthresh < fMin + fMax * 0.3)
		piMsg += "[pi] Is tat wind?..";
	else if (m_dthresh < fMin + fMax * 0.4)
		piMsg += "[pi] Whoosh..";
	else if (m_dthresh < fMin + fMax * 0.5)
		piMsg += "[pi] Tat is fast..";
	else if (m_dthresh < fMin + fMax * 0.6)
		piMsg += "[pi] Swift as a bird..";
	else if (m_dthresh < fMin + fMax * 0.7)
		piMsg += "[pi] Eikk a bug?..";
	else if (m_dthresh < fMin + fMax * 0.8)
		piMsg += "[pi] Birdyy ..";
	else
		piMsg += "[pi] Too many noise!";

	piMsg += "\n";

	if (m_dTime > 0)
	{
		piMsg += "Process Time : " + std::to_string(m_dTime) + " ms \n";
	}

	piMsg += "Bird-out Count : " + std::to_string(m_nCount_Out) + "\n";
	piMsg += "Bird-in  Count : " + std::to_string(m_nCount_In) + "\n";

	for (int i = 0; i < m_preRatio.size(); i++)
	{
		piMsg += std::to_string(m_preRatio[i]) + "\n";
	}
	piMsg += "' ";

	std::string piSys = piCmd + piMsg + piImg + piParallel;
	system(piSys.c_str());

	printBirdLog();
	picom.printStdLog( "Tweeted Image!");

}

void CBirdCounter::tweet_graph(std::string strdate)
{
	PiCommon picom;
	std::string piMsg;
	std::string piCmd_image = "sudo python /home/pi/Desktop/Twitter/./twitter_pic.py ";
	std::string piCmd_analyzeHisto = "python3 /home/pi/Desktop/Twitter/analyze_birdlog.py ";
	std::string piImg_path = " /home/pi/openCV_test/histogram.jpg ";
	std::string piParallel = " &";

	// Calculate and create histogram
	std::string folder_path = " /home/pi/openCV_test/bird_log/";
	std::string infile_path = folder_path + strdate + "_in.txt ";
	std::string outfile_path = folder_path + strdate + "_out.txt ";


	std::string piSys = piCmd_analyzeHisto + infile_path + outfile_path + piImg_path;
	//system(piSys.c_str());
	picom.printStdLog("Cmd1 : " + piSys);
	std::string str_out = picom.getString_fromCmd(piSys);
	picom.printStdLog("Result : " + str_out);
	picom.uniSleep(10000);

	piMsg = " '";
	piMsg += strdate + " histogram ";
	//piMsg += "Bird-out Total Count : " + std::to_string(count_out) + "\n";
	//piMsg += "Bird-in  Total Count : " + std::to_string(count_in) + "\n";
	//piMsg += str_out;
	piMsg += "' ";

	piSys = piCmd_image + piMsg + piImg_path + piParallel;
	picom.printStdLog("Cmd2 : " + piSys);
	system(piSys.c_str());

	
	picom.printStdLog("Tweeted Graph!\n");

	
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
	int nIn_Line = 0.4 * nSrcWidth;
	int nOut_Line = 0.8 * nSrcWidth;
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
// Thread for all process
void CBirdCounter::save_thread()
{
	PiCommon picom;
	cv::VideoWriter vWriter;

	// Set video name
	int nVideoCount = 0;
	std::string videoName;
	int nFps = m_nFps;
	// Limit store length
	long nLengthLimit = nFps * 60 * 5; //store for 10 minutes
	long nLengthCount = 0;

	picom.printStdLog("Video thread started ");
	for (;;)
	{
		if (m_videoFrames.size() > 0)
		{
			if (!vWriter.isOpened())
			{
				videoName = "/home/pi/openCV_test/cctv_" + picom.get_current_time_and_date() + ".avi";

				// Saving options
				picom.printStdLog("Set video : " + videoName);
				vWriter.open(videoName, CV_FOURCC('M', 'P', '4', '2'), nFps, cv::Size(320, 240));
				nVideoCount++;
			}

		
			cv::Mat matTemp;
			m_mutex.lock();	
					
			matTemp = m_videoFrames.front();
			m_videoFrames.pop_front();
			
			m_mutex.unlock();
			

			if (!matTemp.empty()) {
				vWriter.write(matTemp);
				nLengthCount++;
				//if(nLengthCount%100 ==0 )
				//	std::cout << "Writing frame : " << nLengthCount << "  " << m_videoFrames.size() << std::endl;
			}
	

			if (nLengthCount > nLengthLimit)
			{
				m_mutex.lock(); 
				if(m_bRecord)
					m_bRecord = false;
				m_mutex.unlock();

				if (m_videoFrames.size() == 0)
				{
					vWriter.release();
					picom.printStdLog("Saving to : " + videoName + " DONE ");
					picom.printStdLog("Remaining frames :" + std::to_string(m_videoFrames.size()));
					//break;
				}	
			}
		}
		else
		{
			picom.uniSleep(100);
		}

	}
}
void CBirdCounter::process_thread()
{
	PiCommon picom;
	double dTime = 0;
	for (;;)
	{
		dTime = cv::getTickCount();
		double dForeRatio = 0.0;
		int nFrameWidth = m_nFrameWidth;
		int nFrameHeight = m_nFrameHeight;

		// Exceptions
		if (m_matFrameGray.empty())
			continue;

		cv::Mat matLocalGray;
		cv::Mat matLocalColor;
		cv::Mat matLocalFore;
		matLocalGray = m_matFrameGray.front();
		matLocalColor = m_matFrameColor.front();



		//std::cout << "Remaining Frames : " << m_matFrameGray.size() << "\n";

		m_matFrameGray.pop_front();
		m_matFrameColor.pop_front();

		// Motion detections
		// Reduce size for faster calculation
		cv::Mat smallLocalGray;
		cv::resize(matLocalGray, smallLocalGray, cv::Size(0, 0), 0.5, 0.5);
		//matLocalGray.copyTo(smallLocalGray);
		//cv::resize(matLocalGray, smallLocalGray, cv::Size(0, 0), 0.5, 0.5);
		cv::Rect cntROI(0.4 * smallLocalGray.cols, 0, 0.2 * smallLocalGray.cols, smallLocalGray.rows);


		if (m_nToggleLearn > 10) {
			m_pMOG->apply(smallLocalGray, matLocalFore);
			m_nToggleLearn = 0;
		}
		m_nToggleLearn++;

		// Get the background image
		cv::Mat matBG;
		m_pMOG->getBackgroundImage(matBG);
		if (matBG.empty())
		{
			continue;
		}

		// Find subtraction
		cv::Mat aDiff;
		//cv::absdiff(matSmallGrayFrame, matBg, aDiff);
		cv::absdiff(smallLocalGray, matBG, aDiff);
		cv::blur(aDiff, aDiff, cv::Size(3, 3));
		//cv::threshold(aDiff, matLocalFore, 7, 255, CV_THRESH_BINARY);
		cv::threshold(aDiff, matLocalFore, 3, 255, CV_THRESH_BINARY);

		// Remove upper and lower noise
		int nimgOff = 0.02 * matLocalFore.rows;
		matLocalFore(cv::Rect(0, 0, matLocalFore.cols, nimgOff)).setTo(0);
		matLocalFore(cv::Rect(0, matLocalFore.rows - nimgOff, matLocalFore.cols, nimgOff)).setTo(0);

		// Remove Noise Area
		cv::Rect noiseRect(0, 0, 0.25 * matLocalFore.cols, 0.25 * matLocalFore.rows);
		matLocalFore(noiseRect).setTo(0);
		matLocalFore(noiseRect + cv::Point(0, 0.75 * matLocalFore.rows)).setTo(0);



		cv::Mat matElement = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));
		cv::morphologyEx(matLocalFore, matLocalFore, cv::MORPH_OPEN, matElement, cv::Point(-1, -1), 2);
		cv::morphologyEx(matLocalFore, matLocalFore, cv::MORPH_CLOSE, matElement, cv::Point(-1, -1), 2);

		//cv::erode(matLocalFore, matLocalFore, cv::Mat(), cv::Point(-1, -1), 1);
		//cv::dilate(matLocalFore, matLocalFore, cv::Mat(), cv::Point(-1, -1), 3);
		//cv::erode(matLocalFore, matLocalFore, cv::Mat(), cv::Point(-1, -1), 2);


#ifdef PERSONAL_COMPUTER
		if (!matLocalFore.empty())
		{
			cv::imshow("matLocalFore", matLocalFore);
			cv::waitKey(1);
		}
#endif


		// Calculate ratio
		// Detect sudden change of video quality
		dForeRatio = (double)cv::countNonZero(matLocalFore(cntROI)) / (double)(cntROI.area());
		m_ratios[0] = dForeRatio;
		

		cv::Mat matDisplayWithBirds;
		// Skip when too large changes occur
		if (dForeRatio > 0.35) {
			m_nSaturationCount++;
			m_nCountContinuosValid = 0;
			
			// Clear all bird datas
			m_birds.clear();
		}
		else {
		
			bool bExistBird = countBird(matLocalFore, matLocalColor, matDisplayWithBirds,true);
			if (bExistBird) {
				m_nCountContinuosValid++;
				printBirdLog();
			}
		}
		
		

		// If dForeRatio is higher add count continous ratio
		// Only inform when some thing middle sized moved.
		// Avoid room light change

		
		if (m_bOnce)
		{
			m_nCountContinuosValid = 1;
			m_bTweeterFlag = true;
			m_nSecCount = 10000;
		}

		
#ifdef PERSONAL_COMPUTER
		if (!matDisplayWithBirds.empty())
		{
			cv::imshow("matDisplayWithBirds",matDisplayWithBirds);
			cv::waitKey(0);
		}
#endif


		// Record video if threshold shows continuos 10 frames
		if (m_nCountContinuosValid >= 1)
		{
			// For Uploading pictures to twitter every 60 min interval
			if (m_nSecCount > 60 * 120) // Write
			{
				prepareSaveImage(matLocalFore, matDisplayWithBirds, m_nFps_real, dForeRatio);
				m_bMotionDetected = true;
				m_bTweeterFlag = true;
				m_dthresh = dForeRatio;
				m_nSecCount = 0;
				m_preRatio = m_ratios;

			}
			m_nCountContinuosValid = 0;

		}


#ifndef PERSONAL_COMPUTER
		// usleep(100);
#endif

		dTime = cv::getTickCount() - dTime;
		dTime = (dTime / cv::getTickFrequency() * 1000);
		m_dTime = (m_dTime + dTime) / 2;
		//std::cout << dTime << "\t" << m_dTime << "\n";
		
		// Upload graph
		if (picom.isDateDiff())
		{
			if (!m_bTweeterGraph)
			{
				m_nCountIn_tweet = m_nCount_In;
				m_nCountOut_tweet = m_nCount_Out;
				m_bTweeterGraph = true;
				picom.printStdLog("date_difference");
				resetBirdCount();
			}
			
		}

		// Status update
		auto tempTime = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed_seconds = tempTime - m_statusTime;
		if (elapsed_seconds.count() > 60 * 5)
		{
			m_statusTime = tempTime;
			m_bSaveStatus = true;
		}

	

	}
}
bool CBirdCounter::initVideo()
{
	PiCommon picom;
	m_cap = cv::VideoCapture(0); // open the default camera

	 // Init the camera with initial parameters
	if (!initPiCamera(m_cap))
	{
		picom.printStdLog("Init PiCamera Failed");
		return false;
	}
	return true;
}

void CBirdCounter::getVideoFrame_thread(std::string filepath)
{
// 	PiCommon picom;
// 	m_cap.open(filepath);
// 	for (;;)
// 	{
// 		countFPSStart();
// 		cv::Mat frame, frameColor;
// 		if (m_cap.isOpened())
// 			m_cap.read(frameColor); // get a new frame from camera
// 		else
// 			break;

// 		if (!frameColor.empty())
// 		{
// 			// Preprocess the frames
// 			//cv::flip(frameColor, frameColor, 0);
// 			//cv::flip(frameColor, frameColor, 1);
// 			//cv::blur(frameColor, frameColor, cv::Size(3, 3));
// 			cvtColor(frameColor, frame, cv::COLOR_BGR2GRAY);


// 			// Copy to main matrix
// 			m_mutex.lock();


// 			if (m_matFrameGray.size() > 50000)
// 			{
// 				picom.printStdLog("OverFlow : " + std::to_string(m_nOverflowCount));
// 				m_nOverflowCount++;
// 				m_matFrameGray.clear();
// 				m_matFrameColor.clear();

// 			}
			
// 			//cv::Mat matNoNoise = m_cv.pi_RemoveHorizontalNoise(frame);
// 			//cvtColor(matNoNoise, frameColor, cv::COLOR_GRAY2BGR);
			
// 			m_matFrameColor.push_back(frameColor);
// 			m_matFrameGray.push_back(frame);


			
// 			// for recordings
// 			//m_videoFrames.push_back(m_matFrameGray);
// 			m_mutex.unlock();
// 			picom.uniSleep(3);
// 		}
// 		else
// 			break;

// 		countFPSEnd();

// #ifndef PERSONAL_COMPUTER
// 		usleep(100);
// #endif

//	}
}


void CBirdCounter::run_threads()
{
	
	std::vector<std::thread> threads;
	printf("Init Thread");
	//threads.push_back(std::thread(&CBirdCounter::getVideoFrame_thread, CBirdCounter()));
	//threads.push_back(std::thread(&CBirdCounter::process_thread, CBirdCounter()));
	//threads.push_back(std::thread(save_thread));

	for (auto& thread : threads) {
		thread.join();
	}
}

void CBirdCounter::tweet_thread()
{
	PiCommon picom;
	for (;;)
	{
		if (m_bTweeterFlag)
		{
			
			// Saves the image onto HD before python can process 
#ifndef PERSONAL_COMPUTER
			system("sudo rm /home/pi/openCV_test/image.jpg");
			imwrite("/home/pi/openCV_test/image.jpg", m_matSaveImage);
			//imwrite("/home/pi/openCV_test/FG.jpg", matFg);
			picom.printStdLog("Saved Image!");
#endif
			
			// Time to wait before tweeting
			picom.uniSleep(5000); // rest 5 seconds
			tweet_bird();
			m_bTweeterFlag = false;
		}
		else if (m_bTweeterGraph)
		{
			picom.uniSleep(5000); // rest 5 seconds
			tweet_graph(picom.get_yesterday_date());
			m_bTweeterGraph = false;
		}
		else if (m_bSaveStatus)
		{
			picom.uniSleep(5000);
			printStatus();
			m_bSaveStatus = false;
		}
		else 
		{
			picom.uniSleep(5000); // rest 5 seconds
		}
	
	}
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



void CBirdCounter::printBirdLog()
{
	PiCommon picom;
	std::ofstream in_file, out_file;
	std::string filename = "/home/pi/openCV_test/bird_log/" + picom.get_current_date();
	
	in_file.open(filename + "_in.txt", std::fstream::out | std::fstream::app);
	out_file.open(filename + "_out.txt", std::fstream::out | std::fstream::app);
	
	// in-data print
	for (int i = 0; i < m_inBird_logs.size(); i++)
	{

		std::time_t tt = std::chrono::system_clock::to_time_t(m_inBird_logs[i].detect_time);
		
		in_file << std::strtok(std::ctime(&tt), "\n") << "\t"
			<< m_inBird_logs[i].nSpeed << "\t"
			<< m_inBird_logs[i].nTrailLength << "\n";
	}

	// out-data print
	for (int i = 0; i < m_outBird_logs.size(); i++)
	{
		std::time_t tt = std::chrono::system_clock::to_time_t(m_outBird_logs[i].detect_time);

		out_file << std::strtok(std::ctime(&tt), "\n") << "\t"
			<< m_outBird_logs[i].nSpeed << "\t"
			<< m_outBird_logs[i].nTrailLength << "\n";
	}

	m_inBird_logs.clear();
	m_outBird_logs.clear();

	in_file.close();
	out_file.close();
}



void CBirdCounter::printStatus()
{
	PiCommon picom;
	std::ofstream status_file;
	std::string filename = "/home/pi/openCV_test/bird_log/" + picom.get_current_date();

	status_file.open(filename + "_status.txt", std::fstream::out | std::fstream::app);
	
	// in-data print
	auto timenow = std::chrono::system_clock::now();
	auto tt = std::chrono::system_clock::to_time_t(timenow);
	auto temperature_val = picom.getString_fromCmd("/opt/vc/bin/vcgencmd measure_temp");

	status_file << std::strtok(std::ctime(&tt), "\n") << "\t"
			<< "Sat=" << m_nSaturationCount << "\t"
			<< "Over=" << m_nOverflowCount << "\t"
			<< "Time=" << std::to_string((int)m_dTime) << "\t"
			<< "Bird_in=" << m_nCount_In << "\t"
			<< "Bird_out=" << m_nCount_Out << "\t"
			<< temperature_val;

	std::cout << std::strtok(std::ctime(&tt), "\n") << "\t"
		<< "Sat=" << m_nSaturationCount << "\t"
		<< "Over=" << m_nOverflowCount << "\t"
		<< "Time=" << std::to_string((int)m_dTime) << "\t"
		<< "Bird_in=" << m_nCount_In << "\t"
		<< "Bird_out=" << m_nCount_Out << "\t"
		<< temperature_val;
			

	m_nSaturationCount = 0;
	m_nOverflowCount = 0;

	status_file.close();

}