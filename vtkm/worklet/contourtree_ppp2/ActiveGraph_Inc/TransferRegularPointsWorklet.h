//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//  Copyright 2014 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
//  Copyright 2014 UT-Battelle, LLC.
//  Copyright 2014 Los Alamos National Security.
//
//  Under the terms of Contract DE-NA0003525 with NTESS,
//  the U.S. Government retains certain rights in this software.
//
//  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
//  Laboratory (LANL), the U.S. Government retains certain rights in
//  this software.
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


#ifndef vtkm_worklet_contourtree_ppp2_active_graph_transfer_regular_points_worklet_h
#define vtkm_worklet_contourtree_ppp2_active_graph_transfer_regular_points_worklet_h

#include <vtkm/worklet/WorkletMapField.h>
#include <vtkm/worklet/contourtree_ppp2/Types.h>


namespace vtkm
{
namespace worklet
{
namespace contourtree_ppp2
{
namespace active_graph_inc
{


// Worklet for computing the sort indices from the sort order
class TransferRegularPointsWorklet : public vtkm::worklet::WorkletMapField
{
public:
  typedef void ControlSignature(FieldIn<IdType> activeVertices,   // (input) active vertices
                                WholeArrayIn<IdType> hyperarcs,   // (input) hyperarcs
                                WholeArrayOut<IdType> outdegree); // (output) outdegree
  typedef void ExecutionSignature(_1, InputIndex, _2, _3);
  typedef _1 InputDomain;

  // Default Constructor
  VTKM_EXEC_CONT
  TransferRegularPointsWorklet()
    : isJoinGraph(true)
  {
  }

  VTKM_EXEC_CONT
  TransferRegularPointsWorklet(bool joinGraph)
    : isJoinGraph(joinGraph)
  {
  }

  template <typename InFieldPortalType, typename OutFieldPortalType>
  VTKM_EXEC void operator()(const vtkm::Id& vertexId,
                            const vtkm::Id /*vertex*/, // FIXME: Remove unused parameter?
                            const InFieldPortalType& hyperarcsPortal,
                            const OutFieldPortalType& outdegreePortal) const
  {
    // FindGoverningSaddles() set the hyperarcs of the extrema already
    // to ignore them, we check the IS_HYPERNODE flag
    vtkm::Id hyperarcId = hyperarcsPortal.Get(vertexId);
    if (isHypernode(hyperarcId))
      return;

    // we know it isn't a hypernode/pseudo-extrema, so we take the index
    vtkm::Id hypernode = maskedIndex(hyperarcId);

    // since we know it points to a pseudo-extremum, we follow it's hyperarc
    vtkm::Id saddleId = maskedIndex(hyperarcsPortal.Get(hypernode));

    // now, we test whether the saddle is below this vertex
    // since in this version we have an invariant that the IDs are allocated in
    // strict sorted order, we just test indices (flipping for type of tree)
    if (isJoinGraph ? (saddleId < vertexId) : (saddleId > vertexId))
    { // regular point to be pruned
      // at this point, the hyperarc is already correctly set, although
      // it will have a terminal flag set. We will do a lazy transfer of this
      // in the final stages, so for now, all we do is:
      // reset the outdegree to zero
      outdegreePortal.Set(vertexId, 0);
    } // regular point to be pruned

    // In serial this worklet implements the following operation
    /*for (indexType vertex = 0; vertex < activeVertices.size(); vertex++)
                { // per vertex
                // retrieve the actual vertex ID
                indexType vertexID = activeVertices[vertex];

                // FindGoverningSaddles() set the hyperarcs of the extrema already
                // to ignore them, we check the IS_HYPERNODE flag
                indexType hyperarcID = hyperarcs[vertexID];
                if (isHypernode(hyperarcID))
                        continue;

                // we know it isn't a hypernode/pseudo-extrema, so we take the index
                indexType hypernode = maskedIndex(hyperarcID);

                // since we know it points to a pseudo-extremum, we follow it's hyperarc
                indexType saddleID = maskedIndex(hyperarcs[hypernode]);

                // now, we test whether the saddle is below this vertex
                // since in this version we have an invariant that the IDs are allocated in
                // strict sorted order, we just test indices (flipping for type of tree)
                if (isJoinGraph ? (saddleID < vertexID) : (saddleID > vertexID))
                        { // regular point to be pruned
                        // at this point, the hyperarc is already correctly set, although
                        // it will have a terminal flag set. We will do a lazy transfer of this
                        // in the final stages, so for now, all we do is:
                        // reset the outdegree to zero
                        outdegree[vertexID] = 0;
                        } // regular point to be pruned
                } // per vertex */
  }

private:
  bool isJoinGraph; // join graph or split graph?

}; // TransferRegularPointsWorklet


} // namespace active_graph_inc
} // namespace contourtree_ppp2
} // namespace worklet
} // namespace vtkm

#endif
