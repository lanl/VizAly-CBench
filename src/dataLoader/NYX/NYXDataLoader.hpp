/*================================================================================
This software is open source software available under the BSD-3 license.

Copyright (c) 2017, Los Alamos National Security, LLC.
All rights reserved.

Authors:
 - Jesus Pulido
================================================================================*/

#ifndef _NYX_LOADER_H_
#define _NYX_LOADER_H_

#include <sstream>
#include <string>
#include "dataLoaderInterface.hpp"
#include "timer.hpp"
#include "hdf5.h"

#include <list>
#include <iterator>
#include <algorithm>

class UncompressedData {

public:
  std::string paramName = "";
  std::string dataType = "";
  size_t numElements = 0;
  size_t dims[3] = {0, 0, 0};
  void *data = nullptr;

public:
  UncompressedData() = default;
  UncompressedData(UncompressedData const&) = default;
  UncompressedData(UncompressedData&&) = default;
  UncompressedData(
    std::string in_param_name,
    std::string in_data_type,
    size_t in_numElements,
    size_t in_dims[3]
  ) : paramName(in_param_name),
      dataType (in_data_type),
      numElements (in_numElements),
      dims { in_dims[0], in_dims[1], in_dims[2] }
  {}

  inline int allocateMem() {
    if (dataType == "float")
      data = new float[numElements];
    else if (dataType == "double")
      data = new double[numElements];
    else if (dataType == "int8_t")
      data = new int8_t[numElements];
    else if (dataType == "int16_t")
      data = new int16_t[numElements];
    else if (dataType == "int32_t")
      data = new int32_t[numElements];
    else if (dataType == "int64_t")
      data = new int64_t[numElements];
    else if (dataType == "uint8_t")
      data = new uint8_t[numElements];
    else if (dataType == "uint16_t")
      data = new uint16_t[numElements];
    else if (dataType == "uint32_t")
      data = new uint32_t[numElements];
    else if (dataType == "uint64_t")
      data = new uint64_t[numElements];
    else
      return 0;

    return 1;
  }


  inline int deAllocateMem() {
    if (data == nullptr) // already deallocated!
      return 1;

    if (dataType == "float")
      delete[](float *) data;
    else if (dataType == "double")
      delete[](double *) data;
    else if (dataType == "int8_t")
      delete[](int8_t *) data;
    else if (dataType == "int16_t")
      delete[](int16_t *) data;
    else if (dataType == "int32_t")
      delete[](int32_t *) data;
    else if (dataType == "int64_t")
      delete[](int64_t *) data;
    else if (dataType == "uint8_t")
      delete[](uint8_t *) data;
    else if (dataType == "uint16_t")
      delete[](uint16_t *) data;
    else if (dataType == "uint32_t")
      delete[](uint32_t *) data;
    else if (dataType == "uint64_t")
      delete[](uint64_t *) data;
    else
      return 0;

    data = nullptr;
    return 1;
  }
};


class Partition {
public:
  int min_x = 0;
  int min_y = 0;
  int min_z = 0;
  int max_x = 0;
  int max_y = 0;
  int max_z = 0;

public:
  Partition() = default;
  Partition(
    int in_min_x, int in_min_y, int in_min_z,
    int in_max_x, int in_max_y, int in_max_z
  ) : min_x(in_min_x),
      min_y(in_min_y),
      min_z(in_min_z),
      max_x(in_max_x),
      max_y(in_max_y),
      max_z(in_max_z)
  {}

  void print() {
    std::cout << min_x << ", " << min_y << ", " << min_z << " - "
              << max_x << ", " << max_y << ", " << max_z << std::endl;
  }
};

Partition getPartition(int myRank, int numRanks, int extentsX, int extentsY, int extentsZ) {

  std::list<Partition> partitions;
  partitions.push_back(Partition{0, 0, 0, extentsX, extentsY, extentsZ});

  Partition first_half, second_half;

  int axis = 0;
  while (partitions.size() < numRanks) {
    int numCurrentPartitions = partitions.size();
    for (int i = 0; i < numCurrentPartitions; i++) {
      Partition parent = partitions.front();
      partitions.pop_front();


      if (axis == 0) {
        first_half.min_x = parent.min_x;
        first_half.max_x = (parent.min_x + parent.max_x) / 2;
        second_half.min_x = first_half.max_x;
        second_half.max_x = parent.max_x;

        first_half.min_y = second_half.min_y = parent.min_y;
        first_half.max_y = second_half.max_y = parent.max_y;
        first_half.min_z = second_half.min_z = parent.min_z;
        first_half.max_z = second_half.max_z = parent.max_z;
      }

      if (axis == 1) {
        first_half.min_y = parent.min_y;
        first_half.max_y = (parent.min_y + parent.max_y) / 2;
        second_half.min_y = first_half.max_y;
        second_half.max_y = parent.max_y;

        first_half.min_x = second_half.min_x = parent.min_x;
        first_half.max_x = second_half.max_x = parent.max_x;
        first_half.min_z = second_half.min_z = parent.min_z;
        first_half.max_z = second_half.max_z = parent.max_z;
      }

      if (axis == 2) {
        first_half.min_z = parent.min_z;
        first_half.max_z = (parent.min_z + parent.max_z) / 2;
        second_half.min_z = first_half.max_z;
        second_half.max_z = parent.max_z;

        first_half.min_x = second_half.min_x = parent.min_x;
        first_half.max_x = second_half.max_x = parent.max_x;
        first_half.min_y = second_half.min_y = parent.min_y;
        first_half.max_y = second_half.max_y = parent.max_y;
      }

      partitions.push_back(first_half);
      partitions.push_back(second_half);

      if (partitions.size() >= numRanks)
        break;
    }

    axis++;
    if (axis == 3)
      axis = 0;
  }


  auto it = partitions.begin();
  std::advance(it, myRank);

  return *it;

}


class NYXDataLoader : public DataLoaderInterface {
  int myRank = 0;
  int numRanks = 0;
  std::string groupName = "native_fields";

  std::vector<UncompressedData> toWriteData;

public:
  NYXDataLoader() {
    loader = "NYX";
    saveData = false;
  }

  ~NYXDataLoader() { deAllocateMem(); }

  int allocateMem(std::string dataType, size_t numElements, int offset);
  int deAllocateMem();

  void init(std::string _filename, MPI_Comm _comm);
  int loadData(std::string paramName);
  int saveCompData(std::string paramName, void *cData);
  int writeData(std::string _filename);
  int writeDataOld(std::string _filename);
  int saveInputFileParameters() { return 1; } 
  int close() { return deAllocateMem(); }
  void setParam(std::string paramName, std::string type, std::string value);
};

inline void NYXDataLoader::init(std::string _filename, MPI_Comm _comm) {
  filename = _filename;
  comm = _comm;
  saveData = false;

  MPI_Comm_size(comm, &numRanks);
  MPI_Comm_rank(comm, &myRank);
}

inline void NYXDataLoader::setParam(std::string paramName, std::string type, std::string value) {
  if (paramName == "group")
    groupName = value;
}

// TODO: refactor it in a util class
inline int NYXDataLoader::allocateMem(std::string dataType, size_t numElements, int offset) {
  // Allocate mem
  if (dataType == "float")
    data = new float[numElements + offset];
  else if (dataType == "double")
    data = new double[numElements + offset];
  else if (dataType == "int8_t")
    data = new int8_t[numElements + offset];
  else if (dataType == "int16_t")
    data = new int16_t[numElements + offset];
  else if (dataType == "int32_t")
    data = new int32_t[numElements + offset];
  else if (dataType == "int64_t")
    data = new int64_t[numElements + offset];
  else if (dataType == "uint8_t")
    data = new uint8_t[numElements + offset];
  else if (dataType == "uint16_t")
    data = new uint16_t[numElements + offset];
  else if (dataType == "uint32_t")
    data = new uint32_t[numElements + offset];
  else if (dataType == "uint64_t")
    data = new uint64_t[numElements + offset];
  else
    return 0;

  // Get size
  if (dataType == "float")
    elemSize = sizeof(float);
  else if (dataType == "double")
    elemSize = sizeof(double);
  else if (dataType == "int8_t")
    elemSize = sizeof(int8_t);
  else if (dataType == "int16_t")
    elemSize = sizeof(int16_t);
  else if (dataType == "int32_t")
    elemSize = sizeof(int32_t);
  else if (dataType == "int64_t")
    elemSize = sizeof(int64_t);
  else if (dataType == "uint8_t")
    elemSize = sizeof(uint8_t);
  else if (dataType == "uint16_t")
    elemSize = sizeof(uint16_t);
  else if (dataType == "uint32_t")
    elemSize = sizeof(uint32_t);
  else if (dataType == "uint64_t")
    elemSize = sizeof(uint64_t);
  else
    return 0;

  return 1;
}


inline int NYXDataLoader::deAllocateMem() {
  if (data == NULL) // already deallocated!
    return 1;

  if (dataType == "float")
    delete[](float *) data;
  else if (dataType == "double")
    delete[](double *) data;
  else if (dataType == "int8_t")
    delete[](int8_t *) data;
  else if (dataType == "int16_t")
    delete[](int16_t *) data;
  else if (dataType == "int32_t")
    delete[](int32_t *) data;
  else if (dataType == "int64_t")
    delete[](int64_t *) data;
  else if (dataType == "uint8_t")
    delete[](uint8_t *) data;
  else if (dataType == "uint16_t")
    delete[](uint16_t *) data;
  else if (dataType == "uint32_t")
    delete[](uint32_t *) data;
  else if (dataType == "uint64_t")
    delete[](uint64_t *) data;
  else
    return 0;

  data = NULL;

  return 1;
}


inline int NYXDataLoader::loadData(std::string paramName) {

  Timer clock;
  log.str("");

  clock.start();


  log << "filename: " << filename << " param name: " << paramName << std::endl;

  hid_t file = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
  hid_t group = H5Gopen(file, groupName.c_str(), H5P_DEFAULT);

  hsize_t fields;
  herr_t err = H5Gget_num_objs(group, &fields);


  paramName = "/" + groupName + "/" + paramName;
  log << "paramName: " << paramName << std::endl;
  hid_t dataset = H5Dopen(file, paramName.c_str(), H5P_DEFAULT);
  hid_t dataspace = H5Dget_space(dataset);
  hid_t memspace = H5Dget_space(dataset);


  hsize_t tdims[3];
  int ndims = H5Sget_simple_extent_dims(dataspace, tdims, NULL);
  origDims[0] = tdims[0];
  origDims[1] = tdims[1];
  origDims[2] = tdims[2];

  totalNumberOfElements = tdims[0] * tdims[1] * tdims[2];

  log << "Param: " << paramName << std::endl;
  log << "Data dimensions: " << tdims[0] << " " << tdims[1] << " " << tdims[2] << " | totalNumberOfElements "
      << totalNumberOfElements << "\n";


  // Read only a subset of the file
  hsize_t count[3];
  hsize_t offset[3];


  Partition current = getPartition(myRank, numRanks, tdims[0], tdims[1], tdims[2]);
  count[0] = current.max_x - current.min_x;
  count[1] = current.max_y - current.min_y;
  count[2] = current.max_z - current.min_z;

  sizePerDim[0] = count[0];
  sizePerDim[1] = count[1];
  sizePerDim[2] = count[2];

  offset[0] = current.min_x;
  offset[1] = current.min_y;
  offset[2] = current.min_z;

  numElements = count[0] * count[1] * count[2];

  rankOffset[0] = offset[0];
  rankOffset[1] = offset[1];
  rankOffset[2] = offset[2];


  log << myRank << " ~ Count : " << count[0] << " " << count[1] << " " << count[2] << " | " << numElements << "\n";
  log << myRank << " ~ Offset: " << offset[0] << " " << offset[1] << " " << offset[2] << "\n";


  herr_t status = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset, NULL, count, NULL);
  hid_t memspaceRead = H5Screate_simple(3, count, NULL);

  hsize_t zero[3] = {0, 0, 0};
  status = H5Sselect_hyperslab(memspaceRead, H5S_SELECT_SET, zero, NULL, count, NULL);


  // Set-up data stream
  dataType = "float";
  allocateMem(dataType, numElements, 0);
  log << myRank << " ~ numElements: " << numElements << "\n";

  status = H5Dread(dataset, H5T_NATIVE_FLOAT, memspaceRead, dataspace, H5P_DEFAULT, data);

  H5Dclose(dataset);
  H5Fclose(file);


  clock.stop();
  log << "Loading data took " << clock.getDuration() << " s" << std::endl;

  return 1; // All good
}


inline int NYXDataLoader::saveCompData(std::string paramName, void *cData) {
  size_t _dims[3];
  _dims[0] = sizePerDim[0];
  _dims[1] = sizePerDim[1];
  _dims[2] = sizePerDim[2];

  UncompressedData temp(paramName, "float", numElements, _dims);
  temp.allocateMem();
  std::memcpy(temp.data, cData, numElements * sizeof(float));

  toWriteData.push_back(temp);

  return 1;
}


inline int NYXDataLoader::writeData(std::string _filename) {
  log << "writing to " << _filename << std::endl;

  hid_t plist_id = H5Pcreate(H5P_FILE_ACCESS);
  H5Pset_fapl_mpio(plist_id, comm, MPI_INFO_NULL);

  hid_t file_id = H5Fcreate(_filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, plist_id);
  H5Pclose(plist_id);

  hsize_t fileDims[3];
  fileDims[0] = origDims[0];
  fileDims[1] = origDims[1];
  fileDims[2] = origDims[2];
  hid_t filespace = H5Screate_simple(3, fileDims, NULL);

  hid_t group = H5Gcreate2(file_id, ("/" + groupName).c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  int numDatasets = toWriteData.size();
  hid_t *dset_id = new hid_t[numDatasets];

  int datasetCount = 0;
  for (auto item = toWriteData.begin(); item != toWriteData.end(); item++) {
    std::string fieldname = "/" + groupName + "/" + (*item).paramName;

    dset_id[datasetCount] = H5Dcreate(file_id, fieldname.c_str(), H5T_NATIVE_FLOAT, filespace, H5P_DEFAULT, H5P_DEFAULT,
                                      H5P_DEFAULT);
    datasetCount++;

    log << "creating fieldname " << fieldname << ", status: " << (dset_id[datasetCount - 1] == -1) << std::endl;
  }


  {
    char strformat[] = "nyx-lyaf";
    hid_t type = H5Tcopy(H5T_C_S1);
    int32_t len = strlen(strformat);
    H5Tset_size(type, len);
    hid_t ds = H5Screate(H5S_SCALAR);

    hid_t attribute_id = H5Acreate(file_id, "format", type, ds, H5P_DEFAULT, H5P_DEFAULT);
    H5Awrite(attribute_id, type, &strformat);

    H5Aclose(attribute_id);
  }


  hsize_t count[3];
  count[0] = sizePerDim[0];
  count[1] = sizePerDim[1];
  count[2] = sizePerDim[2];

  hsize_t offset[3];
  offset[0] = rankOffset[0];
  offset[1] = rankOffset[1];
  offset[2] = rankOffset[2];
  hid_t memspace = H5Screate_simple(3, count, NULL);

  log << "count " << count[0] << ", " << count[1] << ", " << count[2] << std::endl;
  log << "offset " << offset[0] << ", " << offset[1] << ", " << offset[2] << std::endl;


  filespace = H5Dget_space(dset_id[0]);
  H5Sselect_hyperslab(filespace, H5S_SELECT_SET, offset, NULL, count, NULL);

  plist_id = H5Pcreate(H5P_DATASET_XFER);
  H5Pset_dxpl_mpio(plist_id, H5FD_MPIO_COLLECTIVE);

  datasetCount = 0;
  for (auto item = toWriteData.begin(); item != toWriteData.end(); item++) {
    herr_t status = H5Dwrite(dset_id[datasetCount], H5T_NATIVE_FLOAT, memspace, filespace, plist_id,
                             (float *) (*item).data);
    datasetCount++;

    log << "writing fieldname " << (*item).paramName << ", status: " << (status == -1) << std::endl;
    log << ((float *) (*item).data)[0] << ", " << ((float *) (*item).data)[1] << ", " <<
        ((float *) (*item).data)[count[0] * count[1] * count[2] - 2] << ", "
        << ((float *) (*item).data)[count[0] * count[1] * count[2] - 1] << std::endl;
  }

  for (int i = 0; i < numDatasets; i++) {
    herr_t status = H5Dclose(dset_id[i]);
    log << "dataset " << i << " of " << numDatasets << " closing status: " << (status == -1) << std::endl;
  }

  herr_t status = H5Gclose(group);

  log << "group close status: " << (status == -1) << std::endl;

  H5Sclose(filespace);
  H5Sclose(memspace);
  H5Pclose(plist_id);
  H5Fclose(file_id);

  for (int i = 0; i < toWriteData.size(); i++)
    toWriteData[i].deAllocateMem();
  toWriteData.clear();
}

#endif
