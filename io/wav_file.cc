/*
 *  Copyright (c) 2014 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "wav_file.h"
#include <algorithm>
#include <cstdio>
#include <limits>
#include <sstream>
#include <cassert>
#include "wav_header.h"
#include "base/audio_util.h"

#define RTC_CHECK(p) assert(p) 
#define RTC_CHECK_EQ(p,q) assert(p == q)
// We write 16-bit PCM WAV files.
static const WavFormat kWavFormat = kWavFormatPcm;
static const size_t kBytesPerSample = 2;

// Doesn't take ownership of the file handle and won't close it.
class ReadableWavFile : public ReadableWav {
 public:
  explicit ReadableWavFile(FILE* file) : file_(file) {}
  virtual size_t Read(void* buf, size_t num_bytes) {
    return fread(buf, 1, num_bytes, file_);
  }

 private:
  FILE* file_;
};


WavReader::WavReader(const std::string& filename)
    : file_handle_(fopen(filename.c_str(), "rb")) {
 // RTC_CHECK(file_handle_) << "Could not open wav file for reading.";

  ReadableWavFile readable(file_handle_);
  WavFormat format = kWavFormatPcm;
  size_t bytes_per_sample = 0;
  RTC_CHECK(ReadWavHeader(&readable, &num_channels_, &sample_rate_, &format,
                          &bytes_per_sample, &num_samples_));
  num_samples_remaining_ = num_samples_;
  RTC_CHECK_EQ(kWavFormat, format);
  RTC_CHECK_EQ(kBytesPerSample, bytes_per_sample);
  m_init = true;
}

WavReader::~WavReader() {
  Close();
}

size_t WavReader::ReadSamples(size_t num_samples, int16_t* samples) {
#ifndef WEBRTC_ARCH_LITTLE_ENDIAN
#error "Need to convert samples to big-endian when reading from WAV file"
#endif
    if (!m_init)
    {
        return 0;
    }
  // There could be metadata after the audio; ensure we don't read it.
  num_samples = std::min(num_samples, num_samples_remaining_);
  const size_t read =
      fread(samples, sizeof(*samples), num_samples, file_handle_);
  // If we didn't read what was requested, ensure we've reached the EOF.
  RTC_CHECK(read == num_samples || feof(file_handle_));
//  RTC_CHECK_LE(read, num_samples_remaining_);
  num_samples_remaining_ -= read;
  return read;
}

size_t WavReader::ReadSamples(size_t num_samples, float* samples)
{
    if ( !m_init )
    {
        return 0;
    }
  static const size_t kChunksize = 4096 / sizeof(uint16_t);
  size_t read = 0;
  for (size_t i = 0; i < num_samples; i += kChunksize) {
    int16_t isamples[kChunksize];
    size_t chunk = std::min(kChunksize, num_samples - i);
    chunk = ReadSamples(chunk, isamples);
    for (size_t j = 0; j < chunk; ++j)
      samples[i + j] = isamples[j];
    read += chunk;
  }
  num_samples_remaining_ -= read;
  return read;
}

void WavReader::Close() {
  RTC_CHECK_EQ(0, fclose(file_handle_));
  file_handle_ = NULL;
}

size_t WavReader::RemainSamples() const
{
    return num_samples_remaining_;
}

bool WavReader::SeekSamples( size_t pos )
{
    if ( !m_init )
    {
        return 0;
    }
    if (file_handle_)
    {
        fseek( file_handle_, 0, SEEK_SET );
    }
    else
    {
        return false;
    }
    ReadableWavFile readable( file_handle_ );
    WavFormat format = kWavFormatPcm;
    size_t bytes_per_sample = 0;
    RTC_CHECK( ReadWavHeader( &readable, &num_channels_, &sample_rate_, &format,
        &bytes_per_sample, &num_samples_ ) );
    num_samples_remaining_ = num_samples_;
    RTC_CHECK_EQ( kWavFormat, format );
    RTC_CHECK_EQ( kBytesPerSample, bytes_per_sample );
    fseek(file_handle_,pos*2,SEEK_CUR);
    num_samples_remaining_ -= pos;

    return true;
}

bool WavReader::SeekTime( double sec )
{
    return SeekSamples(static_cast<size_t>(sample_rate_*sec*num_channels_));
}

bool WavReader::SetSpeed( double /*times*/ )
{
    // fix me
    return false;
}

WavWriter::WavWriter(const std::string& filename, int sample_rate,
                     size_t num_channels)
    : sample_rate_(sample_rate),
      num_channels_(num_channels),
      num_samples_(0),
      file_handle_(fopen(filename.c_str(), "wb")) {
//  RTC_CHECK(file_handle_) << "Could not open wav file for writing.";
  RTC_CHECK(CheckWavParameters(num_channels_, sample_rate_, kWavFormat,
                               kBytesPerSample, num_samples_));

  // Write a blank placeholder header, since we need to know the total number
  // of samples before we can fill in the real data.
  static const uint8_t blank_header[kWavHeaderSize] = {0};
  RTC_CHECK_EQ(1u, fwrite(blank_header, kWavHeaderSize, 1, file_handle_));
  m_init = true;
}

WavWriter::~WavWriter() {
  Close();
}

void WavWriter::WriteSamples(const int16_t* samples, size_t num_samples) {
#ifndef WEBRTC_ARCH_LITTLE_ENDIAN
#error "Need to convert samples to little-endian when writing to WAV file"
#endif
    if (!m_init)
    {
        return;
    }
  const size_t written =
      fwrite(samples, sizeof(*samples), num_samples, file_handle_);
  RTC_CHECK_EQ(num_samples, written);
  num_samples_ += written;
  RTC_CHECK(num_samples_ >= written);  // detect size_t overflow
}

void WavWriter::WriteSamples(const float* samples, size_t num_samples) 
{
    if ( !m_init )
    {
        return;
    }
  static const size_t kChunksize = 4096 / sizeof(uint16_t);
  for (size_t i = 0; i < num_samples; i += kChunksize) {
    int16_t isamples[kChunksize];
    const size_t chunk = std::min(kChunksize, num_samples - i);
    FloatS16ToS16(samples + i, chunk, isamples);
    WriteSamples(isamples, chunk);
  }
}

void WavWriter::Close() {
  RTC_CHECK_EQ(0, fseek(file_handle_, 0, SEEK_SET));
  uint8_t header[kWavHeaderSize];
  WriteWavHeader(header, num_channels_, sample_rate_, kWavFormat,
                 kBytesPerSample, num_samples_);
  RTC_CHECK_EQ(1u, fwrite(header, kWavHeaderSize, 1, file_handle_));
  RTC_CHECK_EQ(0, fclose(file_handle_));
  file_handle_ = NULL;
}