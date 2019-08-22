/*================================================================================
This software is open source software available under the BSD-3 license.

Copyright (c) 2017, Los Alamos National Security, LLC.
All rights reserved.

Authors:
 - Pascal Grosset
================================================================================*/

#ifndef _DATA_LOADER_INTERFACE_H_
#define _DATA_LOADER_INTERFACE_H_

#include <string>
#include <mpi.h>
#include <sstream>
#include <unordered_map>

#include "gioData.hpp"


class DataLoaderInterface {

protected:
	std::string loader;
	std::string filename;
	std::string dataType;
	std::string param;
	std::stringstream log;

	bool saveData;
	int origNumDims;

	size_t origDims[5]{ 0,0,0,0,0 };	// Global dataset size
	size_t sizePerDim[5]{ 0,0,0,0,0 };	// For compression, size of this mpi rank
	size_t rankOffset[3];				// Rank offset, for parallel operations
	size_t elemSize;					// sizeof() in bytes of that parameter
	size_t totalNumberOfElements;		// total number of points/cells for input file
	size_t numElements;					// number of points/cells for this mpi rank

	MPI_Comm comm;						// global mpi handler

public:   // TO_CHANGE, *data should not be public
	void *data;							// raw data pointer (do not touch outside!!)
	
	std::unordered_map<std::string, std::string> loaderParams;  // data-specific parameters, used by binary reader
	std::vector<GioData> inOutData;		// passthrough data, used by genericio reader

  public:
	virtual void init(std::string _filename, MPI_Comm _comm) = 0;
	virtual int loadData(std::string paramName) = 0;					// Read op
	virtual int saveCompData(std::string paramName, void * cData) = 0;	// Stores the data-pointer for future writing
	virtual int writeData(std::string _filename) = 0;					// Write op
	virtual int saveInputFileParameters() = 0;							// Reads file header only and stores data properties
	virtual int close() = 0;
	virtual void setParam(std::string paramName, std::string type, std::string value) = 0;
	virtual bool loadUncompressedFields(nlohmann::json const& jsonInput) = 0; // Data passthrough, used by hdf5 reader

	size_t getNumElements() { return numElements; }
	size_t * getSizePerDim() { return sizePerDim; }
	size_t getTypeSize() { return elemSize; }
	std::string getType() { return dataType; }
	std::string getParam() { return param; }
	std::string getDataInfo();
	std::string getLog() { return log.str(); }

	void setSave(bool state) { saveData = state; }	// if true, write out decomp data
};


inline std::string DataLoaderInterface::getDataInfo()
{
	std::stringstream dataInfo;
	dataInfo << "\nLoader type: " << loader << std::endl;
	dataInfo << "Filename: " << filename << std::endl;
	dataInfo << "Total number of elements: " << totalNumberOfElements << std::endl;
	dataInfo << "Param: " << param << std::endl;
	dataInfo << "dataType: " << dataType << std::endl;
	dataInfo << "numElements: " << numElements << std::endl;
	dataInfo << "sizePerDim: " << sizePerDim[0] << " " << sizePerDim[1] 
           << " " << sizePerDim[2] << " " << sizePerDim[3] << " " 
           << sizePerDim[4] << std::endl;

	return dataInfo.str();
}

#endif
