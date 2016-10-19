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

#include <vtkm/rendering/MapperGL.h>

#include <vtkm/cont/TryExecute.h>
#include <vtkm/rendering/internal/OpenGLHeaders.h>
#include <vtkm/rendering/internal/RunTriangulator.h>
#include <vtkm/worklet/DispatcherMapField.h>
#include <vtkm/worklet/WorkletMapField.h>
#include <vtkm/cont/ArrayHandleIndex.h>

#include <vector>

namespace vtkm {
namespace rendering {

namespace {

struct TypeListTagId4 : vtkm::ListTagBase<vtkm::Vec<Id, 4> > { };
typedef TypeListTagId4 Id4Type;

class MapColorAndVertices : public vtkm::worklet::WorkletMapField
{
public:
  const vtkm::rendering::ColorTable ColorTable;
  const vtkm::Float32 SMin, SDiff;

  VTKM_CONT
  MapColorAndVertices(const vtkm::rendering::ColorTable &colorTable,
                      vtkm::Float32 sMin, vtkm::Float32 sDiff)
    : ColorTable(colorTable),
      SMin(sMin),
      SDiff(sDiff)
  {}
  typedef void ControlSignature(FieldIn<IdType> vertexId,
                                WholeArrayIn<Id4Type> indices,
                                WholeArrayIn<Scalar> scalar,
                                WholeArrayIn<Vec3> verts,
                                WholeArrayOut<Scalar> out_color,
                                WholeArrayOut<Scalar> out_vertices);
  typedef void ExecutionSignature(_1, _2, _3, _4, _5, _6);

  template<typename InputArrayIndexPortalType,
           typename InputArrayPortalType,
           typename InputArrayV3PortalType,
           typename OutputArrayPortalType>
  VTKM_EXEC
  void operator()(const vtkm::Id &i,
                  InputArrayIndexPortalType &indices,
                  const InputArrayPortalType &scalar,
                  const InputArrayV3PortalType &verts,
                  OutputArrayPortalType &c_array,
                  OutputArrayPortalType &v_array) const
  {
    vtkm::Vec<vtkm::Id, 4> idx = indices.Get(i);
    vtkm::Id i1 = idx[1];
    vtkm::Id i2 = idx[2];
    vtkm::Id i3 = idx[3];

    vtkm::Vec<vtkm::Float32, 3> p1 = verts.Get(idx[1]);
    vtkm::Vec<vtkm::Float32, 3> p2 = verts.Get(idx[2]);
    vtkm::Vec<vtkm::Float32, 3> p3 = verts.Get(idx[3]);

    vtkm::Float32 s;
    vtkm::rendering::Color color;

    const vtkm::Id offset = 9;

    s = scalar.Get(i1);
    s = (s-SMin)/SDiff;
    color = ColorTable.MapRGB(s);
    v_array.Set(i*offset, p1[0]);
    v_array.Set(i*offset+1, p1[1]);
    v_array.Set(i*offset+2, p1[2]);
    c_array.Set(i*offset, color.Components[0]);
    c_array.Set(i*offset+1, color.Components[1]);
    c_array.Set(i*offset+2, color.Components[2]);

    s = scalar.Get(i2);
    s = (s-SMin)/SDiff;
    color = ColorTable.MapRGB(s);
    v_array.Set(i*offset+3, p2[0]);
    v_array.Set(i*offset+4, p2[1]);
    v_array.Set(i*offset+5, p2[2]);
    c_array.Set(i*offset+3, color.Components[0]);
    c_array.Set(i*offset+4, color.Components[1]);
    c_array.Set(i*offset+5, color.Components[2]);

    s = scalar.Get(i3);
    s = (s-SMin)/SDiff;
    color = ColorTable.MapRGB(s);
    v_array.Set(i*offset+6, p3[0]);
    v_array.Set(i*offset+7, p3[1]);
    v_array.Set(i*offset+8, p3[2]);
    c_array.Set(i*offset+6, color.Components[0]);
    c_array.Set(i*offset+7, color.Components[1]);
    c_array.Set(i*offset+8, color.Components[2]);
  }
};

template<typename PtType>
struct MapColorAndVerticesInvokeFunctor
{
  vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Id, 4> > TriangleIndices;
  vtkm::rendering::ColorTable ColorTable;
  const vtkm::cont::ArrayHandle<vtkm::Float32> Scalar;
  const vtkm::Range ScalarRange;
  const PtType Vertices;
  MapColorAndVertices Worklet;
  vtkm::cont::ArrayHandle<vtkm::Float32> OutColor;
  vtkm::cont::ArrayHandle<vtkm::Float32> OutVertices;

  VTKM_CONT
  MapColorAndVerticesInvokeFunctor(
      const vtkm::cont::ArrayHandle< vtkm::Vec<vtkm::Id, 4> > &indices,
      const vtkm::rendering::ColorTable &colorTable,
      const vtkm::cont::ArrayHandle<Float32> &scalar,
      const vtkm::Range &scalarRange,
      const PtType &vertices,
      vtkm::Float32 s_min,
      vtkm::Float32 s_max,
      vtkm::cont::ArrayHandle<Float32> &out_color,
      vtkm::cont::ArrayHandle<Float32> &out_vertices):
    TriangleIndices(indices),
    ColorTable(colorTable),
    Scalar(scalar),
    ScalarRange(scalarRange),
    Vertices(vertices),
    Worklet(colorTable, s_min, s_max - s_min),

    OutColor(out_color),
    OutVertices(out_vertices)
  {}

  template<typename Device>
  VTKM_CONT
  bool operator()(Device) const
  {
    VTKM_IS_DEVICE_ADAPTER_TAG(Device);

    vtkm::worklet::DispatcherMapField<MapColorAndVertices, Device>
        dispatcher(this->Worklet);

    vtkm::cont:: ArrayHandleIndex indexArray (this->TriangleIndices.GetNumberOfValues());
    dispatcher.Invoke(indexArray, this->TriangleIndices, this->Scalar,
                      this->Vertices, this->OutColor, this->OutVertices);
    return true;
  }
};

template<typename PtType>
VTKM_CONT
void RenderTriangles(MapperGL &mapper,
                     vtkm::Id numTri, const PtType &verts,
                     const vtkm::cont::ArrayHandle< vtkm::Vec<vtkm::Id, 4> > &indices,
                     const vtkm::cont::ArrayHandle<vtkm::Float32> &scalar,
                     const vtkm::rendering::ColorTable &ct,
                     const vtkm::Range &scalarRange,
                     const vtkm::rendering::Camera &camera)
{
  if (!mapper.loaded)
  {
    GLenum GlewInitResult = glewInit();
    if (GlewInitResult)
        std::cout << "ERROR: " << glewGetErrorString(GlewInitResult) << std::endl;
    mapper.loaded = true;

    vtkm::Float32 sMin = vtkm::Float32(scalarRange.Min);
    vtkm::Float32 sMax = vtkm::Float32(scalarRange.Max);
    vtkm::cont::ArrayHandle<vtkm::Float32> out_vertices, out_color;
    out_vertices.Allocate(9*indices.GetNumberOfValues());
    out_color.Allocate(9*indices.GetNumberOfValues());

    vtkm::cont::TryExecute(
        MapColorAndVerticesInvokeFunctor<PtType>(indices,
                                                 ct,
                                                 scalar,
                                                 scalarRange,
                                                 verts,
                                                 sMin,
                                                 sMax,
                                                 out_color,
                                                 out_vertices));

    vtkm::Id vtx_cnt = out_vertices.GetNumberOfValues();
    vtkm::Float32 *v_ptr = out_vertices.GetStorage().StealArray();
    vtkm::Float32 *c_ptr = out_color.GetStorage().StealArray();

    vtkm::Id floatSz = static_cast<vtkm::Id>(sizeof(vtkm::Float32));
    GLsizeiptr sz = static_cast<GLsizeiptr>(vtx_cnt*floatSz);
    
    GLuint points_vbo = 0;
    glGenBuffers(1, &points_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
    glBufferData(GL_ARRAY_BUFFER, sz, v_ptr, GL_STATIC_DRAW);

    GLuint colours_vbo = 0;
    glGenBuffers(1, &colours_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, colours_vbo);
    glBufferData(GL_ARRAY_BUFFER, sz, c_ptr, GL_STATIC_DRAW);
    
    mapper.vao = 0;
    glGenVertexArrays(1, &mapper.vao);
    glBindVertexArray(mapper.vao);
    glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, colours_vbo);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    
    const char *vertex_shader =
        "#version 120\n"
        "attribute vec3 vertex_position;"
        "attribute vec3 vertex_color;"
        "varying vec3 ourColor;"
        "uniform mat4 mv_matrix;"
        "uniform mat4 p_matrix;"
        
        "void main() {"
        "  gl_Position = p_matrix*mv_matrix * vec4(vertex_position, 1.0);"
        "  ourColor = vertex_color;"
        "}";
    const char *fragment_shader =
        "#version 120\n"
        "varying vec3 ourColor;"
        "void main() {"
        "  gl_FragColor = vec4 (ourColor, 1.0);"
        "}";
    
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader, NULL);
    glCompileShader(vs);
    GLint isCompiled = 0;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
      GLint maxLength = 0;
      glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &maxLength);

      std::string msg;
      if (maxLength <= 0)
        msg = "No error message";
      else
      {
        // The maxLength includes the NULL character
        GLchar *strInfoLog = new GLchar[maxLength + 1];
        glGetShaderInfoLog(vs, maxLength, &maxLength, strInfoLog);
        msg = std::string(strInfoLog);
        delete [] strInfoLog;
      }
      throw vtkm::cont::ErrorControlBadValue("Shader compile error:"+msg);
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader, NULL);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
      GLint maxLength = 0;
      glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &maxLength);

      std::string msg;      
      if (maxLength <= 0)
          msg = "No error message";
      else
      {
        // The maxLength includes the NULL character
        GLchar *strInfoLog = new GLchar[maxLength + 1];
        glGetShaderInfoLog(vs, maxLength, &maxLength, strInfoLog);
        msg = std::string(strInfoLog);          
        delete [] strInfoLog;
      }
      throw vtkm::cont::ErrorControlBadValue("Shader compile error:"+msg);
    }

    mapper.shader_programme = glCreateProgram();
    if (mapper.shader_programme > 0)
    {
      glAttachShader(mapper.shader_programme, fs);
      glAttachShader(mapper.shader_programme, vs);
      glBindAttribLocation (mapper.shader_programme, 0, "vertex_position");
      glBindAttribLocation (mapper.shader_programme, 1, "vertex_color");
      
      glLinkProgram (mapper.shader_programme);
      GLint linkStatus;
      glGetProgramiv(mapper.shader_programme, GL_LINK_STATUS, &linkStatus);
      if (!linkStatus)
      {
        char log[2048];
        GLsizei len;
        glGetProgramInfoLog(mapper.shader_programme, 2048, &len, log);
        std::string msg = std::string("Shader program link failed: ")+std::string(log);
        throw vtkm::cont::ErrorControlBadValue(msg);
      }
    }
  }

  if (mapper.shader_programme > 0)
  {
    vtkm::Matrix<vtkm::Float32,4,4> viewM = camera.CreateViewMatrix();
    vtkm::Matrix<vtkm::Float32,4,4> projM = camera.CreateProjectionMatrix(512,512);

    MatrixHelpers::CreateOGLMatrix(viewM, mapper.mvMat);
    MatrixHelpers::CreateOGLMatrix(projM, mapper.pMat);

    glUseProgram(mapper.shader_programme);
    GLint mvID = glGetUniformLocation(mapper.shader_programme, "mv_matrix");
    glUniformMatrix4fv(mvID, 1, GL_FALSE, mapper.mvMat);
    GLint pID = glGetUniformLocation(mapper.shader_programme, "p_matrix");
    glUniformMatrix4fv(pID, 1, GL_FALSE, mapper.pMat);
    glBindVertexArray(mapper.vao);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(numTri*3));
    glUseProgram(0);
  }
}
    
} // anonymous namespace

MapperGL::MapperGL()
{ this->loaded = false; }

MapperGL::~MapperGL()
{  }

void MapperGL::RenderCells(const vtkm::cont::DynamicCellSet &cellset,
                           const vtkm::cont::CoordinateSystem &coords,
                           const vtkm::cont::Field &scalarField,
                           const vtkm::rendering::ColorTable &colorTable,
                           const vtkm::rendering::Camera &camera,
                           const vtkm::Range &scalarRange)
{
  vtkm::cont::ArrayHandle< vtkm::Vec<vtkm::Id, 4> > indices;
  vtkm::Id numTri;
  vtkm::rendering::internal::RunTriangulator(cellset, indices, numTri);

  vtkm::cont::ArrayHandle<vtkm::Float32> sf;
  sf = scalarField.GetData().Cast<vtkm::cont::ArrayHandle<vtkm::Float32> >();

  vtkm::cont::DynamicArrayHandleCoordinateSystem dcoords = coords.GetData();
  vtkm::cont::ArrayHandleUniformPointCoordinates uVerts;
  vtkm::cont::ArrayHandle< vtkm::Vec<vtkm::Float32,3> > eVerts;

  if(dcoords.IsSameType(vtkm::cont::ArrayHandleUniformPointCoordinates()))
  {
    uVerts = dcoords.Cast<vtkm::cont::ArrayHandleUniformPointCoordinates>();
    RenderTriangles(*this, numTri, uVerts, indices, sf, colorTable, scalarRange, camera);
  }
  else if(dcoords.IsSameType(vtkm::cont::ArrayHandle< vtkm::Vec<vtkm::Float32,3> >()))
  {
    eVerts = dcoords.Cast<vtkm::cont::ArrayHandle< vtkm::Vec<vtkm::Float32,3> > > ();
    RenderTriangles(*this, numTri, eVerts, indices, sf, colorTable, scalarRange, camera);
  }
  else if(dcoords.IsSameType(vtkm::cont::ArrayHandleCartesianProduct<
                             vtkm::cont::ArrayHandle<vtkm::FloatDefault>,
                             vtkm::cont::ArrayHandle<vtkm::FloatDefault>,
                             vtkm::cont::ArrayHandle<vtkm::FloatDefault> >()))
  {
    vtkm::cont::ArrayHandleCartesianProduct<
        vtkm::cont::ArrayHandle<vtkm::FloatDefault>,
        vtkm::cont::ArrayHandle<vtkm::FloatDefault>,
        vtkm::cont::ArrayHandle<vtkm::FloatDefault> > rVerts;
    rVerts = dcoords.Cast<vtkm::cont::ArrayHandleCartesianProduct<
                              vtkm::cont::ArrayHandle<vtkm::FloatDefault>,
                              vtkm::cont::ArrayHandle<vtkm::FloatDefault>,
                              vtkm::cont::ArrayHandle<vtkm::FloatDefault> > > ();
    RenderTriangles(*this, numTri, rVerts, indices, sf, colorTable, scalarRange, camera);
  }
  glFinish();
  glFlush();
}

void MapperGL::StartScene()
{
  // Nothing needs to be done.
}

void MapperGL::EndScene()
{
  // Nothing needs to be done.
}

void MapperGL::SetCanvas(vtkm::rendering::Canvas *)
{
  // Nothing needs to be done.
}

vtkm::rendering::Mapper *MapperGL::NewCopy() const
{
  return new vtkm::rendering::MapperGL(*this);
}

}
} // namespace vtkm::rendering
