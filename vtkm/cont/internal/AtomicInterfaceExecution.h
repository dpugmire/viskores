//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//============================================================================
#ifndef vtk_m_cont_internal_AtomicInterfaceExecution_h
#define vtk_m_cont_internal_AtomicInterfaceExecution_h

#include <vtkm/Atomic.h>

namespace vtkm
{
namespace cont
{
namespace internal
{

template <typename DeviceTag>
struct AtomicInterfaceExecution
{
  using WordTypes = vtkm::AtomicTypesSupported;
  using WordTypePreferred = vtkm::AtomicTypePreferred;

  template <typename T>
  VTKM_EXEC_CONT static T Load(const T* addr)
  {
    return vtkm::AtomicLoad(addr);
  }

  template <typename T>
  VTKM_EXEC_CONT static void Store(T* addr, T value)
  {
    vtkm::AtomicStore(addr, value);
  }

  template <typename T>
  VTKM_EXEC_CONT static T Add(T* addr, T arg)
  {
    return vtkm::AtomicAdd(addr, arg);
  }

  template <typename T>
  VTKM_EXEC_CONT static T Not(T* addr)
  {
    return vtkm::AtomicNot(addr);
  }

  template <typename T>
  VTKM_EXEC_CONT static T And(T* addr, T mask)
  {
    return vtkm::AtomicAnd(addr, mask);
  }

  template <typename T>
  VTKM_EXEC_CONT static T Or(T* addr, T mask)
  {
    return vtkm::AtomicOr(addr, mask);
  }

  template <typename T>
  VTKM_EXEC_CONT static T Xor(T* addr, T mask)
  {
    return vtkm::AtomicXor(addr, mask);
  }

  template <typename T>
  VTKM_EXEC_CONT static T CompareAndSwap(T* addr, T newWord, T expected)
  {
    return vtkm::AtomicCompareAndSwap(addr, expected, newWord);
  }
};
}
}
} // end namespace vtkm::cont::internal

#endif // vtk_m_cont_internal_AtomicInterfaceExecution_h
