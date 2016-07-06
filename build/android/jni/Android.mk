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

LOCAL_CFLAGS := -std=c++11\
                -DWEBRTC_ANDROID\
				-DWEBRTC_POSIX\
				-DWEBRTC_ARCH_ARM_V7A\
				-DWEBRTC_LINUX\
				-DNOMINMAX\
				-DENABLE_WEBRTC=1\
				-DNO_TCMALLOC\
				-DALLOCATOR_SHIM\
				-D__STD_C\
				-DWEBRTC_NS_FIXED\
				-D__STDC_FORMAT_MACROS\
				-D__STDC_CONSTANT_MACROS\
				-D__UCLIBC__\
				-DWEBRTC_DETECT_NEON\
				-frtti\
				-fno-omit-frame-pointer\
				-mthumb\
				-fexceptions\
				-mfloat-abi=softfp\
				-mfpu=neon\
				-fpermissive
                
LOCAL_MODULE        :=audio_processing
MY_CLIENT_PATH      :=../../src
MY_SRC_PATH         :=$(MY_CLIENT_PATH)##
LOCAL_CPP_EXTENSION :=.cpp .cc .c
LOCAL_C_INCLUDES    :=$(LOCAL_PATH)/../../src/ \
                      C:/ProgramData/Microsoft/AndroidNDK64/android-ndk-r10e/sources/cxx-stl/gnu-libstdc++/4.9/libs/armeabi-v7a/include\
					  C:/ProgramData/Microsoft\AndroidNDK64/android-ndk-r10e/platforms/android-21/arch-arm/usr/include\
					  $(LOCAL_PATH)/../../src/webrtc/common_audio/signal_processing/include\
					  $(LOCAL_PATH)/../../src/webrtc/system_wrappers/source/ \
					  $(LOCAL_PATH)/../../src/webrtc/modules/audio_coding/codecs/isac/main/include


SRC_WEBRTC_BASE     :=$(MY_SRC_PATH)/webrtc/base

SRC_WEBRTC_COMMON_AUDIO :=$(MY_SRC_PATH)/webrtc/common_audio
SRC_WEBRTC_SYSTEM_SWRAPPERS :=$(MY_SRC_PATH)/webrtc/system_wrappers/source
SRC_WEBRTC_ISAC :=$(MY_SRC_PATH)/webrtc/modules/audio_coding/codecs/isac
SRC_WEBRTC_AUDIO_PROCESSING :=$(MY_SRC_PATH)/webrtc/modules/audio_processing


MY_SRC_FILES :=$(SRC_WEBRTC_BASE)/timeutils.cc\
               $(SRC_WEBRTC_BASE)/thread_checker_impl.cc\
			   $(SRC_WEBRTC_BASE)/systeminfo.cc\
			   $(SRC_WEBRTC_BASE)/stringutils.cc\
			   $(SRC_WEBRTC_BASE)/stringencode.cc\
			   $(SRC_WEBRTC_BASE)/platform_thread.cc\
			   $(SRC_WEBRTC_BASE)/platform_file.cc\
			   $(SRC_WEBRTC_BASE)/event_tracer.cc\
			   $(SRC_WEBRTC_BASE)/criticalsection.cc\
			   $(SRC_WEBRTC_BASE)/common.cc\
			   $(SRC_WEBRTC_BASE)/checks.cc\
			   $(SRC_WEBRTC_BASE)/base_logging.cc\
			   $(SRC_WEBRTC_BASE)/base_event.cc\
			   $(SRC_WEBRTC_COMMON_AUDIO)/vad/vad.cc\
			   $(SRC_WEBRTC_COMMON_AUDIO)/vad/vad_core.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/vad/vad_filterbank.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/vad/vad_sp.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/vad/webrtc_vad.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/resampler/push_resampler.cc\
			   $(SRC_WEBRTC_COMMON_AUDIO)/resampler/push_sinc_resampler.cc\
			   $(SRC_WEBRTC_COMMON_AUDIO)/resampler/resampler.cc\
			   $(SRC_WEBRTC_COMMON_AUDIO)/resampler/sinc_resampler.cc\
			   $(SRC_WEBRTC_COMMON_AUDIO)/resampler/sinc_resampler_neon.cc\
			   $(SRC_WEBRTC_COMMON_AUDIO)/resampler/sinusoidal_linear_chirp_source.cc\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/auto_corr_to_refl_coef.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/auto_correlation.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/complex_bit_reverse.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/complex_fft.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/copy_set_operations.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/cross_correlation.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/cross_correlation_neon.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/division_operations.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/dot_product_with_scale.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/downsample_fast.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/downsample_fast_neon.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/energy.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/filter_ar.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/filter_ar_fast_q12.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/filter_ma_fast_q12.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/get_hanning_window.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/get_scaling_square.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/ilbc_specific_functions.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/levinson_durbin.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/lpc_to_refl_coef.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/min_max_operations.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/min_max_operations_neon.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/randomization_functions.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/real_fft.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/refl_coef_to_lpc.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/resample.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/resample_48khz.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/resample_by_2.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/resample_by_2_internal.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/resample_fractional.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/spl_init.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/spl_splitting_filter.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/spl_sqrt.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/spl_sqrt_floor.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/sqrt_of_one_minus_x_squared.c\
			   $(SRC_WEBRTC_COMMON_AUDIO)/signal_processing/vector_scaling_operations.c\
			   $(SRC_WEBRTC_SYSTEM_SWRAPPERS)/aligned_malloc.cc\
			   $(SRC_WEBRTC_SYSTEM_SWRAPPERS)/cpu-features.c\
			   $(SRC_WEBRTC_SYSTEM_SWRAPPERS)/cpu_features.cc\
			   $(SRC_WEBRTC_SYSTEM_SWRAPPERS)/cpu_features_android.c\
			   $(SRC_WEBRTC_SYSTEM_SWRAPPERS)/cpu_info.cc\
			   $(SRC_WEBRTC_SYSTEM_SWRAPPERS)/event.cc\
			   $(SRC_WEBRTC_SYSTEM_SWRAPPERS)/event_timer_posix.cc\
			   $(SRC_WEBRTC_SYSTEM_SWRAPPERS)/field_trial_default.cc\
			   $(SRC_WEBRTC_SYSTEM_SWRAPPERS)/file_impl.cc\
			   $(SRC_WEBRTC_SYSTEM_SWRAPPERS)/logging.cc\
			   $(SRC_WEBRTC_SYSTEM_SWRAPPERS)/metrics_default.cc\
			   $(SRC_WEBRTC_SYSTEM_SWRAPPERS)/rw_lock.cc\
			   $(SRC_WEBRTC_SYSTEM_SWRAPPERS)/rw_lock_posix.cc\
			   $(SRC_WEBRTC_SYSTEM_SWRAPPERS)/sleep.cc\
			   $(SRC_WEBRTC_SYSTEM_SWRAPPERS)/sort.cc\
			   $(SRC_WEBRTC_SYSTEM_SWRAPPERS)/tick_util.cc\
			   $(SRC_WEBRTC_SYSTEM_SWRAPPERS)/trace_impl.cc\
			   $(SRC_WEBRTC_SYSTEM_SWRAPPERS)/trace_posix.cc\
			   $(SRC_WEBRTC_ISAC)/../audio_encoder.cc\
			   $(SRC_WEBRTC_ISAC)/../audio_encoder.cc\
			   $(SRC_WEBRTC_ISAC)/main/source/arith_routines.c\
			   $(SRC_WEBRTC_ISAC)/main/source/arith_routines_hist.c\
			   $(SRC_WEBRTC_ISAC)/main/source/arith_routines_logist.c\
			   $(SRC_WEBRTC_ISAC)/main/source/audio_decoder_isac.cc\
			   $(SRC_WEBRTC_ISAC)/main/source/audio_encoder_isac.cc\
			   $(SRC_WEBRTC_ISAC)/main/source/bandwidth_estimator.c\
			   $(SRC_WEBRTC_ISAC)/main/source/decode.c\
			   $(SRC_WEBRTC_ISAC)/main/source/decode_bwe.c\
			   $(SRC_WEBRTC_ISAC)/main/source/encode.c\
			   $(SRC_WEBRTC_ISAC)/main/source/encode_lpc_swb.c\
			   $(SRC_WEBRTC_ISAC)/main/source/entropy_coding.c\
			   $(SRC_WEBRTC_ISAC)/main/source/fft.c\
			   $(SRC_WEBRTC_ISAC)/main/source/filter_functions.c\
			   $(SRC_WEBRTC_ISAC)/main/source/filterbank_tables.c\
			   $(SRC_WEBRTC_ISAC)/main/source/filterbanks.c\
			   $(SRC_WEBRTC_ISAC)/main/source/intialize.c\
			   $(SRC_WEBRTC_ISAC)/main/source/isac.c\
			   $(SRC_WEBRTC_ISAC)/main/source/lattice.c\
			   $(SRC_WEBRTC_ISAC)/locked_bandwidth_info.cc\
			   $(SRC_WEBRTC_ISAC)/main/source/lpc_analysis.c\
			   $(SRC_WEBRTC_ISAC)/main/source/lpc_gain_swb_tables.c\
			   $(SRC_WEBRTC_ISAC)/main/source/lpc_shape_swb12_tables.c\
			   $(SRC_WEBRTC_ISAC)/main/source/lpc_shape_swb16_tables.c\
			   $(SRC_WEBRTC_ISAC)/main/source/lpc_tables.c\
			   $(SRC_WEBRTC_ISAC)/main/source/pitch_estimator.c\
			   $(SRC_WEBRTC_ISAC)/main/source/pitch_filter.c\
			   $(SRC_WEBRTC_ISAC)/main/source/pitch_gain_tables.c\
			   $(SRC_WEBRTC_ISAC)/main/source/pitch_lag_tables.c\
			   $(SRC_WEBRTC_ISAC)/main/source/spectrum_ar_model_tables.c\
			   $(SRC_WEBRTC_ISAC)/main/source/transform.c\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/aec/aec_core.c\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/aec/aec_core_neon.c\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/aec/aec_rdft.c\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/aec/aec_rdft_neon.c\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/aec/aec_resampler.c\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/aec/echo_cancellation.c\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/aecm/aecm_core.c\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/aecm/aecm_core_c.c\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/aecm/aecm_core_neon.c\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/aecm/echo_control_mobile.c\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/agc/agc.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/agc/agc_manager_direct.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/agc/legacy/analog_agc.c\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/agc/legacy/digital_agc.c\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/agc/histogram.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/agc/utility.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/beamformer/array_util.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/beamformer/covariance_matrix_generator.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/beamformer/nonlinear_beamformer.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/intelligibility/intelligibility_enhancer.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/intelligibility/intelligibility_utils.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/logging/aec_logging_file_handling.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/vad/gmm.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/vad/pitch_based_vad.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/vad/pitch_internal.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/vad/pole_zero_filter.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/vad/standalone_vad.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/vad/vad_audio_proc.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/vad/vad_circular_buffer.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/vad/voice_activity_detector.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/utility/delay_estimator.c\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/utility/delay_estimator_wrapper.c\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/ns/noise_suppression.c\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/ns/noise_suppression_x.c\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/ns/ns_core.c\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/ns/nsx_core.c\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/ns/nsx_core_c.c\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/ns/nsx_core_neon.c\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/transient/click_annotate.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/transient/file_utils.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/transient/moving_moments.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/transient/transient_detector.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/transient/transient_suppressor.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/transient/wpd_node.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/transient/wpd_tree.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/audio_buffer.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/audio_processing_impl.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/echo_cancellation_impl.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/echo_control_mobile_impl.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/gain_control_for_experimental_agc.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/gain_control_impl.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/high_pass_filter_impl.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/level_estimator_impl.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/noise_suppression_impl.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/processing_component.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/rms_level.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/splitting_filter.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/three_band_filter_bank.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/typing_detection.cc\
			   $(SRC_WEBRTC_AUDIO_PROCESSING)/voice_detection_impl.cc\







LOCAL_SRC_FILES   :=$(MY_SRC_FILES)

#LOCAL_LDLIBS = -ldl -llog
include $(BUILD_STATIC_LIBRARY)