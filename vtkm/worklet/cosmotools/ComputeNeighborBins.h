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

#ifndef vtkm_worklet_cosmotools_compute_neighbor_bins_h
#define vtkm_worklet_cosmotools_compute_neighbor_bins_h

#include <vtkm/exec/ExecutionWholeArray.h>
#include <vtkm/worklet/WorkletMapField.h>

namespace vtkm
{
namespace worklet
{
namespace cosmotools
{

// Worklet for computing the left neighbor bin id for every particle in domain
// In 3D there will be 9 "left" neighbors which start 3 consecutive bins = 27
class ComputeNeighborBins : public vtkm::worklet::WorkletMapField
{
public:
  typedef void ControlSignature(FieldIn<IdType> partIndx,
                                FieldIn<IdType> binId,               // (input) bin Id
                                WholeArrayOut<IdType> leftNeighbor); // (output) neighbor Id
  typedef void ExecutionSignature(_1, _2, _3);
  typedef _1 InputDomain;

  vtkm::Id xNum, yNum, zNum;
  vtkm::Id NUM_NEIGHBORS;

  // Constructor
  VTKM_EXEC_CONT
  ComputeNeighborBins(vtkm::Id XNum, vtkm::Id YNum, vtkm::Id ZNum, vtkm::Id NumNeighbors)
    : xNum(XNum)
    , yNum(YNum)
    , zNum(ZNum)
    , NUM_NEIGHBORS(NumNeighbors)
  {
  }

  template <typename OutFieldPortalType>
  VTKM_EXEC void operator()(const vtkm::Id& i,
                            const vtkm::Id& binId,
                            OutFieldPortalType& leftNeighbor) const
  {
    const vtkm::Id xbin = binId % xNum;
    const vtkm::Id ybin = (binId / xNum) % yNum;
    const vtkm::Id zbin = binId / (xNum * yNum);

    vtkm::Id cnt = 0;
    for (vtkm::Id z = zbin - 1; z <= zbin + 1; z++)
    {
      for (vtkm::Id y = ybin - 1; y <= ybin + 1; y++)
      {
        if ((y >= 0) && (y < yNum) && (z >= 0) && (z < zNum))
        {
          if (xbin - 1 >= 0)
            leftNeighbor.Set((NUM_NEIGHBORS * i + cnt), (xbin - 1) + y * xNum + z * xNum * yNum);
          else
            leftNeighbor.Set((NUM_NEIGHBORS * i + cnt), xbin + y * xNum + z * xNum * yNum);
        }
        else
        {
          leftNeighbor.Set((NUM_NEIGHBORS * i + cnt), -1);
        }
        cnt++;
      }
    }
  }
}; // ComputeNeighborBins
}
}
}

#endif
