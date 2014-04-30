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
//  Copyright 2014. Los Alamos National Security
//
//  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
//  the U.S. Government retains certain rights in this software.
//
//  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
//  Laboratory (LANL), the U.S. Government retains certain rights in
//  this software.
//============================================================================
#ifndef vtk_m_cont_internal_FunctionInterface_h
#define vtk_m_cont_internal_FunctionInterface_h

#include <vtkm/Types.h>
#include <vtkm/cont/ErrorControlBadValue.h>

#include <boost/function_types/components.hpp>
#include <boost/function_types/function_arity.hpp>
#include <boost/function_types/function_type.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/integer/static_min_max.hpp>
#include <boost/mpl/advance.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/begin.hpp>
#include <boost/mpl/erase.hpp>
#include <boost/mpl/insert.hpp>
#include <boost/mpl/push_back.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/dec.hpp>
#include <boost/preprocessor/inc.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_shifted.hpp>
#include <boost/preprocessor/repetition/enum_shifted_binary_params.hpp>
#include <boost/preprocessor/repetition/enum_shifted_params.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/utility/enable_if.hpp>

#define VTKM_MAX_FUNCTION_PARAMETERS 10

namespace vtkm {
namespace internal {

/// This struct is used internally by FunctionInterface to store the return
/// value of a function. There is a special implementation for a return type of
/// void, which stores nothing.
///
template<typename T>
struct FunctionInterfaceReturnContainer {
  T Value;
  static const bool VALID = true;
};

template<>
struct FunctionInterfaceReturnContainer<void> {
  // Nothing to store for void return.
  static const bool VALID = false;
};

namespace detail {

// If you get a compiler error stating that this class is not specialized, that
// probably means that you are using FunctionInterface with an unsupported
// number of arguments.
template<typename FunctionSignature>
struct ParameterContainer;

// The following code uses the Boost preprocessor utilities to create
// definitions of ParameterContainer for all supported number of arguments.
// The created classes are conceptually defined as follows:
//
// template<typename P0, // Return type
//          typename P1,
//          typename P2, ...>
// struct ParameterContainer<P0(P1,P2,...)> {
//   P1 Parameter1;
//   P2 Parameter2;
//   ...
// };
//
// These are defined for 0 to VTKM_MAX_FUNCTION_PARAMETERS parameters.

#define VTK_M_PARAMETER_DEFINITION(z, ParamIndex, data) \
  BOOST_PP_IF(ParamIndex, \
              BOOST_PP_CAT(P,ParamIndex) BOOST_PP_CAT(Parameter,ParamIndex);,)

#define VTK_M_PARAMETER_CONTAINER(NumParamsPlusOne) \
  template<BOOST_PP_ENUM_PARAMS(NumParamsPlusOne, typename P)> \
  struct ParameterContainer<P0(BOOST_PP_ENUM_SHIFTED_PARAMS(NumParamsPlusOne, P))> \
  { \
    BOOST_PP_REPEAT(NumParamsPlusOne, VTK_M_PARAMETER_DEFINITION,) \
  };

#define VTK_M_PARAMETER_CONTAINER_REPEAT(z, NumParams, data) \
  VTK_M_PARAMETER_CONTAINER(BOOST_PP_INC(NumParams))
BOOST_PP_REPEAT(BOOST_PP_INC(VTKM_MAX_FUNCTION_PARAMETERS),
                VTK_M_PARAMETER_CONTAINER_REPEAT,)

#undef VTK_M_PARAMETER_CONTAINER_REPEAT
#undef VTK_M_PARAMETER_CONTAINER
#undef VTK_M_PARAMETER_DEFINITION

template<int ParameterIndex, typename FunctionSignature>
struct ParameterContainerAccess;

// The following code uses the Boost preprocessor utilities to create
// definitions of ParameterContainerAccess for all supported number of
// arguments. The created class specalizations conceptually create the
// following interface:
//
// template<int ParameterIndex, typename R(P1,P2,...)>
// struct ParameterContainerAccess
// {
//   VTKM_EXEC_CONT_EXPORT
//   static ParameterType
//   GetParameter(const ParameterContainer<R(P1,P2,...)> &parameters);
//
//   VTKM_EXEC_CONT_EXPORT
//   static void SetParameter(ParameterContainer<R(P1,P2,...)> &parameters,
//                            const ParameterType &value);
// };
//
// Here ParameterType is the P# type in the function signature for the given
// ParameterIndex. It is the same you would get for
// FunctionInterface::ParameterType.

#define VTK_M_PARAMETER_CONTAINER_ACCESS(ParameterIndex) \
  template<typename FunctionSignature> \
  struct ParameterContainerAccess<ParameterIndex, FunctionSignature> { \
    typedef typename boost::mpl::at_c< \
        boost::function_types::components<FunctionSignature>, \
        ParameterIndex>::type ParameterType; \
    VTKM_EXEC_CONT_EXPORT \
    static \
    ParameterType \
    GetParameter(const ParameterContainer<FunctionSignature> &parameters) { \
      return parameters.BOOST_PP_CAT(Parameter, ParameterIndex); \
    } \
    VTKM_EXEC_CONT_EXPORT \
    static \
    void SetParameter(ParameterContainer<FunctionSignature> &parameters, \
                      const ParameterType &value) { \
      parameters.BOOST_PP_CAT(Parameter, ParameterIndex) = value; \
    } \
  };

#define VTK_M_PARAMETER_CONTAINER_ACCESS_REPEAT(z, i, data) \
  VTK_M_PARAMETER_CONTAINER_ACCESS(BOOST_PP_INC(i))
BOOST_PP_REPEAT(BOOST_PP_INC(VTKM_MAX_FUNCTION_PARAMETERS),
                VTK_M_PARAMETER_CONTAINER_ACCESS_REPEAT,)

#undef VTK_M_PARAMETER_CONTAINER_ACCESS_REPEAT
#undef VTK_M_PARAMETER_CONTAINER_ACCESS

template<int ParameterIndex, typename FunctionSignature>
VTKM_EXEC_CONT_EXPORT
typename ParameterContainerAccess<ParameterIndex,FunctionSignature>::ParameterType
GetParameter(const ParameterContainer<FunctionSignature> &parameters) {
  return ParameterContainerAccess<ParameterIndex,FunctionSignature>::GetParameter(parameters);
}

template<int ParameterIndex, typename FunctionSignature>
VTKM_EXEC_CONT_EXPORT

void SetParameter(ParameterContainer<FunctionSignature> &parameters,
                  const typename ParameterContainerAccess<ParameterIndex,FunctionSignature>::ParameterType &value) {
  return ParameterContainerAccess<ParameterIndex,FunctionSignature>::SetParameter(parameters, value);
}

struct IdentityFunctor {
  template<typename T>
  VTKM_EXEC_CONT_EXPORT
  T &operator()(T &x) const { return x; }

  template<typename T>
  VTKM_EXEC_CONT_EXPORT
  const T &operator()(const T &x) const { return x; }
};

// The following code uses the Boost preprocessor utilities to create
// definitions of DoInvoke functions for all supported number of arguments.
// The created functions are conceptually defined as follows:
//
// template<typename Function, typename Signature, typename TransformFunctor>
// VTKM_CONT_EXPORT
// void DoInvokeCont(const Function &f,
//                   ParameterContainer<Signature> &parameters,
//                   FunctionInterfaceReturnContainer<R> &result,
//                   const TransformFunctor &transform)
// {
//   result.Value = transform(f(transform(parameters.Parameter1),...));
// }
//
// We define multiple DoInvokeCont and DoInvokeExec that do identical things
// with different exports. It is important to have these separate definitions
// instead of a single version with VTKM_EXEC_CONT_EXPORT because the function
// to be invoked may only be viable in one or the other. There are also
// separate versions that support const functions and non-const functions.
// (However, the structures from the FunctionInterface must always be
// non-const.) Finally, there is a special version for functions that return
// void so that the function does not try to invalidly save a void value.

#define VTK_M_DO_INVOKE_TPARAM(z, count, data) \
  transform(BOOST_PP_CAT(parameters.Parameter, count))

#define VTK_M_DO_INVOKE(NumParamsPlusOne) \
  template<typename Function, \
           typename TransformFunctor, \
           BOOST_PP_ENUM_PARAMS(NumParamsPlusOne, typename P)> \
  VTK_M_DO_INVOKE_EXPORT \
  void VTK_M_DO_INVOKE_NAME( \
      VTK_M_DO_INVOKE_FUNCTION_CONST Function &f, \
      ParameterContainer<P0(BOOST_PP_ENUM_SHIFTED_PARAMS(NumParamsPlusOne, P))> &parameters, \
      FunctionInterfaceReturnContainer<P0> &result, \
      const TransformFunctor &transform) \
  { \
    (void)parameters; \
    (void)transform; \
    result.Value = \
      transform( \
        f(BOOST_PP_ENUM_SHIFTED(NumParamsPlusOne, VTK_M_DO_INVOKE_TPARAM, ))); \
  } \
  \
  template<typename Function, \
           typename TransformFunctor BOOST_PP_COMMA_IF(BOOST_PP_DEC(NumParamsPlusOne)) \
           BOOST_PP_ENUM_SHIFTED_PARAMS(NumParamsPlusOne, typename P)> \
  VTK_M_DO_INVOKE_EXPORT \
  void VTK_M_DO_INVOKE_NAME( \
      VTK_M_DO_INVOKE_FUNCTION_CONST Function &f, \
      ParameterContainer<void(BOOST_PP_ENUM_SHIFTED_PARAMS(NumParamsPlusOne, P))> &parameters, \
      FunctionInterfaceReturnContainer<void> &, \
      const TransformFunctor &transform) \
  { \
    (void)parameters; \
    (void)transform; \
    f(BOOST_PP_ENUM_SHIFTED(NumParamsPlusOne, VTK_M_DO_INVOKE_TPARAM, )); \
  }
#define VTK_M_DO_INVOKE_REPEAT(z, NumParams, data) \
  VTK_M_DO_INVOKE(BOOST_PP_INC(NumParams));

#define VTK_M_DO_INVOKE_NAME DoInvokeCont
#define VTK_M_DO_INVOKE_EXPORT VTKM_CONT_EXPORT
#define VTK_M_DO_INVOKE_FUNCTION_CONST const
BOOST_PP_REPEAT(BOOST_PP_INC(VTKM_MAX_FUNCTION_PARAMETERS),
                VTK_M_DO_INVOKE_REPEAT,)
#undef VTK_M_DO_INVOKE_NAME
#undef VTK_M_DO_INVOKE_EXPORT
#undef VTK_M_DO_INVOKE_FUNCTION_CONST

#define VTK_M_DO_INVOKE_NAME DoInvokeCont
#define VTK_M_DO_INVOKE_EXPORT VTKM_CONT_EXPORT
#define VTK_M_DO_INVOKE_FUNCTION_CONST
BOOST_PP_REPEAT(BOOST_PP_INC(VTKM_MAX_FUNCTION_PARAMETERS),
                VTK_M_DO_INVOKE_REPEAT,)
#undef VTK_M_DO_INVOKE_NAME
#undef VTK_M_DO_INVOKE_EXPORT
#undef VTK_M_DO_INVOKE_FUNCTION_CONST

#define VTK_M_DO_INVOKE_NAME DoInvokeExec
#define VTK_M_DO_INVOKE_EXPORT VTKM_EXEC_EXPORT
#define VTK_M_DO_INVOKE_FUNCTION_CONST const
BOOST_PP_REPEAT(BOOST_PP_INC(VTKM_MAX_FUNCTION_PARAMETERS),
                VTK_M_DO_INVOKE_REPEAT,)
#undef VTK_M_DO_INVOKE_NAME
#undef VTK_M_DO_INVOKE_EXPORT
#undef VTK_M_DO_INVOKE_FUNCTION_CONST

#define VTK_M_DO_INVOKE_NAME DoInvokeExec
#define VTK_M_DO_INVOKE_EXPORT VTKM_EXEC_EXPORT
#define VTK_M_DO_INVOKE_FUNCTION_CONST
BOOST_PP_REPEAT(BOOST_PP_INC(VTKM_MAX_FUNCTION_PARAMETERS),
                VTK_M_DO_INVOKE_REPEAT,)
#undef VTK_M_DO_INVOKE_NAME
#undef VTK_M_DO_INVOKE_EXPORT
#undef VTK_M_DO_INVOKE_FUNCTION_CONST

#undef VTK_M_DO_INVOKE_REPEAT
#undef VTK_M_DO_INVOKE
#undef VTK_M_DO_INVOKE_TPARAM


// These functions exist to help copy components of a FunctionInterface.

template<int NumToCopy, int ParameterIndex = 1>
struct FunctionInterfaceCopyParameters {
  template<typename DestSignature, typename SrcSignature>
  static
  VTKM_EXEC_CONT_EXPORT
  void Copy(vtkm::internal::detail::ParameterContainer<DestSignature> &dest,
            const vtkm::internal::detail::ParameterContainer<SrcSignature> &src)
  {
    vtkm::internal::detail::SetParameter<ParameterIndex>(
          dest,vtkm::internal::detail::GetParameter<ParameterIndex>(src));
   FunctionInterfaceCopyParameters<NumToCopy-1,ParameterIndex+1>::Copy(dest, src);
  }
};

template<int ParameterIndex>
struct FunctionInterfaceCopyParameters<0, ParameterIndex> {
  template<typename DestSignature, typename SrcSignature>
  static
  VTKM_EXEC_CONT_EXPORT
  void Copy(vtkm::internal::detail::ParameterContainer<DestSignature> &,
            const vtkm::internal::detail::ParameterContainer<SrcSignature> &)
  {
    // Nothing left to copy.
  }
};

template<typename OriginalFunction,
         typename NewFunction,
         typename TransformFunctor,
         typename FinishFunctor>
class FunctionInterfaceDynamicTransformContContinue;

} // namespace detail

template<typename FunctionSignature>
class FunctionInterface
{
  template<typename OtherSignature>
  friend class FunctionInterface;

public:
  typedef FunctionSignature Signature;

  typedef typename boost::function_types::result_type<FunctionSignature>::type
      ResultType;
  template<int ParameterIndex>
  struct ParameterType {
    typedef typename boost::mpl::at_c<
        boost::function_types::components<FunctionSignature>,
        ParameterIndex>::type type;
  };
  static const bool RETURN_VALID = FunctionInterfaceReturnContainer<ResultType>::VALID;

  /// The number of parameters in this \c Function Interface.
  ///
  static const int ARITY =
      boost::function_types::function_arity<FunctionSignature>::value;

  /// Returns the number of parameters held in this \c FunctionInterface. The
  /// return value is the same as \c ARITY.
  ///
  VTKM_EXEC_CONT_EXPORT
  int GetArity() const { return ARITY; }

  /// Retrieves the return value from the last invocation called. This method
  /// will result in a compiler error if used with a function having a void
  /// return type.
  ///
  VTKM_EXEC_CONT_EXPORT
  ResultType GetReturnValue() const { return this->Result.Value; }

  /// Retrieves the return value from the last invocation wrapped in a \c
  /// FunctionInterfaceReturnContainer object. This call can succeed even if
  /// the return type is void. You still have to somehow check to make sure the
  /// return is non-void before trying to use it, but using this method can
  /// simplify templated programming.
  ///
  VTKM_EXEC_CONT_EXPORT
  const FunctionInterfaceReturnContainer<ResultType> &GetReturnValueSafe() const
  {
    return this->Result;
  }
  VTKM_EXEC_CONT_EXPORT
  FunctionInterfaceReturnContainer<ResultType> &GetReturnValueSafe()
  {
    return this->Result;
  }

  /// Gets the value for the parameter of the given index. Parameters are
  /// indexed starting at 1. To use this method you have to specify the index
  /// as a template parameter. If you are using FunctionInterface within a
  /// template (which is almost always the case), then you will have to use the
  /// template keyword. For example, here is a simple implementation of a
  /// method that grabs the first parameter of FunctionInterface.
  ///
  /// \code{.cpp}
  /// template<FunctionSignature>
  /// void Foo(const vtkm::cont::internal::FunctionInterface<FunctionSignature> &fInterface)
  /// {
  ///   bar(fInterface.template GetParameter<1>());
  /// }
  /// \endcode
  ///
  template<int ParameterIndex>
  VTKM_EXEC_CONT_EXPORT
  typename ParameterType<ParameterIndex>::type
  GetParameter() {
    return detail::GetParameter<ParameterIndex>(this->Parameters);
  }

  /// Sets the value for the parameter of the given index. Parameters are
  /// indexed starting at 1. To use this method you have to specify the index
  /// as a template parameter. If you are using FunctionInterface within a
  /// template (which is almost always the case), then you will have to use the
  /// template keyword.
  ///
  template<int ParameterIndex>
  VTKM_EXEC_CONT_EXPORT
  void SetParameter(typename ParameterType<ParameterIndex>::type parameter)
  {
    detail::SetParameter<ParameterIndex>(this->Parameters, parameter);
  }

  /// Copies the parameters and return values from the given \c
  /// FunctionInterface to this object. The types must be copiable from source
  /// to destination. If the number of parameters in the two objects are not
  /// the same, copies the first N arguments, where N is the smaller arity of
  /// the two function interfaces.
  ///
  template<typename SrcFunctionSignature>
  void Copy(const FunctionInterface<SrcFunctionSignature> &src)
  {
    this->Result = src.GetReturnValueSafe();
    detail::FunctionInterfaceCopyParameters<
        boost::static_unsigned_min<ARITY, FunctionInterface<SrcFunctionSignature>::ARITY>::value>::
        Copy(this->Parameters, src.Parameters);
  }

  /// Invoke a function \c f using the arguments stored in this
  /// FunctionInterface.
  ///
  /// If this FunctionInterface specifies a non-void return value, then the
  /// result of the function call is stored within this FunctionInterface and
  /// can be retrieved with GetReturnValue().
  ///
  template<typename Function>
  VTKM_CONT_EXPORT
  void InvokeCont(const Function &f) {
    detail::DoInvokeCont(f,
                         this->Parameters,
                         this->Result,
                         detail::IdentityFunctor());
  }
  template<typename Function>
  VTKM_CONT_EXPORT
  void InvokeCont(Function &f) {
    detail::DoInvokeCont(f,
                         this->Parameters,
                         this->Result,
                         detail::IdentityFunctor());
  }
  template<typename Function>
  VTKM_EXEC_EXPORT
  void InvokeExec(const Function &f) {
    detail::DoInvokeExec(f,
                         this->Parameters,
                         this->Result,
                         detail::IdentityFunctor());
  }
  template<typename Function>
  VTKM_EXEC_EXPORT
  void InvokeExec(Function &f) {
    detail::DoInvokeExec(f,
                         this->Parameters,
                         this->Result,
                         detail::IdentityFunctor());
  }

  /// Invoke a function \c f using the arguments stored in this
  /// FunctionInterface and a transform.
  ///
  /// These versions of invoke also apply a transform to the input arguments.
  /// The transform is a second functor passed a second argument. If this
  /// FunctionInterface specifies a non-void return value, then the result of
  /// the function call is also transformed and stored within this
  /// FunctionInterface and can be retrieved with GetReturnValue().
  ///
  template<typename Function, typename TransformFunctor>
  VTKM_CONT_EXPORT
  void InvokeCont(const Function &f, const TransformFunctor &transform) {
    detail::DoInvokeCont(f, this->Parameters, this->Result, transform);
  }
  template<typename Function, typename TransformFunctor>
  VTKM_CONT_EXPORT
  void InvokeCont(Function &f, const TransformFunctor &transform) {
    detail::DoInvokeCont(f, this->Parameters, this->Result, transform);
  }
  template<typename Function, typename TransformFunctor>
  VTKM_EXEC_EXPORT
  void InvokeExec(const Function &f, const TransformFunctor &transform) {
    detail::DoInvokeExec(f, this->Parameters, this->Result, transform);
  }
  template<typename Function, typename TransformFunctor>
  VTKM_EXEC_EXPORT
  void InvokeExec(Function &f, const TransformFunctor &transform) {
    detail::DoInvokeExec(f, this->Parameters, this->Result, transform);
  }

  template<typename NewType>
  struct AppendType {
    typedef FunctionInterface<
        typename boost::function_types::function_type<
          typename boost::mpl::push_back<
            boost::function_types::components<FunctionSignature>,
            NewType
          >::type
        >::type
      > type;
  };

  /// Returns a new \c FunctionInterface with all the parameters of this \c
  /// FunctionInterface and the given method argument appended to these
  /// parameters. The return type can be determined with the \c AppendType
  /// template.
  ///
  template<typename NewType>
  VTKM_EXEC_CONT_EXPORT
  typename AppendType<NewType>::type
  Append(NewType newParameter) const {
    typename AppendType<NewType>::type appendedFuncInterface;
    appendedFuncInterface.Copy(*this);
    appendedFuncInterface.template SetParameter<ARITY+1>(newParameter);
    return appendedFuncInterface;
  }

  template<int ParameterIndex, typename NewType>
  class ReplaceType {
    typedef boost::function_types::components<FunctionSignature> ThisFunctionComponents;
    typedef typename boost::mpl::advance_c<typename boost::mpl::begin<ThisFunctionComponents>::type, ParameterIndex>::type ToRemovePos;
    typedef typename boost::mpl::erase<ThisFunctionComponents, ToRemovePos>::type ComponentRemoved;
    typedef typename boost::mpl::advance_c<typename boost::mpl::begin<ComponentRemoved>::type, ParameterIndex>::type ToInsertPos;
    typedef typename boost::mpl::insert<ComponentRemoved, ToInsertPos, NewType>::type ComponentInserted;
    typedef typename boost::function_types::function_type<ComponentInserted>::type NewSignature;
  public:
    typedef FunctionInterface<NewSignature> type;
  };

  /// Returns a new \c FunctionInterface with all the parameters of this \c
  /// FunctionInterface except that the parameter indexed at the template
  /// parameter \c ParameterIndex is replaced with the given argument. This
  /// method can be used in place of SetParameter when the parameter type
  /// changes. The return type can be determined with the \c ReplaceType
  /// template.
  ///
  template<int ParameterIndex, typename NewType>
  VTKM_EXEC_CONT_EXPORT
  typename ReplaceType<ParameterIndex, NewType>::type
  Replace(NewType newParameter) const {
    typename ReplaceType<ParameterIndex, NewType>::type replacedFuncInterface;
    detail::FunctionInterfaceCopyParameters<ParameterIndex-1>::
        Copy(replacedFuncInterface.Parameters, this->Parameters);
    replacedFuncInterface.template SetParameter<ParameterIndex>(newParameter);
    detail::FunctionInterfaceCopyParameters<ARITY-ParameterIndex,ParameterIndex+1>::
        Copy(replacedFuncInterface.Parameters, this->Parameters);
    return replacedFuncInterface;
  }

  /// \brief Transforms the \c FunctionInterface based on run-time information.
  ///
  /// The \c DynamicTransform method transforms all the parameters of this \c
  /// FunctionInterface to different types and values based on run-time
  /// information. It operates by accepting two functors. The first functor
  /// accepts two arguments. The first argument is a parameter to transform and
  /// the second is a functor to call with the transformed result.
  ///
  /// The second argument to \c DynamicTransform is another function that
  /// accepts the transformed \c FunctionInterface and does something. If that
  /// transformed \c FunctionInterface has a return value, that return value
  /// will be passed back to this \c FunctionInterface.
  ///
  /// Here is a contrived but illustrative example. This transformation will
  /// pass all arguments except any string that looks like a number will be
  /// converted to a vtkm::Scalar. Note that because the types are not
  /// determined till runtime, this transform cannot be determined at compile
  /// time with meta-template programming.
  ///
  /// \code
  /// struct MyTransformFunctor {
  ///   template<typename InputType, typename ContinueFunctor>
  ///   VTKM_CONT_EXPORT
  ///   void operator()(const InputType &input,
  ///                   const ContinueFunctor &continue) const
  ///   {
  ///     continue(input);
  ///   }
  ///
  ///   template<typename ContinueFunctor>
  ///   VTKM_CONT_EXPORT
  ///   void operator()(const std::string &input,
  ///                   const ContinueFunctor &continueFunc) const
  ///   {
  ///     if ((input[0] >= '0' && (input[0] <= '9'))
  ///     {
  ///       std::stringstream stream(input);
  ///       vtkm::Scalar value;
  ///       stream >> value;
  ///       continueFunc(value);
  ///     }
  ///     else
  ///     {
  ///       continueFunc(input);
  ///     }
  ///   }
  /// };
  ///
  /// struct MyFinishFunctor {
  ///   template<typename FunctionSignature>
  ///   VTKM_CONT_EXPORT
  ///   void operator()(vtkm::internal::FunctionInterface<FunctionSignature> &funcInterface) const
  ///   {
  ///     // Do something
  ///   }
  /// };
  ///
  /// template<typename FunctionSignature>
  /// void ImportantStuff(vtkm::internal::FunctionInterface<FunctionSignature> &funcInterface)
  /// {
  ///   funcInterface.DynamicTransformCont(MyContinueFunctor(), MyFinishFunctor());
  /// }
  /// \endcode
  ///
  /// An interesting feature of \c DynamicTransform is that there does not have
  /// to be a one-to-one transform. It is possible to make many valid
  /// transforms by calling the continue functor multiple times within the
  /// transform functor. It is also possible to abort the transform by not
  /// calling the continue functor.
  ///
  template<typename TransformFunctor, typename FinishFunctor>
  VTKM_CONT_EXPORT
  void
  DynamicTransformCont(const TransformFunctor &transform,
                       const FinishFunctor &finish) {
    typedef detail::FunctionInterfaceDynamicTransformContContinue<
        FunctionSignature,
        ResultType(),
        TransformFunctor,
        FinishFunctor> ContinueFunctorType;

    FunctionInterface<ResultType()> emptyInterface;
    ContinueFunctorType continueFunctor =
        ContinueFunctorType(*this, emptyInterface, transform, finish);

    continueFunctor.DoNextTransform(emptyInterface);
    this->Result = emptyInterface.GetReturnValueSafe();
  }

private:
  vtkm::internal::FunctionInterfaceReturnContainer<ResultType> Result;
  detail::ParameterContainer<FunctionSignature> Parameters;
};

namespace detail {

template<typename OriginalFunction,
         typename NewFunction,
         typename TransformFunctor,
         typename FinishFunctor>
class FunctionInterfaceDynamicTransformContContinue
{
public:
  FunctionInterfaceDynamicTransformContContinue(
      vtkm::internal::FunctionInterface<OriginalFunction> &originalInterface,
      vtkm::internal::FunctionInterface<NewFunction> &newInterface,
      const TransformFunctor &transform,
      const FinishFunctor &finish)
    : OriginalInterface(originalInterface),
      NewInterface(newInterface),
      Transform(transform),
      Finish(finish)
  {  }

  template<typename T>
  VTKM_CONT_EXPORT
  void operator()(T newParameter) const
  {
    typedef typename vtkm::internal::FunctionInterface<NewFunction>::template AppendType<T>::type
        NextInterfaceType;
    NextInterfaceType nextInterface = this->NewInterface.Append(newParameter);
    this->DoNextTransform(nextInterface);
    this->NewInterface.GetReturnValueSafe()
        = nextInterface.GetReturnValueSafe();
  }

  template<typename NextFunction>
  VTKM_CONT_EXPORT
  typename boost::enable_if_c<
    vtkm::internal::FunctionInterface<NextFunction>::ARITY
    < vtkm::internal::FunctionInterface<OriginalFunction>::ARITY>::type
  DoNextTransform(
      vtkm::internal::FunctionInterface<NextFunction> &nextInterface) const
  {
    typedef FunctionInterfaceDynamicTransformContContinue<
        OriginalFunction,NextFunction,TransformFunctor,FinishFunctor> NextContinueType;
    NextContinueType nextContinue = NextContinueType(this->OriginalInterface,
                                                     nextInterface,
                                                     this->Transform,
                                                     this->Finish);
    this->Transform(this->OriginalInterface.template GetParameter<vtkm::internal::FunctionInterface<NextFunction>::ARITY + 1>(),
                    nextContinue);
  }

  template<typename NextFunction>
  VTKM_CONT_EXPORT
  typename boost::disable_if_c<
    vtkm::internal::FunctionInterface<NextFunction>::ARITY
    < vtkm::internal::FunctionInterface<OriginalFunction>::ARITY>::type
  DoNextTransform(
      vtkm::internal::FunctionInterface<NextFunction> &nextInterface) const
  {
    this->Finish(nextInterface);
  }

private:
  vtkm::internal::FunctionInterface<OriginalFunction> &OriginalInterface;
  vtkm::internal::FunctionInterface<NewFunction> &NewInterface;
  const TransformFunctor &Transform;
  const FinishFunctor &Finish;
};

} // namespace detail

// The following code uses the Boost preprocessor utilities to create
// definitions of make_FunctionInterface for all supported number of arguments.
// The created functions are conceptually defined as follows:
//
// template<typename P0, // Return type
//          typename P1,
//          typename P2, ...>
// VTKM_EXEC_CONT_EXPORT
// FunctionInterface<P0(P1,P2,...)>
// make_FunctionInterface(P1 p1, P2 p2,...) {
//   FunctionInterface<P0(P1,P2,...)> fi;
//   fi.template SetParameters<1>(p1);
//   fi.template SetParameters<2>(p2);
//   ...
//   return fi;
// }

#define VTK_M_SET_PARAMETER(z, ParamIndex, data) \
  BOOST_PP_IF( \
      ParamIndex, \
      fi.template SetParameter<ParamIndex>(BOOST_PP_CAT(p, ParamIndex));,)

#define VTK_M_MAKE_FUNCTION_INTERFACE(NumParamsPlusOne) \
  template<BOOST_PP_ENUM_PARAMS(NumParamsPlusOne, typename P)> \
  VTKM_EXEC_CONT_EXPORT \
  FunctionInterface<P0(BOOST_PP_ENUM_SHIFTED_PARAMS(NumParamsPlusOne, P))> \
  make_FunctionInterface( \
      BOOST_PP_ENUM_SHIFTED_BINARY_PARAMS(NumParamsPlusOne, P, p)) \
  { \
    FunctionInterface<P0(BOOST_PP_ENUM_SHIFTED_PARAMS(NumParamsPlusOne, P))> fi; \
    BOOST_PP_REPEAT(NumParamsPlusOne, VTK_M_SET_PARAMETER,) \
    return fi; \
  }

#define VTK_M_MAKE_FUNCITON_INTERFACE_REPEAT(z, NumParams, data) \
  VTK_M_MAKE_FUNCTION_INTERFACE(BOOST_PP_INC(NumParams))
BOOST_PP_REPEAT(BOOST_PP_INC(VTKM_MAX_FUNCTION_PARAMETERS),
                VTK_M_MAKE_FUNCITON_INTERFACE_REPEAT,)

#undef VTK_M_MAKE_FUNCITON_INTERFACE_REPEAT
#undef VTK_M_MAKE_FUNCTION_INTERFACE
#undef VTK_M_SET_PARAMETER

}
} // namespace vtkm::internal

#endif //vtk_m_cont_internal_FunctionInterface_h
