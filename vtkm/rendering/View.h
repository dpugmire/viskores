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
#ifndef vtk_m_rendering_View_h
#define vtk_m_rendering_View_h

#include <vtkm/rendering/vtkm_rendering_export.h>

#include <vtkm/rendering/Camera.h>
#include <vtkm/rendering/Canvas.h>
#include <vtkm/rendering/Color.h>
#include <vtkm/rendering/Mapper.h>
#include <vtkm/rendering/Scene.h>

namespace vtkm {
namespace rendering {

class View
{
public:
  VTKM_RENDERING_EXPORT
  View(const vtkm::rendering::Scene &scene,
       const vtkm::rendering::Mapper &mapper,
       const vtkm::rendering::Canvas &canvas,
       const vtkm::rendering::Color &backgroundColor =
         vtkm::rendering::Color(0,0,0,1));

  VTKM_RENDERING_EXPORT
  View(const vtkm::rendering::Scene &scene,
       const vtkm::rendering::Mapper &mapper,
       const vtkm::rendering::Canvas &canvas,
       const vtkm::rendering::Camera &camera,
       const vtkm::rendering::Color &backgroundColor =
         vtkm::rendering::Color(0,0,0,1));

  VTKM_RENDERING_EXPORT
  virtual ~View();

  VTKM_CONT_EXPORT
  const vtkm::rendering::Scene &GetScene() const { return this->Scene; }
  VTKM_CONT_EXPORT
  vtkm::rendering::Scene &GetScene() { return this->Scene; }
  VTKM_CONT_EXPORT
  void SetScene(const vtkm::rendering::Scene &scene) { this->Scene = scene; }

  VTKM_CONT_EXPORT
  const vtkm::rendering::Mapper &GetMapper() const
  {
    return *this->MapperPointer;
  }
  VTKM_CONT_EXPORT
  vtkm::rendering::Mapper &GetMapper()
  {
    return *this->MapperPointer;
  }

  VTKM_CONT_EXPORT
  const vtkm::rendering::Canvas &GetCanvas() const
  {
    return *this->CanvasPointer;
  }
  VTKM_CONT_EXPORT
  vtkm::rendering::Canvas &GetCanvas()
  {
    return *this->CanvasPointer;
  }

  VTKM_CONT_EXPORT
  const vtkm::rendering::WorldAnnotator &GetWorldAnnotator() const
  {
    return *this->WorldAnnotatorPointer;
  }

  VTKM_CONT_EXPORT
  const vtkm::rendering::Camera &GetCamera() const
  {
    return this->Camera;
  }
  VTKM_CONT_EXPORT
  vtkm::rendering::Camera &GetCamera()
  {
    return this->Camera;
  }
  VTKM_CONT_EXPORT
  void SetCamera(const vtkm::rendering::Camera &camera)
  {
    this->Camera = camera;
  }

  VTKM_CONT_EXPORT
  const vtkm::rendering::Color &GetBackgroundColor() const
  {
    return this->CanvasPointer->GetBackgroundColor();
  }

  VTKM_CONT_EXPORT
  void SetBackgroundColor(const vtkm::rendering::Color &color)
  {
    this->CanvasPointer->SetBackgroundColor(color);
  }

  VTKM_RENDERING_EXPORT
  virtual void Initialize();

  VTKM_RENDERING_EXPORT
  virtual void Paint() = 0;
  VTKM_RENDERING_EXPORT
  virtual void RenderScreenAnnotations() = 0;
  VTKM_RENDERING_EXPORT
  virtual void RenderWorldAnnotations() = 0;

  VTKM_RENDERING_EXPORT
  void SaveAs(const std::string &fileName) const;

protected:
  VTKM_RENDERING_EXPORT
  void SetupForWorldSpace(bool viewportClip=true);

  VTKM_RENDERING_EXPORT
  void SetupForScreenSpace(bool viewportClip=false);

private:
  vtkm::rendering::Scene Scene;
  boost::shared_ptr<vtkm::rendering::Mapper> MapperPointer;
  boost::shared_ptr<vtkm::rendering::Canvas> CanvasPointer;
  boost::shared_ptr<vtkm::rendering::WorldAnnotator> WorldAnnotatorPointer;
  vtkm::rendering::Camera Camera;
};

}
} //namespace vtkm::rendering

#endif //vtk_m_rendering_View_h
