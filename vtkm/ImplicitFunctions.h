//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//  Copyright 2014 Sandia Corporation.
//  Copyright 2014 UT-Battelle, LLC.
//  Copyright 2014 Los Alamos National Security.
//
//  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
//  the U.S. Government retains certain rights in this software.
//
//  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
//  Laboratory (LANL), the U.S. Government retains certain rights in
//  this software.
//============================================================================
#ifndef vtk_m_ImplicitFunctions_h
#define vtk_m_ImplicitFunctions_h

#include <vtkm/Types.h>
#include <vtkm/Math.h>
#include <vtkm/VectorAnalysis.h>
#include <iostream>

namespace vtkm {

/// \brief Implicit function for a plane
class Plane
{
public:
  VTKM_CONT
  Plane()
    : Origin(FloatDefault(0)),
      Normal(FloatDefault(0), FloatDefault(0), FloatDefault(1))
  { }

  VTKM_CONT
  explicit Plane(const vtkm::Vec<FloatDefault, 3> &normal)
    : Origin(FloatDefault(0)),
      Normal(normal)
  { }

  VTKM_CONT
  Plane(const vtkm::Vec<FloatDefault, 3> &origin,
        const vtkm::Vec<FloatDefault, 3> &normal)
    : Origin(origin), Normal(normal)
  { }

  VTKM_EXEC_CONT
  const vtkm::Vec<FloatDefault, 3>& GetOrigin() const
  {
    return this->Origin;
  }

  VTKM_EXEC_CONT
  const vtkm::Vec<FloatDefault, 3>& GetNormal() const
  {
    return this->Normal;
  }

  VTKM_EXEC_CONT
  FloatDefault Value(FloatDefault x, FloatDefault y, FloatDefault z) const
  {
    return ((x - this->Origin[0]) * this->Normal[0]) +
           ((y - this->Origin[1]) * this->Normal[1]) +
           ((z - this->Origin[2]) * this->Normal[2]);
  }

  VTKM_EXEC_CONT
  FloatDefault Value(const vtkm::Vec<FloatDefault, 3> &x) const
  {
    return this->Value(x[0], x[1], x[2]);
  }

  VTKM_EXEC_CONT
  vtkm::Vec<FloatDefault, 3> Gradient(FloatDefault, FloatDefault, FloatDefault) const
  {
    return this->Normal;
  }

  VTKM_EXEC_CONT
  vtkm::Vec<FloatDefault, 3> Gradient(const vtkm::Vec<FloatDefault, 3>&) const
  {
    return this->Normal;
  }

private:
  vtkm::Vec<FloatDefault, 3> Origin;
  vtkm::Vec<FloatDefault, 3> Normal;
};


/// \brief Implicit function for a sphere
class Sphere
{
public:
  VTKM_CONT
  Sphere() : Radius(FloatDefault(0.2)), Center(FloatDefault(0))
  { }

  VTKM_CONT
  explicit Sphere(FloatDefault radius) : Radius(radius), Center(FloatDefault(0))
  { }

  VTKM_CONT
  Sphere(vtkm::Vec<FloatDefault, 3> center, FloatDefault radius)
    : Radius(radius), Center(center)
  { }

  VTKM_EXEC_CONT
  FloatDefault GetRadius() const
  {
    return this->Radius;
  }

  VTKM_EXEC_CONT
  const vtkm::Vec<FloatDefault, 3>& GetCenter() const
  {
    return this->Center;
  }

  VTKM_EXEC_CONT
  FloatDefault Value(FloatDefault x, FloatDefault y, FloatDefault z) const
  {
    return ((x - this->Center[0]) * (x - this->Center[0]) +
            (y - this->Center[1]) * (y - this->Center[1]) +
            (z - this->Center[2]) * (z - this->Center[2])) -
           (this->Radius * this->Radius);
  }

  VTKM_EXEC_CONT
  FloatDefault Value(const vtkm::Vec<FloatDefault, 3> &x) const
  {
    return this->Value(x[0], x[1], x[2]);
  }

  VTKM_EXEC_CONT
  vtkm::Vec<FloatDefault, 3> Gradient(FloatDefault x, FloatDefault y, FloatDefault z)
    const
  {
    return this->Gradient(vtkm::Vec<FloatDefault, 3>(x, y, z));
  }

  VTKM_EXEC_CONT
  vtkm::Vec<FloatDefault, 3> Gradient(const vtkm::Vec<FloatDefault, 3> &x) const
  {
    return FloatDefault(2) * (x - this->Center);
  }

private:
  FloatDefault Radius;
  vtkm::Vec<FloatDefault, 3> Center;
};

/// \brief Implicit function for a box
class Box
{
public:
  VTKM_CONT
  Box() : MinPoint(vtkm::Vec<FloatDefault,3>(FloatDefault(0), FloatDefault(0), FloatDefault(0))), 
          MaxPoint(vtkm::Vec<FloatDefault,3>(FloatDefault(1), FloatDefault(1), FloatDefault(1)))
  { }

  VTKM_CONT
  Box(vtkm::Vec<FloatDefault, 3> minPoint, vtkm::Vec<FloatDefault, 3> maxPoint)
    : MinPoint(minPoint), MaxPoint(maxPoint)
  { }

  VTKM_CONT
  Box(FloatDefault xmin, FloatDefault xmax,
      FloatDefault ymin, FloatDefault ymax,
      FloatDefault zmin, FloatDefault zmax)
  {
    MinPoint[0] = xmin;  MaxPoint[0] = xmax;
    MinPoint[1] = ymin;  MaxPoint[1] = ymax;
    MinPoint[2] = zmin;  MaxPoint[2] = zmax;
  }

  VTKM_EXEC_CONT
  const vtkm::Vec<FloatDefault, 3>& GetMinPoint() const
  {
    return this->MinPoint;
  }

  VTKM_EXEC_CONT
  const vtkm::Vec<FloatDefault, 3>& GetMaxPoint() const
  {
    return this->MaxPoint;
  }

  VTKM_EXEC_CONT
  FloatDefault Value(const vtkm::Vec<FloatDefault, 3> &x) const
  {
    FloatDefault minDistance = vtkm::NegativeInfinity32();
    FloatDefault diff, t, dist;
    FloatDefault distance = FloatDefault(0.0);
    vtkm::IdComponent inside = 1;

    for (vtkm::IdComponent d = 0; d < 3; d++)
    {
      diff = this->MaxPoint[d] - this->MinPoint[d];
      if (diff != FloatDefault(0.0))
      {
        t = (x[d] - this->MinPoint[d]) / diff;
        // Outside before the box
        if (t < FloatDefault(0.0))
        {
          inside = 0;
          dist = this->MinPoint[d] - x[d];
        }
        // Outside after the box
        else if (t > FloatDefault(1.0))
        {
          inside = 0;
          dist = x[d] - this->MaxPoint[d];
        }
        else
        {
          // Inside the box in lower half
          if (t <= FloatDefault(0.5))
          {
          dist = MinPoint[d] - x[d];
          }
          // Inside the box in upper half
          else
          {
             dist = x[d] - MaxPoint[d];
          }
          if (dist > minDistance)
          {
            minDistance = dist;
          }
        }
      }
      else
      {
        dist = vtkm::Abs(x[d] - MinPoint[d]);
        if (dist > FloatDefault(0.0))
        {
          inside = 0;
        }
      }
      if (dist > FloatDefault(0.0))
      {
        distance += dist*dist;
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

  VTKM_EXEC_CONT
  FloatDefault Value(FloatDefault x, FloatDefault y, FloatDefault z) const
  {
    return this->Value(vtkm::Vec<vtkm::FloatDefault,3>(x, y, z));
  }

  VTKM_EXEC_CONT
  vtkm::Vec<FloatDefault, 3> Gradient(const vtkm::Vec<FloatDefault, 3> &x) const
  {
    vtkm::IdComponent minAxis = 0;
    FloatDefault dist = 0.0;
    FloatDefault minDist = vtkm::Infinity32();
    vtkm::Vec<vtkm::IdComponent,3> location;
    vtkm::Vec<FloatDefault,3> normal;
    vtkm::Vec<FloatDefault,3> inside(FloatDefault(0), FloatDefault(0), FloatDefault(0));
    vtkm::Vec<FloatDefault,3> outside(FloatDefault(0), FloatDefault(0), FloatDefault(0));
    vtkm::Vec<FloatDefault,3> center((this->MaxPoint[0] + this->MinPoint[0]) * FloatDefault(0.5),
                                     (this->MaxPoint[1] + this->MinPoint[1]) * FloatDefault(0.5),
                                     (this->MaxPoint[2] + this->MinPoint[2]) * FloatDefault(0.5));

    // Compute the location of the point with respect to the box
    // Point will lie in one of 27 separate regions around or within the box
    // Gradient vector is computed differently in each of the regions.
    for (vtkm::IdComponent d = 0; d < 3; d++)
    { 
      if (x[d] < this->MinPoint[d])
      {
        // Outside the box low end
        location[d] = 0;
        outside[d] = -1.0;
      }
      else if (x[d] > this->MaxPoint[d])
      {
        // Outside the box high end
        location[d] = 2;
        outside[d] = 1.0; 
      }
      else
      {
        location[d] = 1;
        if (x[d] <= center[d])
        {
          // Inside the box low end
          dist = x[d] - this->MinPoint[d];
          inside[d] = -1.0;
        }
        else
        {
          // Inside the box high end
          dist = this->MaxPoint[d] - x[d];
          inside[d] = 1.0;
        }
        if (dist < minDist) // dist is negative
        {
          minDist = dist;
          minAxis = d;
        }
      }
    }

    vtkm::Id indx = location[0] + 3*location[1] + 9*location[2];
    switch (indx)
    { 
      // verts - gradient points away from center point
      case 0: case 2: case 6: case 8: case 18: case 20: case 24: case 26:
        for (vtkm::IdComponent d = 0; d < 3; d++)
        {
          normal[d] = x[d] - center[d];
        }
        vtkm::Normalize(normal);
        break;
  
      // edges - gradient points out from axis of cube
      case 1: case 3: case 5: case 7:
      case 9: case 11: case 15: case 17:
      case 19: case 21: case 23: case 25:
        for (vtkm::IdComponent d = 0; d < 3; d++)
        {
          if (outside[d] != 0.0)
          {
            normal[d] = x[d] - center[d];
          }
          else 
          {
            normal[d] = 0.0;
          }
        }
        vtkm::Normalize(normal);
        break; 
  
      // faces - gradient points perpendicular to face
      case 4: case 10: case 12: case 14: case 16: case 22:
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
  
  VTKM_EXEC_CONT
  vtkm::Vec<FloatDefault, 3> Gradient(FloatDefault x, FloatDefault y, FloatDefault z)
    const
  {
    return this->Gradient(vtkm::Vec<FloatDefault, 3>(x, y, z));
  }

private:
  vtkm::Vec<FloatDefault, 3> MinPoint;
  vtkm::Vec<FloatDefault, 3> MaxPoint;
};

/// \brief A function object that evaluates the contained implicit function
template <typename ImplicitFunction>
class ImplicitFunctionValue
{
public:
  VTKM_CONT
  ImplicitFunctionValue()
    : Function()
  { }

  VTKM_CONT
  explicit ImplicitFunctionValue(const ImplicitFunction &func)
    : Function(func)
  { }

  VTKM_EXEC_CONT
  FloatDefault operator()(const vtkm::Vec<FloatDefault, 3> x) const
  {
    return this->Function.Value(x);
  }

  VTKM_EXEC_CONT
  FloatDefault operator()(FloatDefault x, FloatDefault y, FloatDefault z) const
  {
    return this->Function.Value(x, y, z);
  }

private:
  ImplicitFunction Function;
};

/// \brief A function object that computes the gradient of the contained implicit
/// function and the specified point.
template <typename ImplicitFunction>
class ImplicitFunctionGradient
{
public:
  VTKM_CONT
  ImplicitFunctionGradient()
    : Function()
  { }

  VTKM_CONT
  explicit ImplicitFunctionGradient(const ImplicitFunction &func)
    : Function(func)
  { }

  VTKM_EXEC_CONT
  FloatDefault operator()(const vtkm::Vec<FloatDefault, 3> x) const
  {
    return this->Function.Gradient(x);
  }

  VTKM_EXEC_CONT
  FloatDefault operator()(FloatDefault x, FloatDefault y, FloatDefault z) const
  {
    return this->Function.Gradient(x, y, z);
  }

private:
  ImplicitFunction Function;
};

} // namespace vtkm

#endif // vtk_m_ImplicitFunctions_h
