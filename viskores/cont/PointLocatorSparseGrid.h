//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//============================================================================
#ifndef viskores_cont_PointLocatorSparseGrid_h
#define viskores_cont_PointLocatorSparseGrid_h

#include <viskores/cont/PointLocatorBase.h>
#include <viskores/exec/PointLocatorSparseGrid.h>

namespace viskores
{
namespace cont
{

/// \brief A locator that bins points in a sparsely stored grid.
///
/// `PointLocatorSparseGrid` creates a very dense logical grid over the region containing
/// the points of the provided data set. Although this logical grid has uniform structure,
/// it is stored sparsely. So, it is expected that most of the bins in the structure will
/// be empty but not explicitly stored. This makes `PointLocatorSparseGrid` a good
/// representation for unstructured or irregular collections of points.
///
/// The algorithm used in `PointLocatorSparseGrid` is described in the following publication:
///
/// Abhishek Yenpure, Hank Childs, and Kenneth Moreland. "Efficient Point Merging Using Data
/// Parallel Techniques." In _Eurographics Symposium on Parallel Graphics and Visualization
/// (EGPGV)_, June 2019. DOI 10.2312/pgv.20191112.
///
class VISKORES_CONT_EXPORT PointLocatorSparseGrid : public viskores::cont::PointLocatorBase
{
public:
  using RangeType = viskores::Vec<viskores::Range, 3>;

  void SetRange(const RangeType& range)
  {
    if (this->Range != range)
    {
      this->Range = range;
      this->SetModified();
    }
  }

  const RangeType& GetRange() const { return this->Range; }

  void SetComputeRangeFromCoordinates()
  {
    if (!this->IsRangeInvalid())
    {
      this->Range = { { 0.0, -1.0 } };
      this->SetModified();
    }
  }

  void SetNumberOfBins(const viskores::Id3& bins)
  {
    if (this->Dims != bins)
    {
      this->Dims = bins;
      this->SetModified();
    }
  }

  const viskores::Id3& GetNumberOfBins() const { return this->Dims; }

  VISKORES_CONT
  viskores::exec::PointLocatorSparseGrid PrepareForExecution(viskores::cont::DeviceAdapterId device,
                                                         viskores::cont::Token& token) const;

private:
  bool IsRangeInvalid() const
  {
    return (this->Range[0].Max < this->Range[0].Min) || (this->Range[1].Max < this->Range[1].Min) ||
      (this->Range[2].Max < this->Range[2].Min);
  }

  VISKORES_CONT void Build() override;

  RangeType Range = { { 0.0, -1.0 } };
  viskores::Id3 Dims = { 32 };

  viskores::cont::ArrayHandle<viskores::Id> PointIds;
  viskores::cont::ArrayHandle<viskores::Id> CellLower;
  viskores::cont::ArrayHandle<viskores::Id> CellUpper;
};
}
}
#endif //viskores_cont_PointLocatorSparseGrid_h
