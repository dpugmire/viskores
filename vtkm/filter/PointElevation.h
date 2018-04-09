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

#ifndef vtk_m_filter_PointElevation_h
#define vtk_m_filter_PointElevation_h

#include <vtkm/filter/FilterField.h>
#include <vtkm/worklet/PointElevation.h>

namespace vtkm
{
namespace filter
{
/// \brief  generate a scalar field along a specified direction
///
/// Generate scalar field from a dataset.
/// The scalar field values lie within a user specified range, and
/// are generated by computing a projection of each dataset point onto
/// a line. The line can be oriented arbitrarily. A typical example is
/// to generate scalars based on elevation or height above a plane.
class PointElevation : public vtkm::filter::FilterField<PointElevation>
{
public:
  VTKM_CONT
  PointElevation();

  VTKM_CONT
  void SetLowPoint(vtkm::Float64 x, vtkm::Float64 y, vtkm::Float64 z);

  VTKM_CONT
  void SetHighPoint(vtkm::Float64 x, vtkm::Float64 y, vtkm::Float64 z);

  VTKM_CONT
  void SetRange(vtkm::Float64 low, vtkm::Float64 high);

  template <typename T, typename StorageType, typename DerivedPolicy, typename DeviceAdapter>
  VTKM_CONT vtkm::cont::DataSet DoExecute(const vtkm::cont::DataSet& input,
                                          const vtkm::cont::ArrayHandle<T, StorageType>& field,
                                          const vtkm::filter::FieldMetadata& fieldMeta,
                                          const vtkm::filter::PolicyBase<DerivedPolicy>& policy,
                                          const DeviceAdapter& tag);

private:
  vtkm::worklet::PointElevation Worklet;
};

template <>
class FilterTraits<PointElevation>
{
public:
  //Point Elevation can only convert Float and Double Vec3 arrays
  using InputFieldTypeList = vtkm::TypeListTagFieldVec3;
};
}
} // namespace vtkm::filter

#include <vtkm/filter/PointElevation.hxx>

#endif // vtk_m_filter_PointElevation_h
