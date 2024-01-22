# autoExposure_SLM_and_CCD_joint_control
本项目为联合控制空间光调制器（SLM）与相机（CCD）的代码，每投影一个SLM的图样，就需要拍摄一张对应的图片

由于每个图样所导致的散斑光强不相同，因此需要加入自动曝光算法（auto exposure）实现自动调节曝光时间，在不过曝的条件下尽量增加曝光时间，减小量化误差

代码control_code.cpp为函数主题

代码autoEXPfunc.cpp为自动曝光代码

所采用的相机为大恒图像MER-120-030UM，开发该相机相关的SDK可见官网

所采用的SLM为meadowlark1920*1200系列，开发该SLM的相关SDK可见官网

support文件夹包含了开发相机和SLM的头文件，以及开发SLM的动态链接库
