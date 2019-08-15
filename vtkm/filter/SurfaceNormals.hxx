//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//============================================================================

#include <vtkm/filter/SurfaceNormals.h>

#include <vtkm/cont/ErrorFilterExecution.h>

#include <vtkm/worklet/OrientNormals.h>
#include <vtkm/worklet/SurfaceNormals.h>
#include <vtkm/worklet/TriangleWinding.h>

namespace vtkm
{
namespace filter
{

namespace internal
{

inline std::string ComputePointNormalsName(const SurfaceNormals* filter)
{
  if (!filter->GetPointNormalsName().empty())
  {
    return filter->GetPointNormalsName();
  }
  else if (!filter->GetOutputFieldName().empty())
  {
    return filter->GetOutputFieldName();
  }
  else
  {
    return "Normals";
  }
}

inline std::string ComputeCellNormalsName(const SurfaceNormals* filter)
{
  if (!filter->GetCellNormalsName().empty())
  {
    return filter->GetCellNormalsName();
  }
  else if (!filter->GetGeneratePointNormals() && !filter->GetOutputFieldName().empty())
  {
    return filter->GetOutputFieldName();
  }
  else
  {
    return "Normals";
  }
}

} // internal

inline SurfaceNormals::SurfaceNormals()
  : GenerateCellNormals(false)
  , NormalizeCellNormals(true)
  , GeneratePointNormals(true)
  , AutoOrientNormals(false)
  , FlipNormals(false)
  , Consistency(true)
{
  this->SetUseCoordinateSystemAsField(true);
}

template <typename T, typename StorageType, typename DerivedPolicy>
inline vtkm::cont::DataSet SurfaceNormals::DoExecute(
  const vtkm::cont::DataSet& input,
  const vtkm::cont::ArrayHandle<vtkm::Vec<T, 3>, StorageType>& points,
  const vtkm::filter::FieldMetadata& fieldMeta,
  vtkm::filter::PolicyBase<DerivedPolicy> policy)
{
  VTKM_ASSERT(fieldMeta.IsPointField());

  if (!this->GenerateCellNormals && !this->GeneratePointNormals)
  {
    throw vtkm::cont::ErrorFilterExecution("No normals selected.");
  }

  const auto cellset =
    vtkm::filter::ApplyPolicyUnstructured(input.GetCellSet(this->GetActiveCellSetIndex()), policy);
  const auto& coords = input.GetCoordinateSystem(this->GetActiveCoordinateSystemIndex()).GetData();

  vtkm::cont::ArrayHandle<vtkm::Vec3f> faceNormals;
  vtkm::worklet::FacetedSurfaceNormals faceted;
  faceted.SetNormalize(this->NormalizeCellNormals);
  faceted.Run(cellset, points, faceNormals);

  vtkm::cont::DataSet result;
  vtkm::cont::ArrayHandle<vtkm::Vec3f> pointNormals;
  if (this->GeneratePointNormals)
  {
    vtkm::worklet::SmoothSurfaceNormals smooth;
    smooth.Run(cellset, faceNormals, pointNormals);


    result = CreateResultFieldPoint(input, pointNormals, internal::ComputePointNormalsName(this));
    if (this->GenerateCellNormals)
    {
      result.AddField(vtkm::cont::Field(internal::ComputeCellNormalsName(this),
                                        vtkm::cont::Field::Association::CELL_SET,
                                        cellset.GetName(),
                                        faceNormals));
    }
  }
  else
  {
    result =
      CreateResultFieldCell(input, faceNormals, internal::ComputeCellNormalsName(this), cellset);
  }

  if (this->AutoOrientNormals)
  {
    using Orient = vtkm::worklet::OrientNormals;

    if (this->GenerateCellNormals && this->GeneratePointNormals)
    {
      Orient::RunPointAndCellNormals(cellset, coords, pointNormals, faceNormals);
    }
    else if (this->GenerateCellNormals)
    {
      Orient::RunCellNormals(cellset, coords, faceNormals);
    }
    else if (this->GeneratePointNormals)
    {
      Orient::RunPointNormals(cellset, coords, pointNormals);
    }

    if (this->FlipNormals)
    {
      if (this->GenerateCellNormals)
      {
        Orient::RunFlipNormals(faceNormals);
      }
      if (this->GeneratePointNormals)
      {
        Orient::RunFlipNormals(pointNormals);
      }
    }
  }

  if (this->Consistency && this->GenerateCellNormals)
  {
    auto newCells = vtkm::worklet::TriangleWinding::Run(cellset, coords, faceNormals);

    // Add the new cells into the result:
    vtkm::cont::DataSet newResult;
    for (vtkm::Id i = 0; i < result.GetNumberOfCoordinateSystems(); ++i)
    {
      newResult.AddCoordinateSystem(result.GetCoordinateSystem(i));
    }
    const vtkm::Id activeCells = this->GetActiveCellSetIndex();
    for (vtkm::Id i = 0; i < result.GetNumberOfCellSets(); ++i)
    {
      if (i != activeCells)
      {
        newResult.AddCellSet(result.GetCellSet(i));
      }
      else
      {
        newResult.AddCellSet(newCells);
      }
    }
    for (vtkm::Id i = 0; i < result.GetNumberOfFields(); ++i)
    {
      newResult.AddField(result.GetField(i));
    }

    result = newResult;
  }

  return result;
}
}
} // vtkm::filter
