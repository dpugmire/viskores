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
//  This code is an extension of the algorithm presented in the paper:
//  Parallel Peak Pruning for Scalable SMP Contour Tree Computation.
//  Hamish Carr, Gunther Weber, Christopher Sewell, and James Ahrens.
//  Proceedings of the IEEE Symposium on Large Data Analysis and Visualization
//  (LDAV), October 2016, Baltimore, Maryland.
//
//  The PPP2 algorithm and software were jointly developed by
//  Hamish Carr (University of Leeds), Gunther H. Weber (LBNL), and
//  Oliver Ruebel (LBNL)
//==============================================================================


#ifndef vtkm_worklet_contourtree_ppp2_mesh_dem_triangulation_3d_freudenthal_h
#define vtkm_worklet_contourtree_ppp2_mesh_dem_triangulation_3d_freudenthal_h

#include <cstdlib>
#include <unistd.h>
#include <vtkm/Types.h>
#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/ArrayHandleGroupVec.h>

#include <vtkm/cont/ExecutionObjectBase.h>
#include <vtkm/worklet/contourtree_ppp2/Mesh_DEM_MeshTypes/Freudenthal_3D_Inc/ExecutionObject_MeshStructure.h>
#include <vtkm/worklet/contourtree_ppp2/Mesh_DEM_MeshTypes/Freudenthal_3D_Inc/Types.h>
#include <vtkm/worklet/contourtree_ppp2/Mesh_DEM_Triangulation.h>

//Define namespace alias for the freudenthal types to make the code a bit more readable
namespace m3d_freudenthal_inc_ns = vtkm::worklet::contourtree_ppp2::mesh_dem_3d_freudenthal_inc;

namespace vtkm
{
namespace worklet
{
namespace contourtree_ppp2
{

template <typename T, typename StorageType, typename DeviceAdapter>
class Mesh_DEM_Triangulation_3D_Freudenthal
  : public Mesh_DEM_Triangulation_3D<T, StorageType, DeviceAdapter>,
    public vtkm::cont::ExecutionObjectBase
{ // class Mesh_DEM_Triangulation
public:
  // Constants and case tables
  m3d_freudenthal_inc_ns::edgeBoundaryDetectionMasksType edgeBoundaryDetectionMasks;
  m3d_freudenthal_inc_ns::neighbourOffsetsType neighbourOffsets;
  m3d_freudenthal_inc_ns::linkComponentCaseTableType linkComponentCaseTable;

  // Mesh helper functions
  void setPrepareForExecutionBehavior(bool getMax);

  template <typename DeviceTag>
  mesh_dem_3d_freudenthal_inc::ExecutionObject_MeshStructure<DeviceTag> PrepareForExecution(
    DeviceTag) const;

  Mesh_DEM_Triangulation_3D_Freudenthal(vtkm::Id nrows, vtkm::Id ncols, vtkm::Id nslices);

private:
  bool mGetMax; // Define the behavior ofr the PrepareForExecution function
};              // class Mesh_DEM_Triangulation

// creates input mesh
template <typename T, typename StorageType, typename DeviceAdapter>
Mesh_DEM_Triangulation_3D_Freudenthal<T, StorageType, DeviceAdapter>::
  Mesh_DEM_Triangulation_3D_Freudenthal(vtkm::Id nrows, vtkm::Id ncols, vtkm::Id nslices)
  : Mesh_DEM_Triangulation_3D<T, StorageType, DeviceAdapter>(nrows, ncols, nslices)

{
  // Initialize the case tables in vtkm
  edgeBoundaryDetectionMasks = vtkm::cont::make_ArrayHandle(
    m3d_freudenthal_inc_ns::edgeBoundaryDetectionMasks, m3d_freudenthal_inc_ns::N_INCIDENT_EDGES);
  neighbourOffsets = vtkm::cont::make_ArrayHandleGroupVec<3>(vtkm::cont::make_ArrayHandle(
    m3d_freudenthal_inc_ns::neighbourOffsets, m3d_freudenthal_inc_ns::N_INCIDENT_EDGES * 3));
  linkComponentCaseTable = vtkm::cont::make_ArrayHandle(
    m3d_freudenthal_inc_ns::linkComponentCaseTable, m3d_freudenthal_inc_ns::LINK_COMPONENT_CASES);
}


template <typename T, typename StorageType, typename DeviceAdapter>
void Mesh_DEM_Triangulation_3D_Freudenthal<T, StorageType, DeviceAdapter>::
  setPrepareForExecutionBehavior(bool getMax)
{
  this->mGetMax = getMax;
}

// Get VTKM execution object that represents the structure of the mesh and provides the mesh helper functions on the device
template <typename T, typename StorageType, typename DeviceAdapter>
template <typename DeviceTag>
mesh_dem_3d_freudenthal_inc::ExecutionObject_MeshStructure<DeviceTag>
  Mesh_DEM_Triangulation_3D_Freudenthal<T, StorageType, DeviceAdapter>::PrepareForExecution(
    DeviceTag) const
{
  return mesh_dem_3d_freudenthal_inc::ExecutionObject_MeshStructure<DeviceTag>(
    this->nRows,
    this->nCols,
    this->nSlices,
    m3d_freudenthal_inc_ns::N_INCIDENT_EDGES,
    this - mGetMax,
    this->sortIndices,
    edgeBoundaryDetectionMasks,
    neighbourOffsets,
    linkComponentCaseTable);
}


} // namespace contourtree_ppp2
} // worklet
} // vtkm

#endif
