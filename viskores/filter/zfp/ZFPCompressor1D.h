//============================================================================
//  The contents of this file are covered by the Viskores license. See
//  LICENSE.txt for details.
//
//  By contributing to this file, all contributors agree to the Developer
//  Certificate of Origin Version 1.1 (DCO 1.1) as stated in DCO.txt.
//============================================================================

//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//============================================================================

#ifndef viskores_filter_zfp_ZFPCompressor1D_h
#define viskores_filter_zfp_ZFPCompressor1D_h

#include <viskores/filter/Filter.h>
#include <viskores/filter/zfp/viskores_filter_zfp_export.h>

namespace viskores
{
namespace filter
{
namespace zfp
{

/// \brief Compress a scalar field using ZFP.
///
/// Takes as input a 1D array and generates an
/// output of compressed data.
/// @warning
/// This filter currently only supports 1D structured cell sets.
class VISKORES_FILTER_ZFP_EXPORT ZFPCompressor1D : public viskores::filter::Filter
{
public:
  /// @brief Specifies the rate of compression.
  void SetRate(viskores::Float64 _rate) { rate = _rate; }
  /// @copydoc SetRate
  viskores::Float64 GetRate() { return rate; }

private:
  VISKORES_CONT viskores::cont::DataSet DoExecute(const viskores::cont::DataSet& input);

  viskores::Float64 rate = 0;
};

} // namespace zfp
} // namespace filter
} // namespace viskores::filter

#endif // viskores_filter_zfp_ZFPCompressor1D_h
