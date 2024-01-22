//
// Created by 纪定一
//

//#define _AFXDLL
//SLM与DMD的联合调试

#include"GxIAPI.h"
#include <cstdio>
#include <cstdlib>
#include <opencv2/opencv.hpp>
#include <iostream>
#include<conio.h>
#include <string>
#include "Blink_C_wrapper.h"
#include "autoEXPfunc.cpp"
#include <TCHAR.h>
#include <windows.h>

using namespace cv;
using namespace std;

static void GX_STDC OnFrameCallbackFun();


//定义全局变量
bool global_ccd_catch_control_flag = false;
int global_loop_num = 0;
int EXPtimeDone = 0;
int lowPreExpTime = 12;
int highPreExpTime = 1000;
int preEXPtime = 150;
vector<int> temp{0,0,0,0};
GX_DEV_HANDLE hDevice = nullptr;



// 图像读取路径与保存路径
string saveFile = "./catched_speckle/";
string loadFile = "./input_phase_pattern/";


//图像回调处理函数
static void GX_STDC OnFrameCallbackFun(GX_FRAME_CALLBACK_PARAM* pFrame)

{
    int nWidth = pFrame->nWidth;
    int nHeight = pFrame->nHeight;
    //取出图像的索引
    const void * index=pFrame->pImgBuf;
    cv::namedWindow("window");
    cv::Size srcSize = cv::Size(nWidth,nHeight);
    cv::Mat img(srcSize, CV_8U, (int*)index);
    flip(img, img, -1);



    if(!EXPtimeDone){
        //调节曝光时间
        autoEXP test;
        temp = test.autoEXPfunction(preEXPtime,img,lowPreExpTime,highPreExpTime);
        preEXPtime = temp[0];
        EXPtimeDone = temp[1];
        lowPreExpTime = temp[2];
        highPreExpTime = temp[3];
        GXSetFloat(hDevice, GX_FLOAT_EXPOSURE_TIME, preEXPtime);
        return;
    }

    else if (pFrame->status == GX_FRAME_STATUS_SUCCESS && global_ccd_catch_control_flag)
    {
        //完成曝光时间调节以后的图像处理

        string img_string = format("catched_speckle_%d.bmp",global_loop_num);
        string temp_string = saveFile+img_string;

        // 图片保存
        cv::imwrite(temp_string,img);

        //图片展示
        cv::imshow("window", img);
        cv::waitKey(1);

        //关闭采集
        global_ccd_catch_control_flag = false;
    }
}



int main(int argc, char* argv[])

{

    //对反馈优化的相关参数进行定义

    int DMD_record[48*48];
    const int I_target_dst_rate=10;//目标是让增强后的中心点强度是全亮时的rate倍
    fill(DMD_record,DMD_record+48*48,1);
    //循环轮次设置为loop变量
    int loop=0;
    int input_num = 5; //入射的光pattern的数量


    /****************   一   ********************/
    //先对于相机相关进行初始化
    GX_STATUS status = GX_STATUS_OFFLINE;

    GX_OPEN_PARAM stOpenParam;
    uint32_t nDeviceNum = 0;
    // 初始化库
    status = GXInitLib();

    if (status != GX_STATUS_SUCCESS)
    {
        return 0;
    }
    // 枚举设备列表
    status = GXUpdateDeviceList(&nDeviceNum, 1000);
    if ((status != GX_STATUS_SUCCESS) || (nDeviceNum <= 0))
    {
        return 0;
    }
    printf("device number= %d\n", nDeviceNum);
    //打开设备
    stOpenParam.accessMode = GX_ACCESS_EXCLUSIVE;
    stOpenParam.openMode = GX_OPEN_INDEX;
    stOpenParam.pszContent = "1";
    status = GXOpenDevice(&stOpenParam, &hDevice);
    size_t nSize = 0;
    //首先获取字符串允许的最大长度(此长度包含结束符’\0’)
    status = GXGetStringMaxLength(hDevice, GX_STRING_DEVICE_VENDOR_NAME, &nSize);
    //根据获取的长度申请内存
    char* pszText = nullptr;
    pszText = (char*)malloc(nSize);

    if (pszText == nullptr) {
        exit(1);
    }
    status = GXGetString(hDevice, GX_STRING_DEVICE_VENDOR_NAME, pszText, &nSize);
    printf("%s\n", pszText);

    size_t nsize;
    status = GXGetStringMaxLength(hDevice, GX_STRING_DEVICE_MODEL_NAME, &nsize);
    char* pszTex = nullptr;
    pszTex = (char*)malloc(nsize);
    status = GXGetString(hDevice, GX_STRING_DEVICE_MODEL_NAME, pszTex, &nsize);
    printf("%s\n", pszTex);


    //尝试设置曝光时间,单位毫秒？
    GX_FLOAT_RANGE shutterRange;
    status = GXGetFloatRange(hDevice, GX_FLOAT_EXPOSURE_TIME, &shutterRange);
    //设置最小曝光时间
    //status = GXSetFloat(hDevice, GX_FLOAT_EXPOSURE_TIME, shutterRange.dMin);
    //设置最大曝光时间
    //status = GXSetFloat(hDevice, GX_FLOAT_EXPOSURE_TIME, shutterRange.dMax);
    //设置某一曝光时间

    status = GXSetFloat(hDevice, GX_FLOAT_EXPOSURE_TIME, preEXPtime);
    std::cout<<"exposure time"<<shutterRange.dMax<<endl;



/****************   二   ********************/

    // 对于SLM进行初始化
    FreeConsole();

    // Initialize the SDK. The requirements of Matlab, LabVIEW, C++ and Python are different, so pass
    // the constructor a boolean indicating if we are calling from C++/Python (true), or Matlab/LabVIEW (flase)
    bool bCppOrPython = true;
    Create_SDK(true);

    //Read the height and width of the SLM found. The default is 1920x1152 if no SLM is found. The default location
    //is immediately right of the primary monitor. If you monitor is smaller than 1920x1152 you can still run, you will
    //just see a sub-section of the image on the monitor.
    int height = Get_Height();
    int width = Get_Width();
    int depth = Get_Depth(); //will return 8 for 8-bit, or 10 for 10-bit

    // Path to the calibration of voltage to phase. The name "1920x1152_linearVoltage.lut" is a bit confusing.
    // This linearly maps input voltages to output voltages, thus removing any calibration.
    // To be generic this example uses 1920x1152_linearVoltage.lut, but you should link to your custom LUT delivered
    // with the SLM.
    if(height == 1152)
        Load_lut("C:\\Program Files\\Meadowlark Optics\\Blink 1920 HDMI\\LUT Files\\slm6286_at532_HDMI.lut");
    else
    {
        if(depth == 8)
            Load_lut("C:\\Program Files\\Meadowlark Optics\\Blink 1920 HDMI\\LUT Files\\slm6286_at532_HDMI.lut");
        if(depth == 10)
            Load_lut("C:\\Program Files\\Meadowlark Optics\\Blink 1920 HDMI\\LUT Files\\slm6286_at532_HDMI.lut");
    }

    // Create two vectors to hold image data, init as RGB images
    unsigned char* ImageOne = new unsigned char[width*height];
    memset(ImageOne, 0, width*height);
//    unsigned char* ImageTwo = new unsigned char[width*height*3];
//    memset(ImageTwo, 0, width*height * 3);

    //wavefront correction - leave blank for now
    unsigned char* WFC = new unsigned char[width*height];
    memset(WFC, 0, width*height);


/****************   三   ********************/
    //开始进入实际的运行阶段
    cv::Mat temp_image;

    if (status == GX_STATUS_SUCCESS)
    {

        //注册图像处理回调函数
        GXRegisterCaptureCallback(hDevice, nullptr, OnFrameCallbackFun);
        //发送开采命令
        status = GXSendCommand(hDevice, GX_COMMAND_ACQUISITION_START);
        printf("status=%d\n", status);
        //GX_STATUS status = GX_STATUS_SUCCESS;
        //读取当前pixelformat
        int64_t nPixelFormat = 0;
        status = GXGetEnum(hDevice, GX_ENUM_PIXEL_FORMAT, &nPixelFormat);
        //设置pixelformat为bayer格式为BG的位深数据
        nPixelFormat = GX_PIXEL_FORMAT_BAYER_BG10;
        status = GXSetEnum(hDevice, GX_ENUM_PIXEL_FORMAT, nPixelFormat);
        //读取当前pixelsize
        int64_t nPixelSize = 0;
        status = GXGetEnum(hDevice, GX_ENUM_PIXEL_SIZE, &nPixelSize);
        //读取当前colorfilter
        int64_t nColorFilter = 0;
        status = GXGetEnum(hDevice, GX_ENUM_PIXEL_COLOR_FILTER, &nColorFilter);
        printf("nPixelFormat=%lld,nPixelSize=%lld,nColorFilter=%lld\n", nPixelFormat, nPixelSize, nColorFilter);



        //SLM的数据加载和图像采集
        for (global_loop_num=0;global_loop_num<input_num;global_loop_num++)
        {

            string img_string_0 = format("random_input_%d.bmp",global_loop_num);
            string temp_string_0 = loadFile+img_string_0;
            temp_image = cv::imread(temp_string_0);

//            cv::imshow("window000", temp_image);
//            cv::waitKey(100);

            //indicate that our images are "not" RGB images
            bool isEightBit = true;
            //循环多次直到中心点强度大于目标值
            //向SLM中加载图像
            ImageOne = temp_image.data;
            Write_image(ImageOne, isEightBit);
            //开启保存
            global_ccd_catch_control_flag = true;
            Sleep(100);





            //在这个区间图像会通过OnFrameCallbackFun接口返给用户
            //这部分的执行分两部分，即先调节曝光时间，再进行图像处理

            EXPtimeDone = 0;




            //发送停采命令
            status = GXSendCommand(hDevice, GX_COMMAND_ACQUISITION_STOP);
            //注销采集回调
            status = GXUnregisterCaptureCallback(hDevice);
            //重置采集flag
            global_ccd_catch_control_flag = true;
            cout<<"loop time "<< global_loop_num <<endl;

        }

    }
    //关闭SLM设备
    //clean up allocated arrays
    delete[] ImageOne;
    // Destruct
    Delete_SDK();

    //关闭相机设备和库文件
    status = GXCloseDevice(hDevice);
    status = GXCloseLib();

    return 0;
}

