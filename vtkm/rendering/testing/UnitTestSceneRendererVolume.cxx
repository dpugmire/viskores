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
#include <vtkm/cont/testing/MakeTestDataSet.h>
#include <vtkm/rendering/Window.h>
#include <vtkm/rendering/RenderSurface.h>
#include <vtkm/rendering/WorldAnnotator.h>
#include <vtkm/rendering/Scene.h>
#include <vtkm/rendering/Plot.h>
#include <vtkm/rendering/SceneRendererVolume.h>
#include <vtkm/rendering/RenderSurfaceRayTracer.h>
#include <vtkm/cont/DeviceAdapter.h>
#include <vtkm/cont/testing/Testing.h>

namespace {

void Set3DView(vtkm::rendering::View &view,
               const vtkm::cont::CoordinateSystem &coords,
               vtkm::Int32 w, vtkm::Int32 h)
{
    vtkm::Float64 coordsBounds[6]; // Xmin,Xmax,Ymin..
    coords.GetBounds(coordsBounds,VTKM_DEFAULT_DEVICE_ADAPTER_TAG());
    //set up a default view  
    vtkm::Vec<vtkm::Float32,3> totalExtent; 
    totalExtent[0] = vtkm::Float32(coordsBounds[1] - coordsBounds[0]); 
    totalExtent[1] = vtkm::Float32(coordsBounds[3] - coordsBounds[2]);
    totalExtent[2] = vtkm::Float32(coordsBounds[5] - coordsBounds[4]);
    vtkm::Float32 mag = vtkm::Magnitude(totalExtent);
    vtkm::Normalize(totalExtent);

    view = vtkm::rendering::View(vtkm::rendering::View::VIEW_3D);
    view.view3d.pos = totalExtent * (mag * 2.f);
    view.view3d.up = vtkm::Vec<vtkm::Float32,3>(0.f, 1.f, 0.f);
    view.view3d.lookAt = totalExtent * (mag * .5f);
    view.view3d.fieldOfView = 60.f;
    view.nearPlane = 1.f;
    view.farPlane = 100.f;
    view.width = w;
    view.height = h;
    
    std::cout<<"View3d:  pos: "<<view.view3d.pos<<std::endl;
    std::cout<<"      lookAt: "<<view.view3d.lookAt<<std::endl;
    std::cout<<"          up: "<<view.view3d.up<<std::endl;
    std::cout<<"near/far/fov: "<<view.nearPlane<<"/"<<view.farPlane<<" "<<view.view3d.fieldOfView<<std::endl;
    std::cout<<"         w/h: "<<view.width<<"/"<<view.height<<std::endl;
}    

void Render(const vtkm::cont::DataSet &ds,
            const std::string &fieldNm,
            const std::string &ctName,
            const std::string &outputFile)    
{
    const vtkm::Int32 W = 512, H = 512;
    const vtkm::cont::CoordinateSystem coords = ds.GetCoordinateSystem();
    vtkm::rendering::SceneRendererVolume<VTKM_DEFAULT_DEVICE_ADAPTER_TAG> sceneRenderer;
    
    vtkm::rendering::View view;
    Set3DView(view, coords, W, H);
  
    vtkm::rendering::ColorTable colorTable(ctName);
    colorTable.AddAlphaControlPoint(0.0f, .01f);
    colorTable.AddAlphaControlPoint(1.0f, .01f);
    
    vtkm::rendering::Scene3D scene;
    vtkm::rendering::Color bg(0.2f, 0.2f, 0.2f, 1.0f);
    vtkm::rendering::RenderSurfaceRayTracer surface(W,H,bg);
    scene.plots.push_back(vtkm::rendering::Plot(ds.GetCellSet(),
                                                ds.GetCoordinateSystem(),
                                                ds.GetField(fieldNm),
                                                colorTable));

    //TODO: W/H in window.  bg in window (window sets surface/renderer).
    vtkm::rendering::Window3D<vtkm::rendering::SceneRendererVolume<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>,
                              vtkm::rendering::RenderSurfaceRayTracer,
                              vtkm::rendering::WorldAnnotator>
        w(scene, sceneRenderer, surface, view, bg);
    
    w.Initialize();
    w.Paint();
    w.SaveAs(outputFile);
    
}

void RenderTests()
{
    vtkm::cont::testing::MakeTestDataSet maker;

    //3D tests.
    Render(maker.Make3DRegularDataSet0(),
             "pointvar", "thermal", "reg3D.pnm");
    Render(maker.Make3DRectilinearDataSet0(),
             "pointvar", "thermal", "rect3D.pnm");
}

} //namespace
int UnitTestSceneRendererVolume(int, char *[])
{
    return vtkm::cont::testing::Testing::Run(RenderTests);
}
