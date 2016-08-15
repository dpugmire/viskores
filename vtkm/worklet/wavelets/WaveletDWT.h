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

#ifndef vtk_m_worklet_wavelets_waveletdwt_h
#define vtk_m_worklet_wavelets_waveletdwt_h

#include <vtkm/worklet/wavelets/WaveletBase.h>

#include <vtkm/worklet/wavelets/WaveletTransforms.h>

#include <vtkm/cont/ArrayHandleConcatenate.h>
#include <vtkm/cont/ArrayHandleCounting.h>
#include <vtkm/cont/ArrayHandlePermutation.h>

#include <vtkm/Math.h>
#include <vtkm/cont/Timer.h>

namespace vtkm {
namespace worklet {
namespace wavelets {

class WaveletDWT : public WaveletBase
{
public:

  // Constructor
  WaveletDWT( WaveletName name ) : WaveletBase( name ) {} 


  // Func: Extend 1D signal
  template< typename SigInArrayType, typename SigExtendedArrayType >
  VTKM_CONT_EXPORT
  vtkm::Id Extend1D( const SigInArrayType                     &sigIn,   // Input
                     SigExtendedArrayType                     &sigOut,  // Output
                     vtkm::Id                                 addLen,
                     vtkm::worklet::wavelets::DWTMode         leftExtMethod,
                     vtkm::worklet::wavelets::DWTMode         rightExtMethod, 
                     bool                                     attachZeroRightLeft, 
                     bool                                     attachZeroRightRight )
  { 
    // "right extension" can be attached a zero on either end, but not both ends.
    VTKM_ASSERT( !attachZeroRightRight || !attachZeroRightLeft );
    typedef typename SigInArrayType::ValueType      ValueType;
    typedef vtkm::cont::ArrayHandle< ValueType >    ExtensionArrayType;
    ExtensionArrayType                              leftExtend;
    leftExtend.Allocate( addLen );

    vtkm::Id sigInLen = sigIn.GetNumberOfValues();

    typedef vtkm::worklet::wavelets::LeftSYMHExtentionWorklet  LeftSYMH;
    typedef vtkm::worklet::wavelets::LeftSYMWExtentionWorklet  LeftSYMW;
    typedef vtkm::worklet::wavelets::RightSYMHExtentionWorklet RightSYMH;
    typedef vtkm::worklet::wavelets::RightSYMWExtentionWorklet RightSYMW;

    switch( leftExtMethod )
    {
      case SYMH:
      {
          LeftSYMH worklet( addLen );
          vtkm::worklet::DispatcherMapField< LeftSYMH > dispatcher( worklet );
          dispatcher.Invoke( leftExtend, sigIn );
          break;
      }
      case SYMW:
      {
          LeftSYMW worklet( addLen );
          vtkm::worklet::DispatcherMapField< LeftSYMW > dispatcher( worklet );
          dispatcher.Invoke( leftExtend, sigIn );
          break;
      }
      default:
      {
        vtkm::cont::ErrorControlInternal("Left extension mode not supported!");
        return 1;
      }
    }

    ExtensionArrayType rightExtend;
    if( attachZeroRightRight )
      rightExtend.Allocate( addLen + 1 );
    else                  // attachZeroRightLeft mode will attach zero later.
      rightExtend.Allocate( addLen );

    switch( rightExtMethod )
    {
      case SYMH:
      {
          RightSYMH worklet( sigInLen );
          vtkm::worklet::DispatcherMapField< RightSYMH > dispatcher( worklet );
          dispatcher.Invoke( rightExtend, sigIn );
          break;
      }
      case SYMW:
      {
          RightSYMW worklet( sigInLen );
          vtkm::worklet::DispatcherMapField< RightSYMW > dispatcher( worklet );
          dispatcher.Invoke( rightExtend, sigIn );
          break;
      }
      default:
      {
        vtkm::cont::ErrorControlInternal("Right extension mode not supported!");
        return 1;
      }
    }

    if( attachZeroRightRight )
      WaveletBase::DeviceAssignZero( rightExtend, addLen );
    else if( attachZeroRightLeft )
    {
      ExtensionArrayType rightExtendPlusOne;
      rightExtendPlusOne.Allocate( addLen + 1 );
      WaveletBase::DeviceCopyStartX( rightExtend, rightExtendPlusOne, 1 );
      WaveletBase::DeviceAssignZero( rightExtendPlusOne, 0 );
      rightExtend = rightExtendPlusOne ;
    }

    typedef vtkm::cont::ArrayHandleConcatenate< ExtensionArrayType, SigInArrayType> 
            ArrayConcat;
    ArrayConcat leftOn( leftExtend, sigIn );    
    sigOut = vtkm::cont::make_ArrayHandleConcatenate( leftOn, rightExtend );

std::cerr << "Extend1D: output array length: " << sigOut.GetNumberOfValues() << std::endl;

    return 0;
  }


  // Func:
  // Performs one level of 1D discrete wavelet transform 
  // It takes care of boundary conditions, etc.
  template< typename SignalArrayType, typename CoeffArrayType>
  VTKM_CONT_EXPORT
  vtkm::Id DWT1D( const SignalArrayType &sigIn,     // Input
                  CoeffArrayType        &coeffOut,  // Output: cA followed by cD
                  std::vector<vtkm::Id> &L )        // Output: how many cA and cD.
  {
    vtkm::Id sigInLen = sigIn.GetNumberOfValues();
    if( GetWaveletMaxLevel( sigInLen ) < 1 )
    {
      vtkm::cont::ErrorControlInternal( "Signal is too short to perform DWT!" ); 
      return -1;
    } 

    VTKM_ASSERT( L.size() == 3 );
    L[0] = WaveletBase::GetApproxLength( sigInLen );
    L[1] = WaveletBase::GetDetailLength( sigInLen );
    L[2] = sigInLen;

    VTKM_ASSERT( L[0] + L[1] == L[2] );

    vtkm::Id filterLen = WaveletBase::filter.GetFilterLength();

    bool doSymConv = false;
    if( WaveletBase::filter.isSymmetric() )
    {
      if( ( WaveletBase::wmode == SYMW && ( filterLen % 2 != 0 ) ) ||
          ( WaveletBase::wmode == SYMH && ( filterLen % 2 == 0 ) ) )
        doSymConv = true;
    }

    vtkm::Id sigConvolvedLen = L[0] + L[1];     // approx + detail coeffs
    vtkm::Id addLen;                            // for extension
    bool oddLow  = true;
    bool oddHigh = true;
    if( filterLen % 2 != 0 )
      oddLow = false;
    if( doSymConv )
    {
      addLen = filterLen / 2;
      if( sigInLen % 2 != 0 )
        sigConvolvedLen += 1;
    }
    else
      addLen = filterLen - 1; 
  
    vtkm::Id sigExtendedLen = sigInLen + 2 * addLen;

    typedef typename SignalArrayType::ValueType             SigInValueType;
    typedef vtkm::cont::ArrayHandle<SigInValueType>         SigInBasic;
    //SigInBasic sigInExtended;

    typedef vtkm::cont::ArrayHandleConcatenate< SigInBasic, SignalArrayType >
            ConcatType1;
    typedef vtkm::cont::ArrayHandleConcatenate< ConcatType1, SigInBasic >
            ConcatType2;

    ConcatType2 sigInExtended;

    this->Extend1D( sigIn, sigInExtended, addLen, 
                    WaveletBase::wmode, WaveletBase::wmode, false, false ); 
    VTKM_ASSERT( sigInExtended.GetNumberOfValues() == sigExtendedLen );

    // initialize a worklet for forward transform
    vtkm::worklet::wavelets::ForwardTransform forwardTransform;
    forwardTransform.SetFilterLength( filterLen );
    forwardTransform.SetCoeffLength( L[0], L[1] );
    forwardTransform.SetOddness( oddLow, oddHigh );

    coeffOut.Allocate( sigInExtended.GetNumberOfValues() );
    vtkm::worklet::DispatcherMapField<vtkm::worklet::wavelets::ForwardTransform> 
        dispatcher(forwardTransform);
    dispatcher.Invoke( sigInExtended, 
                       WaveletBase::filter.GetLowDecomposeFilter(),
                       WaveletBase::filter.GetHighDecomposeFilter(),
                       coeffOut );

    VTKM_ASSERT( L[0] + L[1] <= coeffOut.GetNumberOfValues() );
    coeffOut.Shrink( L[0] + L[1] );
    
    return 0;  
  } // Function DWT1D
    

  // Func: 
  // Performs one level of inverse wavelet transform
  // It takes care of boundary conditions, etc.
  template< typename CoeffArrayType, typename SignalArrayType>
  VTKM_CONT_EXPORT
  vtkm::Id IDWT1D( const CoeffArrayType  &coeffIn,     // Input, cA followed by cD
                   std::vector<vtkm::Id> &L,           // Input, how many cA and cD
                   SignalArrayType       &sigOut )     // Output
  {
    VTKM_ASSERT( L.size() == 3 );
    VTKM_ASSERT( L[2] == coeffIn.GetNumberOfValues() );

    vtkm::Id filterLen = WaveletBase::filter.GetFilterLength();
    bool doSymConv = false;
    vtkm::worklet::wavelets::DWTMode cALeftMode  = WaveletBase::wmode;
    vtkm::worklet::wavelets::DWTMode cARightMode = WaveletBase::wmode;
    vtkm::worklet::wavelets::DWTMode cDLeftMode  = WaveletBase::wmode;
    vtkm::worklet::wavelets::DWTMode cDRightMode = WaveletBase::wmode;
  
    if( WaveletBase::filter.isSymmetric() )
    {
      if(( WaveletBase::wmode == SYMW && (filterLen % 2 != 0) ) || 
         ( WaveletBase::wmode == SYMH && (filterLen % 2 == 0) ) )
      {
        doSymConv = true;

        if( WaveletBase::wmode == SYMH )
        {
          cDLeftMode = ASYMH;
          if( L[2] % 2 != 0 )
          {
            cARightMode = SYMW;
            cDRightMode = ASYMW;
          }
          else
            cDRightMode = ASYMH;
        }
        else
        {
          cDLeftMode = SYMH;
          if( L[2] % 2 != 0 )
          {
            cARightMode = SYMW;
            cDRightMode = SYMH;
          }
          else
            cARightMode = SYMH;
        }
      }
    } 

    vtkm::Id cATempLen, cDTempLen;  //, reconTempLen;
    vtkm::Id addLen = 0;
    vtkm::Id cDPadLen  = 0;
    if( doSymConv )   // extend cA and cD
    {
      addLen = filterLen / 4;   // addLen == 0 for Haar kernel
      if( (L[0] > L[1]) && (WaveletBase::wmode == SYMH) )
        cDPadLen = L[0];  
      cATempLen = L[0] + 2 * addLen;
      cDTempLen = cATempLen;  // same length
    }
    else              // not extend cA and cD
    {                 //  (biorthogonal kernels won't come into this case)
      cATempLen = L[0];
      cDTempLen = L[1];
    }

    /* reconTempLen = L[2];
    if( reconTempLen % 2 != 0 )
      reconTempLen++; */

    typedef vtkm::cont::ArrayHandleCounting< vtkm::Id >      IdArrayType;
    typedef vtkm::cont::ArrayHandlePermutation< IdArrayType, CoeffArrayType > 
            PermutArrayType;

    // Separate cA and cD
    IdArrayType approxIndices( 0,    1, L[0] );
    IdArrayType detailIndices( L[0], 1, L[1] );
    PermutArrayType cA( approxIndices, coeffIn );
    PermutArrayType cD( detailIndices, coeffIn );
    

    typedef typename CoeffArrayType::ValueType                    CoeffValueType;
    typedef vtkm::cont::ArrayHandle< CoeffValueType >             ExtensionArrayType;
    typedef vtkm::cont::ArrayHandleConcatenate< ExtensionArrayType, PermutArrayType >
            Concat1;
    typedef vtkm::cont::ArrayHandleConcatenate< Concat1, ExtensionArrayType >
            Concat2;

    Concat2 cATemp, cDTemp;

    if( doSymConv )   // Actually extend cA and cD
    {
      // first extend cA to be cATemp
      this->Extend1D( cA, cATemp, addLen, cALeftMode, cARightMode, false, false );
      if( cDPadLen > 0 )  
      {
        // Add back the missing final cD, 0.0, at the beginning of right extension
        this->Extend1D( cD, cDTemp, addLen, cDLeftMode, cDRightMode, true, false );
      }
      else
      {
        vtkm::Id cDTempLenWouldBe = L[1] + 2 * addLen;
        if( cDTempLenWouldBe ==  cDTempLen )
        { std::cout << "case 1 " << std::endl;
          this->Extend1D( cD, cDTemp, addLen, cDLeftMode, cDRightMode, false, false);
std::cout << "case 1: cDTemp actual length = " << cDTemp.GetNumberOfValues() << std::endl;
        }
        else if( cDTempLenWouldBe ==  cDTempLen - 1 )
        { std::cout << "case 2" << std::endl;
          this->Extend1D( cD, cDTemp, addLen, cDLeftMode, cDRightMode, false, true );
        }
        else
        {
          std::cout << "case 3" << std::endl;
          vtkm::cont::ErrorControlInternal("cDTemp Length not match!");
          return 1;
        }
      }
    }     
    else    // !doSymConv (biorthogonals kernel won't come into this case)
    { 
      // make cATemp
      ExtensionArrayType dummyArray;
      dummyArray.Allocate(0);
      Concat1 cALeftOn( dummyArray, cA );
      cATemp = vtkm::cont::make_ArrayHandleConcatenate< Concat1, ExtensionArrayType >
               ( cALeftOn, dummyArray );
      
      // make cDTemp
      Concat1 cDLeftOn( dummyArray, cD );
      cDTemp = vtkm::cont::make_ArrayHandleConcatenate< Concat1, ExtensionArrayType >
               ( cDLeftOn, dummyArray );
    }

    // make sure signal extension went as expected
    VTKM_ASSERT( cATemp.GetNumberOfValues() == cATempLen );
std::cout << "cDTemp actual length = " << cDTemp.GetNumberOfValues() << std::endl;
std::cout << "cDTempLen = " << cDTempLen << std::endl;
    VTKM_ASSERT( cDTemp.GetNumberOfValues() == cDTempLen );

    vtkm::cont::ArrayHandleConcatenate< Concat2, Concat2>
        coeffInExtended( cATemp, cDTemp );
    // Allocate memory for sigOut
    sigOut.Allocate( cATempLen + cDTempLen );

    if( filterLen % 2 != 0 )
    {
      vtkm::worklet::wavelets::InverseTransformOdd inverseXformOdd;
      inverseXformOdd.SetFilterLength( filterLen );
      inverseXformOdd.SetCALength( L[0], cATempLen );
      vtkm::worklet::DispatcherMapField<vtkm::worklet::wavelets::InverseTransformOdd>
            dispatcher( inverseXformOdd );
      dispatcher.Invoke( coeffInExtended,
                         WaveletBase::filter.GetLowReconstructFilter(),
                         WaveletBase::filter.GetHighReconstructFilter(),
                         sigOut );
    }
    else
    {
      vtkm::worklet::wavelets::InverseTransformEven inverseXformEven
            ( filterLen, L[0], cATempLen, !doSymConv );
      vtkm::worklet::DispatcherMapField<vtkm::worklet::wavelets::InverseTransformEven>
            dispatcher( inverseXformEven );
      dispatcher.Invoke( coeffInExtended,
                         WaveletBase::filter.GetLowReconstructFilter(),
                         WaveletBase::filter.GetHighReconstructFilter(),
                         sigOut );
    }

    sigOut.Shrink( L[2] );

    return 0;

  }   // function IDWT1D
  
};    // class WaveletDWT

}     // namespace wavelets
}     // namespace worklet
}     // namespace vtkm

#endif 
