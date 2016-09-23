//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//  Copyright 2016 Sandia Corporation.
//  Copyright 2016 UT-Battelle, LLC.
//  Copyright 2016 Los Alamos National Security.
//
//  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
//  the U.S. Government retains certain rights in this software.
//
//  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
//  Laboratory (LANL), the U.S. Government retains certain rights in
//  this software.
//============================================================================

#include <vtkm/rendering/internal/RunTriangulator.h>

#include <vtkm/cont/TryExecute.h>
#include <vtkm/rendering/Triangulator.h>

namespace vtkm {
namespace rendering {
namespace internal {

namespace {

struct TriangulatorFunctor
{
  vtkm::cont::DynamicCellSet CellSet;
  vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Id,4> > Indices;
  vtkm::Id NumberOfTriangles;

  VTKM_CONT_EXPORT
  TriangulatorFunctor(vtkm::cont::DynamicCellSet cellSet)
    : CellSet(cellSet)
  {  }

  template<typename Device>
  VTKM_CONT_EXPORT
  bool operator()(Device)
  {
    vtkm::rendering::Triangulator<Device> triangulator;
    triangulator.Run(this->CellSet, this->Indices, this->NumberOfTriangles);
    return true;
  }
};

} // anonymous namespace

void RunTriangulator(const vtkm::cont::DynamicCellSet &cellSet,
                     vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Id,4> > &indices,
                     vtkm::Id &numberOfTriangles,
                     vtkm::cont::internal::RuntimeDeviceTracker &tracker)
{
  // TODO: Should the rendering library support policies or some other way to
  // configure with custom devices?
  TriangulatorFunctor triangulatorFunctor(cellSet);
  if (!vtkm::cont::TryExecute(triangulatorFunctor,
                              tracker,
                              VTKM_DEFAULT_DEVICE_ADAPTER_LIST_TAG()))
  {
    throw vtkm::cont::ErrorExecution("Failed to execute triangulator.");
  }

  indices = triangulatorFunctor.Indices;
  numberOfTriangles = triangulatorFunctor.NumberOfTriangles;
}

void RunTriangulator(const vtkm::cont::DynamicCellSet &cellSet,
                     vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Id,4> > &indices,
                     vtkm::Id &numberOfTriangles)
{
  vtkm::cont::internal::RuntimeDeviceTracker tracker;
  vtkm::rendering::internal::RunTriangulator(
        cellSet, indices, numberOfTriangles, tracker);
}

}
}
} // namespace vtkm::rendering::internal
