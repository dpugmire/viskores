//============================================================================
//  The contents of this file are covered by the Viskores license. See
//  LICENSE.txt for details.
//
//  By contributing to this file, all contributors agree to the Developer
//  Certificate of Origin Version 1.1 (DCO 1.1) as stated in DCO.txt.
//============================================================================

//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//============================================================================

#include <viskores/cont/ArrayHandleZip.h>

#include <viskores/cont/testing/Testing.h>

namespace
{

template<typename ArrayHandleType>
void CheckArray(const ArrayHandleType array)
{
  VISKORES_TEST_ASSERT(array.GetNumberOfValues() == 3, "Permuted array has wrong size.");

  typename ArrayHandleType::ReadPortalType portal = array.ReadPortal();
  VISKORES_TEST_ASSERT(portal.GetNumberOfValues() == 3,
                       "Permuted portal has wrong size.");

  using PairType = viskores::Pair<viskores::Id, viskores::Float64>;

  VISKORES_TEST_ASSERT(test_equal(portal.Get(0), PairType(3, 0.0)),
                       "Zipped array has wrong value.");
  VISKORES_TEST_ASSERT(test_equal(portal.Get(1), PairType(0, 0.1)),
                       "Zipped array has wrong value.");
  VISKORES_TEST_ASSERT(test_equal(portal.Get(2), PairType(1, 0.2)),
                       "Zipped array has wrong value.");
}

void Test()
{
  ////
  //// BEGIN-EXAMPLE ArrayHandleZip
  ////
  using ArrayType1 = viskores::cont::ArrayHandle<viskores::Id>;
  using PortalType1 = ArrayType1::WritePortalType;

  using ArrayType2 = viskores::cont::ArrayHandle<viskores::Float64>;
  using PortalType2 = ArrayType2::WritePortalType;

  // Create an array of viskores::Id with values [3, 0, 1]
  ArrayType1 array1;
  array1.Allocate(3);
  PortalType1 portal1 = array1.WritePortal();
  portal1.Set(0, 3);
  portal1.Set(1, 0);
  portal1.Set(2, 1);

  // Create a second array of viskores::Float32 with values [0.0, 0.1, 0.2]
  ArrayType2 array2;
  array2.Allocate(3);
  PortalType2 portal2 = array2.WritePortal();
  portal2.Set(0, 0.0);
  portal2.Set(1, 0.1);
  portal2.Set(2, 0.2);

  // Zip the two arrays together to create an array of
  // viskores::Pair<viskores::Id, viskores::Float64> with values [(3,0.0), (0,0.1), (1,0.2)]
  viskores::cont::ArrayHandleZip<ArrayType1, ArrayType2> zipArray(array1, array2);
  ////
  //// END-EXAMPLE ArrayHandleZip
  ////

  CheckArray(zipArray);

  CheckArray(
    ////
    //// BEGIN-EXAMPLE MakeArrayHandleZip
    ////
    viskores::cont::make_ArrayHandleZip(array1, array2)
    ////
    //// END-EXAMPLE MakeArrayHandleZip
    ////
  );
}

} // anonymous namespace

int GuideExampleArrayHandleZip(int argc, char* argv[])
{
  return viskores::cont::testing::Testing::Run(Test, argc, argv);
}
