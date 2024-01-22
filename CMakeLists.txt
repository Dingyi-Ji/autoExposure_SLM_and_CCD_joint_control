# CMakeList.txt : CMake project for CMakeProject1, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
project(control_code)

# Add source to this project's executable.
add_executable (control_code  "autoEXPfunc.cpp" "control_code.cpp")

# TODO: Add tests and install targets if needed.
#set(OpenCV_DIR "D:/Software/opencv/opencv/build/x64/vc15/lib")
#find_package(OpenCV REQUIRED)
#include_directories(${OpenCV_INCLUDE_DIRS})
#
#target_link_libraries(control_code PUBLIC ${OpenCV_LIBS})
set(OpenCV_DIR D://Software//opencv//opencv//build//x64//vc15/lib)

find_package(OpenCV 4 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(control_code PUBLIC ${OpenCV_LIBS})


target_include_directories(control_code PUBLIC
        "support/inc")

#target_link_directories(CMakeProject1 PUBLIC
#       "support/lib/x64")

#target_link_libraries(CMakeProject1 PUBLIC
#       DxImageProc )
#target_link_libraries(control_code PUBLIC
#        alpV42)

#加载SLM的动态链接库

target_link_directories(control_code PUBLIC
        "support/lib/slm_lib")
target_link_libraries(control_code PUBLIC
        Blink_C_wrapper)

target_link_directories(control_code PUBLIC
        "support/lib/x6442")
target_link_libraries(control_code PUBLIC
        GxIAPI )





#target_link_libraries(CMakeProject1 PUBLIC
#        alp4395)
