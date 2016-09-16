/*
 *  Copyright (c) 2014 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_COMMON_AUDIO_WAV_FILE_H_
#define WEBRTC_COMMON_AUDIO_WAV_FILE_H_


#include <stdint.h>
#include <cstddef>
#include <string>
#include "include\audiowriter.h"
#include "include\audioreader.h"

#ifndef WEBRTC_ARCH_LITTLE_ENDIAN
#define WEBRTC_ARCH_LITTLE_ENDIAN
#endif


// Simple C++ class for writing 16-bit PCM WAV files. All error handling is
// by calls to RTC_CHECK(), making it unsuitable for anything but debug code.
class WavWriter final : public AudioWriter{
 public:
  // Open a new WAV file for writing.
  WavWriter(const std::string& filename, int sample_rate, size_t num_channels);

  // Close the WAV file, after writing its header.
  ~WavWriter();
  void   Destroy() { delete this; }
  virtual bool   Initialized() { return m_init; }
  // Write additional samples to the file. Each sample is in the range
  // [-32768,32767], and there must be the previously specified number of
  // interleaved channels.
  void WriteSamples(const float* samples, size_t num_samples);
  void WriteSamples(const int16_t* samples, size_t num_samples);

  int SampleRate() const override { return sample_rate_; }
  size_t NumChannels() const override { return num_channels_; }
  size_t NumSamples() const override { return num_samples_; }

 private:
  void Close();
  const int sample_rate_;
  const size_t num_channels_;
  size_t num_samples_;  // Total number of samples written to file.
  FILE* file_handle_;  // Output file, owned by this class
  bool m_init = false;
  WavWriter( const WavWriter& ) = delete;
  void operator=( const WavWriter& ) = delete;
};

// Follows the conventions of WavWriter.
class WavReader final : public AudioReader {
 public:
  // Opens an existing WAV file for reading.
  explicit WavReader(const std::string& filename);

  // Close the WAV file.
  ~WavReader();
  virtual void   Destroy() { delete this; }

  virtual bool   SeekSamples( size_t pos );
  virtual bool   SeekTime( double sec );
  virtual bool   SetSpeed( double times );
  virtual bool   Initialized() { return m_init; }
  // Returns the number of samples read. If this is less than requested,
  // verifies that the end of the file was reached.
  size_t ReadSamples(size_t num_samples, float* samples);
  size_t ReadSamples(size_t num_samples, int16_t* samples);

  int SampleRate() const override { return sample_rate_; }
  size_t NumChannels() const override { return num_channels_; }
  size_t NumSamples() const override { return num_samples_; }
  virtual size_t RemainSamples()const;
 private:
  void Close();
  int sample_rate_;
  size_t num_channels_;
  size_t num_samples_;  // Total number of samples in the file.
  size_t num_samples_remaining_;
  FILE* file_handle_;  // Input file, owned by this class.
  bool m_init = false;
  //RTC_DISALLOW_COPY_AND_ASSIGN(WavReader);
};

#endif  // WEBRTC_COMMON_AUDIO_WAV_FILE_H_
