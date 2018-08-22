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


class DataLoaderInterface
{
  protected:
	std::string loader;
	std::string filename;
	size_t totalNumberOfElements;

	std::string dataType;
	std::string param;

	size_t numElements;
	size_t elemSize;

	MPI_Comm comm;
	std::stringstream log;

  public:   // TO_CHANGE
	void *data;

  public:
	virtual void init(std::string _filename, MPI_Comm _comm) = 0;
	virtual int loadData(std::string paramName) = 0;
	virtual int close() = 0;

	size_t getNumElements() { return numElements; }
	size_t getTypeSize() { return elemSize; }
	std::string getType(){ return dataType; }
	std::string getParam(){ return param; }

	std::string getDataInfo();
	std::string getLog() { return log.str(); }
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

	return dataInfo.str();
}

#endif