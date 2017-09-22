//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//  Copyright 2016 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
//  Copyright 2016 UT-Battelle, LLC.
//  Copyright 2016 Los Alamos National Security.
//
//  Under the terms of Contract DE-NA0003525 with NTESS,
//  the U.S. Government retains certain rights in this software.
//
//  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
//  Laboratory (LANL), the U.S. Government retains certain rights in
//  this software.
//============================================================================
#ifndef vtk_m_rendering_WorldAnnotator_h
#define vtk_m_rendering_WorldAnnotator_h

#include <vtkm/rendering/vtkm_rendering_export.h>

#include <vtkm/Types.h>
#include <vtkm/rendering/Canvas.h>
#include <vtkm/rendering/Color.h>

namespace vtkm
{
namespace rendering
{

class Canvas;

class VTKM_RENDERING_EXPORT WorldAnnotator
{
public:
  WorldAnnotator(const vtkm::rendering::Canvas* canvas);

  virtual ~WorldAnnotator();

  virtual void AddLine(const vtkm::Vec<vtkm::Float64, 3>& point0,
                       const vtkm::Vec<vtkm::Float64, 3>& point1,
                       vtkm::Float32 lineWidth,
                       const vtkm::rendering::Color& color,
                       bool inFront = false) const;

  VTKM_CONT
  void AddLine(vtkm::Float64 x0,
               vtkm::Float64 y0,
               vtkm::Float64 z0,
               vtkm::Float64 x1,
               vtkm::Float64 y1,
               vtkm::Float64 z1,
               vtkm::Float32 lineWidth,
               const vtkm::rendering::Color& color,
               bool inFront = false) const
  {
    this->AddLine(
      vtkm::make_Vec(x0, y0, z0), vtkm::make_Vec(x1, y1, z1), lineWidth, color, inFront);
  }

  virtual void AddText(const vtkm::Vec<vtkm::Float32, 3>& origin,
                       const vtkm::Vec<vtkm::Float32, 3>& right,
                       const vtkm::Vec<vtkm::Float32, 3>& up,
                       vtkm::Float32 scale,
                       const vtkm::Vec<vtkm::Float32, 2>& anchor,
                       const vtkm::rendering::Color& color,
                       const std::string& text) const;

  VTKM_CONT
  void AddText(vtkm::Float32 originX,
               vtkm::Float32 originY,
               vtkm::Float32 originZ,
               vtkm::Float32 rightX,
               vtkm::Float32 rightY,
               vtkm::Float32 rightZ,
               vtkm::Float32 upX,
               vtkm::Float32 upY,
               vtkm::Float32 upZ,
               vtkm::Float32 scale,
               vtkm::Float32 anchorX,
               vtkm::Float32 anchorY,
               const vtkm::rendering::Color& color,
               const std::string& text) const
  {
    this->AddText(vtkm::make_Vec(originX, originY, originZ),
                  vtkm::make_Vec(rightX, rightY, rightZ),
                  vtkm::make_Vec(upX, upY, upZ),
                  scale,
                  vtkm::make_Vec(anchorX, anchorY),
                  color,
                  text);
  }

private:
  const vtkm::rendering::Canvas* Canvas;
};
}
} //namespace vtkm::rendering

#endif // vtk_m_rendering_WorldAnnotator_h
