set OPENVX_ROOT=%cd%\..
set AMDAPPSDK_PATH=C:\Program Files (x86)\AMD APP SDK\3.0
set OPENCV_LIB_PATH=D:\opencv-3.1.0\vs2015\install\x64\vc14\lib
set VX_OPENCL_INCLUDE_PATH=%AMDAPPSDK_PATH%\include
set VX_OPENCL_LIB_PATH=%AMDAPPSDK_PATH%\lib\x86_64
set OPENVX_INSTALL_ROOT=%OPENVX_ROOT%\install\Win\x64\Debug
set VX_CL_INCLUDE_DIR=%OPENVX_INSTALL_ROOT%\include
set VX_CL_SOURCE_DIR=%OPENVX_ROOT%\kernels\opencl
%OPENVX_ROOT%\build\Win\x64\OpenVX.sln