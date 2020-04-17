//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//============================================================================
#ifndef vtk_m_rendering_EncodePNG_h
#define vtk_m_rendering_EncodePNG_h

#include <vtkm/Types.h>
#include <vtkm/rendering/vtkm_rendering_export.h>

#include <vector>

namespace vtkm
{
namespace rendering
{

//
VTKM_RENDERING_EXPORT
vtkm::UInt32 EncodePNG(std::vector<unsigned char> const& image,
                       unsigned long width,
                       unsigned long height,
                       unsigned char* out_png,
                       std::size_t out_size);

VTKM_RENDERING_EXPORT
vtkm::UInt32 SavePNG(std::string filename,
                     std::vector<unsigned char> const& image,
                     unsigned long width,
                     unsigned long height);
}
} // vtkm::rendering

#endif //vtk_m_rendering_EncodePNG_h
