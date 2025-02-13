//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//============================================================================
#ifndef viskores_cont_CellLocatorUniformBins_h
#define viskores_cont_CellLocatorUniformBins_h

#include <viskores/cont/ArrayHandle.h>
#include <viskores/cont/CellSetList.h>

#include <viskores/cont/ArrayHandleGroupVecVariable.h>
#include <viskores/cont/CellLocatorBase.h>

#include <viskores/exec/CellLocatorMultiplexer.h>
#include <viskores/exec/CellLocatorUniformBins.h>


namespace viskores
{
namespace cont
{

/// \brief A locator that uses a uniform grid
///
/// `CellLocatorUniformBins` creates a cell search structure using a single uniform
/// grid. The size of the uniform grid is specified using the `SetDims` method.
/// In general, the `CellLocatorTwoLevel` has the better performance. However,
/// there are some cases where this is not the case. One example of this is
/// a uniformly dense triangle grid. In some cases the `CellLocatorUniformBins`
/// produces a more efficient search structure, especially for GPUs where memory
/// access patterns are critical to performance.

class VISKORES_CONT_EXPORT CellLocatorUniformBins : public viskores::cont::CellLocatorBase
{
  template <typename CellSetCont>
  using CellSetContToExec =
    typename CellSetCont::template ExecConnectivityType<viskores::TopologyElementTagCell,
                                                        viskores::TopologyElementTagPoint>;

public:
  using SupportedCellSets = VISKORES_DEFAULT_CELL_SET_LIST;

  using CellExecObjectList = viskores::ListTransform<SupportedCellSets, CellSetContToExec>;
  using CellLocatorExecList =
    viskores::ListTransform<CellExecObjectList, viskores::exec::CellLocatorUniformBins>;

  using ExecObjType =
    viskores::ListApply<CellLocatorExecList, viskores::exec::CellLocatorMultiplexer>;
  using LastCell = typename ExecObjType::LastCell;

  CellLocatorUniformBins() {}
  void SetDims(const viskores::Id3& dims) { this->UniformDims = dims; }
  viskores::Id3 GetDims() const { return this->UniformDims; }

  void PrintSummary(std::ostream& out) const;

public:
  ExecObjType PrepareForExecution(viskores::cont::DeviceAdapterId device,
                                  viskores::cont::Token& token) const;

private:
  VISKORES_CONT void Build() override;

  viskores::Vec3f InvSpacing;
  viskores::Vec3f MaxPoint;
  viskores::Vec3f Origin;
  viskores::Id3 UniformDims;
  viskores::Id3 MaxCellIds;

  using CellIdArrayType = viskores::cont::ArrayHandle<viskores::Id>;
  using CellIdOffsetArrayType = viskores::cont::ArrayHandle<viskores::Id>;

  viskores::cont::ArrayHandleGroupVecVariable<CellIdArrayType, CellIdOffsetArrayType> CellIds;

  struct MakeExecObject;
};

}
} // viskores::cont

#endif // viskores_cont_CellLocatorUniformBins_h
