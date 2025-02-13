//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//============================================================================

#include <viskores/filter/connected_components/CellSetConnectivity.h>
#include <viskores/filter/connected_components/worklet/CellSetConnectivity.h>

namespace viskores
{
namespace filter
{
namespace connected_components
{
VISKORES_CONT viskores::cont::DataSet CellSetConnectivity::DoExecute(
  const viskores::cont::DataSet& input)
{
  viskores::cont::ArrayHandle<viskores::Id> component;

  viskores::worklet::connectivity::CellSetConnectivity::Run(input.GetCellSet(), component);

  return this->CreateResultFieldCell(input, this->GetOutputFieldName(), component);
}
} // namespace connected_components
} // namespace filter
} // namespace viskores
