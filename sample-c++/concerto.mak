
_MODULE:=vxcpp
include $(PRELUDE)
ifeq ($(TARGET_OS),Windows_NT)
# SKIPBUILD:=1
endif
TARGET:=vxcpp
TARGETTYPE:=dsmo
DEFS += VX_EXPORTING
#DEFFILE:=vxcpp.def
IDIRS += $(HOST_ROOT)/$(OPENVX_SRC)/include $(HOST_ROOT)/kernels/c_model $(HOST_ROOT)/kernels/debug $(HOST_ROOT)/kernels/extras
CPPSOURCES:=$(filter-out vx.cpp,$(call all-cpp-files))
STATIC_LIBS:=openvx-debug-lib openvx-c_model-lib openvx-debug_k-lib  openvx-extras_k-lib openvx-helper
SHARED_LIBS:=openvx
include $(FINALE)

_MODULE:=vxcpp_test
include $(PRELUDE)
ifeq ($(TARGET_OS),Windows_NT)
# SKIPBUILD:=1
endif
TARGET:=vxcpp_test
TARGETTYPE:=exe
CPPSOURCES:=vx.cpp
STATIC_LIBS:=openvx-debug-lib openvx-helper
SHARED_LIBS:=vxcpp openvx
SYS_SHARED_LIBS:=$(PLATFORM_LIBS)
include $(FINALE)

