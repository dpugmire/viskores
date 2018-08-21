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


#ifndef vtkm_worklet_contourtree_ppp2_types_h
#define vtkm_worklet_contourtree_ppp2_types_h

#include <vtkm/Types.h>
#include <vtkm/cont/ArrayHandle.h>

// macros for bit flags
#ifndef VTKM_USE_64BIT_IDS // 32 bit Ids

#define NO_SUCH_ELEMENT 0x80000000L
#define TERMINAL_ELEMENT 0x40000000L
#define IS_SUPERNODE 0x20000000L
#define IS_HYPERNODE 0x10000000L
#define IS_ASCENDING 0x08000000L
#define INDEX_MASK 0x07FFFFFFL
#define CV_OTHER_FLAG                                                                              \
  0x10000000L // Flag used by CombinedVector class used by the ContourTreeMesh to merge contour trees

#else // 64 bit Ids

#define NO_SUCH_ELEMENT 0x8000000000000000LL
#define TERMINAL_ELEMENT 0x4000000000000000LL
#define IS_SUPERNODE 0x2000000000000000LL
#define IS_HYPERNODE 0x1000000000000000LL
#define IS_ASCENDING 0x0800000000000000LL
#define INDEX_MASK 0x07FFFFFFFFFFFFFFLL
#define CV_OTHER_FLAG                                                                              \
  0x1000000000000000LL // Flag used by CombinedVector class used by the ContourTreeMesh to merge contour trees
#endif

namespace vtkm
{
namespace worklet
{
namespace contourtree_ppp2
{


typedef vtkm::cont::ArrayHandle<vtkm::Id> IdArrayType;

typedef typename vtkm::Pair<vtkm::Id, vtkm::Id>
  EdgePair; // here EdgePair.first=low and EdgePair.second=high
typedef typename vtkm::cont::ArrayHandle<EdgePair> EdgePairArray; // Array of edge pairs

// inline functions for retrieving flags or index
VTKM_EXEC_CONT
inline bool noSuchElement(vtkm::Id flaggedIndex)
{ // noSuchElement()
  return ((flaggedIndex & (vtkm::Id)NO_SUCH_ELEMENT) != 0);
} // noSuchElement()

VTKM_EXEC_CONT
inline bool isTerminalElement(vtkm::Id flaggedIndex)
{ // isTerminalElement()
  return ((flaggedIndex & TERMINAL_ELEMENT) != 0);
} // isTerminalElement()

VTKM_EXEC_CONT
inline bool isSupernode(vtkm::Id flaggedIndex)
{ // isSupernode()
  return ((flaggedIndex & IS_SUPERNODE) != 0);
} // isSupernode()

VTKM_EXEC_CONT
inline bool isHypernode(vtkm::Id flaggedIndex)
{ // isHypernode()
  return ((flaggedIndex & IS_HYPERNODE) != 0);
} // isHypernode()

VTKM_EXEC_CONT
inline bool isAscending(vtkm::Id flaggedIndex)
{ // isAscending()
  return ((flaggedIndex & IS_ASCENDING) != 0);
} // isAscending()

VTKM_EXEC_CONT
inline vtkm::Id maskedIndex(vtkm::Id flaggedIndex)
{ // maskedIndex()
  return (flaggedIndex & INDEX_MASK);
} // maskedIndex()

template <typename T>
struct MaskedIndexFunctor
{
  VTKM_EXEC_CONT

  MaskedIndexFunctor() {}

  VTKM_EXEC_CONT
  vtkm::Id operator()(T x) const { return maskedIndex(x); }
};

inline std::string flagString(vtkm::Id flaggedIndex)
{ // flagString()
  std::string fString("");
  fString += (noSuchElement(flaggedIndex) ? "n" : ".");
  fString += (isTerminalElement(flaggedIndex) ? "t" : ".");
  fString += (isSupernode(flaggedIndex) ? "s" : ".");
  fString += (isHypernode(flaggedIndex) ? "h" : ".");
  fString += (isAscending(flaggedIndex) ? "a" : ".");
  return fString;
} // flagString()



} // namespace contourtree_ppp2
} // worklet
} // vtkm

#endif
