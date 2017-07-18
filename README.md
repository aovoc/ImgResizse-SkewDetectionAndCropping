# ImgResizse-SkewDetectionAndCropping
## SkewDetectionAndCorrection
驾驶证倾斜矫正模块

### 输入输出
输入文件是res/driving_license_origin文件夹下的jpg格式图片；输出文件和输入文件同名，在源图片所在文件夹的子文件夹result下

### 使用方法：     

	cd ImgResizse-SkewDetectionAndCropping
	make clean && make
	./bin/main.bin



## ImgResize
图像resize

### 输入输出
输入图像路径为"/root/bankCardRec/image2/"(根据具体放置情况修改test/imgResize.cpp 中的pathPrefix 路径)，图像路径下包含要resize的图像路径的文件imgName.txt.
输出图像覆盖输入图像。

### 使用方法：     

	cd ImgResizse-SkewDetectionAndCropping
	make clean && make
	./bin/imgResize.bin
