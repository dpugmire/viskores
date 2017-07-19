//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//  Copyright 2015 Sandia Corporation.
//  Copyright 2015 UT-Battelle, LLC.
//  Copyright 2015 Los Alamos National Security.
//
//  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
//  the U.S. Government retains certain rights in this software.
//
//  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
//  Laboratory (LANL), the U.S. Government retains certain rights in
//  this software.
//============================================================================
#ifndef vtk_m_rendering_raytracing_Ray_Operations_h
#define vtk_m_rendering_raytracing_Ray_Operations_h

#include <vtkm/rendering/raytracing/ChannelBufferOperations.h>
#include <vtkm/rendering/raytracing/Ray.h>
#include <vtkm/rendering/raytracing/Worklets.h>

namespace vtkm
{
namespace rendering
{
namespace raytracing
{
namespace detail
{

class RayStatusFilter : public vtkm::worklet::WorkletMapField
{
public:
  VTKM_CONT
  RayStatusFilter() {}
  typedef void ControlSignature(FieldIn<>, FieldInOut<>);
  typedef void ExecutionSignature(_1, _2);
  VTKM_EXEC
  void operator()(const vtkm::Id& hitIndex, vtkm::UInt8& rayStatus) const
  {
    if (hitIndex == -1)
      rayStatus = RAY_EXITED_DOMAIN;
    else if (rayStatus != RAY_EXITED_DOMAIN && rayStatus != RAY_TERMINATED)
      rayStatus = RAY_ACTIVE;
    //else printf("Bad status state %d \n",(int)rayStatus);
  }
}; //class RayStatusFileter

} // namespace detail

class RayOperations
{
public:
  template <typename Device, typename T>
  static void ResetStatus(Ray<T>& rays, vtkm::UInt8 status, Device)
  {
    vtkm::worklet::DispatcherMapField<MemSet<vtkm::UInt8>, Device>(MemSet<vtkm::UInt8>(status))
      .Invoke(rays.Status);
  }

  //
  // Some worklets like triangle intersection do not set the
  // ray status, so this operation sets the status based on
  // the ray hit index
  //
  template <typename Device, typename T>
  static void UpdateRayStatus(Ray<T>& rays, Device)
  {
    vtkm::worklet::DispatcherMapField<detail::RayStatusFilter, Device>(detail::RayStatusFilter())
      .Invoke(rays.HitIdx, rays.Status);
  }

  template <typename Device, typename T>
  static vtkm::Id RaysInMesh(Ray<T>& rays, Device)
  {
    vtkm::Vec<UInt8, 2> maskValues;
    maskValues[0] = RAY_ACTIVE;
    maskValues[1] = RAY_LOST;

    vtkm::cont::ArrayHandle<vtkm::UInt8> masks;

    vtkm::worklet::DispatcherMapField<ManyMask<vtkm::UInt8, 2>, Device>(
      ManyMask<vtkm::UInt8, 2>(maskValues))
      .Invoke(rays.Status, masks);
    vtkm::cont::ArrayHandleCast<vtkm::Id, vtkm::cont::ArrayHandle<vtkm::UInt8>> castedMasks(masks);
    const vtkm::Id initVal = 0;
    vtkm::Id count = vtkm::cont::DeviceAdapterAlgorithm<Device>::Reduce(castedMasks, initVal);

    return count;
  }

  template <typename Device, typename T>
  static vtkm::Id GetStatusCount(Ray<T>& rays, vtkm::Id status, Device)
  {
    vtkm::UInt8 statusUInt8;
    if (status < 0 || status > 255)
    {
      throw vtkm::cont::ErrorBadValue("Rays GetStatusCound: invalid status");
      return 0;
    }

    statusUInt8 = static_cast<vtkm::UInt8>(status);
    vtkm::cont::ArrayHandle<vtkm::UInt8> masks;

    vtkm::worklet::DispatcherMapField<Mask<vtkm::UInt8>, Device>(Mask<vtkm::UInt8>(statusUInt8))
      .Invoke(rays.Status, masks);
    vtkm::cont::ArrayHandleCast<vtkm::Id, vtkm::cont::ArrayHandle<vtkm::UInt8>> castedMasks(masks);
    const vtkm::Id initVal = 0;
    vtkm::Id count = vtkm::cont::DeviceAdapterAlgorithm<Device>::Reduce(castedMasks, initVal);

    return count;
  }

  template <typename Device, typename T>
  static vtkm::Id RaysProcessed(Ray<T>& rays, Device)
  {
    vtkm::Vec<UInt8, 3> maskValues;
    maskValues[0] = RAY_TERMINATED;
    maskValues[1] = RAY_EXITED_DOMAIN;
    maskValues[2] = RAY_ABANDONED;

    vtkm::cont::ArrayHandle<vtkm::UInt8> masks;

    vtkm::worklet::DispatcherMapField<ManyMask<vtkm::UInt8, 3>, Device>(
      ManyMask<vtkm::UInt8, 3>(maskValues))
      .Invoke(rays.Status, masks);
    vtkm::cont::ArrayHandleCast<vtkm::Id, vtkm::cont::ArrayHandle<vtkm::UInt8>> castedMasks(masks);
    const vtkm::Id initVal = 0;
    vtkm::Id count = vtkm::cont::DeviceAdapterAlgorithm<Device>::Reduce(castedMasks, initVal);

    return count;
  }

  template <typename Device, typename T>
  static vtkm::cont::ArrayHandle<vtkm::UInt8> CompactActiveRays(Ray<T>& rays, Device)
  {
    vtkm::Vec<UInt8, 1> maskValues;
    maskValues[0] = RAY_ACTIVE;
    vtkm::UInt8 statusUInt8 = static_cast<vtkm::UInt8>(RAY_ACTIVE);
    vtkm::cont::ArrayHandle<vtkm::UInt8> masks;

    vtkm::worklet::DispatcherMapField<Mask<vtkm::UInt8>, Device>(Mask<vtkm::UInt8>(statusUInt8))
      .Invoke(rays.Status, masks);

    //
    // Make empty composite vectors so we dont use up extra storage
    //
    vtkm::IdComponent inComp[3];
    inComp[0] = 0;
    inComp[1] = 1;
    inComp[2] = 2;

    vtkm::cont::ArrayHandle<T> emptyHandle;


    rays.Normal = vtkm::cont::make_ArrayHandleCompositeVector(
      emptyHandle, inComp[0], emptyHandle, inComp[1], emptyHandle, inComp[2]);

    rays.Origin = vtkm::cont::make_ArrayHandleCompositeVector(
      emptyHandle, inComp[0], emptyHandle, inComp[1], emptyHandle, inComp[2]);

    rays.Dir = vtkm::cont::make_ArrayHandleCompositeVector(
      emptyHandle, inComp[0], emptyHandle, inComp[1], emptyHandle, inComp[2]);
    const vtkm::Int32 numFloatArrays = 18;
    vtkm::cont::ArrayHandle<T>* floatArrayPointers[numFloatArrays];
    floatArrayPointers[0] = &rays.OriginX;
    floatArrayPointers[1] = &rays.OriginY;
    floatArrayPointers[2] = &rays.OriginZ;
    floatArrayPointers[3] = &rays.DirX;
    floatArrayPointers[4] = &rays.DirY;
    floatArrayPointers[5] = &rays.DirZ;
    floatArrayPointers[6] = &rays.Distance;
    floatArrayPointers[7] = &rays.MinDistance;
    floatArrayPointers[8] = &rays.MaxDistance;

    floatArrayPointers[9] = &rays.Scalar;
    floatArrayPointers[10] = &rays.IntersectionX;
    floatArrayPointers[11] = &rays.IntersectionY;
    floatArrayPointers[12] = &rays.IntersectionZ;
    floatArrayPointers[13] = &rays.U;
    floatArrayPointers[14] = &rays.V;
    floatArrayPointers[15] = &rays.NormalX;
    floatArrayPointers[16] = &rays.NormalY;
    floatArrayPointers[17] = &rays.NormalZ;

    const int breakPoint = rays.IntersectionDataEnabled ? -1 : 9;
    for (int i = 0; i < numFloatArrays; ++i)
    {
      if (i == breakPoint)
      {
        break;
      }
      vtkm::cont::ArrayHandle<T> compacted;
      vtkm::cont::DeviceAdapterAlgorithm<Device>::CopyIf(*floatArrayPointers[i], masks, compacted);
      *floatArrayPointers[i] = compacted;
    }

    //
    // restore the composite vectors
    //
    rays.Normal = vtkm::cont::make_ArrayHandleCompositeVector(
      rays.NormalX, inComp[0], rays.NormalY, inComp[1], rays.NormalZ, inComp[2]);

    rays.Origin = vtkm::cont::make_ArrayHandleCompositeVector(
      rays.OriginX, inComp[0], rays.OriginY, inComp[1], rays.OriginZ, inComp[2]);

    rays.Dir = vtkm::cont::make_ArrayHandleCompositeVector(
      rays.DirX, inComp[0], rays.DirY, inComp[1], rays.DirZ, inComp[2]);

    vtkm::cont::ArrayHandle<vtkm::Id> compactedHits;
    vtkm::cont::DeviceAdapterAlgorithm<Device>::CopyIf(rays.HitIdx, masks, compactedHits);
    rays.HitIdx = compactedHits;

    vtkm::cont::ArrayHandle<vtkm::Id> compactedPixels;
    vtkm::cont::DeviceAdapterAlgorithm<Device>::CopyIf(rays.PixelIdx, masks, compactedPixels);
    rays.PixelIdx = compactedPixels;

    vtkm::cont::ArrayHandle<vtkm::UInt8> compactedStatus;
    vtkm::cont::DeviceAdapterAlgorithm<Device>::CopyIf(rays.Status, masks, compactedStatus);
    rays.Status = compactedStatus;

    rays.NumRays = rays.Status.GetPortalConstControl().GetNumberOfValues();

    const size_t bufferCount = static_cast<size_t>(rays.Buffers.size());
    for (size_t i = 0; i < bufferCount; ++i)
    {
      ChannelBufferOperations::Compact(rays.Buffers[i], masks, rays.NumRays, Device());
    }
    return masks;
  }

  template <typename Device, typename T>
  static void Resize(Ray<T>& rays, const vtkm::Int32 newSize, Device)
  {
    if (newSize == rays.NumRays)
      return; //nothing to do

    rays.NumRays = newSize;

    if (rays.IntersectionDataEnabled)
    {
      rays.IntersectionX.PrepareForOutput(rays.NumRays, Device());
      rays.IntersectionY.PrepareForOutput(rays.NumRays, Device());
      rays.IntersectionZ.PrepareForOutput(rays.NumRays, Device());
      rays.U.PrepareForOutput(rays.NumRays, Device());
      rays.V.PrepareForOutput(rays.NumRays, Device());
      rays.Scalar.PrepareForOutput(rays.NumRays, Device());

      rays.NormalX.PrepareForOutput(rays.NumRays, Device());
      rays.NormalY.PrepareForOutput(rays.NumRays, Device());
      rays.NormalZ.PrepareForOutput(rays.NumRays, Device());
    }

    rays.OriginX.PrepareForOutput(rays.NumRays, Device());
    rays.OriginY.PrepareForOutput(rays.NumRays, Device());
    rays.OriginZ.PrepareForOutput(rays.NumRays, Device());

    rays.DirX.PrepareForOutput(rays.NumRays, Device());
    rays.DirY.PrepareForOutput(rays.NumRays, Device());
    rays.DirZ.PrepareForOutput(rays.NumRays, Device());

    rays.Distance.PrepareForOutput(rays.NumRays, Device());
    rays.MinDistance.PrepareForOutput(rays.NumRays, Device());
    rays.MaxDistance.PrepareForOutput(rays.NumRays, Device());
    rays.Status.PrepareForOutput(rays.NumRays, Device());
    rays.HitIdx.PrepareForOutput(rays.NumRays, Device());
    rays.PixelIdx.PrepareForOutput(rays.NumRays, Device());

    const size_t bufferCount = static_cast<size_t>(rays.Buffers.size());
    for (size_t i = 0; i < bufferCount; ++i)
    {
      rays.Buffers[i].Resize(rays.NumRays, Device());
    }
  }

  template <typename Device, typename T>
  static void CopyDistancesToMin(Ray<T> rays, Device, const T offset = 0.f)
  {
    vtkm::worklet::DispatcherMapField<CopyAndOffsetMask<T>, Device>(
      CopyAndOffsetMask<T>(offset, RAY_EXITED_MESH))
      .Invoke(rays.Distance, rays.MinDistance, rays.Status);
  }
};
}
}
} // namespace vktm::rendering::raytracing
#endif
