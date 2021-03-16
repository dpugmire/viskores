//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//============================================================================

#ifndef vtk_m_filter_particle_density_ngp_h
#define vtk_m_filter_particle_density_ngp_h

#include <vtkm/filter/FilterField.h>

namespace vtkm
{
namespace filter
{
/// \brief Estimate the density of particles using the Nearest Grid Point method
/// This filter takes a DataSet with CellSetExplicit of shape Vertex as input data.
/// The vertices are treated as a set of particles with infinitesimal size and finite
/// mass (or other scalar attributes such as charge). The filter estimates density by
/// imposing a regular grid as specified in the constructor and summing the mass of
/// particles within each cell in the grid.

// We only need the CoordinateSystem and scalar fields of the input dataset thus a FilterField
class ParticleDensityNearestGridPoint
  : public vtkm::filter::FilterField<ParticleDensityNearestGridPoint>
{
public:
  // deposit scalar field associated with particles, e.g. mass/charge to mesh cells
  using SupportedTypes = vtkm::TypeListFieldScalar;

  ParticleDensityNearestGridPoint(const vtkm::Id3& dimension,
                                  const vtkm::Vec3f& origin,
                                  const vtkm::Vec3f& spacing);

  ParticleDensityNearestGridPoint(const vtkm::Id3& dimension, const vtkm::Bounds& bounds);

  template <typename T, typename StorageType, typename Policy>
  VTKM_CONT vtkm::cont::DataSet DoExecute(const vtkm::cont::DataSet& input,
                                          const vtkm::cont::ArrayHandle<T, StorageType>& field,
                                          const vtkm::filter::FieldMetadata& fieldMeta,
                                          vtkm::filter::PolicyBase<Policy> policy);
  VTKM_CONT SetDivideByVolume(bool yes) { this->DivideByVolume = yes; }

  VTKM_CONT bool GetDivideByVolume() const { return this->DivideByVolume; }

private:
  vtkm::Id3 Dimension; // Cell dimension
  vtkm::Vec3f Origin;
  vtkm::Vec3f Spacing;
  bool DivideByVolume;
};
}
}

#include <vtkm/filter/ParticleDensityNearestGridPoint.hxx>

#endif //vtk_m_filter_particle_density_ngp_h
