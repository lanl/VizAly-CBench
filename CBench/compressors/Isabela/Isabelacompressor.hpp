#ifdef CBENCH_HAS_ISABELA

#ifndef _ISABELA_COMPRESSOR_H_
#define _ISABELA_COMPRESSOR_H_

#include "isabela.h"

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <sstream>
#include "compressorInterface.hpp"
#include "timer.hpp"

class IsabelaCompressor: public CompressorInterface
{
    
  public:
    IsabelaCompressor();
    ~IsabelaCompressor();

    void init();
    int compress(void *input, void *&output, std::string dataType, size_t dataTypeSize, size_t * n);
    int decompress(void *&input, void *&output, std::string dataType, size_t dataTypeSize, size_t * n);
    void close();
};

inline IsabelaCompressor::IsabelaCompressor()
{
    compressorName = "Isabela";
}

inline IsabelaCompressor::~IsabelaCompressor()
{
    
}


inline void IsabelaCompressor::init()
{

}

inline int IsabelaCompressor::compress(void *input, void *&output, std::string dataType, size_t dataTypeSize, size_t * n)
{
	size_t numel = n[0];
	for (int i = 1; i < 5; i++)
		if (n[i] != 0)
			numel *= n[i];

	Timer cTime; cTime.start();

	output = malloc(dataTypeSize * numel);

	enum ISABELA_status status;
    struct isabela_stream i_strm;
    struct isabela_config config;

    double tol = 1E-3;
    std::unordered_map<std::string, std::string>::const_iterator got = compressorParameters.find("tolerance");
    if (got != compressorParameters.end())
        if (compressorParameters["tolerance"] != "")
            tol = strConvert::to_double(compressorParameters["tolerance"]);

    int pcnt = 30;
    got = compressorParameters.find("pcnt");
    if (got != compressorParameters.end())
        if (compressorParameters["pcnt"] != "")
            pcnt = strConvert::to_int(compressorParameters["pcnt"]);

    // Compress window_size numebr of elements at a time
    config.window_size = 1024;
    got = compressorParameters.find("window-size");
    if(got != compressorParameters.end())
        if (compressorParameters["window-size"] != "")
            config.window_size = strConvert::to_int(compressorParameters["window-size"]);

    // Approximate each window with 30 coefficients
    config.ncoefficients = pcnt;

    // Relative error between approximate and original values should be
    // no more than 5%, default = 1;
    config.error_rate = tol;

    // Size of each element
    config.element_byte_size = dataTypeSize;

    // Use either BSplines or Wavelets.
    config.transform = ISABELA_BSPLINES;
    // config.transform = ISABELA_WAVELETS;

    // Setup compression (deflate) with isabela_config
    status = isabelaDeflateInit (&i_strm, dataTypeSize, &config);
    assert (status == ISABELA_SUCCESS);

    i_strm.next_in = input;
    i_strm.avail_in = dataTypeSize * numel;
    i_strm.next_out = output;

    // Perform compression
    status = isabelaDeflate (&i_strm, ISABELA_FINISH);
    assert (status == ISABELA_SUCCESS);

    size_t csize = i_strm.avail_out;

    // Cleanup
    status = isabelaDeflateEnd (&i_strm);
    assert (status == ISABELA_SUCCESS);

	cTime.stop();

	cbytes = csize;

	log << "\n" << compressorName << " ~ InputBytes: " << dataTypeSize*numel << ", OutputBytes: " << csize << ", cRatio: " << (dataTypeSize*numel / (float)csize) << ", #elements: " << numel << std::endl;
	log << compressorName << " ~ CompressTime: " << cTime.getDuration() << " s " << std::endl;

	return 1;
}

inline int IsabelaCompressor::decompress(void *&input, void *&output, std::string dataType, size_t dataTypeSize, size_t * n)
{
	size_t numel = n[0];
	for (int i = 1; i < 5; i++)
		if (n[i] != 0)
			numel *= n[i];

	Timer dTime; dTime.start();

	output = malloc(dataTypeSize * numel);

	enum ISABELA_status status;
    struct isabela_stream i_strm;
    struct isabela_config config;

    double tol = 1E-3;
    std::unordered_map<std::string, std::string>::const_iterator got = compressorParameters.find("tolerance");
    if (got != compressorParameters.end())
        if (compressorParameters["tolerance"] != "")
            tol = strConvert::to_double(compressorParameters["tolerance"]);
    int pcnt = 30;
    got = compressorParameters.find("pcnt");
    if (got != compressorParameters.end())
        if (compressorParameters["pcnt"] != "")
            pcnt = strConvert::to_int(compressorParameters["pcnt"]);

    config.window_size = 1024;
    got = compressorParameters.find("window-size");
    if(got != compressorParameters.end())
        if (compressorParameters["window-size"] != "")
            config.window_size = strConvert::to_int(compressorParameters["window-size"]);

    config.ncoefficients = pcnt;
    config.error_rate = tol;
    config.element_byte_size = dataTypeSize;
    config.transform = ISABELA_BSPLINES;

    // Setup compression (deflate) with isabela_config
    status = isabelaInflateInit (&i_strm, dataTypeSize, &config);

    i_strm.next_in = input;
    i_strm.avail_in = dataTypeSize * numel;
    i_strm.next_out = output;

    // Perform Decompression
    status = isabelaInflate (&i_strm, ISABELA_FINISH);
    assert (status == ISABELA_SUCCESS);

    // Cleanup
    status = isabelaInflateEnd (&i_strm);
    assert (status == ISABELA_SUCCESS);
    
	std::free(input);	input=NULL;

	log << compressorName << " ~ DecompressTime: " << dTime.getDuration() << " s " << std::endl;

	return 1;
}

inline void IsabelaCompressor::close()
{

}

#endif // _ISABELA_COMPRESSOR_H_
#endif // CBENCH_HAS_ISABELA

