//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//============================================================================
#ifndef vtk_m_cont_Token_h
#define vtk_m_cont_Token_h

#include <vtkm/cont/vtkm_cont_export.h>

#include <vtkm/Types.h>

#include <condition_variable>
#include <memory>
#include <mutex>
#include <type_traits>

namespace vtkm
{
namespace cont
{

/// \brief A token to hold the scope of an `ArrayHandle` or other object.
///
/// A `Token` is an object that is held in the stack or state of another object and
/// is used when creating references to resouces that may be used by other threads.
/// For example, when preparing an `ArrayHandle` or `ExecutionObject` for a device,
/// a `Token` is given. The returned object will be valid as long as the `Token`
/// remains in scope.
///
class VTKM_CONT_EXPORT Token final
{
  class InternalStruct;
  std::unique_ptr<InternalStruct> Internals;

  struct HeldReference;

  struct ObjectReference
  {
    virtual ~ObjectReference() = default;
  };

  template <typename ObjectType>
  struct ObjectReferenceImpl : ObjectReference
  {
    ObjectType Object;

    ObjectReferenceImpl(const ObjectType& object)
      : Object(object)
    {
    }

    ObjectReferenceImpl(ObjectType&& object)
      : Object(std::move(object))
    {
    }

    ObjectReferenceImpl() = delete;
    ObjectReferenceImpl(const ObjectReferenceImpl&) = delete;

    ~ObjectReferenceImpl() override = default;
  };

public:
  Token();
  ~Token();

  /// Use this type to represent counts of how many tokens are holding a resource.
  using ReferenceCount = vtkm::IdComponent;

  /// Detaches this `Token` from all resources to allow them to be used elsewhere
  /// or deleted.
  ///
  void DetachFromAll();

  /// \brief Add an object to attach to the `Token`.
  ///
  /// To attach an object to a `Token`, you need the object itself, a pointer to
  /// a `Token::ReferenceCount` that is used to count how many `Token`s hold the
  /// object, a pointer to a `std::mutex` used to safely use the `ReferenceCount`,
  /// and a pointer to a `std::condition_variable` that other threads will wait
  /// on if they are blocked by the `Token`.
  ///
  /// When the `Token` is attached, it will increment the reference count (safely
  /// with the mutex) and store away these items. Other items will be able tell
  /// if a token is attached to the object by looking at the reference count.
  ///
  /// When the `Token` is released, it will decrement the reference count (safely
  /// with the mutex) and then notify all threads waiting on the condition
  /// variable.
  ///
  template <typename T>
  void Attach(T&& object,
              vtkm::cont::Token::ReferenceCount* referenceCountPointer,
              std::mutex* mutexPointer,
              std::condition_variable* conditionVariablePointer)
  {
    this->Attach(std::unique_ptr<ObjectReference>(
                   new ObjectReferenceImpl<typename std::decay<T>::type>(std::forward<T>(object))),
                 referenceCountPointer,
                 mutexPointer,
                 conditionVariablePointer);
  }

private:
  void Attach(std::unique_ptr<vtkm::cont::Token::ObjectReference>&& objectReference,
              vtkm::cont::Token::ReferenceCount* referenceCountPointer,
              std::mutex* mutexPointer,
              std::condition_variable* conditionVariablePointer);
};
}
} // namespace vtkm::cont

#endif //vtk_m_cont_Token_h
