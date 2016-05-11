# Git clone
You should replace VX_OPENCL_INCLUDE_PATH and VX_OPENCL_LIB_PATH environment variables to your OpenCL path.
```
git clone git@github.com:eric100lin/My-OpenVX-1.1.git
cd My-OpenVX-1.1

export OPENVX_ROOT=$(pwd)
export AMDAPPSDK_PATH=/home/thlin/AMDAPPSDK-3.0
export VX_OPENCL_INCLUDE_PATH=$AMDAPPSDK_PATH/include
export VX_OPENCL_LIB_PATH=$AMDAPPSDK_PATH/lib/x86_64/sdk/libOpenCL.so
```

# BUILD FOR AMD64 Desktop with XML support
debug version:
```
python Build.py --os=linux --arch=64 --conf=Debug --openmp=1 --opencl=1
cd $OPENVX_ROOT/build/Linux/x64/Debug
export OPENVX_INSTALL_ROOT=$OPENVX_ROOT/install/Linux/x64/Debug
cmake $OPENVX_ROOT -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=$OPENVX_INSTALL_ROOT -DBUILD_X64=1  -DEXPERIMENTAL_USE_OPENMP=1 -DEXPERIMENTAL_USE_OPENCL=1 -DEXPERIMENTAL_USE_TARGET=1 -DEXPERIMENTAL_USE_XML=1 -DCMAKE_C_FLAGS="-DCL_USE_DEPRECATED_OPENCL_1_0_APIS -DCL_USE_DEPRECATED_OPENCL_1_1_APIS -DCL_USE_DEPRECATED_OPENCL_2_0_APIS" -DCMAKE_CXX_FLAGS="-DCL_USE_DEPRECATED_OPENCL_1_0_APIS -DCL_USE_DEPRECATED_OPENCL_1_1_APIS -DCL_USE_DEPRECATED_OPENCL_2_0_APIS"
make -j4 && make install
```

release version:
```
python Build.py --os=linux --arch=64 --conf=Release --openmp=1 --opencl=1
cd $OPENVX_ROOT/build/Linux/x64/Release
export OPENVX_INSTALL_ROOT=$OPENVX_ROOT/install/Linux/x64/Release
cmake $OPENVX_ROOT -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$OPENVX_INSTALL_ROOT -DBUILD_X64=1  -DEXPERIMENTAL_USE_OPENMP=1 -DEXPERIMENTAL_USE_OPENCL=1 -DEXPERIMENTAL_USE_TARGET=1 -DEXPERIMENTAL_USE_XML=1 -DCMAKE_C_FLAGS="-DCL_USE_DEPRECATED_OPENCL_1_0_APIS -DCL_USE_DEPRECATED_OPENCL_1_1_APIS -DCL_USE_DEPRECATED_OPENCL_2_0_APIS" -DCMAKE_CXX_FLAGS="-DCL_USE_DEPRECATED_OPENCL_1_0_APIS -DCL_USE_DEPRECATED_OPENCL_1_1_APIS -DCL_USE_DEPRECATED_OPENCL_2_0_APIS"
make -j4 && make install
```

# BUILD FOR Windows with VS2015
debug version(without libxml):
```
set OPENVX_ROOT=%cd%
set AMDAPPSDK_PATH=C:\Program Files (x86)\AMD APP SDK\3.0
set VX_OPENCL_INCLUDE_PATH=%AMDAPPSDK_PATH%\include
set VX_OPENCL_LIB_PATH=%AMDAPPSDK_PATH%\lib\x86_64\OpenCL.lib
set OPENVX_INSTALL_ROOT=%OPENVX_ROOT%\install\Win\x64\Debug
set VX_OPENCV_INCLUDE_PATH=D:\opencv-3.1.0\vs2015\install\include
set VX_OPENCV_LIB_PATH=D:\opencv-3.1.0\vs2015\install\x64\vc14\lib\
mkdir build\Win\x64
cd build\Win\x64
cmake %OPENVX_ROOT% -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=%OPENVX_INSTALL_ROOT% -DBUILD_X64=1  -DEXPERIMENTAL_USE_OPENMP=1 -DEXPERIMENTAL_USE_OPENCL=1 -DEXPERIMENTAL_USE_TARGET=1 -DCMAKE_C_FLAGS="-DCL_USE_DEPRECATED_OPENCL_1_0_APIS -DCL_USE_DEPRECATED_OPENCL_1_1_APIS -DCL_USE_DEPRECATED_OPENCL_2_0_APIS" -DCMAKE_CXX_FLAGS="-DCL_USE_DEPRECATED_OPENCL_1_0_APIS -DCL_USE_DEPRECATED_OPENCL_1_1_APIS -DCL_USE_DEPRECATED_OPENCL_2_0_APIS" -G "Visual Studio 14 2015 Win64"
%OPENVX_ROOT%\build\Win\x64\OpenVX.sln
```

# BUILD FOR Android
Set debug/release version by BUILD_CONF environment variable
```
export BUILD_CON=Debug
```
```
export BUILD_CON=Release
```

You should replace the ANDROID_NDK environment variable to your NDK path, also you can modifiy ANDROID_ABI for your Android target.
Other Android build options can be found in cmake_utils/android.toolchain.cmake.
```
git submodule init
export ANDROID_NDK=/home/thlin/Android/android-ndk-r11c
export ANDROID_ABI=armeabi-v7a
export OPENVX_INSTALL_ROOT=$OPENVX_ROOT/install/Android/$ANDROID_ABI/$BUILD_CON
mkdir -p build/Android/$ANDROID_ABI && cd build/Android/$ANDROID_ABI
```
*With prebuilt OpenCL library

Put your prebuilt OpenCL library in the $OPENVX_ROOT/build/Android/$ANDROID_ABI/prebuilt-cl-lib.
You can adb pull prebuilt OpenCL library from your device. (e.g. from path /system/vendor/lib/libOpenCL.so)
```
mkdir $(pwd)/prebuilt-cl-lib
export VX_OPENCL_INCLUDE_PATH=$AMDAPPSDK_PATH/include
export VX_OPENCL_LIB_PATH=$(pwd)/prebuilt-cl-lib/libOpenCL.so
cmake $OPENVX_ROOT -DCMAKE_BUILD_TYPE=$BUILD_CON -DCMAKE_INSTALL_PREFIX=$OPENVX_INSTALL_ROOT -DBUILD_X64=0  -DEXPERIMENTAL_USE_OPENMP=1 -DEXPERIMENTAL_USE_OPENCL=1 -DEXPERIMENTAL_USE_TARGET=1 -DCMAKE_TOOLCHAIN_FILE=$OPENVX_ROOT/cmake_utils/android.toolchain.cmake
make -j4 && make install
```
*With POCL-OpenCL
```
cmake $OPENVX_ROOT -DCMAKE_BUILD_TYPE=$BUILD_CON -DCMAKE_INSTALL_PREFIX=$OPENVX_INSTALL_ROOT -DBUILD_X64=0  -DEXPERIMENTAL_USE_OPENMP=1 -DEXPERIMENTAL_USE_OPENCL=1 -DUSE_POCL_OPENCL=1 -DEXPERIMENTAL_USE_TARGET=1   -DCMAKE_TOOLCHAIN_FILE=$OPENVX_ROOT/cmake_utils/android.toolchain.cmake
make -j4 && make install
```
*With prebuilt OpenCL library on Windows Host using MinGW Make

Set debug/release version by BUILD_CONF environment variable too
```
set BUILD_CON=Debug
```
```
set BUILD_CON=Release
```
```
set OPENVX_ROOT=D:\Android\AndroidWorkspace\OpenVX_Scheduling\my-openvx
set VX_OPENCL_INCLUDE_PATH=D:\Android\AndroidWorkspace\OpenVX_Scheduling\my-openvx\build\armeabi-v7a\cl_include
set VX_OPENCL_LIB_PATH=D:\Android\AndroidWorkspace\OpenVX_Scheduling\my-openvx\build\armeabi-v7a\cl_libs\libOpenCL.so
set ANDROID_NDK=D:\Android\android-ndk-r11c
set ANDROID_ABI=armeabi-v7a
set OPENVX_INSTALL_ROOT=%OPENVX_ROOT%\install\Android\%ANDROID_ABI%\%BUILD_CON%
cmake %OPENVX_ROOT% -DCMAKE_BUILD_TYPE=%BUILD_CON% -DCMAKE_INSTALL_PREFIX=%OPENVX_INSTALL_ROOT% -DBUILD_X64=0  -DEXPERIMENTAL_USE_OPENMP=1 -DEXPERIMENTAL_USE_OPENCL=1 -DEXPERIMENTAL_USE_TARGET=1 -DCMAKE_TOOLCHAIN_FILE=%OPENVX_ROOT%/cmake_utils/android.toolchain.cmake -DANDROID_NDK=%ANDROID_NDK% -DPROCESSOR_COUNT=4 -G "Unix Makefiles" -DVX_OPENCL_INCLUDE_PATH=%VX_OPENCL_INCLUDE_PATH% -DVX_OPENCL_LIB_PATH=%VX_OPENCL_LIB_PATH% -DBUILD_CON=%BUILD_CON% -DANDROID_ABI=%ANDROID_ABI%
make -j8
```

# Build OpenVX application
The OPENVX_ROOT and OPENVX_INSTALL_ROOT environment variable should have been setup upon preceding build process.
Now setup for runtime environment variable.
```
export LD_LIBRARY_PATH=$OPENVX_INSTALL_ROOT/bin:$LD_LIBRARY_PATH
export VX_CL_INCLUDE_DIR=$OPENVX_INSTALL_ROOT/include
export VX_CL_SOURCE_DIR=$OPENVX_ROOT/kernels/opencl
```