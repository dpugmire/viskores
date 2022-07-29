//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//============================================================================

#include <vtkm/cont/ArrayCopy.h>
#include <vtkm/filter/zfp/ZFPDecompressor3D.h>
#include <vtkm/filter/zfp/worklet/ZFPDecompress.h>

namespace vtkm
{
namespace filter
{
namespace zfp
{
//-----------------------------------------------------------------------------
VTKM_CONT vtkm::cont::DataSet ZFPDecompressor3D::DoExecute(const vtkm::cont::DataSet& input)
{
  // FIXME: it looks like the compressor can compress Ints and Floats but only decompressed
  //  to Float64?
  vtkm::cont::ArrayHandle<vtkm::Int64> compressed;
  vtkm::cont::ArrayCopyShallowIfPossible(this->GetFieldFromDataSet(input).GetData(), compressed);

  vtkm::cont::CellSetStructured<3> cellSet;
  input.GetCellSet().AsCellSet(cellSet);
  vtkm::Id3 pointDimensions = cellSet.GetPointDimensions();

  vtkm::cont::ArrayHandle<vtkm::Float64> decompressed;
  vtkm::worklet::ZFPDecompressor decompressor;
  decompressor.Decompress(compressed, decompressed, this->rate, pointDimensions);

  return this->CreateResultFieldPoint(input, "decompressed", decompressed);
}
} // namespace zfp
} // namespace filter
} // namespace vtkm
