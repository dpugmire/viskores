//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//  Copyright 2018 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
//  Copyright 2018 UT-Battelle, LLC.
//  Copyright 2018 Los Alamos National Security.
//
//  Under the terms of Contract DE-NA0003525 with NTESS,
//  the U.S. Government retains certain rights in this software.
//
//  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
//  Laboratory (LANL), the U.S. Government retains certain rights in
//  this software.
//============================================================================

#include <vtkm/cont/DeviceAdapterTag.h>

#include <vtkm/cont/RuntimeDeviceInformation.h>

namespace vtkm
{
namespace cont
{

DeviceAdapterNameType DeviceAdapterId::GetName() const
{
  vtkm::cont::RuntimeDeviceInformation info;
  return info.GetName(*this);
}

DeviceAdapterId make_DeviceAdapterId(const DeviceAdapterNameType& name)
{
  vtkm::cont::RuntimeDeviceInformation info;
  return info.GetId(name);
}
}
} // end namespace vtkm::cont
