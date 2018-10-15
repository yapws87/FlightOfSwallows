#include "PiTwitter.h"



PiTwitter::PiTwitter()
{
}


PiTwitter::~PiTwitter()
{
}


void tweet_image( std::string piCmd
    , std::string piImg
    , std::string piMsg
    , cv::Mat matImage)
{
    PiCommon piCom;

    std::string piParallel = " &";

    // Saves the image onto HD before python can process 
#ifndef PERSONAL_COMPUTER

    // Save the image in SD for further process
    std::string remove_img = "sudo rm " + piImg;

    piCom.printStdLog(remove_img,1);
    system(remove_img.c_str());
    imwrite(piImg,matImage);
    //imwrite("",matImage);
    
    piCom.uniSleep(1000);
    piCom.printStdLog("Saved Image!");
#endif

	
    std::string piSys = piCmd + piMsg + piImg + piParallel;
    piCom.printStdLog(piSys,1);
	system(piSys.c_str());

}

void tweet_bird_proc(cv::Mat matBirdResult
    , std::string piCmd
    , std::string piImg
    , double dThresh
    , double dTime
	, double dTemperature
    , int nCountIn
    , int nCountOut
    )
{
    PiCommon picom;
	std::string piMsg;

	float fMax = 0.05;
	float fMin = 0.01;

	piMsg = " '";
	if (dThresh < 0) {
		piMsg += "[pi] TEST IMAGE... printing printing";
	}
	else if (dThresh <= fMin + fMax * 0.1)
		piMsg += "[pi] What was that?";
	else if (dThresh < fMin + fMax * 0.2)
		piMsg += "[pi] Something is there..";
	else if (dThresh < fMin + fMax * 0.3)
		piMsg += "[pi] Is tat wind?..";
	else if (dThresh < fMin + fMax * 0.4)
		piMsg += "[pi] Whoosh..";
	else if (dThresh < fMin + fMax * 0.5)
		piMsg += "[pi] Tat is fast..";
	else if (dThresh < fMin + fMax * 0.6)
		piMsg += "[pi] Swift as a bird..";
	else if (dThresh < fMin + fMax * 0.7)
		piMsg += "[pi] Eikk a bug?..";
	else if (dThresh < fMin + fMax * 0.8)
		piMsg += "[pi] Birdyy ..";
	else
		piMsg += "[pi] Too many noise!";

	piMsg += "\n";

	if (dTime > 0)
	{
		piMsg += "Process Time : " + std::to_string(dTime) + " ms \n";
	}

	piMsg += "Pi Temperature : " + std::to_string(dTemperature) + "\n";
	piMsg += "Bird-out Count : " + std::to_string(nCountOut) + "\n";
	piMsg += "Bird-in  Count : " + std::to_string(nCountIn) + "\n";


	// for (int i = 0; i < preRatio.size(); i++)
	// {
	// 	piMsg += std::to_string(preRatio[i]) + "\n";
	// }
	piMsg += "' ";

    picom.printStdLog( "pre_tweet_image",1);
	tweet_image(piCmd,piImg,piMsg,matBirdResult);
	picom.printStdLog( "Tweeted Image!");
}

void PiTwitter::tweet_bird_thread(cv::Mat matBirdResult
    , double dThresh
    , double dTime
	, double dTemperature
    , int nCountIn
    , int nCountOut
    , std::deque<double> preRatio)
{
    std::thread t(tweet_bird_proc
        , matBirdResult
        , m_piCmd_tweetPic
        , m_piImg
        , dThresh
        , dTime
		, dTemperature
        , nCountIn
        , nCountOut
        );

        t.detach();
}

void tweet_graph_proc(std::string strdate
    , std::string piCmd_image
    , std::string piCmd_analyzeHisto
    , std::string piHisto_path
	, std::string piLineGraph_path
    ,std::string folder_path )
{
	PiCommon picom;
	std::string piMsg;

	std::string piParallel = " &";

	// Calculate and create histogram
	std::string infile_path = folder_path + strdate + "_in.txt ";
	std::string outfile_path = folder_path + strdate + "_out.txt ";
	std::string dailyFilePath = folder_path + "daily_bird.txt ";

	// Analyze Data to form histogram and line graph
	std::string piSys = piCmd_analyzeHisto  
		+ " " + infile_path 
		+ " " + outfile_path 
		+ " " + piHisto_path 
		+ " " + dailyFilePath 
		+ " " + piLineGraph_path
	;
	//system(piSys.c_str());
	picom.printStdLog("Cmd1 : " + piSys);
	std::string str_out = picom.getString_fromCmd(piSys);
	picom.printStdLog("Result : " + str_out);
	picom.uniSleep(2000);

	// Tweet histogram
	piMsg = " '";
	piMsg += strdate + " histogram ";
	piMsg += "' ";

	piSys = piCmd_image + piMsg + piHisto_path + piParallel;
	picom.printStdLog("Cmd2 : " + piSys);
	system(piSys.c_str());

	picom.printStdLog("Tweeted Histogram!\n");

	// Tweet line graph
	piMsg = " '";
	piMsg +=" Line Graph ";
	piMsg += "' ";

	piSys = piCmd_image + piMsg + piLineGraph_path + piParallel;
	picom.printStdLog("Cmd2 : " + piSys);
	system(piSys.c_str());

	picom.printStdLog("Tweeted Line graph!\n");
}

void PiTwitter::tweet_graph_thread(std::string strdate)
{
    std::thread t(tweet_graph_proc
        , strdate
        , m_piCmd_tweetPic
        , m_piCmd_analyzeHisto
        , m_piHisto
		, m_piLineGraph
        , m_piBirdLog
    );
    t.detach();
}