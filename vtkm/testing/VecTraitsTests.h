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
#ifndef vtkm_testing_VecTraitsTest_h
#define vtkm_testing_VecTraitsTest_h

#include <vtkm/VecTraits.h>

#include <vtkm/TypeTraits.h>

#include <vtkm/testing/Testing.h>

namespace vtkm
{
namespace testing
{

namespace detail
{

inline void CompareDimensionalityTags(vtkm::TypeTraitsScalarTag, vtkm::VecTraitsTagSingleComponent)
{
  // If we are here, everything is fine.
}
inline void CompareDimensionalityTags(vtkm::TypeTraitsVectorTag,
                                      vtkm::VecTraitsTagMultipleComponents)
{
  // If we are here, everything is fine.
}

template <vtkm::IdComponent NUM_COMPONENTS, typename T>
inline void CheckIsStatic(const T&, vtkm::VecTraitsTagSizeStatic)
{
  VTKM_TEST_ASSERT(vtkm::VecTraits<T>::NUM_COMPONENTS == NUM_COMPONENTS,
                   "Traits returns unexpected number of components");
}

template <vtkm::IdComponent NUM_COMPONENTS, typename T>
inline void CheckIsStatic(const T&, vtkm::VecTraitsTagSizeVariable)
{
  // If we are here, everything is fine.
}

template <typename VecType>
struct VecIsWritable
{
  using type = std::true_type;
};

template <typename ComponentType>
struct VecIsWritable<vtkm::VecCConst<ComponentType>>
{
  using type = std::false_type;
};

// Part of TestVecTypeImpl that writes to the Vec type
template <vtkm::IdComponent NUM_COMPONENTS, typename T, typename VecCopyType>
static void TestVecTypeWritableImpl(const T& inVector, const VecCopyType& vectorCopy, T& outVector,
                                    std::true_type)
{
  using Traits = vtkm::VecTraits<T>;
  using ComponentType = typename Traits::ComponentType;

  {
    const ComponentType multiplier = 4;
    for (vtkm::IdComponent i = 0; i < NUM_COMPONENTS; i++)
    {
      Traits::SetComponent(outVector, i,
                           ComponentType(multiplier * Traits::GetComponent(inVector, i)));
    }
    vtkm::Vec<ComponentType, NUM_COMPONENTS> resultCopy;
    Traits::CopyInto(outVector, resultCopy);
    VTKM_TEST_ASSERT(test_equal(resultCopy, multiplier * vectorCopy),
                     "Got bad result for scalar multiple");
  }

  {
    const ComponentType multiplier = 7;
    for (vtkm::IdComponent i = 0; i < NUM_COMPONENTS; i++)
    {
      Traits::GetComponent(outVector, i) =
        ComponentType(multiplier * Traits::GetComponent(inVector, i));
    }
    vtkm::Vec<ComponentType, NUM_COMPONENTS> resultCopy;
    Traits::CopyInto(outVector, resultCopy);
    VTKM_TEST_ASSERT(test_equal(resultCopy, multiplier * vectorCopy),
                     "Got bad result for scalar multiple");
  }
}

template <vtkm::IdComponent NUM_COMPONENTS, typename T, typename VecCopyType>
static void TestVecTypeWritableImpl(const T& vtkmNotUsed(inVector),
                                    const VecCopyType& vtkmNotUsed(vectorCopy),
                                    T& vtkmNotUsed(outVector), std::false_type)
{
  // Skip writable functionality.
}

/// Compares some manual arithmetic through type traits to arithmetic with
/// the Tuple class.
template <vtkm::IdComponent NUM_COMPONENTS, typename T>
static void TestVecTypeImpl(const typename std::remove_const<T>::type& inVector,
                            typename std::remove_const<T>::type& outVector)
{
  using Traits = vtkm::VecTraits<T>;
  using ComponentType = typename Traits::ComponentType;
  using NonConstT = typename std::remove_const<T>::type;

  CheckIsStatic<NUM_COMPONENTS>(inVector, typename Traits::IsSizeStatic());

  VTKM_TEST_ASSERT(Traits::GetNumberOfComponents(inVector) == NUM_COMPONENTS,
                   "Traits returned wrong number of components.");

  vtkm::Vec<ComponentType, NUM_COMPONENTS> vectorCopy;
  Traits::CopyInto(inVector, vectorCopy);
  VTKM_TEST_ASSERT(test_equal(vectorCopy, inVector), "CopyInto does not work.");

  {
    ComponentType result = 0;
    for (vtkm::IdComponent i = 0; i < NUM_COMPONENTS; i++)
    {
      ComponentType component = Traits::GetComponent(inVector, i);
      result = ComponentType(result + (component * component));
    }
    VTKM_TEST_ASSERT(test_equal(result, vtkm::dot(vectorCopy, vectorCopy)),
                     "Got bad result for dot product");
  }

  // This will fail to compile if the tags are wrong.
  detail::CompareDimensionalityTags(typename vtkm::TypeTraits<T>::DimensionalityTag(),
                                    typename vtkm::VecTraits<T>::HasMultipleComponents());

  TestVecTypeWritableImpl<NUM_COMPONENTS, NonConstT>(inVector, vectorCopy, outVector,
                                                     typename VecIsWritable<NonConstT>::type());
}

inline void CheckVecComponentsTag(vtkm::VecTraitsTagMultipleComponents)
{
  // If we are running here, everything is fine.
}

} // namespace detail

/// Checks to make sure that the HasMultipleComponents tag is actually for
/// multiple components. Should only be called for vector classes that actually
/// have multiple components.
///
template <class T>
inline void TestVecComponentsTag()
{
  // This will fail to compile if the tag is wrong
  // (i.e. not vtkm::VecTraitsTagMultipleComponents)
  detail::CheckVecComponentsTag(typename vtkm::VecTraits<T>::HasMultipleComponents());
}

namespace detail
{

inline void CheckScalarComponentsTag(vtkm::VecTraitsTagSingleComponent)
{
  // If we are running here, everything is fine.
}

} // namespace detail

/// Compares some manual arithmetic through type traits to arithmetic with
/// the Tuple class.
template <vtkm::IdComponent NUM_COMPONENTS, typename T>
static void TestVecType(const T& inVector, T& outVector)
{
  detail::TestVecTypeImpl<NUM_COMPONENTS, T>(inVector, outVector);
  detail::TestVecTypeImpl<NUM_COMPONENTS, const T>(inVector, outVector);
}

/// Checks to make sure that the HasMultipleComponents tag is actually for a
/// single component. Should only be called for "vector" classes that actually
/// have only a single component (that is, are really scalars).
///
template <class T>
inline void TestScalarComponentsTag()
{
  // This will fail to compile if the tag is wrong
  // (i.e. not vtkm::VecTraitsTagSingleComponent)
  detail::CheckScalarComponentsTag(typename vtkm::VecTraits<T>::HasMultipleComponents());
}
}
} // namespace vtkm::testing

#endif //vtkm_testing_VecTraitsTest_h
