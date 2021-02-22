//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//============================================================================
#ifndef vtk_m_ImplicitFunction_h
#define vtk_m_ImplicitFunction_h

#include <vtkm/Bounds.h>
#include <vtkm/Deprecated.h>
#include <vtkm/Math.h>
#include <vtkm/VectorAnalysis.h>

#include <vtkm/exec/internal/Variant.h>

// For interface class only.
#include <vtkm/cont/ExecutionAndControlObjectBase.h>

#ifndef VTKM_NO_DEPRECATED_VIRTUAL
#include <vtkm/VirtualObjectBase.h>
#endif // VTKM_NO_DEPRECATED_VIRTUAL

namespace vtkm
{

//============================================================================
#ifndef VTKM_NO_DEPRECATED_VIRTUAL
class VTKM_DEPRECATED(1.6, "ImplicitFunction with virtual methods no longer supported.")
  VTKM_ALWAYS_EXPORT ImplicitFunction : public vtkm::VirtualObjectBase
{
public:
  using Scalar = vtkm::FloatDefault;
  using Vector = vtkm::Vec<Scalar, 3>;

  VTKM_EXEC_CONT virtual Scalar Value(const Vector& point) const = 0;
  VTKM_EXEC_CONT virtual Vector Gradient(const Vector& point) const = 0;

  VTKM_DEPRECATED_SUPPRESS_BEGIN
  VTKM_EXEC_CONT Scalar Value(Scalar x, Scalar y, Scalar z) const
  {
    return this->Value(Vector(x, y, z));
  }

  VTKM_EXEC_CONT Vector Gradient(Scalar x, Scalar y, Scalar z) const
  {
    return this->Gradient(Vector(x, y, z));
  }
  VTKM_DEPRECATED_SUPPRESS_END
};
#endif // VTKM_NO_DEPRECATED_VIRTUAL

//============================================================================
namespace internal
{

/// \brief Base class for all `ImplicitFunction` classes.
///
/// `ImplicitFunctionBase` uses the curiously recurring template pattern (CRTP). Subclasses
/// must provide their own type for the template parameter. Subclasses must implement
/// `Value` and `Gradient` methods.
///
/// Also, all subclasses must be trivially copiable. This is so they can be copied among
/// host and devices.
///
template <typename Derived>
class ImplicitFunctionBase : public vtkm::cont::ExecutionAndControlObjectBase
{
public:
  using Scalar = vtkm::FloatDefault;
  using Vector = vtkm::Vec<Scalar, 3>;

  VTKM_EXEC_CONT Scalar Value(Scalar x, Scalar y, Scalar z) const
  {
    return reinterpret_cast<const Derived*>(this)->Value(Vector(x, y, z));
  }

  VTKM_EXEC_CONT Vector Gradient(Scalar x, Scalar y, Scalar z) const
  {
    return reinterpret_cast<const Derived*>(this)->Gradient(Vector(x, y, z));
  }

  VTKM_CONT const Derived& PrepareForExecution(vtkm::cont::DeviceAdapterId,
                                               vtkm::cont::Token&) const
  {
    return *reinterpret_cast<const Derived*>(this);
  }

  VTKM_CONT const Derived& PrepareForControl() const
  {
    return *reinterpret_cast<const Derived*>(this);
  }
};

} // namespace vtkm::internal

//============================================================================
#ifndef VTKM_NO_DEPRECATED_VIRTUAL
VTKM_DEPRECATED_SUPPRESS_BEGIN

/// A helpful functor that calls the (virtual) value method of a given ImplicitFunction. Can be
/// passed to things that expect a functor instead of an ImplictFunction class (like an array
/// transform).
///
class VTKM_DEPRECATED(1.6,
                      "Use ImplicitFunctionValueFunctor.") VTKM_ALWAYS_EXPORT ImplicitFunctionValue
{
public:
  using Scalar = vtkm::ImplicitFunction::Scalar;
  using Vector = vtkm::ImplicitFunction::Vector;

  VTKM_EXEC_CONT ImplicitFunctionValue()
    : Function(nullptr)
  {
  }

  VTKM_EXEC_CONT ImplicitFunctionValue(const ImplicitFunction* function)
    : Function(function)
  {
  }

  VTKM_EXEC_CONT Scalar operator()(const Vector& point) const
  {
    return this->Function->Value(point);
  }

private:
  const vtkm::ImplicitFunction* Function;
};

/// A helpful functor that calls the (virtual) gradient method of a given ImplicitFunction. Can be
/// passed to things that expect a functor instead of an ImplictFunction class (like an array
/// transform).
///
class VTKM_DEPRECATED(1.6, "Use ImplicitFunctionGradientFunctor.")
  VTKM_ALWAYS_EXPORT ImplicitFunctionGradient
{
public:
  using Scalar = vtkm::ImplicitFunction::Scalar;
  using Vector = vtkm::ImplicitFunction::Vector;

  VTKM_EXEC_CONT ImplicitFunctionGradient()
    : Function(nullptr)
  {
  }

  VTKM_EXEC_CONT ImplicitFunctionGradient(const ImplicitFunction* function)
    : Function(function)
  {
  }

  VTKM_EXEC_CONT Vector operator()(const Vector& point) const
  {
    return this->Function->Gradient(point);
  }

private:
  const vtkm::ImplicitFunction* Function;
};

VTKM_DEPRECATED_SUPPRESS_END
#endif // VTKM_NO_DEPRECATED_VIRTUAL

//============================================================================
/// A helpful functor that calls the value method of a given `ImplicitFunction`. Can be
/// passed to things that expect a functor instead of an `ImplictFunction` class (like an array
/// transform).
///
template <typename FunctionType>
class ImplicitFunctionValueFunctor
{
public:
  using Scalar = typename FunctionType::Scalar;
  using Vector = typename FunctionType::Vector;

  ImplicitFunctionValueFunctor() = default;

  VTKM_EXEC_CONT ImplicitFunctionValueFunctor(
    const vtkm::internal::ImplicitFunctionBase<FunctionType>& function)
    : Function(reinterpret_cast<const FunctionType&>(function))
  {
  }

  VTKM_EXEC_CONT ImplicitFunctionValueFunctor(const FunctionType& function)
    : Function(function)
  {
  }

  VTKM_EXEC_CONT Scalar operator()(const Vector& point) const
  {
    return this->Function.Value(point);
  }

private:
  FunctionType Function;
};

template <typename FunctionType>
vtkm::ImplicitFunctionValueFunctor<FunctionType> make_ImplicitFunctionValueFunctor(
  const vtkm::internal::ImplicitFunctionBase<FunctionType>& function)
{
  return ImplicitFunctionValueFunctor<FunctionType>(function);
}

/// A helpful functor that calls the gradient method of a given `ImplicitFunction`. Can be
/// passed to things that expect a functor instead of an `ImplictFunction` class (like an array
/// transform).
///
template <typename FunctionType>
class ImplicitFunctionGradientFunctor
{
public:
  using Scalar = typename FunctionType::Scalar;
  using Vector = typename FunctionType::Vector;

  ImplicitFunctionGradientFunctor() = default;

  VTKM_EXEC_CONT ImplicitFunctionGradientFunctor(
    const vtkm::internal::ImplicitFunctionBase<FunctionType>& function)
    : Function(reinterpret_cast<const FunctionType&>(function))
  {
  }

  VTKM_EXEC_CONT ImplicitFunctionGradientFunctor(const FunctionType& function)
    : Function(function)
  {
  }

  VTKM_EXEC_CONT Vector operator()(const Vector& point) const
  {
    return this->Function->Gradient(point);
  }

private:
  FunctionType Function;
};

template <typename FunctionType>
vtkm::ImplicitFunctionGradientFunctor<FunctionType> make_ImplicitFunctionGradientFunctor(
  const vtkm::internal::ImplicitFunctionBase<FunctionType>& function)
{
  return ImplicitFunctionGradientFunctor<FunctionType>(function);
}

//============================================================================
/// \brief Implicit function for a box
///
/// \c Box computes the implicit function and/or gradient for a axis-aligned
/// bounding box. Each side of the box is orthogonal to all other sides
/// meeting along shared edges and all faces are orthogonal to the x-y-z
/// coordinate axes.

class VTKM_ALWAYS_EXPORT Box : public internal::ImplicitFunctionBase<Box>
{
public:
  /// \brief Construct box with center at (0,0,0) and each side of length 1.0.
  VTKM_EXEC_CONT Box()
    : MinPoint(Vector(Scalar(-0.5)))
    , MaxPoint(Vector(Scalar(0.5)))
  {
  }

  VTKM_EXEC_CONT Box(const Vector& minPoint, const Vector& maxPoint)
    : MinPoint(minPoint)
    , MaxPoint(maxPoint)
  {
  }

  VTKM_EXEC_CONT Box(Scalar xmin, Scalar xmax, Scalar ymin, Scalar ymax, Scalar zmin, Scalar zmax)
    : MinPoint(xmin, ymin, zmin)
    , MaxPoint(xmax, ymax, zmax)
  {
  }

  VTKM_CONT Box(const vtkm::Bounds& bounds) { this->SetBounds(bounds); }

  VTKM_CONT void SetMinPoint(const Vector& point) { this->MinPoint = point; }

  VTKM_CONT void SetMaxPoint(const Vector& point) { this->MaxPoint = point; }

  VTKM_EXEC_CONT const Vector& GetMinPoint() const { return this->MinPoint; }

  VTKM_EXEC_CONT const Vector& GetMaxPoint() const { return this->MaxPoint; }

  VTKM_CONT void SetBounds(const vtkm::Bounds& bounds)
  {
    this->SetMinPoint({ Scalar(bounds.X.Min), Scalar(bounds.Y.Min), Scalar(bounds.Z.Min) });
    this->SetMaxPoint({ Scalar(bounds.X.Max), Scalar(bounds.Y.Max), Scalar(bounds.Z.Max) });
  }

  VTKM_EXEC_CONT vtkm::Bounds GetBounds() const
  {
    return vtkm::Bounds(vtkm::Range(this->MinPoint[0], this->MaxPoint[0]),
                        vtkm::Range(this->MinPoint[1], this->MaxPoint[1]),
                        vtkm::Range(this->MinPoint[2], this->MaxPoint[2]));
  }

  VTKM_EXEC_CONT Scalar Value(const Vector& point) const
  {
    Scalar minDistance = vtkm::NegativeInfinity32();
    Scalar diff, t, dist;
    Scalar distance = Scalar(0.0);
    vtkm::IdComponent inside = 1;

    for (vtkm::IdComponent d = 0; d < 3; d++)
    {
      diff = this->MaxPoint[d] - this->MinPoint[d];
      if (diff != Scalar(0.0))
      {
        t = (point[d] - this->MinPoint[d]) / diff;
        // Outside before the box
        if (t < Scalar(0.0))
        {
          inside = 0;
          dist = this->MinPoint[d] - point[d];
        }
        // Outside after the box
        else if (t > Scalar(1.0))
        {
          inside = 0;
          dist = point[d] - this->MaxPoint[d];
        }
        else
        {
          // Inside the box in lower half
          if (t <= Scalar(0.5))
          {
            dist = MinPoint[d] - point[d];
          }
          // Inside the box in upper half
          else
          {
            dist = point[d] - MaxPoint[d];
          }
          if (dist > minDistance)
          {
            minDistance = dist;
          }
        }
      }
      else
      {
        dist = vtkm::Abs(point[d] - MinPoint[d]);
        if (dist > Scalar(0.0))
        {
          inside = 0;
        }
      }
      if (dist > Scalar(0.0))
      {
        distance += dist * dist;
      }
    }

    distance = vtkm::Sqrt(distance);
    if (inside)
    {
      return minDistance;
    }
    else
    {
      return distance;
    }
  }

  VTKM_EXEC_CONT Vector Gradient(const Vector& point) const
  {
    vtkm::IdComponent minAxis = 0;
    Scalar dist = 0.0;
    Scalar minDist = vtkm::Infinity32();
    vtkm::IdComponent3 location;
    Vector normal(Scalar(0));
    Vector inside(Scalar(0));
    Vector outside(Scalar(0));
    Vector center((this->MaxPoint + this->MinPoint) * Scalar(0.5));

    // Compute the location of the point with respect to the box
    // Point will lie in one of 27 separate regions around or within the box
    // Gradient vector is computed differently in each of the regions.
    for (vtkm::IdComponent d = 0; d < 3; d++)
    {
      if (point[d] < this->MinPoint[d])
      {
        // Outside the box low end
        location[d] = 0;
        outside[d] = -1.0;
      }
      else if (point[d] > this->MaxPoint[d])
      {
        // Outside the box high end
        location[d] = 2;
        outside[d] = 1.0;
      }
      else
      {
        location[d] = 1;
        if (point[d] <= center[d])
        {
          // Inside the box low end
          dist = point[d] - this->MinPoint[d];
          inside[d] = -1.0;
        }
        else
        {
          // Inside the box high end
          dist = this->MaxPoint[d] - point[d];
          inside[d] = 1.0;
        }
        if (dist < minDist) // dist is negative
        {
          minDist = dist;
          minAxis = d;
        }
      }
    }

    vtkm::Id indx = location[0] + 3 * location[1] + 9 * location[2];
    switch (indx)
    {
      // verts - gradient points away from center point
      case 0:
      case 2:
      case 6:
      case 8:
      case 18:
      case 20:
      case 24:
      case 26:
        for (vtkm::IdComponent d = 0; d < 3; d++)
        {
          normal[d] = point[d] - center[d];
        }
        vtkm::Normalize(normal);
        break;

      // edges - gradient points out from axis of cube
      case 1:
      case 3:
      case 5:
      case 7:
      case 9:
      case 11:
      case 15:
      case 17:
      case 19:
      case 21:
      case 23:
      case 25:
        for (vtkm::IdComponent d = 0; d < 3; d++)
        {
          if (outside[d] != 0.0)
          {
            normal[d] = point[d] - center[d];
          }
          else
          {
            normal[d] = 0.0;
          }
        }
        vtkm::Normalize(normal);
        break;

      // faces - gradient points perpendicular to face
      case 4:
      case 10:
      case 12:
      case 14:
      case 16:
      case 22:
        for (vtkm::IdComponent d = 0; d < 3; d++)
        {
          normal[d] = outside[d];
        }
        break;

      // interior - gradient is perpendicular to closest face
      case 13:
        normal[0] = normal[1] = normal[2] = 0.0;
        normal[minAxis] = inside[minAxis];
        break;
      default:
        VTKM_ASSERT(false);
        break;
    }
    return normal;
  }

  VTKM_DEPRECATED(1.6, "ImplicitFunctions are no longer pointers. Use . operator.")
  VTKM_EXEC Box* operator->() { return this; }
  VTKM_DEPRECATED(1.6, "ImplicitFunctions are no longer pointers. Use . operator.")
  VTKM_EXEC const Box* operator->() const { return this; }

private:
  Vector MinPoint;
  Vector MaxPoint;
};

//============================================================================
/// \brief Implicit function for a cylinder
///
/// \c Cylinder computes the implicit function and function gradient
/// for a cylinder using F(r)=r^2-Radius^2. By default the Cylinder is
/// centered at the origin and the axis of rotation is along the
/// y-axis. You can redefine the center and axis of rotation by setting
/// the Center and Axis data members.
///
/// Note that the cylinder is infinite in extent.
///
class VTKM_ALWAYS_EXPORT Cylinder : public vtkm::internal::ImplicitFunctionBase<Cylinder>
{
public:
  /// Construct cylinder radius of 0.5; centered at origin with axis
  /// along y coordinate axis.
  VTKM_EXEC_CONT Cylinder()
    : Center(Scalar(0))
    , Axis(Scalar(0), Scalar(1), Scalar(0))
    , Radius(Scalar(0.5))
  {
  }

  VTKM_EXEC_CONT Cylinder(const Vector& axis, Scalar radius)
    : Center(Scalar(0))
    , Axis(axis)
    , Radius(radius)
  {
  }

  VTKM_EXEC_CONT Cylinder(const Vector& center, const Vector& axis, Scalar radius)
    : Center(center)
    , Axis(vtkm::Normal(axis))
    , Radius(radius)
  {
  }

  VTKM_CONT void SetCenter(const Vector& center) { this->Center = center; }

  VTKM_CONT void SetAxis(const Vector& axis) { this->Axis = vtkm::Normal(axis); }

  VTKM_CONT void SetRadius(Scalar radius) { this->Radius = radius; }

  VTKM_EXEC_CONT Scalar Value(const Vector& point) const
  {
    Vector x2c = point - this->Center;
    FloatDefault proj = vtkm::Dot(this->Axis, x2c);
    return vtkm::Dot(x2c, x2c) - (proj * proj) - (this->Radius * this->Radius);
  }

  VTKM_EXEC_CONT Vector Gradient(const Vector& point) const
  {
    Vector x2c = point - this->Center;
    FloatDefault t = this->Axis[0] * x2c[0] + this->Axis[1] * x2c[1] + this->Axis[2] * x2c[2];
    vtkm::Vec<FloatDefault, 3> closestPoint = this->Center + (this->Axis * t);
    return (point - closestPoint) * FloatDefault(2);
  }

  VTKM_DEPRECATED(1.6, "ImplicitFunctions are no longer pointers. Use . operator.")
  VTKM_EXEC Cylinder* operator->() { return this; }
  VTKM_DEPRECATED(1.6, "ImplicitFunctions are no longer pointers. Use . operator.")
  VTKM_EXEC const Cylinder* operator->() const { return this; }

private:
  Vector Center;
  Vector Axis;
  Scalar Radius;
};

//============================================================================
/// \brief Implicit function for a frustum
class VTKM_ALWAYS_EXPORT Frustum : public vtkm::internal::ImplicitFunctionBase<Frustum>
{
public:
  /// \brief Construct axis-aligned frustum with center at (0,0,0) and each side of length 1.0.
  Frustum() = default;

  VTKM_EXEC_CONT Frustum(const Vector points[6], const Vector normals[6])
  {
    this->SetPlanes(points, normals);
  }

  VTKM_EXEC_CONT explicit Frustum(const Vector points[8]) { this->CreateFromPoints(points); }

  VTKM_EXEC void SetPlanes(const Vector points[6], const Vector normals[6])
  {
    for (vtkm::Id index : { 0, 1, 2, 3, 4, 5 })
    {
      this->Points[index] = points[index];
    }
    for (vtkm::Id index : { 0, 1, 2, 3, 4, 5 })
    {
      this->Normals[index] = normals[index];
    }
  }

  VTKM_EXEC void SetPlane(int idx, const Vector& point, const Vector& normal)
  {
    VTKM_ASSERT((idx >= 0) && (idx < 6));
    this->Points[idx] = point;
    this->Normals[idx] = normal;
  }

  VTKM_EXEC_CONT void GetPlanes(Vector points[6], Vector normals[6]) const
  {
    for (vtkm::Id index : { 0, 1, 2, 3, 4, 5 })
    {
      points[index] = this->Points[index];
    }
    for (vtkm::Id index : { 0, 1, 2, 3, 4, 5 })
    {
      normals[index] = this->Normals[index];
    }
  }

  VTKM_EXEC_CONT const Vector* GetPoints() const { return this->Points; }

  VTKM_EXEC_CONT const Vector* GetNormals() const { return this->Normals; }

  // The points should be specified in the order of hex-cell vertices
  VTKM_EXEC_CONT void CreateFromPoints(const Vector points[8])
  {
    // XXX(clang-format-3.9): 3.8 is silly. 3.9 makes it look like this.
    // clang-format off
    int planes[6][3] = {
      { 3, 2, 0 }, { 4, 5, 7 }, { 0, 1, 4 }, { 1, 2, 5 }, { 2, 3, 6 }, { 3, 0, 7 }
    };
    // clang-format on

    for (int i = 0; i < 6; ++i)
    {
      const Vector& v0 = points[planes[i][0]];
      const Vector& v1 = points[planes[i][1]];
      const Vector& v2 = points[planes[i][2]];

      this->Points[i] = v0;
      this->Normals[i] = vtkm::Normal(vtkm::TriangleNormal(v0, v1, v2));
    }
  }

  VTKM_EXEC_CONT Scalar Value(const Vector& point) const
  {
    Scalar maxVal = vtkm::NegativeInfinity<Scalar>();
    for (vtkm::Id index : { 0, 1, 2, 3, 4, 5 })
    {
      const Vector& p = this->Points[index];
      const Vector& n = this->Normals[index];
      const Scalar val = vtkm::Dot(point - p, n);
      maxVal = vtkm::Max(maxVal, val);
    }
    return maxVal;
  }

  VTKM_EXEC_CONT Vector Gradient(const Vector& point) const
  {
    Scalar maxVal = vtkm::NegativeInfinity<Scalar>();
    vtkm::Id maxValIdx = 0;
    for (vtkm::Id index : { 0, 1, 2, 3, 4, 5 })
    {
      const Vector& p = this->Points[index];
      const Vector& n = this->Normals[index];
      Scalar val = vtkm::Dot(point - p, n);
      if (val > maxVal)
      {
        maxVal = val;
        maxValIdx = index;
      }
    }
    return this->Normals[maxValIdx];
  }

  VTKM_DEPRECATED(1.6, "ImplicitFunctions are no longer pointers. Use . operator.")
  VTKM_EXEC Frustum* operator->() { return this; }
  VTKM_DEPRECATED(1.6, "ImplicitFunctions are no longer pointers. Use . operator.")
  VTKM_EXEC const Frustum* operator->() const { return this; }

private:
  Vector Points[6] = { { -0.5f, 0.0f, 0.0f }, { 0.5f, 0.0f, 0.0f },  { 0.0f, -0.5f, 0.0f },
                       { 0.0f, 0.5f, 0.0f },  { 0.0f, 0.0f, -0.5f }, { 0.0f, 0.0f, 0.5f } };
  Vector Normals[6] = { { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f },  { 0.0f, -1.0f, 0.0f },
                        { 0.0f, 1.0f, 0.0f },  { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, 1.0f } };
};

//============================================================================
/// \brief Implicit function for a plane
///
/// A plane is defined by a point in the plane and a normal to the plane.
/// The normal does not have to be a unit vector. The implicit function will
/// still evaluate to 0 at the plane, but the values outside the plane
/// (and the gradient) will be scaled by the length of the normal vector.
class VTKM_ALWAYS_EXPORT Plane : public vtkm::internal::ImplicitFunctionBase<Plane>
{
public:
  /// Construct plane passing through origin and normal to z-axis.
  VTKM_EXEC_CONT Plane()
    : Origin(Scalar(0))
    , Normal(Scalar(0), Scalar(0), Scalar(1))
  {
  }

  /// Construct a plane through the origin with the given normal.
  VTKM_EXEC_CONT explicit Plane(const Vector& normal)
    : Origin(Scalar(0))
    , Normal(normal)
  {
  }

  /// Construct a plane through the given point with the given normal.
  VTKM_EXEC_CONT Plane(const Vector& origin, const Vector& normal)
    : Origin(origin)
    , Normal(normal)
  {
  }

  VTKM_CONT void SetOrigin(const Vector& origin) { this->Origin = origin; }

  VTKM_CONT void SetNormal(const Vector& normal) { this->Normal = normal; }

  VTKM_EXEC_CONT const Vector& GetOrigin() const { return this->Origin; }
  VTKM_EXEC_CONT const Vector& GetNormal() const { return this->Normal; }

  VTKM_EXEC_CONT Scalar Value(const Vector& point) const
  {
    return vtkm::Dot(point - this->Origin, this->Normal);
  }

  VTKM_EXEC_CONT Vector Gradient(const Vector&) const { return this->Normal; }

  VTKM_DEPRECATED(1.6, "ImplicitFunctions are no longer pointers. Use . operator.")
  VTKM_EXEC Plane* operator->() { return this; }
  VTKM_DEPRECATED(1.6, "ImplicitFunctions are no longer pointers. Use . operator.")
  VTKM_EXEC const Plane* operator->() const { return this; }

private:
  Vector Origin;
  Vector Normal;
};

//============================================================================
/// \brief Implicit function for a sphere
///
/// A sphere is defined by its center and a radius.
///
/// The value of the sphere implicit function is the square of the distance
/// from the center biased by the radius (so the surface of the sphere is
/// at value 0).
class VTKM_ALWAYS_EXPORT Sphere : public vtkm::internal::ImplicitFunctionBase<Sphere>
{
public:
  /// Construct sphere with center at (0,0,0) and radius = 0.5.
  VTKM_EXEC_CONT Sphere()
    : Radius(Scalar(0.5))
    , Center(Scalar(0))
  {
  }

  /// Construct a sphere with center at (0,0,0) and the given radius.
  VTKM_EXEC_CONT explicit Sphere(Scalar radius)
    : Radius(radius)
    , Center(Scalar(0))
  {
  }

  VTKM_EXEC_CONT Sphere(Vector center, Scalar radius)
    : Radius(radius)
    , Center(center)
  {
  }

  VTKM_CONT void SetRadius(Scalar radius) { this->Radius = radius; }

  VTKM_CONT void SetCenter(const Vector& center) { this->Center = center; }

  VTKM_EXEC_CONT Scalar GetRadius() const { return this->Radius; }

  VTKM_EXEC_CONT const Vector& GetCenter() const { return this->Center; }

  VTKM_EXEC_CONT Scalar Value(const Vector& point) const
  {
    return vtkm::MagnitudeSquared(point - this->Center) - (this->Radius * this->Radius);
  }

  VTKM_EXEC_CONT Vector Gradient(const Vector& point) const
  {
    return Scalar(2) * (point - this->Center);
  }

  VTKM_DEPRECATED(1.6, "ImplicitFunctions are no longer pointers. Use . operator.")
  VTKM_EXEC Sphere* operator->() { return this; }
  VTKM_DEPRECATED(1.6, "ImplicitFunctions are no longer pointers. Use . operator.")
  VTKM_EXEC const Sphere* operator->() const { return this; }

private:
  Scalar Radius;
  Vector Center;
};

namespace detail
{

struct ImplicitFunctionValueFunctor
{
  template <typename ImplicitFunctionType>
  VTKM_EXEC_CONT typename ImplicitFunctionType::Scalar operator()(
    const ImplicitFunctionType& function,
    const typename ImplicitFunctionType::Vector& point) const
  {
    return function.Value(point);
  }
};

struct ImplicitFunctionGradientFunctor
{
  template <typename ImplicitFunctionType>
  VTKM_EXEC_CONT typename ImplicitFunctionType::Vector operator()(
    const ImplicitFunctionType& function,
    const typename ImplicitFunctionType::Vector& point) const
  {
    return function.Gradient(point);
  }
};

} // namespace detail

template <typename... ImplicitFunctionTypes>
class ImplicitFunctionMultiplexer
  : public vtkm::internal::ImplicitFunctionBase<
      ImplicitFunctionMultiplexer<ImplicitFunctionTypes...>>
{
  vtkm::exec::internal::Variant<ImplicitFunctionTypes...> Variant;

  using Superclass =
    vtkm::internal::ImplicitFunctionBase<ImplicitFunctionMultiplexer<ImplicitFunctionTypes...>>;

public:
  using Scalar = typename Superclass::Scalar;
  using Vector = typename Superclass::Vector;

  ImplicitFunctionMultiplexer() = default;

  template <typename FunctionType>
  VTKM_EXEC_CONT ImplicitFunctionMultiplexer(
    const vtkm::internal::ImplicitFunctionBase<FunctionType>& function)
    : Variant(reinterpret_cast<const FunctionType&>(function))
  {
  }

  VTKM_EXEC_CONT Scalar Value(const Vector& point) const
  {
    return this->Variant.CastAndCall(detail::ImplicitFunctionValueFunctor{}, point);
  }

  VTKM_EXEC_CONT Vector Gradient(const Vector& point) const
  {
    return this->Variant.CastAndCall(detail::ImplicitFunctionGradientFunctor{}, point);
  }
};

class ImplicitFunctionGeneral
  : public vtkm::ImplicitFunctionMultiplexer<vtkm::Box,
                                             vtkm::Cylinder,
                                             vtkm::Frustum,
                                             vtkm::Plane,
                                             vtkm::Sphere>
{
  using Superclass = vtkm::ImplicitFunctionMultiplexer<vtkm::Box,
                                                       vtkm::Cylinder,
                                                       vtkm::Frustum,
                                                       vtkm::Plane,
                                                       vtkm::Sphere>;

public:
  using Superclass::Superclass;
};

} // namespace vtkm

#endif //vtk_m_ImplicitFunction_h
