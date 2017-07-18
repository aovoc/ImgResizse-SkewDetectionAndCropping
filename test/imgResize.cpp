#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
using namespace cv;
using namespace std;


int main()
{
  //fstream infile("/root/bankCardRec/image2/imgName.txt");
	fstream infile("imgName.txt");
	string imgName;
	string pathPrefix = "/root/bankCardRec/image2/";
	cout << "a "<< endl;
	// Read source image.
	while (getline(infile, imgName))
	{
	  //string imgUrl = pathPrefix + imgName;
	  string imgUrl = imgName;
		try
		{
			Mat img = imread(imgUrl);
			int width, height;
			//cout << imgName <<endl;
			if(img.cols >  1200 || img.rows > 900){
				cout << img.cols << " " << img.rows <<endl;
				if(img.cols *3 > img.rows * 4){
					width = 1200;
					height = 1200 * (float)img.rows / img.cols;
				}
				else{				
					width = 900 * img.cols / img.rows;
					height = 900;
				}

				resize(img, img, Size(width, height));
				//cout  << img.cols << " " << img.rows << endl << endl;
				//string savePath = pathPrefix + "result\\" + imgName;
				//cout << savePath <<endl;
				
			}
			imwrite(imgUrl, img);
			//cout << imgUrl <<endl;
		}
		catch (Exception e) {
			cout << "exception" << endl;
		}
		
	}
	
	//system("pause");
	//waitKey(0);
	return 0;


}

