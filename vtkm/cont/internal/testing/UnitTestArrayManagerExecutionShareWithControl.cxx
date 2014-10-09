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
//  Copyright 2014. Los Alamos National Security
//
//  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
//  the U.S. Government retains certain rights in this software.
//
//  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
//  Laboratory (LANL), the U.S. Government retains certain rights in
//  this software.
//============================================================================

#include <vtkm/cont/internal/ArrayManagerExecutionShareWithControl.h>

#include <vtkm/cont/ArrayPortalToIterators.h>
#include <vtkm/cont/StorageBasic.h>

#include <vtkm/cont/testing/Testing.h>

#include <algorithm>
#include <vector>

namespace {

const vtkm::Id ARRAY_SIZE = 10;

template <typename T>
struct TemplatedTests
{
  typedef vtkm::cont::internal::ArrayManagerExecutionShareWithControl<
        T, vtkm::cont::StorageTagBasic>
      ArrayManagerType;
  typedef typename ArrayManagerType::ValueType ValueType;
  typedef vtkm::cont::internal::Storage<T, vtkm::cont::StorageTagBasic>
      StorageType;

  void SetStorage(StorageType &array, const ValueType& value)
  {
    vtkm::cont::ArrayPortalToIterators<typename StorageType::PortalType>
        iterators(array.GetPortal());
    std::fill(iterators.GetBegin(), iterators.GetEnd(), value);
  }

  template <class PortalType>
  bool CheckPortal(const PortalType &portal, const ValueType &value)
  {
    for (vtkm::Id index = 0; index < portal.GetNumberOfValues(); index++)
    {
      if (!test_equal(portal.Get(index), value)) { return false; }
    }
    return true;
  }

  bool CheckStorage(StorageType &array, const ValueType& value)
  {
    return CheckPortal(array.GetPortalConst(), value);
  }

  bool CheckManager(ArrayManagerType &manager, const ValueType &value)
  {
    return CheckPortal(manager.GetPortalConst(), value);
  }

  void InputData()
  {
    const ValueType INPUT_VALUE(45);

    StorageType controlArray;
    controlArray.Allocate(ARRAY_SIZE);
    SetStorage(controlArray, INPUT_VALUE);

    ArrayManagerType executionArray;
    executionArray.LoadDataForInput(controlArray.GetPortalConst());

    // Although the ArrayManagerExecutionShareWithControl class wraps the
    // control array portal in a different array portal, it should still
    // give the same iterator (to avoid any unnecessary indirection).
    VTKM_TEST_ASSERT(
          vtkm::cont::ArrayPortalToIteratorBegin(controlArray.GetPortalConst()) ==
          vtkm::cont::ArrayPortalToIteratorBegin(executionArray.GetPortalConst()),
          "Execution array manager not holding control array iterators.");

    VTKM_TEST_ASSERT(CheckManager(executionArray, INPUT_VALUE),
                     "Did not get correct array back.");
  }

  void InPlaceData()
  {
    const ValueType INPUT_VALUE(50);

    StorageType controlArray;
    controlArray.Allocate(ARRAY_SIZE);
    SetStorage(controlArray, INPUT_VALUE);

    ArrayManagerType executionArray;
    executionArray.LoadDataForInPlace(controlArray.GetPortal());

    // Although the ArrayManagerExecutionShareWithControl class wraps the
    // control array portal in a different array portal, it should still
    // give the same iterator (to avoid any unnecessary indirection).
    VTKM_TEST_ASSERT(
          vtkm::cont::ArrayPortalToIteratorBegin(controlArray.GetPortal()) ==
          vtkm::cont::ArrayPortalToIteratorBegin(executionArray.GetPortal()),
          "Execution array manager not holding control array iterators.");
    VTKM_TEST_ASSERT(
          vtkm::cont::ArrayPortalToIteratorBegin(controlArray.GetPortalConst()) ==
          vtkm::cont::ArrayPortalToIteratorBegin(executionArray.GetPortalConst()),
          "Execution array manager not holding control array iterators.");

    VTKM_TEST_ASSERT(CheckManager(executionArray, INPUT_VALUE),
                     "Did not get correct array back.");
  }

  void OutputData()
  {
    const ValueType OUTPUT_VALUE(12);

    StorageType controlArray;

    ArrayManagerType executionArray;
    executionArray.AllocateArrayForOutput(controlArray, ARRAY_SIZE);

    vtkm::cont::ArrayPortalToIterators<typename ArrayManagerType::PortalType>
        iterators(executionArray.GetPortal());
    std::fill(iterators.GetBegin(), iterators.GetEnd(), OUTPUT_VALUE);

    VTKM_TEST_ASSERT(CheckManager(executionArray, OUTPUT_VALUE),
                     "Did not get correct array back.");

    executionArray.RetrieveOutputData(controlArray);

    VTKM_TEST_ASSERT(CheckStorage(controlArray, OUTPUT_VALUE),
                     "Did not get the right value in the storage.");
  }

  void operator()()
  {

    InputData();
    InPlaceData();
    OutputData();

  }
};

struct TestFunctor
{
  template <typename T>
  void operator()(T) const
  {
    TemplatedTests<T> tests;
    tests();
  }
};

void TestArrayManagerShare()
{
  vtkm::testing::Testing::TryAllTypes(TestFunctor());
}

} // Anonymous namespace

int UnitTestArrayManagerExecutionShareWithControl(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(TestArrayManagerShare);
}
