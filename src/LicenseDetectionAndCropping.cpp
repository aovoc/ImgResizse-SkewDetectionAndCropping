//============================================================================
// Author      : ChenMQ(minqinch@gmail.com)
// Version     : 1.0
// Copyright   : 
// Description : Detect driving license in a image and crop the area out.
//============================================================================


#include "LicenseDetectionAndCropping.h"
using namespace std;
using namespace cv;

LicenseDetectionAndCropping::LicenseDetectionAndCropping()
	:m_squares(), m_svm() {}

LicenseDetectionAndCropping::~LicenseDetectionAndCropping() {}


bool LicenseDetectionAndCropping::init(const string& ClassifierFile) {

#ifdef _WIN32   
	if ((_access(ClassifierFile.c_str(), 0)) != -1) {
		m_svm = new CvSVM();
		//m_svm = LatentSvmDetector::load("svm.xml");
		//m_svm->load("red_stamp.xml");
		//Ptr<SVM>svm = SVM::load("svm.xml");
		//m_svm = LatentSvmDetector::load("svm.xml");;
		m_svm->load(ClassifierFile.c_str());
		//m_svm = cv::SVM::load("red_stamp.xml");
		return true;
	}
	else {
		printf("File %s does not exist.\n", ClassifierFile.c_str());
		return false;
	}
#endif 
#ifdef  __linux__   
	if ((access(ClassifierFile.c_str(), F_OK)) == 0) {
		m_svm = new CvSVM();
		m_svm->load(ClassifierFile.c_str());
		//m_svm = Algorithm::load<ml::SVM>(ClassifierFile.c_str());
		return true;
	}
	else {
		printf("File %s does not exist.\n", ClassifierFile.c_str());
		return false;
	}
#endif
}

bool LicenseDetectionAndCropping::process(const Mat& src, Mat&dst, int level, int width, int height) {
	Mat srcCopy;
	src.convertTo(srcCopy, CV_8UC1);
	bool ret1 = coarseTune(srcCopy, dst, level);
	bool ret2 = false;
	dst = fineTune(ret2, dst);
	resize(dst, dst, Size(width, height));
	return ret1 || ret2;
}

int LicenseDetectionAndCropping::release() {
	m_squares.clear();
	m_svm.release();
	return 0;
}


void LicenseDetectionAndCropping::findSquares(Mat& gray, vector<vector<Point> >& squares) {
	squares.clear();
	vector<vector<Point> > contours;
	findContours(gray, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);
	vector<Point> approx;

	// test each contour
	for (size_t i = 0; i < contours.size(); ++i) {

		// approximate contour with accuracy proportional to the contour perimeter
		approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.02, true);

		// 4 vertices, area greater than 1000, and be convex.
		if (approx.size() == 4 && fabs(contourArea(Mat(approx))) > 1000 && isContourConvex(Mat(approx))) {
			double maxCosine = 0;

			for (int j = 2; j < 5; ++j) {
				// find the maximum cosine of the angle between joint edges
				double cosine = fabs(angle(approx[j % 4], approx[j - 2], approx[j - 1]));
				maxCosine = MAX(maxCosine, cosine);
			}

			// if all angles are ~90 degree, write quandrange vertices to resultant sequence
			if (maxCosine < 0.1)
				squares.push_back(approx);
		}
	}
	return;
}


bool LicenseDetectionAndCropping::cropping(const Mat& originImg, Mat& outputImg, vector<vector<Point> >& squares) {
	for (size_t i = 0; i < squares.size(); ++i) {
		if (!isProperSection(originImg, squares[i])) continue;

		vector<Point2f> stampVertices(4);
		vector<Point2f> licenseVertices(4);
		vector<Point2f> tmplateStamp(4);
		vector<Point2f> tmplateLicense(4);
		vector<Point2f> dst_vertices(4);

		int stamp_x = 65, stamp_y = 535, stamp_w = 265, stamp_h = 265;
		int license_x = 0, license_y = 0, license_w = 1235, license_h = 900;

		Mat warpMatrix;

		RotatedRect box = minAreaRect(Mat(squares[i]));
		Point2f tmp[4];
		box.points(tmp);
		for (int j = 0; j < 4; ++j)
			squares[i][j] = tmp[j];

		//Find top-left vertex as start point.
		int idx = 0, tmpMin = 1000000;
		for (int j = 0; j < 4; ++j) {
			if (squares[i][j].x + squares[i][j].y < tmpMin) {
				tmpMin = squares[i][j].x + squares[i][j].y;
				idx = j;
			}
		}

		for (int j = 0; j < 4; ++j) {
			stampVertices[j].x = squares[i][(j + idx) % 4].x;
			stampVertices[j].y = squares[i][(j + idx) % 4].y;
		}

		//anticlockwise
		if (stampVertices[3].x < stampVertices[1].x  && stampVertices[3].y > stampVertices[1].y)
			swap(stampVertices[3], stampVertices[1]);

		//get red stamp area, check it and select template.
		dst_vertices[0] = Point(float(0), float(0));
		dst_vertices[1] = Point(float(0), float(originImg.rows));
		dst_vertices[2] = Point(float(originImg.cols), float(originImg.rows));
		dst_vertices[3] = Point(float(originImg.cols), float(0));

		Mat redStampImg;
		warpMatrix = getPerspectiveTransform(stampVertices, dst_vertices);
		warpPerspective(originImg, redStampImg, warpMatrix, originImg.size());
		//imshow("redStampImg", redStampImg);
		//waitKey(0);

		int angle = isRedStamp(redStampImg);
		//cout << angle << endl;
		if (angle == -1) continue;
		if (angle == 0) {
			;
		}
		else if (angle == 90) {
			int t_stamp_x = license_h - stamp_h - stamp_y;
			int t_stamp_y = stamp_x;
			stamp_x = t_stamp_x;
			stamp_y = t_stamp_y;
			swap(license_w, license_h);
		}
		else if (angle == 180) {
			int t_stamp_x = license_w - stamp_w - stamp_x;
			int t_stamp_y = license_h - stamp_h - stamp_y;
			stamp_x = t_stamp_x;
			stamp_y = t_stamp_y;

		}
		else if (angle == 270) {
			int t_stamp_x = stamp_y;
			int t_stamp_y = license_w - stamp_w - stamp_x;
			stamp_x = t_stamp_x;
			stamp_y = t_stamp_y;
			swap(license_w, license_h);
		}
		tmplateStamp[0] = Point(stamp_x + float(0), stamp_y + float(0));
		tmplateStamp[1] = Point(stamp_x + float(0), stamp_y + stamp_h);
		tmplateStamp[2] = Point(stamp_x + stamp_w, stamp_y + stamp_h);
		tmplateStamp[3] = Point(stamp_x + stamp_w, stamp_y + float(0));
		tmplateLicense[0] = Point(license_x + float(0), license_y + float(0));
		tmplateLicense[1] = Point(license_x + float(0), license_y + license_h);
		tmplateLicense[2] = Point(license_x + license_w, license_y + license_h);
		tmplateLicense[3] = Point(license_x + license_w, license_y + float(0));

		// get the driving license vertices
		warpMatrix = getPerspectiveTransform(tmplateStamp, stampVertices);
		perspectiveTransform(tmplateLicense, licenseVertices, warpMatrix);

		// get driving license area and rotate it if needed.  
		dst_vertices[0] = Point(float(0), float(0));
		dst_vertices[1] = Point(float(0), float(originImg.rows));
		dst_vertices[2] = Point(float(originImg.cols), float(originImg.rows));
		dst_vertices[3] = Point(float(originImg.cols), float(0));

		warpMatrix = getPerspectiveTransform(licenseVertices, dst_vertices);
		warpPerspective(originImg, outputImg, warpMatrix, originImg.size());
		switch (angle / 90)
		{
		case 1:
			transpose(outputImg, outputImg);
			flip(outputImg, outputImg, 0);
			break;
		case 2:
			flip(outputImg, outputImg, -1);
			break;
		case 3:
			transpose(outputImg, outputImg);
			flip(outputImg, outputImg, 1);
			break;
		default:
			break;
		}
		resize(outputImg, outputImg, Size(800, 600));

		return true;

	}
	return false;
}


bool LicenseDetectionAndCropping::isProperSection(const Mat& src, const std::vector<cv::Point>& square) {
	if (square.size() != 4) return false;

	// max dx,dy of the square edges.
	int max_x = 0, max_y = 0;
	for (int j = 1; j < square.size(); ++j) {
		max_x = max(abs(square[j].x - square[j - 1].x), max_x);
		max_y = max(abs(square[j].y - square[j - 1].y), max_y);
	}

	// coarse rules to remove unqualified conditions.
	if (max_x > src.cols / 3 || max_y > src.rows / 3) return false;
	if (max_x < src.cols / 15 || max_y < src.cols / 10) return false;
	if (abs(max_x - max_y) > max_x || abs(max_x - max_y) > max_y) return false;
	if (max_x > (max_y << 1) || max_y > (max_x << 1)) return false;
	if (max_x < 60 || max_y < 60) return false;
	return true;
}


int LicenseDetectionAndCropping::isRedStamp(const Mat& src) {

	// using classifier file for judgement.
	if (m_svm.empty()) {
		printf("Can not load the classifier file, please call init() function first.\n");
		return false;
	}

	
	// hog + svm
	int ImgWidht = 64;
	int ImgHeight = 64;
	Mat testImg = Mat::zeros(ImgHeight, ImgWidht, CV_8UC3);
	char line[512];

	resize(src, testImg, Size(ImgWidht, ImgHeight), 0, 0, INTER_CUBIC);
	if (testImg.channels() == 4)
		cvtColor(testImg, testImg, CV_RGBA2RGB);
	HOGDescriptor *hog = new HOGDescriptor(cvSize(ImgWidht, ImgHeight), cvSize(16, 16), cvSize(8, 8), cvSize(8, 8), 9);
	vector<float>descriptors;
	hog->compute(testImg, descriptors, Size(1, 1), Size(0, 0));

	Mat SVMTestMat = Mat::zeros(1, descriptors.size(), CV_32FC1);
	int n = 0;
	for (vector<float>::iterator iter = descriptors.begin(); iter != descriptors.end(); ++iter, ++n)
		SVMTestMat.at<float>(0, n) = *iter;

	delete hog;
	return 0;
	return m_svm->predict(SVMTestMat);
}


double LicenseDetectionAndCropping::angle(const Point pt1, const Point pt2, const Point pt0)
{
	double dx1 = pt1.x - pt0.x;
	double dy1 = pt1.y - pt0.y;
	double dx2 = pt2.x - pt0.x;
	double dy2 = pt2.y - pt0.y;
	return (dx1*dx2 + dy1*dy2) / sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

bool LicenseDetectionAndCropping::coarseTune(const Mat& src, Mat&dst, int level) {
	dst = src;
	Mat hsv_image;
	if (src.channels() != 1)
		cvtColor(src, hsv_image, COLOR_BGR2HSV);
	else return false;

	// Threshold of the HSV image, keep only the red pixels
	Mat lower_red_hue_range;
	Mat upper_red_hue_range;
	inRange(hsv_image, Scalar(0, 43, 46), Scalar(10, 255, 255), lower_red_hue_range);
	inRange(hsv_image, Scalar(156, 43, 46), Scalar(180, 255, 255), upper_red_hue_range);

	// Combine the above two images
	Mat red_hue_image;
	addWeighted(lower_red_hue_range, 1.0, upper_red_hue_range, 1.0, 0.0, red_hue_image);

	int cornelSizeInc = 20;
	switch (level)
	{
	case 0: cornelSizeInc = 100; break;
	case 1: cornelSizeInc = 20; break;
	case 2: cornelSizeInc = 10; break;
	case 3: cornelSizeInc = 4; break;
	case 4: cornelSizeInc = 2; break;
	default:
		break;
	}

	for (int t = 1; t < 23; t += cornelSizeInc) {
		GaussianBlur(red_hue_image, red_hue_image, Size(t, t), 3, 3);
		findSquares(red_hue_image, m_squares);
		if (cropping(src, dst, m_squares)) return true;
	}
	return false;
}

Mat LicenseDetectionAndCropping::fineTune(bool &isChange, const Mat& srcMat) {
	Mat imgRet = srcMat;
	isChange = false;
	IplImage* src, *temp_object;
	temp_object = new IplImage(srcMat);
	src = cvCreateImage(cvGetSize(temp_object), 8, 1);

	if (srcMat.channels() != 1) {
		cvCvtColor(temp_object, src, CV_BGR2GRAY);
	}

	//set central area as ROI
	cvSetImageROI(src, Rect(src->width / 5, src->height / 5, src->width * 3 / 5, src->height * 3 / 5));
	
	//canny edge detection
	IplImage* dst = cvCreateImage(cvGetSize(src), 8, 1);
	cvCopy(src, dst);
	cvCanny(dst, dst, 50, 200, 3);

	//use Hough Transform to detect line
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* lines = cvHoughLines2(dst, storage, CV_HOUGH_PROBABILISTIC, 1, CV_PI / 180, 10, 30, 10);
	
	
	//find the longest line.
	int dis = 0, max = 0, idx = 0;
	CvPoint* line;
	CvPoint pointOne, pointTwo;
	int a[100008] = { 0 };
	
	int i;
	for (i = 0; i < lines->total && (i << 2) < 100002; i++)
	{
		line = (CvPoint*)cvGetSeqElem(lines, i);
		dis = (line[1].y - line[0].y)*(line[1].y - line[0].y) + (line[1].x - line[0].x)*(line[1].x - line[0].x);

		a[(i << 2)] = line[0].x;
		a[(i << 2) + 1] = line[0].y;
		a[(i << 2) + 2] = line[1].x;
		a[(i << 2) + 3] = line[1].y;
		if (dis > max){
			max = dis;
			idx = i;
		}
	}
	pointOne.x = a[(idx << 2)];
	pointOne.y = a[(idx << 2) + 1];
	pointTwo.x = a[(idx << 2) + 2];
	pointTwo.y = a[(idx << 2) + 3];

	double Angle = atan2(fabs(double(pointTwo.y - pointOne.y)), fabs(double(pointTwo.x - pointOne.x)));

	if (pointTwo.x > pointOne.x && pointTwo.y > pointOne.y){
		Angle = -Angle;
	}

	Angle = Angle * 180 / CV_PI;
	if (Angle > 5 || Angle < -5 || abs(Angle) < 0.001) isChange = false;
	else {
		isChange = true;
		Mat R = getRotationMatrix2D(Point(srcMat.cols, srcMat.rows), -Angle, 1.0);
		warpAffine(srcMat, imgRet, R, srcMat.size());
	}
	delete temp_object;
	return imgRet;
}

