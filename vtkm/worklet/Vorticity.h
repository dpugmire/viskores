//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//  Copyright 2014 Sandia Corporation.
//  Copyright 2014 UT-Battelle, LLC.
//  Copyright 2014 Los Alamos National Security.
//
//  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
//  the U.S. Government retains certain rights in this software.
//
//  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
//  Laboratory (LANL), the U.S. Government retains certain rights in
//  this software.
//============================================================================

#ifndef vtk_m_worklet_Vorticity_h
#define vtk_m_worklet_Vorticity_h

#include <vtkm/worklet/WorkletMapField.h>

namespace vtkm {
namespace worklet {

struct VorticityInputTypes
    : vtkm::ListTagBase<
                        vtkm::Vec< vtkm::Vec<vtkm::Float32,3>, 3>,
                        vtkm::Vec< vtkm::Vec<vtkm::Float64,3>, 3>
                        > {  };

struct Vorticity : public vtkm::worklet::WorkletMapField
{
  typedef void ControlSignature(FieldIn<VorticityInputTypes> input,
                                FieldOut<Vec3> output);
  typedef void ExecutionSignature(_1,_2);
  typedef _1 InputDomain;

  template<typename InputType, typename OutputType>
  VTKM_EXEC
  void operator()(const InputType &input, OutputType &vorticity) const
  {
    vorticity[0] = input[2][1] - input[1][2];
    vorticity[1] = input[0][2] - input[2][0];
    vorticity[2] = input[1][0] - input[0][1];
  }
};

struct QCriterion : public vtkm::worklet::WorkletMapField
{
  typedef void ControlSignature(FieldIn<VorticityInputTypes> input,
                                FieldOut<Scalar> output);
  typedef void ExecutionSignature(_1,_2);
  typedef _1 InputDomain;

  template<typename InputType, typename OutputType>
  VTKM_EXEC
  void operator()(const InputType &input, OutputType &qcriterion) const
  {
    OutputType t1 =
          ((input[2][1] - input[1][2]) * (input[2][1] - input[1][2]) +
           (input[1][0] - input[0][1]) * (input[1][0] - input[0][1]) +
           (input[0][2] - input[2][0]) * (input[0][2] - input[2][0])) / 2.0f;
      OutputType t2 =
          input[0][0] * input[0][0] + input[1][1] * input[1][1] +
          input[2][2] * input[2][2] +
          ((input[1][0] + input[0][1]) * (input[1][0] + input[0][1]) +
           (input[2][0] + input[0][2]) * (input[2][0] + input[0][2]) +
           (input[2][1] + input[1][2]) * (input[2][1] + input[1][2])) / 2.0f;

    qcriterion = (t1 - t2) / 2.0f;
  }

};

}
} // namespace vtkm::worklet

#endif