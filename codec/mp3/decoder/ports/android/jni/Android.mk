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
                -DHAVE_CONGIG_H
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
               
LOCAL_MODULE        :=mpg123
LOCAL_C_INCLUDES    :=$(LOCAL_PATH)/../../../src/libpmg123 \
                      $(NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.9/libs/armeabi-v7a/include\
					  $(NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.9/include\
					  $(NDK_ROOT)/platforms/android-21/arch-arm/usr/include\
                      $(LOCAL_PATH)/.. \

SRC_BASE := $(LOCAL_PATH)/../../../src/libmpg123
MY_SRC_FILES := \
               $(SRC_BASE)/compat.c \
               $(SRC_BASE)/dct64.c\
			   $(SRC_BASE)/equalizer.c\
               $(SRC_BASE)/feature.c\
               $(SRC_BASE)/format.c\
			   $(SRC_BASE)/frame.c\
			   $(SRC_BASE)/icy.c\
               $(SRC_BASE)/icy2utf8.c\
               $(SRC_BASE)/id3.c\
			   $(SRC_BASE)/index.c\
			   $(SRC_BASE)/layer1.c\
			   $(SRC_BASE)/layer2.c\
               $(SRC_BASE)/layer3.c\
			   $(SRC_BASE)/libmpg123.c\
			   $(SRC_BASE)/ntom.c\
			   $(SRC_BASE)/optimize.c\
			   $(SRC_BASE)/parse.c\
			   $(SRC_BASE)/readers.c\
			   $(SRC_BASE)/stringbuf.c\
			   $(SRC_BASE)/synth.c\
			   $(SRC_BASE)/synth_8bit.c\
			   $(SRC_BASE)/synth_s32.c\
			   $(SRC_BASE)/synth_real.c\
			   $(SRC_BASE)/tabinit.c\
			   $(SRC_BASE)/synth_stereo_neon_s32.S\
			   $(SRC_BASE)/synth_stereo_neon_float.S\
			   $(SRC_BASE)/synth_stereo_neon_accurate.S\
			   $(SRC_BASE)/synth_stereo_neon.S\
			   $(SRC_BASE)/synth_neon_s32.S\
			   $(SRC_BASE)/synth_neon_float.S\
			   $(SRC_BASE)/synth_neon_accurate.S\
			   $(SRC_BASE)/synth_neon.S\
			   $(SRC_BASE)/dct64_neon_float.S\
			   $(SRC_BASE)/dct64_neon.S\
			   $(SRC_BASE)/dct36_neon.S\
			   $(SRC_BASE)/check_neon.S\
			   $(SRC_BASE)/getcpuflags_arm.c\
			   $(SRC_BASE)/synth_arm.S
			   
LOCAL_SRC_FILES   :=$(MY_SRC_FILES)

#LOCAL_LDLIBS = -ldl -llog
include $(BUILD_STATIC_LIBRARY)