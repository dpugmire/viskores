//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//  Copyright 2014 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
//  Copyright 2014 UT-Battelle, LLC.
//  Copyright 2014 Los Alamos National Security.
//
//  Under the terms of Contract DE-NA0003525 with NTESS,
//  the U.S. Government retains certain rights in this software.
//
//  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
//  Laboratory (LANL), the U.S. Government retains certain rights in
//  this software.
//=========================================================================

#include <viskores/filter/mesh_info/MeshQualityVolume.h>

#include <viskores/worklet/WorkletMapTopology.h>

#include <viskores/cont/Algorithm.h>

#include <viskores/ErrorCode.h>

#include <viskores/exec/CellMeasure.h>

namespace
{

struct VolumeWorklet : viskores::worklet::WorkletVisitCellsWithPoints
{
  using ControlSignature = void(CellSetIn cellset,
                                FieldInPoint pointCoords,
                                FieldOutCell metricOut);
  using ExecutionSignature = void(CellShape, PointCount, _2, _3);

  template <typename CellShapeType, typename PointCoordVecType, typename OutType>
  VISKORES_EXEC void operator()(CellShapeType shape,
                                const viskores::IdComponent& numPoints,
                                const PointCoordVecType& pts,
                                OutType& metricValue) const
  {
    viskores::UInt8 thisId = shape.Id;
    switch (thisId)
    {
      viskoresGenericCellShapeMacro(metricValue =
                                      this->ComputeMetric<OutType>(numPoints, pts, CellShapeTag()));
      default:
        this->RaiseError(viskores::ErrorString(viskores::ErrorCode::InvalidShapeId));
        metricValue = OutType(0.0);
    }
  }

private:
  template <typename OutType, typename PointCoordVecType, typename CellShapeType>
  VISKORES_EXEC OutType ComputeMetric(const viskores::IdComponent& numPts,
                                      const PointCoordVecType& pts,
                                      CellShapeType tag) const
  {
    constexpr viskores::IdComponent dims =
      viskores::CellTraits<CellShapeType>::TOPOLOGICAL_DIMENSIONS;
    viskores::ErrorCode errorCode{ viskores::ErrorCode::Success };

    if (dims == 3)
    {
      OutType outValue = viskores::exec::CellMeasure<OutType>(numPts, pts, tag, errorCode);
      if (errorCode != viskores::ErrorCode::Success)
      {
        this->RaiseError(viskores::ErrorString(errorCode));
      }
      return outValue;
    }
    else
    {
      return 0;
    }
  }
};

} // anonymous namespace

namespace viskores
{
namespace filter
{
namespace mesh_info
{

MeshQualityVolume::MeshQualityVolume()
{
  this->SetUseCoordinateSystemAsField(true);
  this->SetOutputFieldName("volume");
}

viskores::Float64 MeshQualityVolume::ComputeTotalVolume(const viskores::cont::DataSet& input)
{
  viskores::cont::Field volumeField;
  if (input.HasCellField(this->GetOutputFieldName()))
  {
    volumeField = input.GetCellField(this->GetOutputFieldName());
  }
  else
  {
    viskores::cont::DataSet volumeData = this->Execute(input);
    volumeField = volumeData.GetCellField(this->GetOutputFieldName());
  }

  viskores::Float64 totalVolume = 0;
  auto resolveType = [&](const auto& concrete)
  { totalVolume = viskores::cont::Algorithm::Reduce(concrete, viskores::Float64{ 0 }); };
  this->CastAndCallScalarField(volumeField, resolveType);
  return totalVolume;
}

viskores::Float64 MeshQualityVolume::ComputeAverageVolume(const viskores::cont::DataSet& input)
{
  viskores::Id numCells = input.GetNumberOfCells();
  if (numCells > 0)
  {
    return this->ComputeTotalVolume(input) / static_cast<viskores::Float64>(numCells);
  }
  else
  {
    return 1;
  }
}

viskores::cont::DataSet MeshQualityVolume::DoExecute(const viskores::cont::DataSet& input)
{
  const auto& field = this->GetFieldFromDataSet(input);
  if (!field.IsPointField())
  {
    throw viskores::cont::ErrorBadValue("Active field for MeshQuality must be point coordinates. "
                                        "But the active field is not a point field.");
  }

  viskores::cont::UnknownArrayHandle outArray;

  auto resolveType = [&](const auto& concrete)
  {
    using T = typename std::decay_t<decltype(concrete)>::ValueType::ComponentType;
    viskores::cont::ArrayHandle<T> result;
    this->Invoke(VolumeWorklet{}, input.GetCellSet(), concrete, result);
    outArray = result;
  };
  this->CastAndCallVecField<3>(field, resolveType);

  return this->CreateResultFieldCell(input, this->GetOutputFieldName(), outArray);
}

} // namespace mesh_info
} // namespace filter
} // namespace viskores
