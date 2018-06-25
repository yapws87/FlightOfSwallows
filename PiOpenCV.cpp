#include "PiOpenCV.h"



PiOpenCV::PiOpenCV()
{
}


PiOpenCV::~PiOpenCV()
{
}




cv::Mat PiOpenCV::pi_DFT(cv::Mat matSrc)
{
	cv::Mat padded;                            //expand input image to optimal size
	int m = cv::getOptimalDFTSize(matSrc.rows);
	int n = cv::getOptimalDFTSize(matSrc.cols); // on the border add zero values
	cv::copyMakeBorder(matSrc, padded, 0, m - matSrc.rows, 0, n - matSrc.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));

	cv::Mat planes[] = { cv::Mat_<float>(padded), cv::Mat::zeros(padded.size(), CV_32F) };
	cv::Mat complexI;
	cv::merge(planes, 2, complexI);         // Add to the expanded another plane with zeros

	cv::dft(complexI, complexI);            // this way the result may fit in the source matrix

	return complexI;
}

cv::Mat PiOpenCV::pi_iDFT(cv::Mat matComplex)
{
	cv::Mat padded;
	cv::Mat padded2;
	int m = cv::getOptimalDFTSize(matComplex.rows);
	int n = cv::getOptimalDFTSize(matComplex.cols); // on the border add zero values
	cv::copyMakeBorder(matComplex, padded, 0, m - matComplex.rows, 0, n - matComplex.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));

	//cv::Mat planes[] = { cv::Mat_<float>(padded), cv::Mat::zeros(padded.size(), CV_32F) };

	// IFFT
	cv::Mat inverseTransform;
	cv::idft(padded, inverseTransform,cv::DFT_SCALE | cv::DFT_REAL_OUTPUT);

	// Back to 8-bits
	cv::Mat finalImage;
	inverseTransform.convertTo(finalImage, CV_8U);
	return finalImage;
}

cv::Mat PiOpenCV::pi_RemoveHorizontalNoise(cv::Mat matSrc)
{
	double dTime = cv::getTickCount();
	cv::Mat matComplex_group[2];
	cv::Mat matComplex = pi_DFT(matSrc);

	dTime = cv::getTickCount() - dTime;
	std::cout << "DFT : " << dTime / cv::getTickFrequency() * 1000 << "ms";

	cv::split(matComplex,  matComplex_group);

	
	// Filter each component of the complex structure
	int nW = matComplex_group[0].cols;
	int nH = matComplex_group[0].rows;
	
	int nOffThick = 7;
	int nLengthCoef = 32;
	int nOffLength = nH / nLengthCoef;
	cv::Mat matMask = cv::Mat::ones(nH, nW, CV_32FC1);
	
	cv::Rect leftFilter(0, nOffLength/2, nOffThick, nOffLength * (nLengthCoef - 1));
	cv::Rect rightFilter(nW - nOffThick, nOffLength/2, nOffThick, nOffLength * (nLengthCoef - 1));

	matMask(leftFilter).setTo(0);
	matMask(rightFilter).setTo(0);
	
	cv::GaussianBlur(matMask, matMask, cv::Size(5, 5), -1);

	cv::multiply(matComplex_group[0], matMask, matComplex_group[0]);
	cv::multiply(matComplex_group[1], matMask, matComplex_group[1]);
	
	dTime = cv::getTickCount() - dTime;
	std::cout << "Process : " << dTime / cv::getTickFrequency() * 1000 << "ms";

	// Invert DFT
	cv::merge(matComplex_group,2,matComplex);
	cv::Mat matResult = pi_iDFT(matComplex);

	dTime = cv::getTickCount() - dTime;
	std::cout << "iDFT : " << dTime / cv::getTickFrequency() * 1000 << "ms" << std::endl;
	return matResult;
}

cv::Mat PiOpenCV::piExample(cv::Mat matSrc)
{
	cv::Mat padded;                            //expand input image to optimal size
	int m = cv::getOptimalDFTSize(matSrc.rows);
	int n = cv::getOptimalDFTSize(matSrc.cols); // on the border add zero values
	cv::copyMakeBorder(matSrc, padded, 0, m - matSrc.rows, 0, n - matSrc.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));

	cv::Mat planes[] = { cv::Mat_<float>(padded), cv::Mat::zeros(padded.size(), CV_32F) };
	cv::Mat complexI;
	cv::merge(planes, 2, complexI);         // Add to the expanded another plane with zeros

	cv::dft(complexI, complexI,cv::DFT_SCALE | cv::DFT_COMPLEX_OUTPUT);            // this way the result may fit in the source matrix

											// compute the magnitude and switch to logarithmic scale
											// => log(1 + sqrt(Re(DFT(I))^2 + Im(DFT(I))^2))
	cv::split(complexI, planes);                   // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
	cv::magnitude(planes[0], planes[1], planes[0]);// planes[0] = magnitude
	cv::Mat magI = planes[0];

	magI += cv::Scalar::all(1);                    // switch to logarithmic scale
	cv::log(magI, magI);

	// crop the spectrum, if it has an odd number of rows or columns
	magI = magI(cv::Rect(0, 0, magI.cols & -2, magI.rows & -2));

	// rearrange the quadrants of Fourier image  so that the origin is at the image center
	int cx = magI.cols / 2;
	int cy = magI.rows / 2;

	cv::Mat q0(magI, cv::Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant
	cv::Mat q1(magI, cv::Rect(cx, 0, cx, cy));  // Top-Right
	cv::Mat q2(magI, cv::Rect(0, cy, cx, cy));  // Bottom-Left
	cv::Mat q3(magI, cv::Rect(cx, cy, cx, cy)); // Bottom-Right

	cv::Mat tmp;                           // swap quadrants (Top-Left with Bottom-Right)
	q0.copyTo(tmp);
	q3.copyTo(q0);
	tmp.copyTo(q3);

	q1.copyTo(tmp);                    // swap quadrant (Top-Right with Bottom-Left)
	q2.copyTo(q1);
	tmp.copyTo(q2);

	cv::normalize(magI, magI, 0, 1, CV_MINMAX); // Transform the matrix with float values into a
												// viewable image form (float between values 0 and 1).

	cv::imshow("Input Image", matSrc);    // Show the result
	cv::imshow("spectrum magnitude", magI);
	cv::waitKey();

	return cv::Mat();
}
