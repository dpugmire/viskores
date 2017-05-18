//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//  Copyright 2016 Sandia Corporation.
//  Copyright 2016 UT-Battelle, LLC.
//  Copyright 2016 Los Alamos National Security.
//
//  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
//  the U.S. Government retains certain rights in this software.
//
//  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
//  Laboratory (LANL), the U.S. Government retains certain rights in
//  this software.
//============================================================================
#ifndef vtk_m_rendering_BoundingBoxAnnotation_h
#define vtk_m_rendering_BoundingBoxAnnotation_h

#include <vtkm/Bounds.h>
#include <vtkm/rendering/Camera.h>
#include <vtkm/rendering/Color.h>
#include <vtkm/rendering/WorldAnnotator.h>

namespace vtkm
{
namespace rendering
{

class VTKM_RENDERING_EXPORT BoundingBoxAnnotation
{
private:
  vtkm::rendering::Color Color;
  vtkm::Bounds Extents;

public:
  BoundingBoxAnnotation();

  virtual ~BoundingBoxAnnotation();

  VTKM_CONT
  const vtkm::Bounds& GetExtents() const { return this->Extents; }

  VTKM_CONT
  void SetExtents(const vtkm::Bounds& extents) { this->Extents = extents; }

  VTKM_CONT
  const vtkm::rendering::Color& GetColor() const { return this->Color; }

  VTKM_CONT
  void SetColor(vtkm::rendering::Color c) { this->Color = c; }

  virtual void Render(const vtkm::rendering::Camera&, const WorldAnnotator& annotator);
};
}
} //namespace vtkm::rendering

#endif // vtk_m_rendering_BoundingBoxAnnotation_h
