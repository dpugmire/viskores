//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//============================================================================

#include <viskores/cont/RuntimeDeviceTracker.h>
#include <viskores/cont/tbb/DeviceAdapterTBB.h>
#include <viskores/cont/testing/TestingDeviceAdapter.h>

int UnitTestTBBDeviceAdapter(int argc, char* argv[])
{
  auto& tracker = viskores::cont::GetRuntimeDeviceTracker();
  tracker.ForceDevice(viskores::cont::DeviceAdapterTagTBB{});
  return viskores::cont::testing::TestingDeviceAdapter<viskores::cont::DeviceAdapterTagTBB>::Run(
    argc, argv);
}
