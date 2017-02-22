//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//  Copyright 2016 Sandia Corporation.
//  Copyright 2016 UT-Battelle, LLC.
//  Copyright 2016 Los Alamos National Security.
//
//  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
//  the U.S. Government retains certain rights in this software.
//
//  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
//  Laboratory (LANL), the U.S. Government retains certain rights in
//  this software.
//============================================================================

#include <vtkm/cont/RuntimeDeviceTracker.h>

#include <vtkm/cont/DeviceAdapter.h>
#include <vtkm/cont/DeviceAdapterListTag.h>
#include <vtkm/cont/ErrorBadValue.h>

#include <vtkm/cont/serial/DeviceAdapterSerial.h>
#include <vtkm/cont/cuda/DeviceAdapterCuda.h>
#include <vtkm/cont/tbb/DeviceAdapterTBB.h>

#include <cstring>
#include <sstream>

#define VTKM_MAX_DEVICE_ADAPTER_ID 8

namespace vtkm {
namespace cont {

namespace detail {

struct RuntimeDeviceTrackerInternals
{
  bool RuntimeValid[VTKM_MAX_DEVICE_ADAPTER_ID];
};

}

VTKM_CONT
RuntimeDeviceTracker::RuntimeDeviceTracker()
  : Internals(new detail::RuntimeDeviceTrackerInternals)
{
  this->Reset();
}

VTKM_CONT
RuntimeDeviceTracker::~RuntimeDeviceTracker()
{  }

VTKM_CONT
void RuntimeDeviceTracker::CheckDevice(
    vtkm::cont::DeviceAdapterId deviceId,
    const vtkm::cont::DeviceAdapterNameType &deviceName) const
{
  if ((deviceId < 0) || (deviceId >= VTKM_MAX_DEVICE_ADAPTER_ID))
  {
    std::stringstream message;
    message << "Device '" << deviceName << "' has invalid ID of " << deviceId;
    throw vtkm::cont::ErrorBadValue(message.str());
  }
}

VTKM_CONT
bool RuntimeDeviceTracker::CanRunOnImpl(
    vtkm::cont::DeviceAdapterId deviceId,
    const vtkm::cont::DeviceAdapterNameType &deviceName) const
{
  this->CheckDevice(deviceId, deviceName);
  return this->Internals->RuntimeValid[deviceId];
}

VTKM_CONT
void RuntimeDeviceTracker::SetDeviceState(
    vtkm::cont::DeviceAdapterId deviceId,
    const vtkm::cont::DeviceAdapterNameType &deviceName,
    bool state)
{
  this->CheckDevice(deviceId, deviceName);
  this->Internals->RuntimeValid[deviceId] = state;
}

namespace {

struct VTKM_NEVER_EXPORT RuntimeDeviceTrackerResetFunctor
{
  vtkm::cont::RuntimeDeviceTracker Tracker;

  VTKM_CONT
  RuntimeDeviceTrackerResetFunctor(
      const vtkm::cont::RuntimeDeviceTracker &tracker)
    : Tracker(tracker)
  {  }

  template<typename Device>
  VTKM_CONT
  void operator()(Device)
  {
    this->Tracker.ResetDevice(Device());
  }
};

}

VTKM_CONT
void RuntimeDeviceTracker::Reset()
{
  std::memset(this->Internals->RuntimeValid, 0, VTKM_MAX_DEVICE_ADAPTER_ID);

  RuntimeDeviceTrackerResetFunctor functor(*this);
  vtkm::ListForEach(functor, VTKM_DEFAULT_DEVICE_ADAPTER_LIST_TAG());
}

}
} // namespace vtkm::cont
