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

#ifndef vtk_m_worklet_KdTree3DNNSearch_h
#define vtk_m_worklet_KdTree3DNNSearch_h

#include <vtkm/Math.h>
#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/ArrayHandleCounting.h>
#include <vtkm/cont/ArrayHandleReverse.h>
#include <vtkm/cont/DeviceAdapter.h>
#include <vtkm/cont/DeviceAdapterAlgorithm.h>
#include <vtkm/cont/arg/ControlSignatureTagBase.h>
#include <vtkm/cont/serial/DeviceAdapterSerial.h>
#include <vtkm/cont/testing/Testing.h>
#include <vtkm/worklet/DispatcherMapField.h>
#include <vtkm/worklet/WorkletMapField.h>
#include <vtkm/worklet/internal/DispatcherBase.h>
#include <vtkm/worklet/internal/WorkletBase.h>

namespace vtkm
{
namespace worklet
{
namespace spatialstructure
{

class KdTree3DNNSearch
{
public:
  class NearestNeighborSearch3DWorklet : public vtkm::worklet::WorkletMapField
  {
  public:
    typedef void ControlSignature(FieldIn<> qcIn,
                                  WholeArrayIn<> treeIdIn,
                                  WholeArrayIn<> treeSplitIdIn,
                                  WholeArrayIn<> treeCoordiIn,
                                  FieldOut<> nnIdOut,
                                  FieldOut<> nnDisOut);
    typedef void ExecutionSignature(_1, _2, _3, _4, _5, _6);

    VTKM_CONT
    NearestNeighborSearch3DWorklet() {}

    template <typename CooriVecT, typename CooriT, typename IdPortalT, typename CoordiPortalT>
    VTKM_EXEC_CONT void NearestNeighborSearch3D(CooriVecT qc,
                                                CooriT& dis,
                                                vtkm::Id& nnpIdx,
                                                vtkm::Int32 level,
                                                vtkm::Id sIdx,
                                                vtkm::Id tIdx,
                                                IdPortalT treePortal,
                                                IdPortalT splitIdPortal,
                                                CoordiPortalT coordiPortal) const
    {
      CooriT qx = qc[0];
      CooriT qy = qc[1];
      CooriT qz = qc[2];

      if (tIdx - sIdx == 1)
      { ///// leaf node
        vtkm::Id leafNodeIdx = treePortal.Get(sIdx);
        CooriT leafX = coordiPortal.Get(leafNodeIdx)[0];
        CooriT leafY = coordiPortal.Get(leafNodeIdx)[1];
        CooriT leafZ = coordiPortal.Get(leafNodeIdx)[2];
        CooriT _dis = vtkm::Sqrt((leafX - qx) * (leafX - qx) + (leafY - qy) * (leafY - qy) +
                                 (leafZ - qz) * (leafZ - qz));
        if (_dis < dis)
        {
          dis = _dis;
          nnpIdx = leafNodeIdx;
        }
      }
      else
      { //normal Node
        vtkm::Id splitNodeLoc = static_cast<vtkm::Id>(vtkm::Ceil((sIdx + tIdx) / 2.0));
        CooriT splitX = coordiPortal.Get(splitIdPortal.Get(splitNodeLoc))[0];
        CooriT splitY = coordiPortal.Get(splitIdPortal.Get(splitNodeLoc))[1];
        CooriT splitZ = coordiPortal.Get(splitIdPortal.Get(splitNodeLoc))[2];

        CooriT splitAxis;
        CooriT queryCoordi;

        if (level % 3 == 0)
        { //x axis level
          splitAxis = splitX;
          queryCoordi = qx;
        }
        else if (level % 3 == 1)
        {
          splitAxis = splitY;
          queryCoordi = qy;
        }
        else
        {
          splitAxis = splitZ;
          queryCoordi = qz;
        }

        if (queryCoordi <= splitAxis)
        { //left tree first
          if (queryCoordi - dis <= splitAxis)
            NearestNeighborSearch3D(qc,
                                    dis,
                                    nnpIdx,
                                    level + 1,
                                    sIdx,
                                    static_cast<vtkm::Id>(vtkm::Ceil((sIdx + tIdx) / 2.0)),
                                    treePortal,
                                    splitIdPortal,
                                    coordiPortal);
          if (queryCoordi + dis > splitAxis)
            NearestNeighborSearch3D(qc,
                                    dis,
                                    nnpIdx,
                                    level + 1,
                                    static_cast<vtkm::Id>(vtkm::Ceil((sIdx + tIdx) / 2.0)),
                                    tIdx,
                                    treePortal,
                                    splitIdPortal,
                                    coordiPortal);
        }
        else
        { //right tree first
          if (queryCoordi + dis > splitAxis)
            NearestNeighborSearch3D(qc,
                                    dis,
                                    nnpIdx,
                                    level + 1,
                                    static_cast<vtkm::Id>(vtkm::Ceil((sIdx + tIdx) / 2.0)),
                                    tIdx,
                                    treePortal,
                                    splitIdPortal,
                                    coordiPortal);
          if (queryCoordi - dis <= splitAxis)
            NearestNeighborSearch3D(qc,
                                    dis,
                                    nnpIdx,
                                    level + 1,
                                    sIdx,
                                    static_cast<vtkm::Id>(vtkm::Ceil((sIdx + tIdx) / 2.0)),
                                    treePortal,
                                    splitIdPortal,
                                    coordiPortal);
        }
      }
    }

    template <typename CoordiVecType,
              typename IdPortalType,
              typename CoordiPortalType,
              typename IdType,
              typename CoordiType>
    VTKM_EXEC void operator()(const CoordiVecType& qc,
                              const IdPortalType& treeIdPortal,
                              const IdPortalType& treeSplitIdPortal,
                              const CoordiPortalType& treeCoordiPortal,
                              IdType& nnId,
                              CoordiType& nnDis) const
    {
      nnDis = std::numeric_limits<CoordiType>::max();

      NearestNeighborSearch3D(qc,
                              nnDis,
                              nnId,
                              0,
                              0,
                              treeIdPortal.GetNumberOfValues(),
                              treeIdPortal,
                              treeSplitIdPortal,
                              treeCoordiPortal);
    }
  };

  // Execute the Neaseat Neighbor Search given kdtree and search points
  // Returns:
  // Vectors of NN point index and NNpoint distance
  template <typename CoordType,
            typename CoordStorageTag1,
            typename CoordStorageTag2,
            typename DeviceAdapter>
  void Run(const vtkm::cont::ArrayHandle<vtkm::Vec<CoordType, 3>, CoordStorageTag1>& coordi_Handle,
           vtkm::cont::ArrayHandle<vtkm::Id>& pointId_Handle,
           vtkm::cont::ArrayHandle<vtkm::Id>& splitId_Handle,
           const vtkm::cont::ArrayHandle<vtkm::Vec<CoordType, 3>, CoordStorageTag2>& qc_Handle,
           vtkm::cont::ArrayHandle<vtkm::Id>& nnId_Handle,
           vtkm::cont::ArrayHandle<CoordType>& nnDis_Handle,
           DeviceAdapter vtkmNotUsed(device))
  {
#if VTKM_DEVICE_ADAPTER == VTKM_DEVICE_ADAPTER_CUDA
    //set up stack size for cuda envinroment
    size_t stackSizeBackup;
    cudaDeviceGetLimit(&stackSizeBackup, cudaLimitStackSize);
    cudaDeviceSetLimit(cudaLimitStackSize, 1024 * 16);
#endif

    NearestNeighborSearch3DWorklet nns3dWorklet;
    vtkm::worklet::DispatcherMapField<NearestNeighborSearch3DWorklet, DeviceAdapter>
      nns3DDispatcher(nns3dWorklet);
    nns3DDispatcher.Invoke(
      qc_Handle, pointId_Handle, splitId_Handle, coordi_Handle, nnId_Handle, nnDis_Handle);
#if VTKM_DEVICE_ADAPTER == VTKM_DEVICE_ADAPTER_CUDA
    cudaDeviceSetLimit(cudaLimitStackSize, stackSizeBackup);
#endif
  }
};
}
}
} // namespace vtkm::worklet

#endif // vtk_m_worklet_KdTree3DNNSearch_h
