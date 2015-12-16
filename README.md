# Git clone
You should replace VX_OPENCL_INCLUDE_PATH and VX_OPENCL_LIB_PATH environment variables to your OpenCL path.
```
git clone git@bitbucket.org:eric100lin/my-openvx-1.0.1-fussion.git
cd My-OpenVX-1.0.1-fussion

export OPENVX_ROOT=$(pwd)
export AMDAPPSDK_PATH=/home/thlin/AMDAPPSDK-3.0-0-Beta
export VX_OPENCL_INCLUDE_PATH=$AMDAPPSDK_PATH/include
export VX_OPENCL_LIB_PATH=$AMDAPPSDK_PATH/lib/x86_64/sdk
```

# BUILD FOR AMD64 Desktop
debug version:
```
python Build.py --os=linux --arch=64 --conf=Debug --openmp=1 --opencl=1
cd $OPENVX_ROOT/build/Linux/x64/Debug
export OPENVX_INSTALL_ROOT=$OPENVX_ROOT/install/Linux/x64/Debug
cmake $OPENVX_ROOT -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=$OPENVX_INSTALL_ROOT -DBUILD_X64=1  -DEXPERIMENTAL_USE_OPENMP=1 -DEXPERIMENTAL_USE_OPENCL=1 -DEXPERIMENTAL_USE_TARGET=1 -DCMAKE_C_FLAGS="-DCL_USE_DEPRECATED_OPENCL_1_0_APIS -DCL_USE_DEPRECATED_OPENCL_1_1_APIS -DCL_USE_DEPRECATED_OPENCL_2_0_APIS" -DCMAKE_CXX_FLAGS="-DCL_USE_DEPRECATED_OPENCL_1_0_APIS -DCL_USE_DEPRECATED_OPENCL_1_1_APIS -DCL_USE_DEPRECATED_OPENCL_2_0_APIS"
make -j4 && make install
```

release version:
```
python Build.py --os=linux --arch=64 --conf=Release --openmp=1 --opencl=1
cd $OPENVX_ROOT/build/Linux/x64/Release
export OPENVX_INSTALL_ROOT=$OPENVX_ROOT/install/Linux/x64/Release
cmake $OPENVX_ROOT -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$OPENVX_INSTALL_ROOT -DBUILD_X64=1  -DEXPERIMENTAL_USE_OPENMP=1 -DEXPERIMENTAL_USE_OPENCL=1 -DEXPERIMENTAL_USE_TARGET=1 -DCMAKE_C_FLAGS="-DCL_USE_DEPRECATED_OPENCL_1_0_APIS -DCL_USE_DEPRECATED_OPENCL_1_1_APIS -DCL_USE_DEPRECATED_OPENCL_2_0_APIS" -DCMAKE_CXX_FLAGS="-DCL_USE_DEPRECATED_OPENCL_1_0_APIS -DCL_USE_DEPRECATED_OPENCL_1_1_APIS -DCL_USE_DEPRECATED_OPENCL_2_0_APIS"
make -j4 && make install
```

# BUILD FOR Android with POCL-OpenCL
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
export ANDROID_NDK=/home/thlin/Android/android-ndk-r10e
export ANDROID_ABI=armeabi-v7a
export OPENVX_INSTALL_ROOT=$OPENVX_ROOT/install/Android/$ANDROID_ABI/$BUILD_CON
mkdir -p build/Android/$ANDROID_ABI && cd build/Android/$ANDROID_ABI
cmake $OPENVX_ROOT -DCMAKE_BUILD_TYPE=$BUILD_CON -DCMAKE_INSTALL_PREFIX=$OPENVX_INSTALL_ROOT -DBUILD_X64=0  -DEXPERIMENTAL_USE_OPENMP=1 -DEXPERIMENTAL_USE_OPENCL=1 -DUSE_POCL_OPENCL=1 -DEXPERIMENTAL_USE_TARGET=1   -DCMAKE_TOOLCHAIN_FILE=$OPENVX_ROOT/cmake_utils/android.toolchain.cmake
make -j4 && make install
```

# Build OpenVX application
The OPENVX_ROOT and OPENVX_INSTALL_ROOT environment variable should have been setup upon preceding build process.
Now setup for runtime environment variable.
```
export LD_LIBRARY_PATH=$OPENVX_INSTALL_ROOT/bin:$LD_LIBRARY_PATH
export VX_CL_INCLUDE_DIR=$OPENVX_INSTALL_ROOT/include
export VX_CL_SOURCE_DIR=$OPENVX_ROOT/kernels/opencl
```