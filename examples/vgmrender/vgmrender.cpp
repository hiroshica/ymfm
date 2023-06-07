//
// Simple vgm renderer.
//
// Leverages em_inflate tiny inflater from https://github.com/emmanuel-marty/em_inflate
//
// Compile with:
//
//   g++ --std=c++14 -I../../src vgmrender.cpp em_inflate.cpp ../../src/ymfm_misc.cpp ../../src/ymfm_opl.cpp ../../src/ymfm_opm.cpp ../../src/ymfm_opn.cpp ../../src/ymfm_adpcm.cpp ../../src/ymfm_pcm.cpp ../../src/ymfm_ssg.cpp -o vgmrender.exe
//
// or:
//
//   clang++ --std=c++14 -I../../src vgmrender.cpp em_inflate.cpp ../../src/ymfm_misc.cpp ../../src/ymfm_opl.cpp ../../src/ymfm_opm.cpp ../../src/ymfm_opn.cpp ../../src/ymfm_adpcm.cpp ../../src/ymfm_pcm.cpp ../../src/ymfm_ssg.cpp -o vgmrender.exe
//
// or:
//
//   cl -I..\..\src vgmrender.cpp em_inflate.cpp ..\..\src\ymfm_misc.cpp ..\..\src\ymfm_opl.cpp ..\..\src\ymfm_opm.cpp ..\..\src\ymfm_opn.cpp ..\..\src\ymfm_adpcm.cpp ..\..\src\ymfm_pcm.cpp ..\..\src\ymfm_ssg.cpp /Od /Zi /std:c++14 /EHsc
//

#define _CRT_SECURE_NO_WARNINGS

#include <cmath>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <list>
#include <string>

#include "em_inflate.h"
#include "vgm_chip_base.h"
#include "vgmplayer.h"

//-------------------------------------------------
//  write_wav - write a WAV file from the provided
//  stereo data
//-------------------------------------------------

int write_wav(char const *filename, uint32_t output_rate, std::vector<int32_t> &wav_buffer_src)
{
	// determine normalization parameters
	int32_t max_scale = 0;
	for (size_t index = 0; index < wav_buffer_src.size(); index++)
	{
		int32_t absval = std::abs(wav_buffer_src[index]);
		max_scale = std::max(max_scale, absval);
	}

	// warn if only silence was detected (and also avoid divide by zero)
	if (max_scale == 0)
	{
		fprintf(stderr, "The WAV file data will only contain silence.\n");
		max_scale = 1;
	}

	// now convert
	std::vector<int16_t> wav_buffer(wav_buffer_src.size());
	for (size_t index = 0; index < wav_buffer_src.size(); index++)
		wav_buffer[index] = wav_buffer_src[index] * 26000 / max_scale;

	// write the WAV file
	FILE *out = fopen(filename, "wb");
	if (out == nullptr)
	{
		fprintf(stderr, "Error creating output file '%s'\n", filename);
		return 6;
	}

	// write the 'RIFF' header
	if (fwrite("RIFF", 1, 4, out) != 4)
	{
		fprintf(stderr, "Error writing to output file\n");
		return 7;
	}

	// write the total size
	uint32_t total_size = 48 + wav_buffer.size() * 2 - 8;
	uint8_t wavdata[4];
	wavdata[0] = total_size >> 0;
	wavdata[1] = total_size >> 8;
	wavdata[2] = total_size >> 16;
	wavdata[3] = total_size >> 24;
	if (fwrite(wavdata, 1, 4, out) != 4)
	{
		fprintf(stderr, "Error writing to output file\n");
		return 7;
	}

	// write the 'WAVE' type
	if (fwrite("WAVE", 1, 4, out) != 4)
	{
		fprintf(stderr, "Error writing to output file\n");
		return 7;
	}

	// write the 'fmt ' tag
	if (fwrite("fmt ", 1, 4, out) != 4)
	{
		fprintf(stderr, "Error writing to output file\n");
		return 7;
	}

	// write the format length
	wavdata[0] = 16;
	wavdata[1] = 0;
	wavdata[2] = 0;
	wavdata[3] = 0;
	if (fwrite(wavdata, 1, 4, out) != 4)
	{
		fprintf(stderr, "Error writing to output file\n");
		return 7;
	}

	// write the format (PCM)
	wavdata[0] = 1;
	wavdata[1] = 0;
	if (fwrite(wavdata, 1, 2, out) != 2)
	{
		fprintf(stderr, "Error writing to output file\n");
		return 7;
	}

	// write the channels
	wavdata[0] = 2;
	wavdata[1] = 0;
	if (fwrite(wavdata, 1, 2, out) != 2)
	{
		fprintf(stderr, "Error writing to output file\n");
		return 7;
	}

	// write the sample rate
	wavdata[0] = output_rate >> 0;
	wavdata[1] = output_rate >> 8;
	wavdata[2] = output_rate >> 16;
	wavdata[3] = output_rate >> 24;
	if (fwrite(wavdata, 1, 4, out) != 4)
	{
		fprintf(stderr, "Error writing to output file\n");
		return 7;
	}

	// write the bytes/second
	uint32_t bps = output_rate * 2 * 2;
	wavdata[0] = bps >> 0;
	wavdata[1] = bps >> 8;
	wavdata[2] = bps >> 16;
	wavdata[3] = bps >> 24;
	if (fwrite(wavdata, 1, 4, out) != 4)
	{
		fprintf(stderr, "Error writing to output file\n");
		return 7;
	}

	// write the block align
	wavdata[0] = 4;
	wavdata[1] = 0;
	if (fwrite(wavdata, 1, 2, out) != 2)
	{
		fprintf(stderr, "Error writing to output file\n");
		return 7;
	}

	// write the bits/sample
	wavdata[0] = 16;
	wavdata[1] = 0;
	if (fwrite(wavdata, 1, 2, out) != 2)
	{
		fprintf(stderr, "Error writing to output file\n");
		return 7;
	}

	// write the 'data' tag
	if (fwrite("data", 1, 4, out) != 4)
	{
		fprintf(stderr, "Error writing to output file\n");
		return 7;
	}

	// write the data length
	uint32_t datalen = wav_buffer.size() * 2;
	wavdata[0] = datalen >> 0;
	wavdata[1] = datalen >> 8;
	wavdata[2] = datalen >> 16;
	wavdata[3] = datalen >> 24;
	if (fwrite(wavdata, 1, 4, out) != 4)
	{
		fprintf(stderr, "Error writing to output file\n");
		return 7;
	}

	// write the data
	if (fwrite(&wav_buffer[0], 1, datalen, out) != datalen)
	{
		fprintf(stderr, "Error writing to output file\n");
		return 7;
	}
	fclose(out);
	return 0;
}

//-------------------------------------------------
//  main - program entry point
//-------------------------------------------------

int main(int argc, char *argv[])
{
	char const *filename = nullptr;
	char const *outfilename = nullptr;
	int output_rate = 44100;

	// parse command line
	bool argerr = false;
	for (int arg = 1; arg < argc; arg++)
	{
		char const *curarg = argv[arg];
		if (*curarg == '-')
		{
			if (strcmp(curarg, "-o") == 0 || strcmp(curarg, "--output") == 0)
				outfilename = argv[++arg];
			else if (strcmp(curarg, "-r") == 0 || strcmp(curarg, "--samplerate") == 0)
				output_rate = atoi(argv[++arg]);
			else
			{
				fprintf(stderr, "Unknown argument: %s\n", curarg);
				argerr = true;
			}
		}
		else
			filename = curarg;
	}

	// if invalid syntax, show usage
	if (argerr || filename == nullptr || outfilename == nullptr)
	{
		fprintf(stderr, "Usage: vgmrender <inputfile> -o <outputfile> [-r <rate>]\n");
		return 1;
	}

	// attempt to read the file
	FILE *file = fopen(filename, "rb");
	if (file == nullptr)
	{
		fprintf(stderr, "Error opening file '%s'\n", filename);
		return 2;
	}

	// get the length and create a buffer
	fseek(file, 0, SEEK_END);
	uint32_t size = ftell(file);
	fseek(file, 0, SEEK_SET);
	std::vector<uint8_t> buffer(size);

	// read the contents
	auto bytes_read = fread(&buffer[0], 1, size, file);
	if (bytes_read != size)
	{
		fprintf(stderr, "Error reading file contents\n");
		return 3;
	}
	fclose(file);

	// check for gzip-format
	if (buffer.size() >= 10 && buffer[0] == 0x1f && buffer[1] == 0x8b && buffer[2] == 0x08)
	{
		// copy the raw data to a new buffer
		std::vector<uint8_t> compressed = buffer;

		// determine uncompressed size and resize the buffer
		uint8_t *end = &compressed[compressed.size()];
		uint32_t uncompressed = end[-4] | (end[-3] << 8) | (end[-2] << 16) | (end[-1] << 24);
		if (size < compressed.size() || size > 32 * 1024 * 1024)
		{
			fprintf(stderr, "File '%s' appears to be a compressed file but has unexpected size of %d\n", filename, size);
			return 4;
		}
		buffer.resize(uncompressed);

		// decompress the data
		auto result = em_inflate(&compressed[0], compressed.size(), &buffer[0], buffer.size());
		if (result == -1)
		{
			fprintf(stderr, "Error decompressing data from file\n");
			return 4;
		}
	}

	// ここから解析
	VGMPlayer m_player;
	m_player.Create(&buffer[0], (uint32_t)buffer.size());
	// if no chips created, fail
	if (m_player.Check_Activechip() == 0)
	{
		fprintf(stderr, "No compatible chips found, exiting.\n");
		return 5;
	}
	// generate the output
	std::vector<int32_t> wav_buffer;
	m_player.Startup(output_rate);
	while (m_player.isActive())
	{
		if (m_player.GetDelay() == 0)
		{
			m_player.Analyze_VGM();
		}
		else
		{
			while (m_player.GetDelay())
			{
				int32_t outputs[2] = {0, 0};
				m_player.Create1Wav(outputs);
				wav_buffer.push_back(outputs[0]);
				wav_buffer.push_back(outputs[1]);
				m_player.AddSubDelay();
			}
		}
	}
	// generate_all(buffer, data_start, output_rate, wav_buffer);

	int err = write_wav(outfilename, output_rate, wav_buffer);
	return err;
}

#if (RUN_NUKED_OPN2)
namespace nuked
{
#include "test/ym3438.c"
}
#endif
