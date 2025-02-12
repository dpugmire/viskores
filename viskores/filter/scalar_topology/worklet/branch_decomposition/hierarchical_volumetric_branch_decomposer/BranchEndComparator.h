//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//============================================================================
// Copyright (c) 2018, The Regents of the University of California, through
// Lawrence Berkeley National Laboratory (subject to receipt of any required approvals
// from the U.S. Dept. of Energy).  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
// (1) Redistributions of source code must retain the above copyright notice, this
//     list of conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
// (3) Neither the name of the University of California, Lawrence Berkeley National
//     Laboratory, U.S. Dept. of Energy nor the names of its contributors may be
//     used to endorse or promote products derived from this software without
//     specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//
//=============================================================================
//
//  This code is an extension of the algorithm presented in the paper:
//  Parallel Peak Pruning for Scalable SMP Contour Tree Computation.
//  Hamish Carr, Gunther Weber, Christopher Sewell, and James Ahrens.
//  Proceedings of the IEEE Symposium on Large Data Analysis and Visualization
//  (LDAV), October 2016, Baltimore, Maryland.
//
//  The PPP2 algorithm and software were jointly developed by
//  Hamish Carr (University of Leeds), Gunther H. Weber (LBNL), and
//  Oliver Ruebel (LBNL)
//==============================================================================

#ifndef viskores_filter_scalar_topology_worklet_branch_decomposition_hierarchical_volumetric_branch_decomposer_branch_end_comparator_h
#define viskores_filter_scalar_topology_worklet_branch_decomposition_hierarchical_volumetric_branch_decomposer_branch_end_comparator_h

#include <viskores/Types.h>
#include <viskores/cont/ArrayHandle.h>
#include <viskores/cont/ExecutionObjectBase.h>
#include <viskores/filter/scalar_topology/worklet/contourtree_augmented/Types.h>

namespace viskores
{
namespace worklet
{
namespace scalar_topology
{
namespace hierarchical_volumetric_branch_decomposer
{

using IdArrayType = viskores::worklet::contourtree_augmented::IdArrayType;

// comparator used for initial sort of data values
template <typename ValueType, bool isLower>
class BranchEndComparatorImpl
{
public:
  using ValueArrayType = typename viskores::cont::ArrayHandle<ValueType>;
  using ValuePermutationType =
    typename viskores::cont::ArrayHandlePermutation<IdArrayType, ValueArrayType>;
  using IdPortalType = typename IdArrayType::ReadPortalType;
  using ValuePortalType = typename ValuePermutationType::ReadPortalType;

  // constructor
  VISKORES_CONT
  BranchEndComparatorImpl(const IdArrayType& branchRoots,
                          const ValuePermutationType& dataValues,
                          const IdArrayType& globalRegularIds,
                          viskores::cont::DeviceAdapterId device,
                          viskores::cont::Token& token)
    : branchRootsPortal(branchRoots.PrepareForInput(device, token))
    , dataValuesPortal(dataValues.PrepareForInput(device, token))
    , globalRegularIdsPortal(globalRegularIds.PrepareForInput(device, token))
  { // constructor
  } // constructor

  // () operator - gets called to do comparison
  VISKORES_EXEC
  bool operator()(const viskores::Id& i, const viskores::Id& j) const
  { // operator()
    viskores::Id branchI =
      viskores::worklet::contourtree_augmented::MaskedIndex(this->branchRootsPortal.Get(i));
    viskores::Id branchJ =
      viskores::worklet::contourtree_augmented::MaskedIndex(this->branchRootsPortal.Get(j));

    // primary sort on branch ID
    if (branchI < branchJ)
    {
      return true;
    }
    if (branchJ < branchI)
    {
      return false;
    }

    ValueType valueI = this->dataValuesPortal.Get(i);
    ValueType valueJ = this->dataValuesPortal.Get(j);

    // secondary sort on data value
    // if isLower is false, lower value first
    // if isLower is true, higher value first
    if (((!isLower) && (valueI < valueJ)) || ((isLower) && (valueI > valueJ)))
    {
      return true;
    }
    if (((!isLower) && (valueI > valueJ)) || ((isLower) && (valueI < valueJ)))
    {
      return false;
    }

    viskores::Id idI =
      viskores::worklet::contourtree_augmented::MaskedIndex(this->globalRegularIdsPortal.Get(i));
    viskores::Id idJ =
      viskores::worklet::contourtree_augmented::MaskedIndex(this->globalRegularIdsPortal.Get(j));

    // third sort on global regular id
    // if isLower is false, lower value first
    // if isLower is true, higher value first
    if (((!isLower) && (idI < idJ)) || ((isLower) && (idI > idJ)))
    {
      return true;
    }
    if (((!isLower) && (idI > idJ)) || ((isLower) && (idI < idJ)))
    {
      return false;
    }

    // fallback just in case
    return false;
  } // operator()

private:
  IdPortalType branchRootsPortal;
  ValuePortalType dataValuesPortal;
  IdPortalType globalRegularIdsPortal;

}; // BranchEndComparatorImpl

/// <summary>
/// Sort comparator for superarcs to determine the upper/lower end of branches
/// </summary>
/// <typeparam name="ValueType">data value type</typeparam>
/// <typeparam name="isLower">true if we look for the lower end</typeparam>
template <typename ValueType, bool isLower>
class BranchEndComparator : public viskores::cont::ExecutionObjectBase
{

public:
  using ValueArrayType = typename viskores::cont::ArrayHandle<ValueType>;
  using ValuePermutationType =
    typename viskores::cont::ArrayHandlePermutation<IdArrayType, ValueArrayType>;

  // constructor
  VISKORES_CONT
  BranchEndComparator(const IdArrayType& branchRoots,
                      const ValuePermutationType& dataValues,
                      const IdArrayType& globalRegularIds)
    : BranchRoots(branchRoots)
    , DataValues(dataValues)
    , GlobalRegularIds(globalRegularIds)
  {
  }

  VISKORES_CONT BranchEndComparatorImpl<ValueType, isLower> PrepareForExecution(
    viskores::cont::DeviceAdapterId device,
    viskores::cont::Token& token) const
  {
    return BranchEndComparatorImpl<ValueType, isLower>(
      this->BranchRoots, this->DataValues, this->GlobalRegularIds, device, token);
  }

private:
  IdArrayType BranchRoots;
  ValuePermutationType DataValues;
  IdArrayType GlobalRegularIds;
}; // BranchEndComparator

} // namespace hierarchical_volumetric_branch_decomposer
} // namespace scalar_topology
} // namespace worklet
} // namespace viskores

#endif
