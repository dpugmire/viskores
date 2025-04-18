//============================================================================
//  The contents of this file are covered by the Viskores license. See
//  LICENSE.txt for details.
//
//  By contributing to this file, all contributors agree to the Developer
//  Certificate of Origin Version 1.1 (DCO 1.1) as stated in DCO.txt.
//============================================================================

//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//  Copyright 2014 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
//  Copyright 2014 UT-Battelle, LLC.
//  Copyright 2014 Los Alamos National Security.
//
//  Under the terms of Contract DE-NA0003525 with NTESS,
//  the U.S. Government retains certain rights in this software.
//
//  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
//  Laboratory (LANL), the U.S. Government retains certain rights in
//  this software.
//============================================================================
// Copyright (c) 2018, The Regents of the University of California, through
// Lawrence Berkeley National Laboratory (subject to receipt of any required approvals
// from the U.S. Dept. of Energy).  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
// (1) Redistributions of source code must retain the above copyright notice, this
//     list of conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
// (3) Neither the name of the University of California, Lawrence Berkeley National
//     Laboratory, U.S. Dept. of Energy nor the names of its contributors may be
//     used to endorse or promote products derived from this software without
//     specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//
//=============================================================================
//
//  I/O functions used by the ContourTreeApp for data read.
//==============================================================================

#ifndef viskores_examples_ContourTreeAppDataIO_hxx
#define viskores_examples_ContourTreeAppDataIO_hxx

#include <viskores/Types.h>
#include <viskores/cont/ArrayCopy.h>
#include <viskores/cont/ArrayHandle.h>
#include <viskores/cont/CellSetStructured.h>
#include <viskores/cont/DataSet.h>
#include <viskores/cont/DataSetBuilderUniform.h>
#include <viskores/cont/PartitionedDataSet.h>
#include <viskores/cont/Timer.h>
#include <viskores/io/BOVDataSetReader.h>

#include <viskores/filter/scalar_topology/worklet/contourtree_augmented/Types.h>

#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <utility>
#include <vector>


#ifdef ENABLE_HDFIO
// #include "H5Cpp.h"
#include "hdf5.h"
//using namespace H5;

#include <mpi.h>


/// Convert a 3D index of a cube to rank index
inline viskores::Id to1DIndex(viskores::Id3 idx, viskores::Id3 dims)
{
  // return (idx[2] * dims[0] * dims[1]) + (idx[1] * dims[0]) + idx[0];
  // Swap first and second dimension
  return (idx[2] * dims[0] * dims[1]) + (idx[0] * dims[1]) + idx[1];
}

/// Convert the rank index to the index of the cube
inline viskores::Id3 to3DIndex(viskores::Id idx, viskores::Id3 dims)
{
  viskores::Id3 res;
  res[2] = idx / (dims[0] * dims[1]);
  idx -= (res[2] * dims[0] * dims[1]);
  // Swap index 0 and 1
  //    res[0] = idx / dims[0];
  //    res[1] = idx % dims[0];
  // Don't swap index here, because this function is used with the original
  // HDF5 layout and the 3D index is swapped later on
  res[1] = idx / dims[0];
  res[0] = idx % dims[0];
  return res;
}


/// Read data from pre-split ASCII files
/// @param[in] rank The current MPI rank the function is called from
/// @param[in] filename Name of the file with %d as placeholder for the integer index of the block
/// @param[in] dataset_name Name of the dataset in the HDF5 file to read
/// @param[in] blocksPerRank Number of data blocks to process on each rank
/// @param[in] blocksPerDim Number of data blocks to use per dimension
/// @param[in] selectSize Select subset of this size from the dataset. Set to (-1,-1,-1) to select the full size
/// @param[out] nDims Number of data dimensions (i.e, 2 or 3)
/// @param[out] useDataSet Viskores partioned dataset to be used with the distributed contour tree filter
/// @param[out] globalSize Global extends of the input mesh (i.e., number of mesh points in each dimension)
/// @param[out] localBlockIndices  Array with the (x,y,z) index of each local data block with
///                              with respect to blocksPerDim
/// @param[out] dataReadTime Time to read the data
/// @param[out] buildDatasetTime Time to construct the Viskores datasets
/// @returns bool indicating whether the read was successful or not
template <typename ValueType>
bool read3DHDF5File(const int& mpi_rank,
                    const std::string& filename,
                    const std::string& dataset_name,
                    const int& blocksPerRank,
                    viskores::Id3& blocksPerDim,
                    const viskores::Id3& selectSize,
                    std::vector<viskores::Float32>::size_type& nDims,
                    viskores::cont::PartitionedDataSet& useDataSet,
                    viskores::Id3& globalSize,
                    viskores::cont::ArrayHandle<viskores::Id3>& localBlockIndices,
                    viskores::Float64& dataReadTime,
                    viskores::Float64& buildDatasetTime)
{
  viskores::cont::Timer totalTime;
  totalTime.Start();
  viskores::Float64 prevTime = 0;
  viskores::Float64 currTime = 0;

  // TODO not supported yet
  if (blocksPerRank > 1)
  {
    VISKORES_LOG_S(
      viskores::cont::LogLevel::Error,
      "HDF5 reader for ContourTreeDistributed does not support multiple blocks per rank yet");
    return false;
  }
  viskores::Id blockNo = 0; // TODO: Update this if we have multiple blocks per rank

  localBlockIndices.Allocate(blocksPerRank);
  auto localBlockIndicesPortal = localBlockIndices.WritePortal();

  herr_t status;
  //Set up file access property list with parallel I/O access
  hid_t plist_id = H5Pcreate(H5P_FILE_ACCESS);
  MPI_Comm comm = MPI_COMM_WORLD;
  MPI_Info info = MPI_INFO_NULL;
  H5Pset_fapl_mpio(plist_id, comm, info);

  // Open the file and the dataset
  //hid_t file = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT); //  plist_id);//
  hid_t file = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, plist_id); //
  hid_t dataset = H5Dopen(file, dataset_name.c_str(), H5P_DEFAULT);
  // Get filespace for rank and dimension
  hid_t filespace = H5Dget_space(dataset);
  // Get number of dimensions in the file dataspace
  nDims = H5Sget_simple_extent_ndims(filespace);
  if (nDims != 3)
  {
    VISKORES_LOG_S(viskores::cont::LogLevel::Error,
                   "HDF5 reader for ContourTreeDistributed requires 3D dataset");
    return false;
  }
  hsize_t dims[nDims]; // dataset dimensions
  status = H5Sget_simple_extent_dims(filespace, dims, NULL);
  globalSize[0] = selectSize[0] < 0 ? dims[0] : selectSize[0];
  globalSize[1] = selectSize[1] < 0 ? dims[1] : selectSize[1];
  globalSize[2] = selectSize[2] < 0 ? dims[2] : selectSize[2];
  // Define the memory space to read dataset.
  hid_t dataspace = H5Dget_space(dataset);
  // Read a hyperslap
  // define the hyperslap
  hsize_t count[3];  // size of the hyperslab in the file
  hsize_t offset[3]; // hyperslab offset in the file

  // Compute the origin and count
  viskores::Id3 blockSize(std::floor(viskores::Id(globalSize[0] / blocksPerDim[0])),
                          std::floor(viskores::Id(globalSize[1] / blocksPerDim[1])),
                          std::floor(viskores::Id(globalSize[2] / blocksPerDim[2])));
  viskores::Id3 blockIndex = to3DIndex(mpi_rank, blocksPerDim);

  // compute the offset and count for the block for this rank
  offset[0] = blockSize[0] * blockIndex[0];
  offset[1] = blockSize[1] * blockIndex[1];
  offset[2] = blockSize[2] * blockIndex[2];
  count[0] = blockSize[0];
  count[1] = blockSize[1];
  count[2] = blockSize[2];
  // add ghost zone on the left
  if (blockIndex[0] > 0)
  {
    offset[0] = offset[0] - 1;
    count[0] = count[0] + 1;
  }
  if (blockIndex[1] > 0)
  {
    offset[1] = offset[1] - 1;
    count[1] = count[1] + 1;
  }
  if (blockIndex[2] > 0)
  {
    offset[2] = offset[2] - 1;
    count[2] = count[2] + 1;
  }
  // Check that we are not running over the end of the dataset
  if (viskores::Id(offset[0] + count[0]) > globalSize[0])
  {
    count[0] = globalSize[0] - offset[0];
  }
  if (viskores::Id(offset[1] + count[1]) > globalSize[1])
  {
    count[1] = globalSize[1] - offset[1];
  }
  if (viskores::Id(offset[2] + count[2]) > globalSize[2])
  {
    count[2] = globalSize[2] - offset[2];
  }
  // Check that we cover the end of the dataset
  if (blockIndex[0] == blocksPerDim[0] - 1 && (viskores::Id(offset[0] + count[0]) != globalSize[0]))
  {
    count[0] = globalSize[0] - offset[0];
  }
  if (blockIndex[1] == blocksPerDim[1] - 1 && (viskores::Id(offset[1] + count[1]) != globalSize[1]))
  {
    count[1] = globalSize[1] - offset[1];
  }
  if (blockIndex[2] == blocksPerDim[2] - 1 && (viskores::Id(offset[2] + count[2]) != globalSize[2]))
  {
    count[2] = globalSize[2] - offset[2];
  }
  blockSize = viskores::Id3{ static_cast<viskores::Id>(count[0]),
                             static_cast<viskores::Id>(count[1]),
                             static_cast<viskores::Id>(count[2]) };
  /*viskores::Id3 blockOrigin = viskores::Id3{ static_cast<viskores::Id>(offset[0]),
                                     static_cast<viskores::Id>(offset[1]),
                                     static_cast<viskores::Id>(offset[2]) };*/

  // Define the hyperslap to read the data into memory
  status = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset, NULL, count, NULL);
  // Define the memory space for reading
  hid_t memspace = H5Screate_simple(nDims, count, NULL);
  // Read data from hyperslab in the file into the hyperslab in
  std::size_t numVertices = count[0] * count[1] * count[2];
  std::vector<ValueType> values(numVertices);
  {
    if (H5Tequal(H5Dget_type(dataset), H5T_NATIVE_DOUBLE))
    {
      double data_out[count[0]][count[1]][count[2]]; // output buffer
      status = H5Dread(dataset, H5T_NATIVE_DOUBLE, memspace, dataspace, H5P_DEFAULT, data_out);
      // Copy data to 1D array of the expected ValueType
      for (hsize_t k = 0; k < count[0]; k++)
      {
        for (hsize_t j = 0; j < count[1]; j++)
        {
          for (hsize_t i = 0; i < count[2]; i++)
          {
            values[to1DIndex(viskores::Id3(k, j, i), blockSize)] = ValueType(data_out[k][j][i]);
          }
        }
      }
    }
    else if (H5Tequal(H5Dget_type(dataset), H5T_NATIVE_FLOAT))
    {
      float data_out[count[0]][count[1]][count[2]]; // output buffer
      status = H5Dread(dataset, H5T_NATIVE_FLOAT, memspace, dataspace, H5P_DEFAULT, data_out);
      // Copy data to 1D array of the expected ValueType
      for (hsize_t k = 0; k < count[0]; k++)
      {
        for (hsize_t j = 0; j < count[1]; j++)
        {
          for (hsize_t i = 0; i < count[2]; i++)
          {
            values[to1DIndex(viskores::Id3(k, j, i), blockSize)] = ValueType(data_out[k][j][i]);
          }
        }
      }
    }
    else if (H5Tequal(H5Dget_type(dataset), H5T_NATIVE_INT))
    {
      int data_out[count[0]][count[1]][count[2]]; // output buffer
      status = H5Dread(dataset, H5T_NATIVE_INT, memspace, dataspace, H5P_DEFAULT, data_out);
      // Copy data to 1D array of the expected ValueType
      for (hsize_t k = 0; k < count[0]; k++)
      {
        for (hsize_t j = 0; j < count[1]; j++)
        {
          for (hsize_t i = 0; i < count[2]; i++)
          {
            values[to1DIndex(viskores::Id3(k, j, i), blockSize)] = ValueType(data_out[k][j][i]);
          }
        }
      }
    }
    else if (H5Tequal(H5Dget_type(dataset), H5T_NATIVE_UCHAR))
    {
      unsigned char data_out[count[0]][count[1]][count[2]]; // output buffer
      status = H5Dread(dataset, H5T_NATIVE_UCHAR, memspace, dataspace, H5P_DEFAULT, data_out);
      // Copy data to 1D array of the expected ValueType
      for (hsize_t k = 0; k < count[0]; k++)
      {
        for (hsize_t j = 0; j < count[1]; j++)
        {
          for (hsize_t i = 0; i < count[2]; i++)
          {
            values[to1DIndex(viskores::Id3(k, j, i), blockSize)] = ValueType(data_out[k][j][i]);
          }
        }
      }
    }
    else
    {
      VISKORES_LOG_S(viskores::cont::LogLevel::Error,
                     "Data type not supported by the example HDF5 reader");
      throw "Data type not supported by the example HDF5 reader";
    }
  }
  // Release HDF5 resources
  H5Sclose(dataspace);
  H5Dclose(dataset);
  H5Fclose(file);

  // Create viskores data set
  viskores::cont::DataSetBuilderUniform dsb;
  viskores::cont::DataSet ds;
  VISKORES_ASSERT(nDims == 3);

  // Swap dimensions so that they are from fastest to slowest growing
  // dims[0] -> col; dims[1] -> row, dims[2] ->slice
  // Swap the dimensions to match the pre-split file reader
  globalSize = viskores::Id3(globalSize[1], globalSize[0], globalSize[2]);
  // Swap also the blocks per dimension accordingly
  blocksPerDim = viskores::Id3(blocksPerDim[1], blocksPerDim[0], blocksPerDim[2]);

  // Swap first and second dimenion here as well for consistency
  const viskores::Vec<ValueType, 3> v_origin{ static_cast<ValueType>(offset[1]),
                                              static_cast<ValueType>(offset[0]),
                                              static_cast<ValueType>(offset[2]) };
  const viskores::Id3 v_dims{ static_cast<viskores::Id>(blockSize[1]),
                              static_cast<viskores::Id>(blockSize[0]),
                              static_cast<viskores::Id>(blockSize[2]) };
  viskores::Vec<ValueType, 3> v_spacing(1, 1, 1);
  ds = dsb.Create(v_dims, v_origin, v_spacing);
  viskores::cont::CellSetStructured<3> cs;
  cs.SetPointDimensions(v_dims);
  cs.SetGlobalPointDimensions(globalSize);
  cs.SetGlobalPointIndexStart(viskores::Id3{ v_origin[0], v_origin[1], v_origin[2] });
  ds.SetCellSet(cs);

  ds.AddPointField("values", values);
  // and add to partition
  useDataSet.AppendPartition(ds);

  // Swap order to match pre-splot
  localBlockIndicesPortal.Set(blockNo, viskores::Id3(blockIndex[1], blockIndex[0], blockIndex[2]));

  // Log information of the (first) local data block
  VISKORES_LOG_S(viskores::cont::LogLevel::Info,
                 "" << std::setw(42) << std::left << "blockSize"
                    << ":" << v_dims << std::endl
                    << std::setw(42) << std::left << "blockOrigin=" << v_origin << std::endl
                    << std::setw(42) << std::left
                    << "blockIndices=" << localBlockIndicesPortal.Get(0) << std::endl
                    << std::setw(42) << std::left << "globalSize=" << globalSize << std::endl);

  // Finished data read
  currTime = totalTime.GetElapsedTime();
  dataReadTime = currTime - prevTime;
  prevTime = currTime;

  currTime = totalTime.GetElapsedTime();
  buildDatasetTime = currTime - prevTime;
  return true;
}
#endif



/// Read data from pre-split ASCII files
/// @param[in] rank The current MPI rank the function is called from
/// @param[in] filename Name of the file with %d as placeholder for the integer index of the block
/// @param[in] blocksPerRank Number of data blocks to process on each rank
/// @param[out] nDims Number of data dimensions (i.e, 2 or 3)
/// @param[out] useDataSet Viskores partioned dataset to be used with the distributed contour tree filter
/// @param[out] globalSize Global extends of the input mesh (i.e., number of mesh points in each dimension)
/// @param[in] blocksPerDim  Number of data blocks used in each data dimension
/// @param[in] localBlockIndices  Array with the (x,y,z) index of each local data block with
///                              with respect to blocksPerDim
/// @param[in] dataReadTime Time to read the data
/// @param[in] buildDatasetTime Time to construct the Viskores datasets
/// @returns bool indicating whether the read was successful or not
template <typename ValueType>
bool readPreSplitFiles(const int& rank,
                       const std::string& filename,
                       const int& blocksPerRank,
                       std::vector<viskores::Float32>::size_type& nDims,
                       viskores::cont::PartitionedDataSet& useDataSet,
                       viskores::Id3& globalSize,
                       viskores::Id3& blocksPerDim,
                       viskores::cont::ArrayHandle<viskores::Id3>& localBlockIndices,
                       viskores::Float64& dataReadTime,
                       viskores::Float64& buildDatasetTime)
{
  viskores::cont::Timer totalTime;
  totalTime.Start();
  viskores::Float64 prevTime = 0;
  viskores::Float64 currTime = 0;

  localBlockIndices.Allocate(blocksPerRank);
  auto localBlockIndicesPortal = localBlockIndices.WritePortal();
  for (int blockNo = 0; blockNo < blocksPerRank; ++blockNo)
  {
    // Translate pattern into filename for this block
    char block_filename[256];
    snprintf(block_filename,
             sizeof(block_filename),
             filename.c_str(),
             static_cast<int>(rank * blocksPerRank + blockNo));
    std::cout << "Reading file " << block_filename << std::endl;

    // Open file
    std::ifstream inFile(block_filename);
    if (!inFile.is_open() || inFile.bad())
    {
      std::cerr << "Error: Couldn't open file " << block_filename << std::endl;
      return false;
    }

    // Read header with dimensions
    std::string line;
    std::string tag;
    viskores::Id dimVertices;

    getline(inFile, line);
    std::istringstream global_extents_stream(line);
    global_extents_stream >> tag;
    if (tag != "#GLOBAL_EXTENTS")
    {
      std::cerr << "Error: Expected #GLOBAL_EXTENTS, got " << tag << std::endl;
      return false;
    }

    std::vector<viskores::Id> global_extents;
    while (global_extents_stream >> dimVertices)
      global_extents.push_back(dimVertices);

    // Swap dimensions so that they are from fastest to slowest growing
    // dims[0] -> col; dims[1] -> row, dims[2] ->slice
    std::swap(global_extents[0], global_extents[1]);

    if (blockNo == 0)
    { // First block: Set globalSize
      globalSize = viskores::Id3{ static_cast<viskores::Id>(global_extents[0]),
                                  static_cast<viskores::Id>(global_extents[1]),
                                  static_cast<viskores::Id>(
                                    global_extents.size() > 2 ? global_extents[2] : 1) };
    }
    else
    { // All other blocks: Consistency check of globalSize
      if (globalSize !=
          viskores::Id3{
            static_cast<viskores::Id>(global_extents[0]),
            static_cast<viskores::Id>(global_extents[1]),
            static_cast<viskores::Id>(global_extents.size() > 2 ? global_extents[2] : 1) })
      {
        std::cerr << "Error: Global extents mismatch between blocks!" << std::endl;
        return false;
      }
    }

    getline(inFile, line);
    std::istringstream offset_stream(line);
    offset_stream >> tag;
    if (tag != "#OFFSET")
    {
      std::cerr << "Error: Expected #OFFSET, got " << tag << std::endl;
      return false;
    }
    std::vector<viskores::Id> offset;
    while (offset_stream >> dimVertices)
      offset.push_back(dimVertices);
    // Swap dimensions so that they are from fastest to slowest growing
    // dims[0] -> col; dims[1] -> row, dims[2] ->slice
    std::swap(offset[0], offset[1]);

    getline(inFile, line);
    std::istringstream bpd_stream(line);
    bpd_stream >> tag;
    if (tag != "#BLOCKS_PER_DIM")
    {
      std::cerr << "Error: Expected #BLOCKS_PER_DIM, got " << tag << std::endl;
      return false;
    }
    std::vector<viskores::Id> bpd;
    while (bpd_stream >> dimVertices)
      bpd.push_back(dimVertices);
    // Swap dimensions so that they are from fastest to slowest growing
    // dims[0] -> col; dims[1] -> row, dims[2] ->slice
    std::swap(bpd[0], bpd[1]);

    getline(inFile, line);
    std::istringstream blockIndex_stream(line);
    blockIndex_stream >> tag;
    if (tag != "#BLOCK_INDEX")
    {
      std::cerr << "Error: Expected #BLOCK_INDEX, got " << tag << std::endl;
      return false;
    }
    std::vector<viskores::Id> blockIndex;
    while (blockIndex_stream >> dimVertices)
      blockIndex.push_back(dimVertices);
    // Swap dimensions so that they are from fastest to slowest growing
    // dims[0] -> col; dims[1] -> row, dims[2] ->slice
    std::swap(blockIndex[0], blockIndex[1]);

    getline(inFile, line);
    std::istringstream linestream(line);
    std::vector<viskores::Id> dims;
    while (linestream >> dimVertices)
    {
      dims.push_back(dimVertices);
    }

    if (dims.size() != global_extents.size() || dims.size() != offset.size())
    {
      std::cerr << "Error: Dimension mismatch" << std::endl;
      return false;
    }
    // Swap dimensions so that they are from fastest to slowest growing
    // dims[0] -> col; dims[1] -> row, dims[2] ->slice
    std::swap(dims[0], dims[1]);

    // Compute the number of vertices, i.e., xdim * ydim * zdim
    nDims = static_cast<unsigned short>(dims.size());
    std::size_t numVertices = static_cast<std::size_t>(
      std::accumulate(dims.begin(), dims.end(), std::size_t(1), std::multiplies<std::size_t>()));

    // Check for fatal input errors
    // Check that the number of dimensiosn is either 2D or 3D
    bool invalidNumDimensions = (nDims < 2 || nDims > 3);
    // Log any errors if found on rank 0
    VISKORES_LOG_IF_S(viskores::cont::LogLevel::Error,
                      invalidNumDimensions && (rank == 0),
                      "The input mesh is " << nDims
                                           << "D. "
                                              "The input data must be either 2D or 3D.");

    // If we found any errors in the setttings than finalize MPI and exit the execution
    if (invalidNumDimensions)
    {
      return false;
    }

    // Read data
    std::vector<ValueType> values(numVertices);
    if (filename.compare(filename.length() - 5, 5, ".bdem") == 0)
    {
      inFile.read(reinterpret_cast<char*>(values.data()),
                  static_cast<std::streamsize>(numVertices * sizeof(ValueType)));
    }
    else
    {
      for (std::size_t vertex = 0; vertex < numVertices; ++vertex)
      {
        inFile >> values[vertex];
      }
    }

    currTime = totalTime.GetElapsedTime();
    dataReadTime = currTime - prevTime;
    prevTime = currTime;

    // Create viskores data set
    viskores::cont::DataSetBuilderUniform dsb;
    viskores::cont::DataSet ds;
    if (nDims == 2)
    {
      const viskores::Id2 v_dims{
        static_cast<viskores::Id>(dims[0]),
        static_cast<viskores::Id>(dims[1]),
      };
      const viskores::Vec<ValueType, 2> v_origin{ static_cast<ValueType>(offset[0]),
                                                  static_cast<ValueType>(offset[1]) };
      const viskores::Vec<ValueType, 2> v_spacing{ 1, 1 };
      ds = dsb.Create(v_dims, v_origin, v_spacing);
      viskores::cont::CellSetStructured<2> cs;
      cs.SetPointDimensions(v_dims);
      cs.SetGlobalPointDimensions(viskores::Id2{ globalSize[0], globalSize[1] });
      cs.SetGlobalPointIndexStart(viskores::Id2{ offset[0], offset[1] });
      ds.SetCellSet(cs);
    }
    else
    {
      VISKORES_ASSERT(nDims == 3);
      const viskores::Id3 v_dims{ static_cast<viskores::Id>(dims[0]),
                                  static_cast<viskores::Id>(dims[1]),
                                  static_cast<viskores::Id>(dims[2]) };
      const viskores::Vec<ValueType, 3> v_origin{ static_cast<ValueType>(offset[0]),
                                                  static_cast<ValueType>(offset[1]),
                                                  static_cast<ValueType>(offset[2]) };
      viskores::Vec<ValueType, 3> v_spacing(1, 1, 1);
      ds = dsb.Create(v_dims, v_origin, v_spacing);
      viskores::cont::CellSetStructured<3> cs;
      cs.SetPointDimensions(v_dims);
      cs.SetGlobalPointDimensions(globalSize);
      cs.SetGlobalPointIndexStart(viskores::Id3{ offset[0], offset[1], offset[2] });
      ds.SetCellSet(cs);
    }

    ds.AddPointField("values", values);
    // and add to partition
    useDataSet.AppendPartition(ds);

    localBlockIndicesPortal.Set(
      blockNo,
      viskores::Id3{ static_cast<viskores::Id>(blockIndex[0]),
                     static_cast<viskores::Id>(blockIndex[1]),
                     static_cast<viskores::Id>(nDims == 3 ? blockIndex[2] : 0) });

    if (blockNo == 0)
    {
      blocksPerDim = viskores::Id3{ static_cast<viskores::Id>(bpd[0]),
                                    static_cast<viskores::Id>(bpd[1]),
                                    static_cast<viskores::Id>(nDims == 3 ? bpd[2] : 1) };
    }
  }
  currTime = totalTime.GetElapsedTime();
  buildDatasetTime = currTime - prevTime;
  return true;
}


/// Read data from a single file and split the data into blocks across ranks
/// This is a simple implementation that will read the full data on all ranks
/// and then extract only the relevant subblock for that rank.
/// The function support reading from BOV as well from ASCII files
///
/// @param[in] rank The current MPI rank the function is called from
/// @param[in] size The number of MPI ranks
/// @param[in] filename Name of the file with %d as placeholder for the integer index of the block
/// @param[in] numBlocks Number of blocks to use during computation
/// @param[in] blocksPerRank Number of data blocks to process on each rank
/// @param[out] nDims Number of data dimensions (i.e, 2 or 3)
/// @param[out] useDataSet Viskores partioned dataset to be used with the distributed contour tree filter
/// @param[out] globalSize Global extends of the input mesh (i.e., number of mesh points in each dimension)
/// @param[in] blocksPerDim  Number of data blocks used in each data dimension
/// @param[in] localBlockIndices  Array with the (x,y,z) index of each local data block with
///                              with respect to blocksPerDim
/// @param[in] dataReadTime Time to read the data
/// @param[in] buildDatasetTime Time to construct the Viskores datasets
/// @returns bool indicating whether the read was successful or not
template <typename ValueType>
bool readSingleBlockFile(const int& rank,
                         const int& size,
                         const std::string& filename,
                         const int& numBlocks,
                         const int& blocksPerRank,
                         std::vector<viskores::Float32>::size_type& nDims,
                         viskores::cont::PartitionedDataSet& useDataSet,
                         viskores::Id3& globalSize,
                         viskores::Id3& blocksPerDim,
                         viskores::cont::ArrayHandle<viskores::Id3>& localBlockIndices,
                         viskores::Float64& dataReadTime,
                         viskores::Float64& buildDatasetTime)
{
  viskores::cont::Timer totalTime;
  totalTime.Start();
  viskores::Float64 prevTime = 0;
  viskores::Float64 currTime = 0;

  localBlockIndices.Allocate(blocksPerRank);
  auto localBlockIndicesPortal = localBlockIndices.WritePortal();

  viskores::cont::DataSet inDataSet;
  // TODO: Currently FloatDefault would be fine, but it could cause problems if we ever read binary files here.
  std::vector<ValueType> values;
  std::vector<viskores::Id> dims;

  // Read BOV data file
  if (filename.compare(filename.length() - 3, 3, "bov") == 0)
  {
    std::cout << "Reading BOV file" << std::endl;
    viskores::io::BOVDataSetReader reader(filename);
    inDataSet = reader.ReadDataSet();
    nDims = 3;
    currTime = totalTime.GetElapsedTime();
    dataReadTime = currTime - prevTime;
    prevTime = currTime;
    // Copy the data into the values array so we can construct a multiblock dataset
    viskores::Id3 pointDimensions;
    auto cellSet = inDataSet.GetCellSet();
    viskores::cont::CastAndCall(
      cellSet, viskores::worklet::contourtree_augmented::GetPointDimensions(), pointDimensions);
    std::cout << "Point dimensions are " << pointDimensions << std::endl;
    dims.resize(3);
    dims[0] = pointDimensions[0];
    dims[1] = pointDimensions[1];
    dims[2] = pointDimensions[2];
    auto tempFieldData = inDataSet.GetField(0).GetData();
    values.resize(static_cast<std::size_t>(tempFieldData.GetNumberOfValues()));
    auto valuesHandle = viskores::cont::make_ArrayHandle(values, viskores::CopyFlag::Off);
    viskores::cont::ArrayCopy(tempFieldData, valuesHandle);
    valuesHandle.SyncControlArray(); //Forces values to get updated if copy happened on GPU
  }
  // Read ASCII data input
  else
  {
    std::cout << "Reading ASCII file" << std::endl;
    std::ifstream inFile(filename);
    if (inFile.bad())
      return 0;

    // Read the dimensions of the mesh, i.e,. number of elementes in x, y, and z
    std::string line;
    getline(inFile, line);
    std::istringstream linestream(line);
    viskores::Id dimVertices;
    while (linestream >> dimVertices)
    {
      dims.push_back(dimVertices);
    }
    // Swap dimensions so that they are from fastest to slowest growing
    // dims[0] -> col; dims[1] -> row, dims[2] ->slice
    std::swap(dims[0], dims[1]);

    // Compute the number of vertices, i.e., xdim * ydim * zdim
    nDims = static_cast<unsigned short>(dims.size());
    std::size_t numVertices = static_cast<std::size_t>(
      std::accumulate(dims.begin(), dims.end(), std::size_t(1), std::multiplies<std::size_t>()));

    // Check the the number of dimensiosn is either 2D or 3D
    bool invalidNumDimensions = (nDims < 2 || nDims > 3);
    // Log any errors if found on rank 0
    VISKORES_LOG_IF_S(viskores::cont::LogLevel::Error,
                      invalidNumDimensions && (rank == 0),
                      "The input mesh is " << nDims
                                           << "D. The input data must be either 2D or 3D.");
    // If we found any errors in the setttings than finalize MPI and exit the execution
    if (invalidNumDimensions)
    {
      return false;
    }

    // Read data
    values.resize(numVertices);
    for (std::size_t vertex = 0; vertex < numVertices; ++vertex)
    {
      inFile >> values[vertex];
    }

    // finish reading the data
    inFile.close();

    currTime = totalTime.GetElapsedTime();
    dataReadTime = currTime - prevTime;
    prevTime = currTime;

  } // END ASCII Read

  // Create a multi-block dataset for multi-block DIY-paralle processing
  blocksPerDim = nDims == 3 ? viskores::Id3(1, 1, numBlocks)
                            : viskores::Id3(1, numBlocks, 1); // Decompose the data into
  globalSize = nDims == 3 ? viskores::Id3(static_cast<viskores::Id>(dims[0]),
                                          static_cast<viskores::Id>(dims[1]),
                                          static_cast<viskores::Id>(dims[2]))
                          : viskores::Id3(static_cast<viskores::Id>(dims[0]),
                                          static_cast<viskores::Id>(dims[1]),
                                          static_cast<viskores::Id>(1));
  std::cout << blocksPerDim << " " << globalSize << std::endl;
  {
    viskores::Id lastDimSize =
      (nDims == 2) ? static_cast<viskores::Id>(dims[1]) : static_cast<viskores::Id>(dims[2]);
    if (size > (lastDimSize / 2.))
    {
      VISKORES_LOG_IF_S(viskores::cont::LogLevel::Error,
                        rank == 0,
                        "Number of ranks too large for data. Use " << lastDimSize / 2
                                                                   << "or fewer ranks");
      return false;
    }
    viskores::Id standardBlockSize = (viskores::Id)(lastDimSize / numBlocks);
    viskores::Id blockSize = standardBlockSize;
    viskores::Id blockSliceSize = nDims == 2 ? static_cast<viskores::Id>(dims[0])
                                             : static_cast<viskores::Id>((dims[0] * dims[1]));
    viskores::Id blockNumValues = blockSize * blockSliceSize;

    viskores::Id startBlock = blocksPerRank * rank;
    viskores::Id endBlock = startBlock + blocksPerRank;
    for (viskores::Id blockIndex = startBlock; blockIndex < endBlock; ++blockIndex)
    {
      viskores::Id localBlockIndex = blockIndex - startBlock;
      viskores::Id blockStart = blockIndex * blockNumValues;
      viskores::Id blockEnd = blockStart + blockNumValues;
      if (blockIndex < (numBlocks - 1)) // add overlap between regions
      {
        blockEnd += blockSliceSize;
      }
      else
      {
        blockEnd = lastDimSize * blockSliceSize;
      }
      viskores::Id currBlockSize = (viskores::Id)((blockEnd - blockStart) / blockSliceSize);

      viskores::cont::DataSetBuilderUniform dsb;
      viskores::cont::DataSet ds;

      // 2D data
      if (nDims == 2)
      {
        viskores::Id2 vdims;
        vdims[0] = static_cast<viskores::Id>(dims[0]);
        vdims[1] = static_cast<viskores::Id>(currBlockSize);
        viskores::Vec<ValueType, 2> origin(0, blockIndex * blockSize);
        viskores::Vec<ValueType, 2> spacing(1, 1);
        ds = dsb.Create(vdims, origin, spacing);
        viskores::cont::CellSetStructured<2> cs;
        cs.SetPointDimensions(vdims);
        cs.SetGlobalPointDimensions(viskores::Id2{ globalSize[0], globalSize[1] });
        cs.SetGlobalPointIndexStart(viskores::Id2{ 0, (blockStart / blockSliceSize) });
        ds.SetCellSet(cs);
        localBlockIndicesPortal.Set(localBlockIndex, viskores::Id3(0, blockIndex, 0));
      }
      // 3D data
      else
      {
        viskores::Id3 vdims;
        vdims[0] = static_cast<viskores::Id>(dims[0]);
        vdims[1] = static_cast<viskores::Id>(dims[1]);
        vdims[2] = static_cast<viskores::Id>(currBlockSize);
        viskores::Vec<ValueType, 3> origin(0, 0, (blockIndex * blockSize));
        viskores::Vec<ValueType, 3> spacing(1, 1, 1);
        ds = dsb.Create(vdims, origin, spacing);
        viskores::cont::CellSetStructured<3> cs;
        cs.SetPointDimensions(vdims);
        cs.SetGlobalPointDimensions(globalSize);
        cs.SetGlobalPointIndexStart(viskores::Id3(0, 0, blockStart / blockSliceSize));
        ds.SetCellSet(cs);
        localBlockIndicesPortal.Set(localBlockIndex, viskores::Id3(0, 0, blockIndex));
      }

      std::vector<viskores::Float32> subValues((values.begin() + blockStart),
                                               (values.begin() + blockEnd));

      ds.AddPointField("values", subValues);
      useDataSet.AppendPartition(ds);
    }
  }
  currTime = totalTime.GetElapsedTime();
  buildDatasetTime = currTime - prevTime;
  return true;
}

#endif
