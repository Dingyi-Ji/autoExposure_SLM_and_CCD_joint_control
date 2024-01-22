//
// Created by 纪定一
//

# include <iostream>
# include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

class autoEXP {

public:
    //自动曝光
    vector<int> autoEXPfunction(int preEXPtime, cv::Mat & img, int lowPreExpTime, int highPreExpTime){
        int nHeight = img.rows;
        int nWidth = img.cols;
        int bestExpTime = 0;

        //得到直方图
        int histSize = 256;  // 直方图的大小
        float range[] = {0, 256};  // 像素值的范围
        const float* histRange = {range};
        bool uniform = true, accumulate = false;
        int hist[256]={0};


        for (int i=0;i<nHeight;i++){
            for (int j=0;j<nWidth;j++){
                hist[int(img.at<uchar>(i,j))]++;
            }
        }

        //二分法查找最优曝光时间
        int isBest = 0;
        if(lowPreExpTime== preEXPtime || highPreExpTime==preEXPtime){  //终止条件
            isBest=1;
            return {preEXPtime, isBest, lowPreExpTime, highPreExpTime};
        }

        float maxRatio = 0;
        for(int i=250;i<256;i++){   //250以上的比例超过10%就需要调整
            maxRatio = maxRatio+hist[i];
        }

        maxRatio = maxRatio/(nHeight*nWidth);

        //左闭右闭
        if(maxRatio>0.1){
            int curEXPtime = (lowPreExpTime+preEXPtime)/2;
            return {curEXPtime, isBest, lowPreExpTime, curEXPtime};
        }
        else if(maxRatio<=0.1){
            int curEXPtime = (highPreExpTime+preEXPtime)/2;
            return {curEXPtime, isBest, curEXPtime, highPreExpTime};
        }


        return {-1,-1,-1,-1};
    }
};

//
//int main() {
//    Mat image = imread(R"(F:\postgraduate research\phase8\SLM_and_CCD_joint_control\catched_speckle_0.bmp)", IMREAD_GRAYSCALE);  // 替换为你的图像路径    Mat image = imread("image.jpg", IMREAD_GRAYSCALE);  // 替换为你的图像路径
//    autoEXP test;
//    test.autoEXPfunction(0,image,0,0);
//
//}
