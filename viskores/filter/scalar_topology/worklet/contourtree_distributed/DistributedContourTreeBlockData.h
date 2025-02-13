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

#ifndef viskores_worklet_contourtree_distributed_contourtreeblockdata_h
#define viskores_worklet_contourtree_distributed_contourtreeblockdata_h

#include <viskores/Types.h>
#include <viskores/filter/scalar_topology/worklet/contourtree_augmented/Types.h>
#include <viskores/filter/scalar_topology/worklet/contourtree_augmented/meshtypes/ContourTreeMesh.h>
#include <viskores/filter/scalar_topology/worklet/contourtree_distributed/HierarchicalAugmenter.h>
#include <viskores/filter/scalar_topology/worklet/contourtree_distributed/HierarchicalContourTree.h>

// clang-format off
VISKORES_THIRDPARTY_PRE_INCLUDE
#include <viskores/thirdparty/diy/diy.h>
VISKORES_THIRDPARTY_POST_INCLUDE
// clang-format on


namespace viskores
{
namespace worklet
{
namespace contourtree_distributed
{
template <typename FieldType>
struct DistributedContourTreeBlockData
{
  // Block metadata
  int GlobalBlockId;              // Global DIY id of this block
  viskores::Id LocalBlockNo;      // Local block id on this rank
  viskores::Id3 BlockOrigin;      // Origin of the data block
  viskores::Id3 BlockSize;        // Extends of the data block
  viskores::Id3 FixedBlockOrigin; // Origin unaffected by fan-in
  viskores::Id3 FixedBlockSize;   // Extends unaffected by fan-in

  // Fan in data
  std::vector<viskores::worklet::contourtree_augmented::ContourTree> ContourTrees;
  std::vector<viskores::worklet::contourtree_augmented::ContourTreeMesh<FieldType>>
    ContourTreeMeshes;
  std::vector<viskores::worklet::contourtree_distributed::InteriorForest> InteriorForests;

  // Fan out data
  viskores::worklet::contourtree_distributed::HierarchicalContourTree<FieldType> HierarchicalTree;

  // Augmentation phase
  viskores::worklet::contourtree_distributed::HierarchicalAugmenter<FieldType>
    HierarchicalAugmenter;
  viskores::worklet::contourtree_distributed::HierarchicalContourTree<FieldType> AugmentedTree;

  // Destroy function allowing DIY to own blocks and clean them up after use
  static void Destroy(void* b)
  {
    delete static_cast<DistributedContourTreeBlockData<FieldType>*>(b);
  }
};
} // namespace contourtree_distributed
} // namespace worklet
} // namespace viskores


namespace viskoresdiy
{

// Struct to serialize ContourTreeMesh objects (i.e., load/save) needed in parralle for DIY
template <typename FieldType>
struct Serialization<viskores::worklet::contourtree_augmented::ContourTreeMesh<FieldType>>
{
  static void save(viskoresdiy::BinaryBuffer& bb,
                   const viskores::worklet::contourtree_augmented::ContourTreeMesh<FieldType>& ctm)
  {
    viskoresdiy::save(bb, ctm.NumVertices);
    viskoresdiy::save(bb, ctm.SortedValues);
    viskoresdiy::save(bb, ctm.GlobalMeshIndex);
    viskoresdiy::save(bb, ctm.NeighborConnectivity);
    viskoresdiy::save(bb, ctm.NeighborOffsets);
    viskoresdiy::save(bb, ctm.MaxNeighbors);
  }

  static void load(viskoresdiy::BinaryBuffer& bb,
                   viskores::worklet::contourtree_augmented::ContourTreeMesh<FieldType>& ctm)
  {
    viskoresdiy::load(bb, ctm.NumVertices);
    viskoresdiy::load(bb, ctm.SortedValues);
    viskoresdiy::load(bb, ctm.GlobalMeshIndex);
    viskoresdiy::load(bb, ctm.NeighborConnectivity);
    viskoresdiy::load(bb, ctm.NeighborOffsets);
    viskoresdiy::load(bb, ctm.MaxNeighbors);
  }
};

} // namespace mangled_viskoresdiy_namespace


#endif
