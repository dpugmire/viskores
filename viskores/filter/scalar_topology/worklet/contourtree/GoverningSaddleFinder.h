//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//============================================================================
//  Copyright (c) 2016, Los Alamos National Security, LLC
//  All rights reserved.
//
//  Copyright 2016. Los Alamos National Security, LLC.
//  This software was produced under U.S. Government contract DE-AC52-06NA25396
//  for Los Alamos National Laboratory (LANL), which is operated by
//  Los Alamos National Security, LLC for the U.S. Department of Energy.
//  The U.S. Government has rights to use, reproduce, and distribute this
//  software.  NEITHER THE GOVERNMENT NOR LOS ALAMOS NATIONAL SECURITY, LLC
//  MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY LIABILITY FOR THE
//  USE OF THIS SOFTWARE.  If software is modified to produce derivative works,
//  such modified software should be clearly marked, so as not to confuse it
//  with the version available from LANL.
//
//  Additionally, redistribution and use in source and binary forms, with or
//  without modification, are permitted provided that the following conditions
//  are met:
//
//  1. Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//  3. Neither the name of Los Alamos National Security, LLC, Los Alamos
//     National Laboratory, LANL, the U.S. Government, nor the names of its
//     contributors may be used to endorse or promote products derived from
//     this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY LOS ALAMOS NATIONAL SECURITY, LLC AND
//  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
//  BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LOS ALAMOS
//  NATIONAL SECURITY, LLC OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
//  USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
//  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//============================================================================

//  This code is based on the algorithm presented in the paper:
//  “Parallel Peak Pruning for Scalable SMP Contour Tree Computation.”
//  Hamish Carr, Gunther Weber, Christopher Sewell, and James Ahrens.
//  Proceedings of the IEEE Symposium on Large Data Analysis and Visualization
//  (LDAV), October 2016, Baltimore, Maryland.

//=======================================================================================
//
// COMMENTS:
//
// This functor replaces a parallel loop examining neighbours - again, for arbitrary
// meshes, it needs to be a reduction, but for regular meshes, it's faster this way.
//
// Any vector needed by the functor for lookup purposes will be passed as a parameter to
// the constructor and saved, with the actual function call being the operator ()
//
// Vectors marked I/O are intrinsically risky unless there is an algorithmic guarantee
// that the read/writes are completely independent - which for our case actually occurs
// The I/O vectors should therefore be justified in comments both here & in caller
//
//=======================================================================================

#ifndef viskores_worklet_contourtree_governing_saddle_finder_h
#define viskores_worklet_contourtree_governing_saddle_finder_h

#include <viskores/worklet/WorkletMapField.h>

namespace viskores
{
namespace worklet
{
namespace contourtree
{

// Worklet for setting initial chain maximum value
class GoverningSaddleFinder : public viskores::worklet::WorkletMapField
{
public:
  using ControlSignature = void(FieldIn edgeNo,           // (input) index into sorted edges
                                WholeArrayIn edgeSorter,  // (input) sorted edge index
                                WholeArrayIn edgeFar,     // (input) high ends of edges
                                WholeArrayIn edgeNear,    // (input) low ends of edges
                                WholeArrayOut prunesTo,   // (output) where vertex is pruned to
                                WholeArrayOut outdegree); // (output) updegree of vertex
  using ExecutionSignature = void(_1, _2, _3, _4, _5, _6);
  using InputDomain = _1;

  // Constructor
  VISKORES_EXEC_CONT
  GoverningSaddleFinder() {}

  template <typename InFieldPortalType, typename OutFieldPortalType>
  VISKORES_EXEC void operator()(const viskores::Id& edgeNo,
                                const InFieldPortalType& edgeSorter,
                                const InFieldPortalType& edgeFar,
                                const InFieldPortalType& edgeNear,
                                const OutFieldPortalType& prunesTo,
                                const OutFieldPortalType& outdegree) const
  {
    // default to true
    bool isBestSaddleEdge = true;

    // retrieve the edge ID
    viskores::Id edge = edgeSorter.Get(edgeNo);

    // edge no. 0 is always best, so skip it
    if (edgeNo != 0)
    {
      // retrieve the previous edge
      viskores::Id prevEdge = edgeSorter.Get(edgeNo - 1);
      // if the previous edge has the same far end
      if (edgeFar.Get(prevEdge) == edgeFar.Get(edge))
        isBestSaddleEdge = false;
    }

    if (isBestSaddleEdge)
    { // found an extremum
      // retrieve the near end as the saddle
      viskores::Id saddle = edgeNear.Get(edge);
      // and the far end as the extremum
      viskores::Id extreme = edgeFar.Get(edge);

      // set the extremum to point to the saddle in the chainExtremum array
      prunesTo.Set(extreme, saddle);

      // and set the outdegree to 0
      outdegree.Set(extreme, 0);
    } // found a extremum
  }   // operator()

}; // GoverningSaddleFinder
}
}
}

#endif
