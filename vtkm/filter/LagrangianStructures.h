//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//============================================================================

#ifndef vtk_m_filter_LagrangianStructures_h
#define vtk_m_filter_LagrangianStructures_h

#include <vtkm/filter/FilterDataSetWithField.h>
#include <vtkm/worklet/ParticleAdvection.h>
#include <vtkm/worklet/particleadvection/GridEvaluators.h>
#include <vtkm/worklet/particleadvection/Integrators.h>

namespace vtkm
{
namespace filter
{

class LagrangianStructures : public vtkm::filter::FilterDataSetWithField<LagrangianStructures>
{
public:
  using Scalar = vtkm::worklet::particleadvection::ScalarType;
  using Vector = vtkm::Vec<Scalar, 3>;

  VTKM_CONT
  LagrangianStructures();

  VTKM_CONT
  void SetStepSize(ScalarType s) { this->StepSize = s; }
  VTKM_CONT
  ScalarType GetStepSize() { return this->StepSize; }

  VTKM_CONT
  void SetNumberOfSteps(vtkm::Id n) { this->NumberOfSteps = n; }
  VTKM_CONT
  vtkm::Id GetNumberOfSteps() { return this->NumberOfSteps; }

  VTKM_CONT
  void SetUseAuxiliaryGrid(bool useAuxiliaryGrid){ this->UseAuxiliaryGrid = useAuxiliaryGrid };
  VTKM_CONT
  bool GetUseAuxiliaryGrid() { return this->UseAuxiliaryGrid; }

  VTKM_CONT
  void SetAuxiliaryGridDimensions(vtkm::Id3 auxiliaryDims){ this->AuxiliaryDims = auxiliaryDims };
  VTKM_CONT
  vtkm::Id3 GetAuxiliaryGridDimensions(){ return this->AuxiliaryDims };

  VTKM_CONT
  void SetUseFlowMapOutput(bool useFLowMapOutput){ this->UseFlowMapOutput = useFlowMapOutput };
  VTKM_CONT
  bool GetUseFlowMapOutput() { return this->UseFlowMapOutput; }

  VTKM_CONT
  inline void SetFlowMapOutput(vtkm::cont::ArrayHandle<Vector>& flowMap){
    this->FlowMapOutput = flowMap
  };
  VTKM_CONT
  inline vtkm::cont::ArrayHandle<Vector> GetFlowMapOutput(){ return this->FlowMapOutput };

  template <typename T, typename StorageType, typename DerivedPolicy>
  VTKM_CONT vtkm::cont::DataSet DoExecute(
    const vtkm::cont::DataSet& input,
    const vtkm::cont::ArrayHandle<vtkm::Vec<T, 3>, StorageType>& field,
    const vtkm::filter::FieldMetadata& fieldMeta,
    const vtkm::filter::PolicyBase<DerivedPolicy>& policy);

  //Map a new field onto the resulting dataset after running the filter
  //this call is only valid
  template <typename T, typename StorageType, typename DerivedPolicy>
  VTKM_CONT bool DoMapField(vtkm::cont::DataSet& result,
                            const vtkm::cont::ArrayHandle<T, StorageType>& input,
                            const vtkm::filter::FieldMetadata& fieldMeta,
                            vtkm::filter::PolicyBase<DerivedPolicy> policy);

private:
  vtkm::worklet::LagrangianStructures Worklet;
  Scalar StepSize;
  vtkm::Id NumberOfSteps;
  bool UseAuxiliaryGrid = false;
  vtkm::Id3 AuxiliaryDims;
  bool UseFlowMapOutput;
  vtkm::cont::ArrayHandle<Vector> FlowMapOutput;
};

template <>
class FilterTraits<LagrangianStructures>
{
public:
  struct TypeListTagLagrangianStructures
    : vtkm::ListTagBase<vtkm::Vec<vtkm::Float32, 3>, vtkm::Vec<vtkm::Float64, 3>>
  {
  };
  using InputFieldTypeList = TypeListTagLagrangianStructures;
};
}
} // namespace vtkm::filter

#include <vtkm/filter/LagrangianStructures.hxx>

#endif // vtk_m_filter_LagrangianStructures_h
