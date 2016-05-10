CC = gcc
CXX = g++
AR = ar
LD = g++
LDFLAGS = -O3
CFLAGS = -g -Wall
CXXFLAGS = -g -Wall #-std=gnu++0x

PROGRAM_NAME = ./vx_cl-node_graph
OPENCV_PATH = /home/thlin/opencv-2.4.9
SOURCE_PATH = ./
INCLUDE_PATH = -I$(OPENVX_INSTALL_ROOT)/include \
			   -I$(OPENVX_ROOT)/debug \
			   -I$(OPENCV_PATH)/sources/include
OPENVX_LIB = $(OPENVX_INSTALL_ROOT)/bin
LD_LIBS = -L$(OPENVX_LIB) -lopenvx \
	-L$(OPENCV_PATH)/build/lib \
	-lopencv_core -lopencv_highgui -lopencv_imgproc \
	-lopencv_objdetect -lopencv_ml -lopencv_video -lopencv_calib3d \
	-lopencv_ocl
OBJ_DIR = .objs
Cmysources = $(wildcard $(SOURCE_PATH:=*.c))
CXXmysources = $(wildcard $(SOURCE_PATH:=*.cpp))
PROGRAM_OBJS := ${Cmysources:.c=.o} ${CXXmysources:.cpp=.o}
PROGRAM_OBJS := $(addprefix $(OBJ_DIR)/,$(PROGRAM_OBJS))

all: before_compile COMPILING after_compile

before_compile: 
	test -d $(OBJ_DIR) || mkdir -p $(OBJ_DIR)

after_compile:

COMPILING: $(PROGRAM_OBJS)
	$(LD) -o $(PROGRAM_NAME) $(addprefix $(OBJ_DIR)/, $(notdir $(PROGRAM_OBJS))) $(LD_LIBS)

$(OBJ_DIR)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE_PATH) -c $< -o $(OBJ_DIR)/$(@F)

$(OBJ_DIR)/%.o : %.c
	$(CC) $(CFLAGS) $(INCLUDE_PATH) -c $< -o $(OBJ_DIR)/$(@F)

clean: 
	rm -fr $(OBJ_DIR)
	rm $(PROGRAM_NAME)

.PHONY: before_compile after_compile
