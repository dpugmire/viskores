//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//============================================================================
#ifndef viskores_cont_CubicHermiteSpline_h
#define viskores_cont_CubicHermiteSpline_h

#include <viskores/cont/viskores_cont_export.h>
#include <viskores/Range.h>
#include <viskores/Types.h>
#include <viskores/cont/ArrayHandle.h>
#include <viskores/cont/ExecutionObjectBase.h>
#include <viskores/cont/UnknownArrayHandle.h>
#include <viskores/exec/CubicHermiteSpline.h>

namespace viskores
{
namespace cont
{

class VISKORES_CONT_EXPORT CubicHermiteSpline : public viskores::cont::ExecutionObjectBase
{
public:
  CubicHermiteSpline() = default;
  virtual ~CubicHermiteSpline() = default;

  VISKORES_CONT void SetData(const viskores::cont::ArrayHandle<viskores::Vec3f>& data) { this->Data = data; }
  VISKORES_CONT void SetData(const std::vector<viskores::Vec3f>& data,
                         viskores::CopyFlag copy = viskores::CopyFlag::On)
  {
    this->Data = viskores::cont::make_ArrayHandle(data, copy);
  }

  VISKORES_CONT void SetKnots(const viskores::cont::ArrayHandle<viskores::FloatDefault>& knots)
  {
    this->Knots = knots;
  }
  VISKORES_CONT void SetKnots(const std::vector<viskores::FloatDefault>& knots,
                          viskores::CopyFlag copy = viskores::CopyFlag::On)
  {
    this->Knots = viskores::cont::make_ArrayHandle(knots, copy);
  }

  VISKORES_CONT void SetTangents(const viskores::cont::ArrayHandle<viskores::Vec3f>& tangents)
  {
    this->Tangents = tangents;
  }
  VISKORES_CONT void SetTangents(const std::vector<viskores::Vec3f>& tangents,
                             viskores::CopyFlag copy = viskores::CopyFlag::On)
  {
    this->Tangents = viskores::cont::make_ArrayHandle(tangents, copy);
  }

  VISKORES_CONT viskores::Range GetParametricRange();

  VISKORES_CONT const viskores::cont::ArrayHandle<viskores::Vec3f>& GetData() const { return this->Data; }
  VISKORES_CONT const viskores::cont::ArrayHandle<viskores::Vec3f>& GetTangents()
  {
    if (this->Tangents.GetNumberOfValues() == 0)
      this->ComputeTangents();
    return this->Tangents;
  }
  VISKORES_CONT const viskores::cont::ArrayHandle<viskores::FloatDefault>& GetKnots()
  {
    if (this->Knots.GetNumberOfValues() == 0)
      this->ComputeKnots();
    return this->Knots;
  }

  VISKORES_CONT viskores::exec::CubicHermiteSpline PrepareForExecution(viskores::cont::DeviceAdapterId device,
                                                               viskores::cont::Token& token);

private:
  VISKORES_CONT void ComputeKnots();
  VISKORES_CONT void ComputeTangents();

  viskores::cont::ArrayHandle<viskores::Vec3f> Data;
  viskores::cont::ArrayHandle<viskores::FloatDefault> Knots;
  viskores::cont::ArrayHandle<viskores::Vec3f> Tangents;
};
}
} // viskores::cont

#endif //viskores_cont_CubicHermiteSpline_h
