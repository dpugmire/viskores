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

#ifndef vtkm_worklet_contourtree_augmented_contourtree_maker_inc_transfer_leaf_chains_init_in_and_outbound_h
#define vtkm_worklet_contourtree_augmented_contourtree_maker_inc_transfer_leaf_chains_init_in_and_outbound_h

#include <vtkm/worklet/WorkletMapField.h>
#include <vtkm/worklet/contourtree_augmented/Types.h>

namespace vtkm
{
namespace worklet
{
namespace contourtree_augmented
{
namespace contourtree_maker_inc
{

// Worklet to loop through each active node to copy join/split to outbound and inbound arrays
class TransferLeafChains_InitInAndOutbound : public vtkm::worklet::WorkletMapField
{
public:
  typedef void ControlSignature(FieldIn<IdType> activeSupernodes, // (input)
                                WholeArrayIn<IdType> inwards,     // (input)
                                WholeArrayIn<IdType> outdegree,   // (input)
                                WholeArrayIn<IdType> indegree,    // (input)
                                WholeArrayOut<IdType> outbound,   // (output)
                                WholeArrayOut<IdType> inbound     // (output)
                                );
  typedef void ExecutionSignature(_1, InputIndex, _2, _3, _4, _5, _6);
  using InputDomain = _1;


  // Default Constructor
  VTKM_EXEC_CONT
  TransferLeafChains_InitInAndOutbound() {}

  template <typename InFieldPortalType, typename OutFieldPortalType>
  VTKM_EXEC void operator()(const vtkm::Id& superID,
                            const vtkm::Id /*activeID*/, // FIXME: Remove unused parameter?
                            const InFieldPortalType& inwardsPortal,
                            const InFieldPortalType& outdegreePortal,
                            const InFieldPortalType& indegreePortal,
                            const OutFieldPortalType& outboundPortal,
                            const OutFieldPortalType& inboundPortal) const
  {
    vtkm::Id inNeighbour = inwardsPortal.Get(superID);

    // if the vertex is a leaf, set it's inbound links
    if ((outdegreePortal.Get(superID) == 0) && (indegreePortal.Get(superID) == 1))
    { // outer leaf
      outboundPortal.Set(superID, superID | TERMINAL_ELEMENT);
      inboundPortal.Set(superID, inNeighbour);
    } // outer leaf
    else if ((outdegreePortal.Get(superID) != 1) || (indegreePortal.Get(superID) != 1))
    { // other critical node
      outboundPortal.Set(superID, superID | TERMINAL_ELEMENT);
      inboundPortal.Set(superID, superID | TERMINAL_ELEMENT);
    } // other critical node
    else
    { // non-critical node
      // copy the inwards link (which is guaranteed not to be the infinite root)
      inboundPortal.Set(superID, inNeighbour);
    } // non-critical node

    // if the inbound neighbour is regular, we want to set its outbound link
    // check whether it's the root in the tree (points to infinite root)
    if (!noSuchElement(inNeighbour))
    { // inwards exists
      // if the inwards neighbour is regular, set the reverse link
      if ((outdegreePortal.Get(inNeighbour) == 1) && (indegreePortal.Get(inNeighbour) == 1))
      {
        outboundPortal.Set(inNeighbour, superID);
      }
    } // inwards exists



    // In serial this worklet implements the following operation
    /*
      for (vtkm::Id activeID = 0; activeID < activeSupernodes.size(); activeID++)
        { // for each active node
          vtkm::Id superID = activeSupernodes[activeID];
          vtkm::Id inNeighbour = inwards[superID];

          // if the vertex is a leaf, set it's inbound links
          if ((outdegree[superID] == 0) && (indegree[superID] == 1))
            { // outer leaf
              outbound[superID] = superID | TERMINAL_ELEMENT;
              inbound[superID] = inNeighbour;
            } // outer leaf
          else if ((outdegree[superID] != 1) || (indegree[superID] != 1))
            { // other critical node
              outbound[superID] = superID | TERMINAL_ELEMENT;
              inbound[superID] = superID | TERMINAL_ELEMENT;
            } // other critical node
          else
            { // non-critical node
              // copy the inwards link (which is guaranteed not to be the infinite root)
              inbound[superID] = inNeighbour;
            } // non-critical node

          // if the inbound neighbour is regular, we want to set its outbound link
          // check whether it's the root in the tree (points to infinite root)
          if (!noSuchElement(inNeighbour))
            { // inwards exists
              // if the inwards neighbour is regular, set the reverse link
              if ((outdegree[inNeighbour] == 1) && (indegree[inNeighbour] == 1))
                {
                  outbound[inNeighbour] = superID;
                }
            } // inwards exists
        } // for each active node

      */
  }

}; // TransferLeafChains_InitInAndOutound

} // namespace contourtree_maker_inc
} // namespace contourtree_augmented
} // namespace worklet
} // namespace vtkm

#endif
