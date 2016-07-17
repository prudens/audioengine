# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# 

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_CPPFLAGS := -std=c++11\
                -DHAVE_CONFIG_H
				-DNOMINMAX\
				-DNO_TCMALLOC\
				-DALLOCATOR_SHIM\
				-D__STD_C\
				-D__STDC_FORMAT_MACROS\
				-D__STDC_CONSTANT_MACROS\
				-D__UCLIBC__\
				-frtti\
				-fno-omit-frame-pointer\
				-mthumb\
				-fexceptions\
				-mfloat-abi=softfp\
				-mfpu=neon\
				-fpermissive\
                -DOPT_MULTI\
                -DOPT_GENERIC\
				-DOPT_NEON\
				-DOPT_ARM\
				-O2
				
LOCAL_CFLAGS := \
                -DHAVE_CONFIG_H
				-DNOMINMAX\
				-DNO_TCMALLOC\
				-DALLOCATOR_SHIM\
				-D__STD_C\
				-D__STDC_FORMAT_MACROS\
				-D__STDC_CONSTANT_MACROS\
				-D__UCLIBC__\
				-fno-omit-frame-pointer\
				-mthumb\
				-mfloat-abi=softfp\
				-mfpu=neon\
                -DOPT_MULTI\
                -DOPT_GENERIC\
				-DOPT_NEON\
				-DOPT_ARM\
				-O2

ifndef $(NDK_ROOT)
NDK_ROOT            :=C:/ProgramData/Microsoft/AndroidNDK/android-ndk-r10e#
endif
               
LOCAL_MODULE        :=mp3lame
LOCAL_C_INCLUDES    :=$(LOCAL_PATH)/../../../libmp3lame \
                      $(LOCAL_PATH)/../../../include \
					  $(LOCAL_PATH)/.. \
                      $(NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.9/libs/armeabi-v7a/include\
					  $(NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.9/include\
					  $(NDK_ROOT)/platforms/android-21/arch-arm/usr/include\
                      $(LOCAL_PATH)/.. \

SRC_BASE := $(LOCAL_PATH)/../../../libmp3lame
MY_SRC_FILES := \
			   $(SRC_BASE)/bitstream.c\
			   $(SRC_BASE)/encoder.c\
			   $(SRC_BASE)/fft.c\
			   $(SRC_BASE)/gain_analysis.c\
			   $(SRC_BASE)/id3tag.c\
			   $(SRC_BASE)/lame.c\
			   $(SRC_BASE)/newmdct.c\
			   $(SRC_BASE)/presets.c\
			   $(SRC_BASE)/psymodel.c\
			   $(SRC_BASE)/quantize.c\
			   $(SRC_BASE)/quantize_pvt.c\
			   $(SRC_BASE)/reservoir.c\
			   $(SRC_BASE)/set_get.c\
			   $(SRC_BASE)/tables.c\
			   $(SRC_BASE)/takehiro.c\
			   $(SRC_BASE)/util.c\
			   $(SRC_BASE)/vbrquantize.c\
			   $(SRC_BASE)/VbrTag.c\
			   $(SRC_BASE)/version.c\
			   $(SRC_BASE)/vector/xmm_quantize_sub.c\

			   
			   
LOCAL_SRC_FILES   :=$(MY_SRC_FILES)

#LOCAL_LDLIBS = -ldl -llog
include $(BUILD_STATIC_LIBRARY)