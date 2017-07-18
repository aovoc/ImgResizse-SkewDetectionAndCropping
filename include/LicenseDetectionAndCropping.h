//============================================================================
// Author      : ChenMQ(minqinch@gmail.com)
// Version     : 1.0
// Copyright   : 
// Description : Detect driving license in a image and crop the area out.
//============================================================================


#ifndef LICENSEDETECTIONANDCROPPING_H_
#define LICENSEDETECTIONANDCROPPING_H_

#include <string>
#include <vector>
#include <math.h>
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv/ml.h>

#ifdef  __linux__   
#include <unistd.h>  
#include <dirent.h>  
#endif  
#ifdef _WIN32  
#include <direct.h>  
#include <io.h>  
#endif  

class LicenseDetectionAndCropping
{
public:
	LicenseDetectionAndCropping();
	~LicenseDetectionAndCropping();

	/** @brief Initiate resources.
	@param classifierFile  The path of classifier file about red stamp.
	*/
	bool init(const std::string& classifierFile);

	/** @brief Detect driving license and cropping.
	@param src  Input image: BGR format.
	@param dst  Output mat that has the same size and the same number of channels as the input arrays.
	@param level  Detect intensity. Range for 0 to 4, default is 1. value not in this range will be reset to default value.
				  The corresponding maxinum detection time of level 0 to 4 is 1,2,3,6,11.
	*/
	bool process(const cv::Mat& src, cv::Mat& dst, int level = 1, int width = 800, int height =600);


	/** @brief Release resources.
	*/
	int release();

private:

	/** @brief  Find out driving license area.
	@param src  Input image: gray image.
	@param squares  Storage vertices of driving license area.
	*/
	void findSquares(cv::Mat& src, std::vector<std::vector<cv::Point> >& squares);


	/** @brief  Check whether the square area is a proper section or not.
	@param src  The original image.
	@param squares  Vertices of potential red stamp area.
	*/
	bool isProperSection(const cv::Mat& src, const std::vector<cv::Point>& square);


	/** @brief  Check whether the square area is a red stamp or not.
	@return  -1: No;  x: yes,rotate x degree
	@param src  potential red stamp image.
	*/
	int isRedStamp(const cv::Mat& src);


	/** @brief  Cropping.
	@param src  The original image.
	@param dst  The output image.
	@param squares  Probable driving license area.
	*/
	bool cropping(const cv::Mat& src, cv::Mat& dst, std::vector<std::vector<cv::Point> >& squares);


	/** @brief  Find a cosine of angle between vectors pt0->pt1 and pt0->pt2.
	@param pt1,pt2,pt0  Vertices of the angle.
	*/
	double angle(const cv::Point pt1, const cv::Point pt2, const cv::Point pt0);


	/* @brief  canny+hough , line detection, rectify angle between -5 to 5.
	@para srcMat  original image.
	@para isChange  flag of whether image has changed or not.
	*/
	cv::Mat fineTune(bool& isChange, const cv::Mat& srcMat);

	/* @brief   red stamp detection, rectify angle in image where red stamp is easily to detected.
	@para srcMat   original image.
	*/
	bool coarseTune(const cv::Mat& src, cv::Mat&dst, int level);

private:
	std::vector<std::vector<cv::Point> > m_squares;
	cv::Ptr<CvSVM> m_svm;
	//LatentSvmDetector
	//cv::Ptr<cv::SVM> m_svm;

};

#endif // LICENSEDETECTIONANDCROPPING_H_


