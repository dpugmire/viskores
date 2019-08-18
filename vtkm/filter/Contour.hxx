//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//============================================================================

#include <vtkm/cont/ArrayHandleIndex.h>
#include <vtkm/cont/CellSetSingleType.h>
#include <vtkm/cont/CellSetStructured.h>
#include <vtkm/cont/DynamicCellSet.h>
#include <vtkm/cont/ErrorFilterExecution.h>

#include <vtkm/worklet/SurfaceNormals.h>

namespace vtkm
{
namespace filter
{

namespace
{

template <typename CellSetList>
bool IsCellSetStructured(const vtkm::cont::DynamicCellSetBase<CellSetList>& cellset)
{
  if (cellset.template IsType<vtkm::cont::CellSetStructured<1>>() ||
      cellset.template IsType<vtkm::cont::CellSetStructured<2>>() ||
      cellset.template IsType<vtkm::cont::CellSetStructured<3>>())
  {
    return true;
  }
  return false;
}
} // anonymous namespace

//-----------------------------------------------------------------------------
inline VTKM_CONT Contour::Contour()
  : vtkm::filter::FilterDataSetWithField<Contour>()
  , IsoValues()
  , GenerateNormals(false)
  , AddInterpolationEdgeIds(false)
  , ComputeFastNormalsForStructured(false)
  , ComputeFastNormalsForUnstructured(true)
  , NormalArrayName("normals")
  , InterpolationEdgeIdsArrayName("edgeIds")
  , Worklet()
{
  // todo: keep an instance of marching cubes worklet as a member variable
}

//-----------------------------------------------------------------------------
inline void Contour::SetNumberOfIsoValues(vtkm::Id num)
{
  if (num >= 0)
  {
    this->IsoValues.resize(static_cast<std::size_t>(num));
  }
}

//-----------------------------------------------------------------------------
inline vtkm::Id Contour::GetNumberOfIsoValues() const
{
  return static_cast<vtkm::Id>(this->IsoValues.size());
}

//-----------------------------------------------------------------------------
inline void Contour::SetIsoValue(vtkm::Id index, vtkm::Float64 v)
{
  std::size_t i = static_cast<std::size_t>(index);
  if (i >= this->IsoValues.size())
  {
    this->IsoValues.resize(i + 1);
  }
  this->IsoValues[i] = v;
}

//-----------------------------------------------------------------------------
inline void Contour::SetIsoValues(const std::vector<vtkm::Float64>& values)
{
  this->IsoValues = values;
}

//-----------------------------------------------------------------------------
inline vtkm::Float64 Contour::GetIsoValue(vtkm::Id index) const
{
  return this->IsoValues[static_cast<std::size_t>(index)];
}

//-----------------------------------------------------------------------------
template <typename T, typename StorageType, typename DerivedPolicy>
inline VTKM_CONT vtkm::cont::DataSet Contour::DoExecute(
  const vtkm::cont::DataSet& input,
  const vtkm::cont::ArrayHandle<T, StorageType>& field,
  const vtkm::filter::FieldMetadata& fieldMeta,
  const vtkm::filter::PolicyBase<DerivedPolicy>& policy)
{
  if (fieldMeta.IsPointField() == false)
  {
    throw vtkm::cont::ErrorFilterExecution("Point field expected.");
  }

  if (this->IsoValues.size() == 0)
  {
    throw vtkm::cont::ErrorFilterExecution("No iso-values provided.");
  }

  // Check the fields of the dataset to see what kinds of fields are present so
  // we can free the mapping arrays that won't be needed. A point field must
  // exist for this algorithm, so just check cells.
  const vtkm::Id numFields = input.GetNumberOfFields();
  bool hasCellFields = false;
  for (vtkm::Id fieldIdx = 0; fieldIdx < numFields && !hasCellFields; ++fieldIdx)
  {
    auto f = input.GetField(fieldIdx);
    hasCellFields = f.IsFieldCell();
  }

  //get the cells and coordinates of the dataset
  const vtkm::cont::DynamicCellSet& cells = input.GetCellSet(this->GetActiveCellSetIndex());

  const vtkm::cont::CoordinateSystem& coords =
    input.GetCoordinateSystem(this->GetActiveCoordinateSystemIndex());

  using Vec3HandleType = vtkm::cont::ArrayHandle<vtkm::Vec3f>;
  Vec3HandleType vertices;
  Vec3HandleType normals;

  vtkm::cont::DataSet output;
  vtkm::cont::CellSetSingleType<> outputCells;

  std::vector<T> ivalues(this->IsoValues.size());
  for (std::size_t i = 0; i < ivalues.size(); ++i)
  {
    ivalues[i] = static_cast<T>(this->IsoValues[i]);
  }

  //not sold on this as we have to generate more signatures for the
  //worklet with the design
  //But I think we should get this to compile before we tinker with
  //a more efficient api

  bool generateHighQualityNormals = IsCellSetStructured(cells)
    ? !this->ComputeFastNormalsForStructured
    : !this->ComputeFastNormalsForUnstructured;
  if (this->GenerateNormals && generateHighQualityNormals)
  {
    outputCells = this->Worklet.Run(&ivalues[0],
                                    static_cast<vtkm::Id>(ivalues.size()),
                                    vtkm::filter::ApplyPolicy(cells, policy),
                                    coords.GetData(),
                                    field,
                                    vertices,
                                    normals);
  }
  else
  {
    outputCells = this->Worklet.Run(&ivalues[0],
                                    static_cast<vtkm::Id>(ivalues.size()),
                                    vtkm::filter::ApplyPolicy(cells, policy),
                                    coords.GetData(),
                                    field,
                                    vertices);
  }

  if (this->GenerateNormals)
  {
    if (!generateHighQualityNormals)
    {
      Vec3HandleType faceNormals;
      vtkm::worklet::FacetedSurfaceNormals faceted;
      faceted.Run(outputCells, vertices, faceNormals);

      vtkm::worklet::SmoothSurfaceNormals smooth;
      smooth.Run(outputCells, faceNormals, normals);
    }

    output.AddField(vtkm::cont::make_FieldPoint(this->NormalArrayName, normals));
  }

  if (this->AddInterpolationEdgeIds)
  {
    vtkm::cont::Field interpolationEdgeIdsField(InterpolationEdgeIdsArrayName,
                                                vtkm::cont::Field::Association::POINTS,
                                                this->Worklet.GetInterpolationEdgeIds());
    output.AddField(interpolationEdgeIdsField);
  }

  //assign the connectivity to the cell set
  output.AddCellSet(outputCells);

  //add the coordinates to the output dataset
  vtkm::cont::CoordinateSystem outputCoords("coordinates", vertices);
  output.AddCoordinateSystem(outputCoords);

  if (!hasCellFields)
  {
    this->Worklet.ReleaseCellMapArrays();
  }

  return output;
}

//-----------------------------------------------------------------------------
template <typename T, typename StorageType, typename DerivedPolicy>
inline VTKM_CONT bool Contour::DoMapField(vtkm::cont::DataSet& result,
                                          const vtkm::cont::ArrayHandle<T, StorageType>& input,
                                          const vtkm::filter::FieldMetadata& fieldMeta,
                                          const vtkm::filter::PolicyBase<DerivedPolicy>&)
{
  vtkm::cont::ArrayHandle<T> fieldArray;

  if (fieldMeta.IsPointField())
  {
    fieldArray = this->Worklet.ProcessPointField(input);
  }
  else if (fieldMeta.IsCellField())
  {
    fieldArray = this->Worklet.ProcessCellField(input);
  }
  else
  {
    return false;
  }

  //use the same meta data as the input so we get the same field name, etc.
  result.AddField(fieldMeta.AsField(fieldArray));
  return true;
}
}
} // namespace vtkm::filter
