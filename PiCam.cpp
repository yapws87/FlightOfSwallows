#include "PiCam.h"

PiCam::PiCam()
{}

PiCam::~PiCam()
{}

void PiCam::status_check(std::string item, std::string value, bool bstatus)
{

    item.resize(30 ,' ');
    value.resize(10, ' ');
    
    if(bstatus)
        m_picom.printStdLog( item + value + ": SUCCESS");
    else
        m_picom.printStdLog( item + value + ": FAIL");
}

bool PiCam::init_cam(int nWidth , int nHeight , int nFPS)
{
    m_nFrameWidth = nWidth;
	m_nFrameHeight = nHeight;
	m_nFPS = nFPS;

    // Open Camera
    bool bCam_status = m_cap.isOpened();
    m_picom.printStdLog("No opened device");
    if(!bCam_status)
    {
        m_picom.printStdLog("Opening device 0");
        m_cap.open(0);
    }
        

    // Check camera status
    bCam_status = m_cap.isOpened();
    status_check("Camera Init"," ",bCam_status);
    
    if(!bCam_status) 
        return false;

	m_picom.uniSleep(500);
	// Set parameters for width
	
	m_cap.set(CV_CAP_PROP_FRAME_WIDTH, m_nFrameWidth);
    m_cap.set(CV_CAP_PROP_FRAME_HEIGHT, m_nFrameHeight);
    m_cap.set(CV_CAP_PROP_FPS, m_nFPS);


	m_picom.uniSleep(500);
    bool bWidth_status  = (m_cap.get(CV_CAP_PROP_FRAME_WIDTH)     == m_nFrameWidth);
    bool bHeight_status = (m_cap.get(CV_CAP_PROP_FRAME_HEIGHT)    == m_nFrameHeight);
    bool bFps_status    = (m_cap.get(CV_CAP_PROP_FPS)             == m_nFPS);
	

    status_check(" [PARAM_FRAME_WIDTH] ", std::to_string(m_nFrameWidth),  bWidth_status);
    status_check(" [PARAM_FRAME_HEIGHT] ", std::to_string(m_nFrameHeight), bHeight_status);
    status_check(" [PARAM_FPS] ", std::to_string(m_nFPS), bFps_status);
   

	m_picom.printStdLog( "Verified Width : " + std::to_string(m_cap.get(CV_CAP_PROP_FRAME_WIDTH)));
	m_picom.printStdLog("Verified Height : " + std::to_string(m_cap.get(CV_CAP_PROP_FRAME_HEIGHT)));
	m_picom.printStdLog("Loaded FPS : " + std::to_string(m_cap.get(CV_CAP_PROP_FPS)));
	
	m_picom.printStdLog("");
	
	return true;
}



void PiCam::runFrame_thread()
{
	PiCommon picom;

	static int m_nCountFrame = 0;
	for (;;)
	{
		picom.countFPSStart();
		
		cv::Mat frame, frameColor,frameBGR;
		if (m_cap.isOpened()){
			m_cap.read(frameColor); // get a new frame from camera
			m_picom.printStdLog( "reading from cam");
		}
		else
		{
			m_picom.printStdLog( "Camera is not loaded properly. Exiting thread...");
			break;
		}
			
		if (!frameColor.empty())
		{
			// Preprocess the frames
			cvtColor(frameColor, frame, cv::COLOR_BGR2GRAY);
			m_nCountFrame++;
			
			// Noise Removal

			// Copy to main matrix
			m_mutex.lock();

			// Prevent overflow
			if (m_matFrameGray.size() > 100)
			{
				picom.printStdLog( "Overflow gray frame");
				m_matFrameGray.clear();
				m_matFrameColor.clear();
			}
			if( m_videoFrames.size() > 100)
			{
				picom.printStdLog( "Overflow recording frame");
				m_videoFrames.clear();
			}
			
			// Que to save image
			if (picom.get_current_time() == "18:58:00" )
			{
				if(!m_bRecord){
					m_bRecord = true;
					picom.printStdLog( "Record flag turned ON" );
				}
					
				//m_videoFrames
			}
				
			m_matFrameColor.push_back(frameColor);
			m_matFrameGray.push_back(frame);
			// for recordings
			if (m_bRecord) {
				m_videoFrames.push_back(frameColor);
			}

			m_mutex.unlock();
		}
		else
			break;

		picom.countFPSEnd();

#ifndef PERSONAL_COMPUTER
		usleep(100);
#endif

	}
}
void PiCam::set_record()
{
    m_mutex.lock(); 
    m_bRecord = true;
    m_mutex.unlock(); 
}

void PiCam::set_save_directory(std::string file_directory)
{
    m_save_directory = file_directory;

}

void PiCam::save_thread()
{
	PiCommon picom;
	cv::VideoWriter vWriter;

	// Set video name
	int nVideoCount = 0;
	std::string videoName;
	int nFps = m_nFPS;

	// Limit store length
	long nLengthLimit = nFps * 60 * m_nSave_duration_minutes; //store for 10 minutes
	long nLengthCount = 0;

	picom.printStdLog(" Video thread started ");
	for (;;)
	{
		if (m_videoFrames.size() > 0)
		{
			if (!vWriter.isOpened())
			{
				videoName = m_save_directory + "/cctv_" + picom.get_current_time_and_date() + ".avi";

				// Saving options
				picom.printStdLog("Set video : " + videoName);
				vWriter.open(videoName, CV_FOURCC('M', 'P', '4', '2'), nFps, cv::Size(320, 240));
				nVideoCount++;
			}
		
            // Extract frame from vector
			cv::Mat matSaveFrame;
			m_mutex.lock();	
			
			matSaveFrame = m_videoFrames.front();
			m_videoFrames.pop_front();
			
			m_mutex.unlock();
			
            // Write frame to video
			if (!matSaveFrame.empty()) {
				vWriter.write(matSaveFrame);
				nLengthCount++;
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
					picom.printStdLog(" Saving to : " + videoName + " DONE ");
					picom.printStdLog(" Remaining frames :" + std::to_string(m_videoFrames.size()));
                    nLengthCount = 0;
				}	
			}
		}
		else
		{
			picom.uniSleep(100);
		}

	}
}

void PiCam::get_frame(cv::Mat &matGray, cv::Mat &matColor)
{
    m_mutex.lock();

    if( m_matFrameGray.size() > 0 &&  m_matFrameColor.size() > 0)
    {
        m_matFrameGray.front().copyTo(matGray);
        m_matFrameColor.front().copyTo(matColor);

        m_matFrameGray.pop_front();
        m_matFrameColor.pop_front();
    }


    m_mutex.unlock();
}