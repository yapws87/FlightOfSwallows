#include "PiTwitter.h"



PiTwitter::PiTwitter()
{
}


PiTwitter::~PiTwitter()
{
}
PiTwitter::tweet_thread()
{
    std::thread t(tweet_thread_proc());
}

PiTwitter::tweet_thread_proc()
{
	PiCommon picom;

    if (m_bTweeterFlag)
    {
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
    else 
    {
        picom.uniSleep(5000); // rest 5 seconds
    }



}

void PiTwitter::tweet_image(std::string piMsg, cv::Mat matImage)
{
    std::string piCmd = m_piCmd_tweetPic;
    std::string piImg = m_piImg;
    std::string piParallel = " &";

    // Saves the image onto HD before python can process 
#ifndef PERSONAL_COMPUTER

    // Save the image in SD for further process
    std::string remove_img = "sudo rm " + piImg;
    system(remove_img.c_str());
    imwrite(piImg, matImage);
    
    piCom.uniSleep(1000);
    picom.printStdLog("Saved Image!");
#endif


    std::string piSys = piCmd + piMsg + piImg + piParallel;
	system(piSys.c_str());

}

void PiTwitter::tweet_bird_thread(cv::Mat matBirdResult)
{
    std::thread(tweet_bird(matBirdResult));
}
void PiTwitter::tweet_bird(cv::Mat matBirdResult)
{
	PiCommon picom;
	std::string piMsg;


	float fMax = 0.1;
	float fMin = 0.01;

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

	tweet_image(piMsg,matBirdResult);

	printBirdLog();
	picom.printStdLog( "Tweeted Image!");

}

void PiTwitter::tweet_graph_thread(std::string strdate)
{
    std::thread(tweet_graph(strdate));
}

void PiTwitter::tweet_graph(std::string strdate)
{
	PiCommon picom;
	std::string piMsg;
	std::string piCmd_image = m_piCmd_tweetPic;
	std::string piCmd_analyzeHisto = m_piCmd_analyzeHisto;
	std::string piHisto_path = m_piHisto;
	std::string piParallel = " &";

	// Calculate and create histogram
	std::string folder_path = m_piBirdLog;
	std::string infile_path = folder_path + strdate + "_in.txt ";
	std::string outfile_path = folder_path + strdate + "_out.txt ";


	std::string piSys = piCmd_analyzeHisto + infile_path + outfile_path + piHisto_path;
	//system(piSys.c_str());
	picom.printStdLog("Cmd1 : " + piSys);
	std::string str_out = picom.getString_fromCmd(piSys);
	picom.printStdLog("Result : " + str_out);
	picom.uniSleep(10000);

	piMsg = " '";
	piMsg += strdate + " histogram ";
	piMsg += "' ";

	piSys = piCmd_image + piMsg + piHisto_path + piParallel;
	picom.printStdLog("Cmd2 : " + piSys);
	system(piSys.c_str());

	picom.printStdLog("Tweeted Graph!\n");

}