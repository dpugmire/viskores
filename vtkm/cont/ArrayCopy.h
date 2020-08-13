//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//============================================================================
#ifndef vtk_m_cont_ArrayCopy_h
#define vtk_m_cont_ArrayCopy_h

#include <vtkm/cont/Algorithm.h>
#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/DeviceAdapterTag.h>
#include <vtkm/cont/ErrorExecution.h>
#include <vtkm/cont/Logging.h>

#include <vtkm/cont/vtkm_cont_export.h>

// TODO: When virtual arrays are available, compile the implementation in a .cxx/.cu file. Common
// arrays are copied directly but anything else would be copied through virtual methods.

namespace vtkm
{
namespace cont
{

namespace detail
{

// Element-wise copy.
// TODO: Remove last argument once ArryHandleNewStyle becomes ArrayHandle
template <typename InArrayType, typename OutArrayType>
void ArrayCopyImpl(const InArrayType& in, OutArrayType& out, std::false_type /* Copy storage */)
{
  // Find the device that already has a copy of the data:
  vtkm::cont::DeviceAdapterId devId = in.GetDeviceAdapterId();

  // If the data is not on any device, let the runtime tracker pick an available
  // parallel copy algorithm.
  if (devId.GetValue() == VTKM_DEVICE_ADAPTER_UNDEFINED)
  {
    devId = vtkm::cont::make_DeviceAdapterId(VTKM_DEVICE_ADAPTER_ANY);
  }

  bool success = vtkm::cont::Algorithm::Copy(devId, in, out);

  if (!success && devId.GetValue() != VTKM_DEVICE_ADAPTER_ANY)
  { // Retry on any device if the first attempt failed.
    VTKM_LOG_S(vtkm::cont::LogLevel::Error,
               "Failed to run ArrayCopy on device '" << devId.GetName()
                                                     << "'. Retrying on any device.");
    success = vtkm::cont::Algorithm::Copy(vtkm::cont::DeviceAdapterTagAny{}, in, out);
  }

  if (!success)
  {
    throw vtkm::cont::ErrorExecution("Failed to run ArrayCopy on any device.");
  }
}

// Copy storage for implicit arrays, must be of same type:
// TODO: This will go away once ArrayHandleNewStyle becomes ArrayHandle.
template <typename ArrayType>
void ArrayCopyImpl(const ArrayType& in, ArrayType& out, std::true_type /* Copy storage */)
{
  // This is only called if in/out are the same type and the handle is not
  // writable. This allows read-only implicit array handles to be copied.
  auto newStorage = in.GetStorage();
  out = ArrayType(newStorage);
}

// TODO: This will go away once ArrayHandleNewStyle becomes ArrayHandle.
template <typename InArrayType, typename OutArrayType>
VTKM_CONT void ArrayCopyImpl(const InArrayType& source, OutArrayType& destination)
{
  using SameTypes = std::is_same<InArrayType, OutArrayType>;
  using IsWritable = vtkm::cont::internal::IsWritableArrayHandle<OutArrayType>;
  using JustCopyStorage = std::integral_constant<bool, SameTypes::value && !IsWritable::value>;
  ArrayCopyImpl(source, destination, JustCopyStorage{});
}

// TODO: ArrayHandleNewStyle will eventually become ArrayHandle, in which case this
// will become the only version with the same array types.
template <typename T, typename S>
VTKM_CONT void ArrayCopyImpl(const vtkm::cont::ArrayHandleNewStyle<T, S>& source,
                             vtkm::cont::ArrayHandleNewStyle<T, S>& destination)
{
  std::size_t numBuffers = static_cast<std::size_t>(source.GetNumberOfBuffers());
  std::vector<vtkm::cont::internal::Buffer> destinationBuffers(numBuffers);
  vtkm::cont::internal::Buffer* sourceBuffers = source.GetBuffers();
  for (std::size_t bufferIndex = 0; bufferIndex < numBuffers; ++bufferIndex)
  {
    sourceBuffers[bufferIndex].DeepCopy(destinationBuffers[bufferIndex]);
  }

  destination = vtkm::cont::ArrayHandleNewStyle<T, S>(destinationBuffers, source.GetStorage());
}

} // namespace detail

/// \brief Does a deep copy from one array to another array.
///
/// Given a source \c ArrayHandle and a destination \c ArrayHandle, this
/// function allocates the destination \c ArrayHandle to the correct size and
/// deeply copies all the values from the source to the destination.
///
/// This method will attempt to copy the data using the device that the input
/// data is already valid on. If the input data is only valid in the control
/// environment, the runtime device tracker is used to try to find another
/// device.
///
/// This should work on some non-writable array handles as well, as long as
/// both \a source and \a destination are the same type.
///
template <typename InValueType, typename InStorage, typename OutValueType, typename OutStorage>
VTKM_CONT void ArrayCopy(const vtkm::cont::ArrayHandle<InValueType, InStorage>& source,
                         vtkm::cont::ArrayHandle<OutValueType, OutStorage>& destination)
{
  using InArrayType = vtkm::cont::ArrayHandle<InValueType, InStorage>;
  using OutArrayType = vtkm::cont::ArrayHandle<OutValueType, OutStorage>;
  using SameTypes = std::is_same<InArrayType, OutArrayType>;
  using IsWritable = vtkm::cont::internal::IsWritableArrayHandle<OutArrayType>;

  // There are three cases handled here:
  // 1. The arrays are the same type:
  //    -> Deep copy the buffers and the Storage object
  // 2. The arrays are different and the output is writable:
  //    -> Do element-wise copy
  // 3. The arrays are different and the output is not writable:
  //    -> fail (cannot copy)

  // Give a nice error message for case 3:
  VTKM_STATIC_ASSERT_MSG(IsWritable::value || SameTypes::value,
                         "Cannot copy to a read-only array with a different "
                         "type than the source.");

  // Static dispatch cases 1 & 2
  detail::ArrayCopyImpl(source, destination);
}

// Forward declaration
// Cannot include VariantArrayHandle.h here due to circular dependency.
template <typename TypeList>
class VariantArrayHandleBase;

namespace detail
{

struct ArrayCopyFunctor
{
  template <typename InValueType, typename InStorage, typename OutValueType, typename OutStorage>
  VTKM_CONT void operator()(const vtkm::cont::ArrayHandle<InValueType, InStorage>& source,
                            vtkm::cont::ArrayHandle<OutValueType, OutStorage>& destination) const
  {
    vtkm::cont::ArrayCopy(source, destination);
  }
};

} // namespace detail

/// \brief Deep copies data in a `VariantArrayHandle` to an array of a known type.
///
/// This form of `ArrayCopy` can be used to copy data from an unknown array type to
/// an array of a known type. Note that regardless of the source type, the data will
/// be deep copied.
///
template <typename InTypeList, typename OutValueType, typename OutStorage>
VTKM_CONT void ArrayCopy(const vtkm::cont::VariantArrayHandleBase<InTypeList>& source,
                         vtkm::cont::ArrayHandle<OutValueType, OutStorage>& destination)
{
  source.CastAndCall(detail::ArrayCopyFunctor{}, destination);
}
}
} // namespace vtkm::cont

#endif //vtk_m_cont_ArrayCopy_h
