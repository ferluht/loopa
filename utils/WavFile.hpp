//=======================================================================
/** @file AudioFile.h
 *  @author Adam Stark
 *  @copyright Copyright (C) 2017  Adam Stark
 *
 * This file is part of the 'AudioFile' library
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
//=======================================================================

#pragma once

#include <iostream>
#include <vector>
#include <assert.h>
#include <string>
#include <A.h>


//=============================================================
/** The different types of audio file, plus some other types to
 * indicate a failure to load a file, or that one hasn't been
 * loaded yet
 */
enum class WavFileFormat
{
    Error,
    NotLoaded,
    Wave,
    Aiff
};

/// @private
template <class T>
class WavFile
{
public:

    //=============================================================
    typedef std::vector<std::vector<T> > AudioBuffer;

    //=============================================================
    /** Constructor */
    WavFile();

    //=============================================================
    /** Loads an audio file from a given file path.
     * @Returns true if the file was successfully loaded
     */
    bool load (std::string filePath);

    /** Saves an audio file to a given file path.
     * @Returns true if the file was successfully saved
     */
    bool save (std::string filePath, WavFileFormat format = WavFileFormat::Wave, int pchunksize=500, int wchunksize=200);

    //=============================================================
    /** @Returns the sample rate */
    uint32_t getSampleRate() const;

    /** @Returns the number of audio channels in the buffer */
    int getNumChannels() const;

    /** @Returns true if the audio file is mono */
    bool isMono() const;

    /** @Returns true if the audio file is stereo */
    bool isStereo() const;

    /** @Returns the bit depth of each sample */
    int getBitDepth() const;

    /** @Returns the number of samples per channel */
    int getNumSamplesPerChannel() const;

    /** @Returns the length in seconds of the audio file based on the number of samples and sample rate */
    double getLengthInSeconds() const;

    /** Prints a summary of the audio file to the console */
    void printSummary() const;

    //=============================================================

    /** Set the audio buffer for this AudioFile by copying samples from another buffer.
     * @Returns true if the buffer was copied successfully.
     */
    bool setAudioBuffer (std::vector<float>& newBuffer, int numChannels, int chunksize=500);

    bool appendAudioBuffer (const float * newBuffer, unsigned int numSamples, int numChannels);

    /** Sets the audio buffer to a given number of channels and number of samples per channel. This will try to preserve
     * the existing audio, adding zeros to any new channels or new samples in a given channel.
     */
    void setAudioBufferSize (int numChannels, int numSamples);

    /** Sets the number of samples per channel in the audio buffer. This will try to preserve
     * the existing audio, adding zeros to new samples in a given channel if the number of samples is increased.
     */
    void setNumSamplesPerChannel (int numSamples);

    /** Sets the number of channels. New channels will have the correct number of samples and be initialised to zero */
    void setNumChannels (int numChannels);

    /** Sets the bit depth for the audio file. If you use the save() function, this bit depth rate will be used */
    void setBitDepth (int numBitsPerSample);

    /** Sets the sample rate for the audio file. If you use the save() function, this sample rate will be used */
    void setSampleRate (uint32_t newSampleRate);

    //=============================================================
    /** A vector of vectors holding the audio samples for the AudioFile. You can
     * access the samples by channel and then by sample index, i.e:
     *
     *      samples[channel][sampleIndex]
     */
    AudioBuffer samples;

    int16_t sampleToSixteenBitInt (T sample);

    void clearAudioBuffer();

    void getArray(std::vector<T> &audio);

private:

    int copied_samples = 0;

    std::vector<uint8_t> fileData;
    int prepared_samples = 0;
    int written_bytes = 0;
    std::ofstream * outputFile;

    //=============================================================
    enum class Endianness
    {
        LittleEndian,
        BigEndian
    };

    //=============================================================
    WavFileFormat determineAudioFileFormat (std::vector<uint8_t>& fileData);
    bool decodeWaveFile (std::vector<uint8_t>& fileData);
    bool decodeAiffFile (std::vector<uint8_t>& fileData);

    //=============================================================
    bool saveToWaveFile (std::string filePath, int pchunksize=500, int wchunksize=200);
    bool saveToAiffFile (std::string filePath);

    //=============================================================
    int32_t fourBytesToInt (std::vector<uint8_t>& source, int startIndex, Endianness endianness = Endianness::LittleEndian);
    int16_t twoBytesToInt (std::vector<uint8_t>& source, int startIndex, Endianness endianness = Endianness::LittleEndian);
    int getIndexOfString (std::vector<uint8_t>& source, std::string s);

    //=============================================================
    T sixteenBitIntToSample (int16_t sample);

    //=============================================================
    uint8_t sampleToSingleByte (T sample);
    T singleByteToSample (uint8_t sample);

    uint32_t getAiffSampleRate (std::vector<uint8_t>& fileData, int sampleRateStartIndex);
    bool tenByteMatch (std::vector<uint8_t>& v1, int startIndex1, std::vector<uint8_t>& v2, int startIndex2);
    void addSampleRateToAiffData (std::vector<uint8_t>& fileData, uint32_t sampleRate);
    T clamp (T v1, T minValue, T maxValue);

    //=============================================================
    void addStringToFileData (std::vector<uint8_t>& fileData, std::string s);
    void addInt32ToFileData (std::vector<uint8_t>& fileData, int32_t i, Endianness endianness = Endianness::LittleEndian);
    void addInt16ToFileData (std::vector<uint8_t>& fileData, int16_t i, Endianness endianness = Endianness::LittleEndian);

    //=============================================================
    bool writeDataToFile (std::vector<uint8_t>& fileData, std::string filePath);

    //=============================================================
    WavFileFormat audioFileFormat;
    uint32_t sampleRate;
    int bitDepth;
};


/// @private
template <class T>
class StreamRecorder {

    std::ofstream * outputFile = nullptr;
    std::vector<uint8_t> fileData;
    int num_channels = 2;
    int bit_depth = 16;
    unsigned int samples_written = 0;

public:

    //=============================================================
    /** Constructor */
    StreamRecorder();

    void openFile(std::string path);
    void writeSamples(T * samples, unsigned int numSamples);
    void closeFile();

    void dumpFiledata();

    //=============================================================
    enum class Endianness
    {
        LittleEndian,
        BigEndian
    };

    //=============================================================
    void addStringToFileData (std::vector<uint8_t>& fileData, std::string s);
    void addInt32ToFileData (std::vector<uint8_t>& fileData, int32_t i, Endianness endianness = Endianness::LittleEndian);
    void addInt16ToFileData (std::vector<uint8_t>& fileData, int16_t i, Endianness endianness = Endianness::LittleEndian);


    uint8_t sampleToSingleByte (T sample);
    int16_t sampleToSixteenBitInt (T sample);

    T clamp(T value, T minValue, T maxValue);

};
