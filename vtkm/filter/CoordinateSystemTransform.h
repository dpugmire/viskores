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

#ifndef vtk_m_filter_CoordianteSystemTransform_h
#define vtk_m_filter_CoordianteSystemTransform_h

#include <vtkm/filter/FilterField.h>
#include <vtkm/worklet/CoordinateSystemTransform.h>

namespace vtkm
{
namespace filter
{
/// \brief
///
/// Generate a coordinate transformation on coordiantes from a dataset.
template <typename S>
class CylindricalCoordinateTransform
  : public vtkm::filter::FilterField<CylindricalCoordinateTransform<S>>
{
public:
  VTKM_CONT
  CylindricalCoordinateTransform();

  VTKM_CONT void SetCartesianToCylindrical() { Worklet.SetCartesianToCylindrical(); }
  VTKM_CONT void SetCylindricalToCartesian() { Worklet.SetCylindricalToCartesian(); }

  template <typename T, typename StorageType, typename DerivedPolicy, typename DeviceAdapter>
  VTKM_CONT vtkm::cont::DataSet DoExecute(const vtkm::cont::DataSet& input,
                                          const vtkm::cont::ArrayHandle<T, StorageType>& field,
                                          const vtkm::filter::FieldMetadata& fieldMeta,
                                          const vtkm::filter::PolicyBase<DerivedPolicy>& policy,
                                          const DeviceAdapter& tag);

private:
  vtkm::worklet::CylindricalCoordinateTransform<S> Worklet;
};

template <typename S>
class FilterTraits<CylindricalCoordinateTransform<S>>
{
public:
  //Point Elevation can only convert Float and Double Vec3 arrays
  using InputFieldTypeList = vtkm::TypeListTagFieldVec3;
};

template <typename T>
class SphericalCoordinateTransform
  : public vtkm::filter::FilterField<SphericalCoordinateTransform<T>>
{
public:
  VTKM_CONT
  SphericalCoordinateTransform();

  VTKM_CONT void SetCartesianToSpherical() { Worklet.SetCartesianToSpherical(); }
  VTKM_CONT void SetSphericalToCartesian() { Worklet.SetSphericalToCartesian(); }

  template <typename StorageType, typename DerivedPolicy, typename DeviceAdapter>
  VTKM_CONT vtkm::cont::DataSet DoExecute(const vtkm::cont::DataSet& input,
                                          const vtkm::cont::ArrayHandle<T, StorageType>& field,
                                          const vtkm::filter::FieldMetadata& fieldMeta,
                                          const vtkm::filter::PolicyBase<DerivedPolicy>& policy,
                                          const DeviceAdapter& tag);

private:
  vtkm::worklet::SphericalCoordinateTransform<T> Worklet;
};

template <typename T>
class FilterTraits<SphericalCoordinateTransform<T>>
{
public:
  //Point Elevation can only convert Float and Double Vec3 arrays
  using InputFieldTypeList = vtkm::TypeListTagFieldVec3;
};
}
} // namespace vtkm::filter

#include <vtkm/filter/CoordinateSystemTransform.hxx>

#endif // vtk_m_filter_CoordianteSystemTransform_h
