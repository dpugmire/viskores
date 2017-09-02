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

#include <vtkm/filter/Gradient.h>

#include <vtkm/cont/testing/MakeTestDataSet.h>
#include <vtkm/cont/testing/Testing.h>

namespace
{

void TestCellGradientUniform3D()
{
  std::cout << "Testing Gradient Filter with cell output on 3D strucutred data" << std::endl;

  vtkm::cont::testing::MakeTestDataSet testDataSet;
  vtkm::cont::DataSet dataSet = testDataSet.Make3DUniformDataSet0();

  vtkm::filter::Result result;
  vtkm::filter::Gradient gradient;
  gradient.SetOutputFieldName("Gradient");

  gradient.SetComputeVorticity(true);  //this wont work as we have a scalar field
  gradient.SetComputeQCriterion(true); //this wont work as we have a scalar field

  result = gradient.Execute(dataSet, dataSet.GetField("pointvar"));

  VTKM_TEST_ASSERT(result.GetField().GetName() == "Gradient", "Field was given the wrong name.");
  VTKM_TEST_ASSERT(result.GetField().GetAssociation() == vtkm::cont::Field::ASSOC_CELL_SET,
                   "Field was given the wrong association.");

  //verify that the vorticity and qcriterion fields don't exist
  const vtkm::cont::DataSet& outputDS = result.GetDataSet();
  VTKM_TEST_ASSERT(outputDS.HasField("Vorticity") == false,
                   "scalar gradients can't generate vorticity");
  VTKM_TEST_ASSERT(outputDS.HasField("QCriterion") == false,
                   "scalar gradients can't generate qcriterion");

  vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Float32, 3>> resultArrayHandle;
  const bool valid = result.FieldAs(resultArrayHandle);
  VTKM_TEST_ASSERT(valid, "result of gradient is not expected type");

  vtkm::Vec<vtkm::Float64, 3> expected[4] = {
    { 10.025, 30.075, 60.125 },
    { 10.025, 30.075, 60.125 },
    { 10.025, 30.075, 60.175 },
    { 10.025, 30.075, 60.175 },
  };
  for (int i = 0; i < 4; ++i)
  {
    VTKM_TEST_ASSERT(test_equal(resultArrayHandle.GetPortalConstControl().Get(i), expected[i]),
                     "Wrong result for CellGradient filter on 3D uniform data");
  }
}

void TestCellGradientUniform3DWithVectorField()
{
  std::cout << "Testing Gradient Filter with vector cell output on 3D strucutred data" << std::endl;
  vtkm::cont::testing::MakeTestDataSet testDataSet;
  vtkm::cont::DataSet dataSet = testDataSet.Make3DUniformDataSet0();

  //Verify that we can compute the gradient of a 3 component vector
  const int nVerts = 18;
  vtkm::Float64 vars[nVerts] = { 10.1,  20.1,  30.1,  40.1,  50.2,  60.2,  70.2,  80.2,  90.3,
                                 100.3, 110.3, 120.3, 130.4, 140.4, 150.4, 160.4, 170.5, 180.5 };
  std::vector<vtkm::Vec<vtkm::Float64, 3>> vec(nVerts);
  for (std::size_t i = 0; i < vec.size(); ++i)
  {
    vec[i] = vtkm::make_Vec(vars[i], vars[i], vars[i]);
  }
  vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Float64, 3>> input = vtkm::cont::make_ArrayHandle(vec);
  vtkm::cont::DataSetFieldAdd::AddPointField(dataSet, "vec_pointvar", input);

  //we need to add Vec3 array to the dataset
  vtkm::filter::Result result;
  vtkm::filter::Gradient gradient;
  gradient.SetOutputFieldName("vec_gradient");
  gradient.SetComputeVorticity(true);
  gradient.SetComputeQCriterion(true);

  result = gradient.Execute(dataSet, dataSet.GetField("vec_pointvar"));

  VTKM_TEST_ASSERT(result.GetField().GetName() == "vec_gradient",
                   "Field was given the wrong name.");

  VTKM_TEST_ASSERT(result.GetField().GetAssociation() == vtkm::cont::Field::ASSOC_CELL_SET,
                   "Field was given the wrong association.");

  //verify that the vorticity and qcriterion fields DO exist
  const vtkm::cont::DataSet& outputDS = result.GetDataSet();
  VTKM_TEST_ASSERT(outputDS.HasField("Vorticity") == true,
                   "vec gradients should generate vorticity");
  VTKM_TEST_ASSERT(outputDS.HasField("QCriterion") == true,
                   "vec gradients should generate qcriterion");

  vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Vec<vtkm::Float64, 3>, 3>> resultArrayHandle;
  const bool valid = result.FieldAs(resultArrayHandle);
  VTKM_TEST_ASSERT(valid, "result of gradient is not expected type");

  vtkm::Vec<vtkm::Vec<vtkm::Float64, 3>, 3> expected[4] = {
    { { 10.025, 10.025, 10.025 }, { 30.075, 30.075, 30.075 }, { 60.125, 60.125, 60.125 } },
    { { 10.025, 10.025, 10.025 }, { 30.075, 30.075, 30.075 }, { 60.125, 60.125, 60.125 } },
    { { 10.025, 10.025, 10.025 }, { 30.075, 30.075, 30.075 }, { 60.175, 60.175, 60.175 } },
    { { 10.025, 10.025, 10.025 }, { 30.075, 30.075, 30.075 }, { 60.175, 60.175, 60.175 } }
  };
  for (int i = 0; i < 4; ++i)
  {
    vtkm::Vec<vtkm::Vec<vtkm::Float64, 3>, 3> e = expected[i];
    vtkm::Vec<vtkm::Vec<vtkm::Float64, 3>, 3> r = resultArrayHandle.GetPortalConstControl().Get(i);

    VTKM_TEST_ASSERT(test_equal(e[0], r[0]),
                     "Wrong result for vec field CellGradient filter on 3D uniform data");
    VTKM_TEST_ASSERT(test_equal(e[1], r[1]),
                     "Wrong result for vec field CellGradient filter on 3D uniform data");
    VTKM_TEST_ASSERT(test_equal(e[2], r[2]),
                     "Wrong result for vec field CellGradient filter on 3D uniform data");
  }
}

void TestCellGradientExplicit()
{
  std::cout << "Testing Gradient Filter with cell output on Explicit data" << std::endl;

  vtkm::cont::testing::MakeTestDataSet testDataSet;
  vtkm::cont::DataSet dataSet = testDataSet.Make3DExplicitDataSet0();

  vtkm::filter::Result result;
  vtkm::filter::Gradient gradient;
  gradient.SetOutputFieldName("gradient");

  result = gradient.Execute(dataSet, dataSet.GetField("pointvar"));

  VTKM_TEST_ASSERT(result.GetField().GetName() == "gradient", "Field was given the wrong name.");
  VTKM_TEST_ASSERT(result.GetField().GetAssociation() == vtkm::cont::Field::ASSOC_CELL_SET,
                   "Field was given the wrong association.");

  vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Float32, 3>> resultArrayHandle;
  const bool valid = result.FieldAs(resultArrayHandle);
  VTKM_TEST_ASSERT(valid, "result of gradient is not expected type");

  vtkm::Vec<vtkm::Float32, 3> expected[2] = { { 10.f, 10.1f, 0.0f }, { 10.f, 10.1f, -0.0f } };
  for (int i = 0; i < 2; ++i)
  {
    VTKM_TEST_ASSERT(test_equal(resultArrayHandle.GetPortalConstControl().Get(i), expected[i]),
                     "Wrong result for CellGradient filter on 3D explicit data");
  }
}

void TestPointGradientUniform3DWithVectorField()
{
  std::cout << "Testing Gradient Filter with vector point output on 3D strucutred data"
            << std::endl;
  vtkm::cont::testing::MakeTestDataSet testDataSet;
  vtkm::cont::DataSet dataSet = testDataSet.Make3DUniformDataSet0();

  //Verify that we can compute the gradient of a 3 component vector
  const int nVerts = 18;
  vtkm::Float64 vars[nVerts] = { 10.1,  20.1,  30.1,  40.1,  50.2,  60.2,  70.2,  80.2,  90.3,
                                 100.3, 110.3, 120.3, 130.4, 140.4, 150.4, 160.4, 170.5, 180.5 };
  std::vector<vtkm::Vec<vtkm::Float64, 3>> vec(nVerts);
  for (std::size_t i = 0; i < vec.size(); ++i)
  {
    vec[i] = vtkm::make_Vec(vars[i], vars[i], vars[i]);
  }
  vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Float64, 3>> input = vtkm::cont::make_ArrayHandle(vec);
  vtkm::cont::DataSetFieldAdd::AddPointField(dataSet, "vec_pointvar", input);

  //we need to add Vec3 array to the dataset
  vtkm::filter::Result result;
  vtkm::filter::Gradient gradient;
  gradient.SetComputePointGradient(true);
  gradient.SetOutputFieldName("vec_gradient");

  result = gradient.Execute(dataSet, dataSet.GetField("vec_pointvar"));

  VTKM_TEST_ASSERT(result.GetField().GetName() == "vec_gradient",
                   "Field was given the wrong name.");

  VTKM_TEST_ASSERT(result.GetField().GetAssociation() == vtkm::cont::Field::ASSOC_POINTS,
                   "Field was given the wrong association.");

  vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Vec<vtkm::Float64, 3>, 3>> resultArrayHandle;
  const bool valid = result.FieldAs(resultArrayHandle);
  VTKM_TEST_ASSERT(valid, "result of gradient is not expected type");

  vtkm::Vec<vtkm::Vec<vtkm::Float64, 3>, 3> expected[4] = {
    { { 10.0, 10.0, 10.0 }, { 30.0, 30.0, 30.0 }, { 60.1, 60.1, 60.1 } },
    { { 10.0, 10.0, 10.0 }, { 30.1, 30.1, 30.1 }, { 60.1, 60.1, 60.1 } },
    { { 10.0, 10.0, 10.0 }, { 30.1, 30.1, 30.1 }, { 60.2, 60.2, 60.2 } },
    { { 10.1, 10.1, 10.1 }, { 30.0, 30.0, 30.0 }, { 60.2, 60.2, 60.2 } }
  };
  for (int i = 0; i < 4; ++i)
  {
    vtkm::Vec<vtkm::Vec<vtkm::Float64, 3>, 3> e = expected[i];
    vtkm::Vec<vtkm::Vec<vtkm::Float64, 3>, 3> r = resultArrayHandle.GetPortalConstControl().Get(i);

    VTKM_TEST_ASSERT(test_equal(e[0], r[0]),
                     "Wrong result for vec field CellGradient filter on 3D uniform data");
    VTKM_TEST_ASSERT(test_equal(e[1], r[1]),
                     "Wrong result for vec field CellGradient filter on 3D uniform data");
    VTKM_TEST_ASSERT(test_equal(e[2], r[2]),
                     "Wrong result for vec field CellGradient filter on 3D uniform data");
  }
}

void TestPointGradientExplicit()
{
  std::cout << "Testing Gradient Filter with point output on Explicit data" << std::endl;

  vtkm::cont::testing::MakeTestDataSet testDataSet;
  vtkm::cont::DataSet dataSet = testDataSet.Make3DExplicitDataSet0();

  vtkm::filter::Result result;
  vtkm::filter::Gradient gradient;
  gradient.SetComputePointGradient(true);
  gradient.SetOutputFieldName("gradient");

  result = gradient.Execute(dataSet, dataSet.GetField("pointvar"));

  VTKM_TEST_ASSERT(result.GetField().GetName() == "gradient", "Field was given the wrong name.");
  VTKM_TEST_ASSERT(result.GetField().GetAssociation() == vtkm::cont::Field::ASSOC_POINTS,
                   "Field was given the wrong association.");

  vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Float32, 3>> resultArrayHandle;
  const bool valid = result.FieldAs(resultArrayHandle);
  VTKM_TEST_ASSERT(valid, "result of gradient is not expected type");

  vtkm::Vec<vtkm::Float32, 3> expected[2] = { { 10.f, 10.1f, 0.0f }, { 10.f, 10.1f, 0.0f } };
  for (int i = 0; i < 2; ++i)
  {
    VTKM_TEST_ASSERT(test_equal(resultArrayHandle.GetPortalConstControl().Get(i), expected[i]),
                     "Wrong result for CellGradient filter on 3D explicit data");
  }
}

void TestGradient()
{
  TestCellGradientUniform3D();
  TestCellGradientUniform3DWithVectorField();
  TestCellGradientExplicit();

  TestPointGradientUniform3DWithVectorField();
  TestPointGradientExplicit();
}
}

int UnitTestGradient(int, char* [])
{
  return vtkm::cont::testing::Testing::Run(TestGradient);
}
