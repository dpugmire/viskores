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

#ifndef vtk_m_filter_internal_waveletdwt_h
#define vtk_m_filter_internal_waveletdwt_h

//#include <vtkm/worklet/WorkletMapField.h>
//#include <vtkm/worklet/DispatcherMapField.h>

#include <vtkm/filter/internal/WaveletBase.h>

#include <vtkm/worklet/WaveletTransforms.h>

#include <vtkm/cont/ArrayHandleConcatenate.h>
#include <vtkm/cont/ArrayHandleConstant.h>

#include <vtkm/Math.h>

namespace vtkm {
namespace filter {
namespace internal {

class WaveletDWT : public WaveletBase
{
public:

  // Constructor
  WaveletDWT( const std::string &w_name ) : WaveletBase( w_name ) {} 

  // Func: Extend 1D signal
  template< typename T >
  vtkm::Id Extend1D( const vtkm::cont::ArrayHandle<T> &sigIn,   // Input
                     vtkm::cont::ArrayHandleConcatenate<        // Output
                        vtkm::cont::ArrayHandleConcatenate< 
                          vtkm::cont::ArrayHandle<T>, vtkm::cont::ArrayHandle<T> >,
                        vtkm::cont::ArrayHandle<T> >  &sigOut,
                     vtkm::Id                         addLen,
                     vtkm::filter::internal::DWTMode  leftExtMethod,
                     vtkm::filter::internal::DWTMode  rightExtMethod )
  { 
    vtkm::cont::ArrayHandle<T> leftExtend, rightExtend;
    leftExtend.Allocate( addLen );
    rightExtend.Allocate( addLen );

    typedef typename vtkm::cont::ArrayHandle<T>     ArrayType;
    typedef typename ArrayType::PortalControl       PortalType;
    typedef typename ArrayType::PortalConstControl  PortalConstType;

    typedef typename vtkm::cont::ArrayHandleConcatenate< ArrayType, ArrayType> 
            ArrayConcat;

    PortalType leftExtendPortal  = leftExtend.GetPortalControl();
    PortalType rightExtendPortal = rightExtend.GetPortalControl();
    PortalConstType sigInPortal  = sigIn.GetPortalConstControl();
    vtkm::Id sigInLen            = sigIn.GetNumberOfValues();

    switch( leftExtMethod )
    {
      case vtkm::filter::internal::SYMH:
      {
          for( vtkm::Id count = 0; count < addLen; count++ )
            leftExtendPortal.Set( count, sigInPortal.Get( addLen - count - 1) );
          break;
      }
      case vtkm::filter::internal::SYMW:
      {
          for( vtkm::Id count = 0; count < addLen; count++ )
            leftExtendPortal.Set( count, sigInPortal.Get( addLen - count ) );
          break;
      }
      default:
      {
        // throw out an error
        return 1;
      }
    }

    switch( rightExtMethod )
    {
      case vtkm::filter::internal::SYMH:
      {
          for( vtkm::Id count = 0; count < addLen; count++ )
            rightExtendPortal.Set( count, sigInPortal.Get( sigInLen - count - 1 ) );
          break;
      }
      case SYMW:
      {
          for( vtkm::Id count = 0; count < addLen; count++ )
            rightExtendPortal.Set( count, sigInPortal.Get( sigInLen - count - 2 ) );
          break;
      }
      default:
      {
        // throw out an error
        return 1;
      }
    }

    ArrayConcat leftOn( leftExtend, sigIn );    
    sigOut = vtkm::cont::make_ArrayHandleConcatenate< ArrayConcat, ArrayType >
                  (leftOn, rightExtend );

    return 0;
  }



  // Performs one level of 1D discrete wavelet transform 
  // It takes care of boundary conditions, etc.
  template< typename SignalArrayType, typename CoeffArrayType >
  vtkm::Id DWT1D( const SignalArrayType &sigIn,     // Input
                  CoeffArrayType        &sigOut,
                  vtkm::Id              L[3] )
  {

    vtkm::Id sigInLen = sigIn.GetNumberOfValues();
    if( GetWaveletMaxLevel( sigInLen ) < 1 )
    {
      // throw an error
      std::cerr << "Cannot transform signal of length " << sigInLen << std::endl;
      return -1;
    } 

    L[0] = this->GetApproxLength( sigInLen );
    L[1] = this->GetDetailLength( sigInLen );
    L[2] = sigInLen;

    vtkm::Id filterLen = this->filter->GetFilterLength();

    bool doSymConv = false;
    if( this->filter->isSymmetric() )
    {
      if( ( this->wmode == SYMW && ( filterLen % 2 != 0 ) ) ||
          ( this->wmode == SYMH && ( filterLen % 2 == 0 ) ) )
        doSymConv = true;
    }

    vtkm::Id sigConvolvedLen = L[0] + L[1];     // approx + detail coeffs
    vtkm::Id addLen;                            // for extension
    bool oddLow  = true;
    bool oddHigh = true;
    if( filterLen %2 != 0 )
      oddLow = false;
    if( doSymConv )
    {
      addLen = filterLen >> 1;
      if( sigInLen % 2 != 0 )
        sigConvolvedLen += 1;
    }
    else
      addLen = filterLen - 1; 
  
    vtkm::Id sigExtendedLen = sigInLen + 2 * addLen;

    typedef typename SignalArrayType::ValueType SigInValueType;
    typedef vtkm::cont::ArrayHandle<SigInValueType>     ArrayType;
    typedef vtkm::cont::ArrayHandleConcatenate< ArrayType, ArrayType> 
              ArrayConcat;
    typedef vtkm::cont::ArrayHandleConcatenate< ArrayConcat, ArrayType > ArrayConcat2;
    
    ArrayConcat2 sigInExtended;
    this->Extend1D( sigIn, sigInExtended, addLen, this->wmode, this->wmode ); 

    ArrayType coeffOutTmp;

    
    // initialize a worklet
    vtkm::worklet::ForwardTransform forwardTransform;
    forwardTransform.SetFilterLength( filterLen );
    forwardTransform.SetCoeffLength( L[0], L[1] );
    forwardTransform.SetOddness( oddLow, oddHigh );

    // setup a timer
    //srand ((unsigned int)time(NULL));
    //vtkm::cont::Timer<> timer;

    // try other types of fancy arrays
    vtkm::cont::ArrayHandleConstant< vtkm::Float64 > constArray( 3, 
        sigInExtended.GetNumberOfValues() );

    vtkm::worklet::DispatcherMapField<vtkm::worklet::ForwardTransform> 
        dispatcher(forwardTransform);
    dispatcher.Invoke( constArray, //sigInExtended, 
                       filter->GetLowDecomposeFilter(),
                       filter->GetHighDecomposeFilter(),
                       coeffOutTmp );

    //vtkm::Id randNum = rand() % sigLen;
    //std::cout << "A random output: " 
    //          << outputArray1.GetPortalConstControl().Get(randNum) << std::endl;

    //vtkm::Float64 elapsedTime = timer.GetElapsedTime();  
    //std::cerr << "Dealing array size " << sigLen/million << " millions takes time " 
    //          << elapsedTime << std::endl;
    if( sigInLen < 21 )
      for (vtkm::Id i = 0; i < coeffOutTmp.GetNumberOfValues(); ++i)
      {
        std::cout << coeffOutTmp.GetPortalConstControl().Get(i) << ", ";
        if( i % 2 != 0 )
          std::cout << std::endl;
      }
 
    return 0;  
  }

};    // Finish class WaveletDWT

}     // Finish namespace internal
}     // Finish namespace filter
}     // Finish namespace vtkm

#endif 
