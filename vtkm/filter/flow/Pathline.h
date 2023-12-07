//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//============================================================================

#ifndef vtk_m_filter_flow_Pathline_h
#define vtk_m_filter_flow_Pathline_h

#include <vtkm/filter/flow/FilterParticleAdvectionUnsteadyState.h>
#include <vtkm/filter/flow/FlowTypes.h>
#include <vtkm/filter/flow/vtkm_filter_flow_export.h>

#include <vtkm/filter/flow/worklet/Analysis.h>
#include <vtkm/filter/flow/worklet/Field.h>
#include <vtkm/filter/flow/worklet/Termination.h>

namespace vtkm
{
namespace filter
{
namespace flow
{

class Pathline;

template <>
struct FlowTraits<Pathline>
{
  using ParticleType = vtkm::Particle;
  using TerminationType = vtkm::worklet::flow::NormalTermination;
  using AnalysisType = vtkm::worklet::flow::StreamlineAnalysis<ParticleType>;
  using ArrayType = vtkm::cont::ArrayHandle<vtkm::Vec3f>;
  using FieldType = vtkm::worklet::flow::VelocityField<ArrayType>;
};

/// \brief Advect particles in a time-varying vector field and display the path they take.
///
/// This filter takes as input a velocity vector field, changing between two time steps,
/// and seed locations. It then traces the path each seed point would take if moving at
/// the velocity specified by the field.
///
/// The output of this filter is a `vtkm::cont::DataSet` containing a collection of poly-lines
/// representing the paths the seed particles take.
class VTKM_FILTER_FLOW_EXPORT Pathline
  : public vtkm::filter::flow::FilterParticleAdvectionUnsteadyState<Pathline>
{
public:
  using ParticleType = typename FlowTraits<Pathline>::ParticleType;
  using TerminationType = typename FlowTraits<Pathline>::TerminationType;
  using AnalysisType = typename FlowTraits<Pathline>::AnalysisType;
  using ArrayType = typename FlowTraits<Pathline>::ArrayType;
  using FieldType = typename FlowTraits<Pathline>::FieldType;

  VTKM_CONT FieldType GetField(const vtkm::cont::DataSet& data) const;

  VTKM_CONT TerminationType GetTermination(const vtkm::cont::DataSet& data) const;

  VTKM_CONT AnalysisType GetAnalysis(const vtkm::cont::DataSet& data) const;
};

}
}
} // namespace vtkm::filter::flow

#endif // vtk_m_filter_flow_Pathline_h
