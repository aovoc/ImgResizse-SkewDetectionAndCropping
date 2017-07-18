//============================================================================
// Author      : ChenMQ(minqinch@gmail.com)
// Version     : 1.0
// Copyright   : 
// Description : Detect driving license in a image and crop the area out.
//============================================================================


#include <iostream>
#include "LicenseDetectionAndCropping.h"
#include <time.h>

#ifdef __linux__  
#include <unistd.h>  
#include <dirent.h>  
#endif  
#ifdef _WIN32  
#include <crtdbg.h>
#include <vld.h>
#include <direct.h>  
#include <io.h>  
#endif  

using namespace std;
using namespace cv;


bool isFileExist(const std::string &path) {
#ifdef _WIN32
	if ((_access(path.c_str(), 0)) != -1) {
		printf("File %s exists.\n", path.c_str());
		return true;
	}
#endif

#ifdef __linux__ 
	if ((access(path.c_str(), F_OK)) == 0) {
		printf("File %s exists.\n", path.c_str());
		return true;
	}
#endif
	return false;
}

//获取所有的文件名  
void GetAllFiles(string readPath, vector<string>& readFiles, vector<string>& writeFiles)
{
	readFiles.clear();
	writeFiles.clear();

#ifdef _WIN32
	long  hFile = 0;
	//文件信息    
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(readPath).append("\\*.jpg").c_str(), &fileinfo)) != -1)
	{
		do {
			readFiles.push_back(p.assign(readPath).append("\\").append(fileinfo.name));
			writeFiles.push_back(p.assign(readPath).append("\\result\\").append(fileinfo.name));
		} while (_findnext(hFile, &fileinfo) == 0);

		_findclose(hFile);
	}
#endif

#ifdef  __linux__ 
	struct dirent* ent = NULL;
	DIR *pDir;
	vector<string> vFile;
	vFile.clear();
	pDir = opendir(readPath.data());

	if (pDir == NULL) {
		return;
	}
	vector<string> vExt;
	vExt.clear();
	string ext = ".jpg";
	string lastExt = ext;


	while (NULL != (ent = readdir(pDir))) {
		string fileName(ent->d_name);
		if (fileName.find(".jpg") == string::npos) continue;
		readFiles.push_back(readPath + "/" + fileName);
		writeFiles.push_back(readPath + "/result/" + fileName);
	}
#endif

}




int main() {
	clock_t start, finish;
	double totaltime;
	vector<string> readFiles;
	vector<string> writeFiles;
	LicenseDetectionAndCropping tmp;

#ifdef _WIN32

	//string path = "H:\\img_for_profile_rot_qcheck";
	string path = "C:\\Users\\chenMQ\\Desktop\\special_case";

	GetAllFiles(path, readFiles, writeFiles);
	tmp.init("SVM_DATA.xml");

#endif // _WIN32

#ifdef  __linux__ 

	string path = "./../res/driving_license_origin";
	GetAllFiles(path, readFiles, writeFiles);

	//tmp.init("/home/eric/SkewDetectionAndCorrection/res/red_stamp.xml");
	tmp.init("./../res/SVM_DATA.xml");

#endif //  __linux__ 

	start = clock();
	cv::Mat dst;
	Mat result;
	
	for (size_t i = 0; i < readFiles.size(); ++i) {
		//if(!isFileExist(readFiles[i])) continue;
		//if (isFileExist(writeFiles[i]))  continue;
		Mat src = cv::imread(readFiles[i]);
		if (tmp.process(src, dst, 1, 1200, 900));
		imwrite(writeFiles[i], dst);
		//imshow("dst", dst);
		//waitKey(0);
	}

	tmp.release();
	finish = clock();
	totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
	cout << "time(s)/image: " << totaltime / readFiles.size() << endl;
#ifdef _WIN32
	system("pause");
#endif

	return 0;
}
