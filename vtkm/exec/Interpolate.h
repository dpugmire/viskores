//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//  Copyright 2015 Sandia Corporation.
//  Copyright 2015 UT-Battelle, LLC.
//  Copyright 2015 Los Alamos National Security.
//
//  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
//  the U.S. Government retains certain rights in this software.
//
//  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
//  Laboratory (LANL), the U.S. Government retains certain rights in
//  this software.
//============================================================================
#ifndef vtk_m_exec_Interpolate_h
#define vtk_m_exec_Interpolate_h

#include <vtkm/CellShape.h>
#include <vtkm/Math.h>
#include <vtkm/VectorAnalysis.h>
#include <vtkm/exec/Assert.h>

namespace vtkm {
namespace exec {

/// \brief Interpolate a point field in a cell.
///
/// Given the point field values for each node and the parametric coordinates
/// of a point within the cell, interpolates the field to that point.
///
template<typename FieldVecType,
         typename ParametricCoordType,
         typename WorkletType>
VTKM_EXEC_EXPORT
typename FieldVecType::ComponentType
CellInterpolate(const FieldVecType &pointFieldValues,
                const vtkm::Vec<ParametricCoordType,3> parametricCoords,
                vtkm::CellShapeTagGeneric shape,
                const WorkletType &worklet)
{
  switch (shape.Id)
  {
    vtkmGenericCellShapeMacro(
          return CellInterpolate(pointFieldValues,
                                 parametricCoords,
                                 CellShapeTag(),
                                 worklet));
  default:
    worklet.RaiseError("Unknown cell shape sent to interpolate.");
  }
}

//-----------------------------------------------------------------------------
template<typename FieldVecType,
         typename ParametricCoordType,
         typename WorkletType>
VTKM_EXEC_EXPORT
typename FieldVecType::ComponentType
CellInterpolate(const FieldVecType &,
                const vtkm::Vec<ParametricCoordType,3> ,
                vtkm::CellShapeTagEmpty,
                const WorkletType &worklet)
{
  worklet.RaiseError("Attempted to interpolate an empty cell.");
  return vtkm::Nan<typename FieldVecType::ComponentType>();
}

//-----------------------------------------------------------------------------
template<typename FieldVecType,
         typename ParametricCoordType,
         typename WorkletType>
VTKM_EXEC_EXPORT
typename FieldVecType::ComponentType
CellInterpolate(const FieldVecType &pointFieldValues,
                const vtkm::Vec<ParametricCoordType,3>,
                vtkm::CellShapeTagVertex,
                const WorkletType &worklet)
{
  VTKM_ASSERT_EXEC(pointFieldValues.GetNumberOfValues() == 1, worklet);
  return pointFieldValues[0];
}

//-----------------------------------------------------------------------------
template<typename FieldVecType,
         typename ParametricCoordType,
         typename WorkletType>
VTKM_EXEC_EXPORT
typename FieldVecType::ComponentType
CellInterpolate(const FieldVecType &pointFieldValues,
                const vtkm::Vec<ParametricCoordType,3> parametricCoords,
                vtkm::CellShapeTagLine,
                const WorkletType &worklet)
{
  VTKM_ASSERT_EXEC(pointFieldValues.GetNumberOfValues() == 2, worklet);
  return vtkm::Lerp(pointFieldValues[0],
                    pointFieldValues[1],
                    parametricCoords[0]);
}

//-----------------------------------------------------------------------------
template<typename FieldVecType,
         typename ParametricCoordType,
         typename WorkletType>
VTKM_EXEC_EXPORT
typename FieldVecType::ComponentType
CellInterpolate(const FieldVecType &field,
                const vtkm::Vec<ParametricCoordType,3> pcoords,
                vtkm::CellShapeTagTriangle,
                const WorkletType &worklet)
{
  VTKM_ASSERT_EXEC(field.GetNumberOfValues() == 3, worklet);
  typedef typename FieldVecType::ComponentType T;
  T result;
  result  = static_cast<T>(field[0] * (1 - pcoords[0] - pcoords[1]));
  result += static_cast<T>(field[1] * pcoords[0]);
  result += static_cast<T>(field[2] * pcoords[1]);
  return result;
}

//-----------------------------------------------------------------------------
template<typename FieldVecType,
         typename ParametricCoordType,
         typename WorkletType>
VTKM_EXEC_EXPORT
typename FieldVecType::ComponentType
CellInterpolate(const FieldVecType &field,
                const vtkm::Vec<ParametricCoordType,3> pcoords,
                vtkm::CellShapeTagPolygon,
                const WorkletType &worklet)
{
  vtkm::IdComponent numPoints = field.GetNumberOfValues();
  VTKM_ASSERT_EXEC(numPoints > 0, worklet);
  switch (numPoints)
  {
    case 1:
      return CellInterpolate(field,pcoords,vtkm::CellShapeTagVertex(),worklet);
    case 2:
      return CellInterpolate(field,pcoords,vtkm::CellShapeTagLine(),worklet);
    case 3:
      return CellInterpolate(field,pcoords,vtkm::CellShapeTagTriangle(),worklet);
    case 4:
      return CellInterpolate(field,pcoords,vtkm::CellShapeTagQuad(),worklet);
  }

  // If we are here, then there are 5 or more points on this polygon.

  // Arrange the points such that they are on the circle circumscribed in the
  // unit square from 0 to 1. That is, the point are on the circle centered
  // at coordinate 0.5,0.5 with radius 0.5.

  // We only care about the first two parametric coordinates.
  vtkm::Vec<ParametricCoordType,2> pcoords2(pcoords[0],pcoords[1]);

  typedef typename FieldVecType::ComponentType FieldType;

  ParametricCoordType angle = 0;
  const ParametricCoordType deltaAngle =
      static_cast<ParametricCoordType>(2*vtkm::Pi()/numPoints);
  const ParametricCoordType epsilon = 8*vtkm::Epsilon<ParametricCoordType>();
  FieldType weightedSum(0);
  ParametricCoordType totalWeight = 0;

  for (vtkm::IdComponent nodeIndex = 0; nodeIndex < numPoints; nodeIndex++)
  {
    vtkm::Vec<ParametricCoordType,2> nodePCoords(0.5*(vtkm::Cos(angle)+1),
                                                 0.5*(vtkm::Sin(angle)+1));
    ParametricCoordType distanceSqr =
        vtkm::MagnitudeSquared(pcoords2-nodePCoords);
    if (distanceSqr < epsilon) { return field[nodeIndex]; }

    ParametricCoordType weight = vtkm::RSqrt(distanceSqr);
    weightedSum = weightedSum + FieldType(weight)*field[nodeIndex];
    totalWeight += weight;

    angle += deltaAngle;
  }

  return weightedSum * FieldType(1/totalWeight);
}

//-----------------------------------------------------------------------------
template<typename FieldVecType,
         typename ParametricCoordType,
         typename WorkletType>
VTKM_EXEC_EXPORT
typename FieldVecType::ComponentType
CellInterpolate(const FieldVecType &field,
                const vtkm::Vec<ParametricCoordType,3> pcoords,
                vtkm::CellShapeTagPixel,
                const WorkletType &worklet)
{
  VTKM_ASSERT_EXEC(field.GetNumberOfValues() == 4, worklet);

  typedef typename FieldVecType::ComponentType T;

  T bottomInterp = vtkm::Lerp(field[0], field[1], pcoords[0]);
  T topInterp = vtkm::Lerp(field[2], field[3], pcoords[0]);

  return vtkm::Lerp(bottomInterp, topInterp, pcoords[1]);
}

//-----------------------------------------------------------------------------
template<typename FieldVecType,
         typename ParametricCoordType,
         typename WorkletType>
VTKM_EXEC_EXPORT
typename FieldVecType::ComponentType
CellInterpolate(const FieldVecType &field,
                const vtkm::Vec<ParametricCoordType,3> pcoords,
                vtkm::CellShapeTagQuad,
                const WorkletType &worklet)
{
  VTKM_ASSERT_EXEC(field.GetNumberOfValues() == 4, worklet);

  typedef typename FieldVecType::ComponentType T;

  T bottomInterp = vtkm::Lerp(field[0], field[1], pcoords[0]);
  T topInterp = vtkm::Lerp(field[3], field[2], pcoords[0]);

  return vtkm::Lerp(bottomInterp, topInterp, pcoords[1]);
}

//-----------------------------------------------------------------------------
template<typename FieldVecType,
         typename ParametricCoordType,
         typename WorkletType>
VTKM_EXEC_EXPORT
typename FieldVecType::ComponentType
CellInterpolate(const FieldVecType &field,
                const vtkm::Vec<ParametricCoordType,3> pcoords,
                vtkm::CellShapeTagTetra,
                const WorkletType &worklet)
{
  VTKM_ASSERT_EXEC(field.GetNumberOfValues() == 4, worklet);
  typedef typename FieldVecType::ComponentType T;
  T result;
  result  = static_cast<T>(field[0] * (1-pcoords[0]-pcoords[1]-pcoords[2]));
  result += static_cast<T>(field[1] * pcoords[0]);
  result += static_cast<T>(field[2] * pcoords[1]);
  result += static_cast<T>(field[3] * pcoords[2]);
  return result;
}

//-----------------------------------------------------------------------------
template<typename FieldVecType,
         typename ParametricCoordType,
         typename WorkletType>
VTKM_EXEC_EXPORT
typename FieldVecType::ComponentType
CellInterpolate(const FieldVecType &field,
                const vtkm::Vec<ParametricCoordType,3> pcoords,
                vtkm::CellShapeTagVoxel,
                const WorkletType &worklet)
{
  VTKM_ASSERT_EXEC(field.GetNumberOfValues() == 8, worklet);

  typedef typename FieldVecType::ComponentType T;

  T bottomFrontInterp = vtkm::Lerp(field[0], field[1], pcoords[0]);
  T bottomBackInterp = vtkm::Lerp(field[2], field[3], pcoords[0]);
  T topFrontInterp = vtkm::Lerp(field[4], field[5], pcoords[0]);
  T topBackInterp = vtkm::Lerp(field[6], field[7], pcoords[0]);

  T bottomInterp = vtkm::Lerp(bottomFrontInterp, bottomBackInterp, pcoords[1]);
  T topInterp = vtkm::Lerp(topFrontInterp, topBackInterp, pcoords[1]);

  return vtkm::Lerp(bottomInterp, topInterp, pcoords[2]);
}

//-----------------------------------------------------------------------------
template<typename FieldVecType,
         typename ParametricCoordType,
         typename WorkletType>
VTKM_EXEC_EXPORT
typename FieldVecType::ComponentType
CellInterpolate(const FieldVecType &field,
                const vtkm::Vec<ParametricCoordType,3> pcoords,
                vtkm::CellShapeTagHexahedron,
                const WorkletType &worklet)
{
  VTKM_ASSERT_EXEC(field.GetNumberOfValues() == 8, worklet);

  typedef typename FieldVecType::ComponentType T;

  T bottomFrontInterp = vtkm::Lerp(field[0], field[1], pcoords[0]);
  T bottomBackInterp = vtkm::Lerp(field[3], field[2], pcoords[0]);
  T topFrontInterp = vtkm::Lerp(field[4], field[5], pcoords[0]);
  T topBackInterp = vtkm::Lerp(field[7], field[6], pcoords[0]);

  T bottomInterp = vtkm::Lerp(bottomFrontInterp, bottomBackInterp, pcoords[1]);
  T topInterp = vtkm::Lerp(topFrontInterp, topBackInterp, pcoords[1]);

  return vtkm::Lerp(bottomInterp, topInterp, pcoords[2]);
}

//-----------------------------------------------------------------------------
template<typename FieldVecType,
         typename ParametricCoordType,
         typename WorkletType>
VTKM_EXEC_EXPORT
typename FieldVecType::ComponentType
CellInterpolate(const FieldVecType &field,
                const vtkm::Vec<ParametricCoordType,3> pcoords,
                vtkm::CellShapeTagWedge,
                const WorkletType &worklet)
{
  VTKM_ASSERT_EXEC(field.GetNumberOfValues() == 6, worklet);

  typedef typename FieldVecType::ComponentType T;

  T bottomInterp;
  bottomInterp  = static_cast<T>(field[0] * (1 - pcoords[0] - pcoords[1]));
  bottomInterp += static_cast<T>(field[1] * pcoords[1]);
  bottomInterp += static_cast<T>(field[2] * pcoords[0]);

  T topInterp;
  topInterp  = static_cast<T>(field[3] * (1 - pcoords[0] - pcoords[1]));
  topInterp += static_cast<T>(field[4] * pcoords[1]);
  topInterp += static_cast<T>(field[5] * pcoords[0]);

  return vtkm::Lerp(bottomInterp, topInterp, pcoords[2]);
}

//-----------------------------------------------------------------------------
template<typename FieldVecType,
         typename ParametricCoordType,
         typename WorkletType>
VTKM_EXEC_EXPORT
typename FieldVecType::ComponentType
CellInterpolate(const FieldVecType &field,
                const vtkm::Vec<ParametricCoordType,3> pcoords,
                vtkm::CellShapeTagPyramid,
                const WorkletType &worklet)
{
  VTKM_ASSERT_EXEC(field.GetNumberOfValues() == 5, worklet);

  typedef typename FieldVecType::ComponentType T;

  T frontInterp = vtkm::Lerp(field[0], field[1], pcoords[0]);
  T backInterp = vtkm::Lerp(field[3], field[2], pcoords[0]);

  T baseInterp = vtkm::Lerp(frontInterp, backInterp, pcoords[1]);

  return vtkm::Lerp(baseInterp, field[4], pcoords[2]);
}

}
} // namespace vtkm::exec

#endif //vtk_m_exec_Interpolate_h
